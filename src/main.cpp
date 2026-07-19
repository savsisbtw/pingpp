#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <cmath>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

struct Target {
    std::string host;
    uint16_t port;
};

static bool parse_target(const std::string& s, Target& t) {
    size_t colon = s.rfind(':');
    if (colon == std::string::npos) return false;
    t.host = s.substr(0, colon);
    try {
        t.port = static_cast<uint16_t>(std::stoi(s.substr(colon + 1)));
    } catch (...) {
        return false;
    }
    return true;
}

static bool tcp_connect_once(const Target& t, double& ms) {
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo* res = nullptr;
    std::string portstr = std::to_string(t.port);
    if (getaddrinfo(t.host.c_str(), portstr.c_str(), &hints, &res) != 0 || !res) return false;

    SOCKET s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (s == INVALID_SOCKET) { freeaddrinfo(res); return false; }

    u_long nonblock = 1;
    ioctlsocket(s, FIONBIO, &nonblock);

    auto start = std::chrono::steady_clock::now();
    int rc = connect(s, res->ai_addr, static_cast<int>(res->ai_addrlen));
    bool ok = false;

    if (rc == 0) {
        ok = true;
    } else if (WSAGetLastError() == WSAEWOULDBLOCK) {
        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(s, &writefds);
        timeval tv{};
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        if (select(0, nullptr, &writefds, nullptr, &tv) > 0) {
            int err = 0;
            int errlen = sizeof(err);
            getsockopt(s, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err), &errlen);
            ok = (err == 0);
        }
    }

    auto end = std::chrono::steady_clock::now();
    ms = std::chrono::duration<double, std::milli>(end - start).count();

    closesocket(s);
    freeaddrinfo(res);
    return ok;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: pingpp <host:port> [host:port ...]" << std::endl;
        return 1;
    }

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }

    const int attempts = 5;

    for (int i = 1; i < argc; ++i) {
        Target t;
        if (!parse_target(argv[i], t)) {
            std::cout << argv[i] << ": bad format, use host:port" << std::endl;
            continue;
        }

        std::vector<double> times;
        for (int a = 0; a < attempts; ++a) {
            double ms;
            if (tcp_connect_once(t, ms)) times.push_back(ms);
        }

        std::cout << t.host << ":" << t.port << " -> ";
        if (times.empty()) {
            std::cout << "no response" << std::endl;
            continue;
        }

        double sum = 0, mn = times[0], mx = times[0];
        for (double v : times) { sum += v; mn = std::min(mn, v); mx = std::max(mx, v); }
        double avg = sum / times.size();

        double var = 0;
        for (double v : times) var += (v - avg) * (v - avg);
        double jitter = std::sqrt(var / times.size());

        std::cout << times.size() << "/" << attempts << " ok, avg " << avg << " ms, "
            << "min " << mn << " ms, max " << mx << " ms, jitter " << jitter << " ms" << std::endl;
    }

    WSACleanup();
    return 0;
}
