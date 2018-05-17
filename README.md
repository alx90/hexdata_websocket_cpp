# __hexdata_websocket_cpp__

Very simple client/server websocket application that communicates straight with raw data (hex bytes).
Libraries used are taken from [Simple-WebSocket-Server](https://github.com/eidheim/Simple-WebSocket-Server) so refer to that for complete library documentation.

### Dependencies
* Boost.Asio or standalone Asio
* Included [Simple-WebSocket-Server](https://github.com/eidheim/Simple-WebSocket-Server) headers

### Compile
Compile with a C++11 supported compiler:
```sh
mkdir build
cd build
cmake ..
make
cd ..
```

### Run application
```sh
./build/hexdata_websocket [arg]
# [arg] can be
#     -> not specifying anything send default test cmd to server
# on  -> send 'on' test cmd to server
# off -> send 'off' test cmd to server
```
### Config
hexdata_websocket.cpp can be modified so that:
1. Client communicates with a local demo server
  - make sure to have this define:
```c
#define USE_DEMO_SERVER
```
2. Client communicates with an external websocket server:
  - comment the previous define:
```c
// #define USE_DEMO_SERVER
```
  - insert webserver address as __SERVER_ENDPOINT__:
  ```c
std::string SERVER_ENDPOINT = "demos.kaazing.com/echo";
  ```
