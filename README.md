# pingpp

[![CI](https://github.com/devsavsis/pingpp/actions/workflows/ci.yml/badge.svg)](https://github.com/devsavsis/pingpp/actions/workflows/ci.yml)
[![license](https://img.shields.io/github/license/ImSavsis/pingpp.svg)](https://github.com/devsavsis/pingpp/blob/master/LICENSE)

меряет задержку и джиттер до хостов через обычный TCP-connect, без ICMP и прав администратора. удобно быстро глянуть какая VPN-нода бодрее.

```mermaid
sequenceDiagram
    loop 5 попыток
        pingpp->>Host: TCP connect
        Host-->>pingpp: SYN-ACK
        Note over pingpp: засекли время
    end
    pingpp->>pingpp: avg / min / max / jitter
```

## сборка

```
cmake -B build
cmake --build build --config Release
```

## юзать

```
build\Release\pingpp.exe 1.1.1.1:443 vpn.example.com:51820
```
