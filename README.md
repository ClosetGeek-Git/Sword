# Sword
Distributed P2P message routing and node discovery system using Chord protocol and Swoole coroutine library.

A work in progress. I'm working on last two tasks
1. Currently only works over hardcoded `127.0.0.1` address. refactoring to connect to any ip
2. Need to replace system sockets with swoole coro socket.

Requires Swoole PHP extension installed from source

```
# to build
phpize
./configure
make
make instal
```

