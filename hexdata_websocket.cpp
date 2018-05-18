#include "client_ws.hpp"
#include "server_ws.hpp"
#include <stdint.h>
#include <cstring>

using namespace std;

// #define USE_DEMO_SERVER

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;

const char CMD_VERSION_INFO[] = { '\xAA', '\x00', '\x01', '\x00', '\x00', '\x55' };
const char CMD_AMP_ON[] = { '\xAA', '\x00', '\x06', '\x00', '\x01', '\x55' };
const char CMD_AMP_OFF[] = { '\xAA', '\x00', '\x06', '\x00', '\x00', '\x55' };
const char CMD_TEMP_INQUIRY[] = { '\xAA', '\x00', '\x04', '\x00', '\x04', '\x55' };
const char CMD_WARN_INQUIRY[] = { '\xAA', '\x00', '\x03', '\x00', '\x00', '\x55' };
char CMD_TO_SEND[6];

const char RES_VERSION_INFO[] = { '\xBB', '\x00', '\x01', '\x00', '\x66', '\x55'};
std::string SERVER_ENDPOINT = "demos.kaazing.com/echo";

void print_bytearray(uint8_t* data, uint32_t size) {
  for(uint32_t i=0; i<size; ++i)
    printf("%02X ", data[i]);
  printf("\n");
};


int main(int argc, char* argv[]) {
  #ifdef USE_DEMO_SERVER // client using embedded demo server on localhost 8080
    // WEBSOCKET DEMO SERVER (using 1 thread)
    WsServer server;
    server.config.port = 8080;
    auto &echo = server.endpoint["^/echo/?$"];
    // server handler on message received
    echo.on_message = [](shared_ptr<WsServer::Connection> connection, shared_ptr<WsServer::Message> message) {
      uint32_t recv_size = message->size();
      printf("Server: received %u bytes - ", recv_size);
      // extract received data into bytearray
      uint8_t recv_bytes[recv_size];
      memset(recv_bytes, 0, sizeof(recv_bytes));
      char c;
      int i = 0;
      while (message->get(c)) {
        recv_bytes[i] = (uint8_t)c;
        i++;
      }
      print_bytearray(recv_bytes, recv_size);
      // send response data
      printf("Server: Sending out %u bytes \n", recv_size);
      auto send_stream = make_shared<WsServer::SendStream>();
      send_stream->write(RES_VERSION_INFO, sizeof(RES_VERSION_INFO));
      // connection->send is an asynchronous function
      connection->send(send_stream, [](const SimpleWeb::error_code &ec) {
        if(ec) {
          cout << "Server: Error sending message. " <<
              // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
              "Error: " << ec << ", error message: " << ec.message() << endl;
        }
      });
    };
    // server handler on connection open
    echo.on_open = [](shared_ptr<WsServer::Connection> connection) {
      cout << "Server: Opened connection " << connection.get() << endl;
    };
    // server handler on connection close- See RFC 6455 7.4.1. for status codes
    echo.on_close = [](shared_ptr<WsServer::Connection> connection, int status, const string & /*reason*/) {
      cout << "Server: Closed connection " << connection.get() << " with status code " << status << endl;
    };
    // server handler on connection error - See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
    echo.on_error = [](shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code &ec) {
      cout << "Server: Error in connection " << connection.get() << ". "
           << "Error: " << ec << ", error message: " << ec.message() << endl;
    };
    // start separate server thread
    thread server_thread([&server]() {
      // Start WS-server
      server.start();
    });
    // Wait for server to start so that the client can connect
    this_thread::sleep_for(chrono::seconds(1));
    SERVER_ENDPOINT = "localhost:8080/echo";
  #endif

  // parse cmd line args
  if ( argc>1 && std::string(argv[1])=="on" ) { // test client using embedded demo server on localhost 8080
    memcpy(CMD_TO_SEND, CMD_AMP_ON, sizeof(CMD_TO_SEND));
  } else if ( argc>1 && std::string(argv[1])=="off" ) {
    memcpy(CMD_TO_SEND, CMD_AMP_OFF, sizeof(CMD_TO_SEND));
  } else if ( argc>1 && std::string(argv[1])=="temp" ) {
    memcpy(CMD_TO_SEND, CMD_TEMP_INQUIRY, sizeof(CMD_TO_SEND));
  } else if ( argc>1 && std::string(argv[1])=="warn" ) {
    memcpy(CMD_TO_SEND, CMD_WARN_INQUIRY, sizeof(CMD_TO_SEND));
  } else {
    memcpy(CMD_TO_SEND, CMD_VERSION_INFO, sizeof(CMD_TO_SEND));
  }

  // WEBSOCKET CLIENT
  WsClient client(SERVER_ENDPOINT);
  // client handler on message received
  client.on_message = [](shared_ptr<WsClient::Connection> connection, shared_ptr<WsClient::Message> message) {
    uint32_t recv_size = message->size();
    printf("Client: received %u bytes - ", recv_size);
    // extract received data into bytearray
    uint8_t recv_bytes[recv_size];
    memset(recv_bytes, 0, sizeof(recv_bytes));
    char c;
    int i = 0;
    while (message->get(c)) {
      recv_bytes[i] = (uint8_t)c;
      i++;
    }
    print_bytearray(recv_bytes, recv_size);
    // FIXME immediately close connection
    cout << "Client: Sending close connection" << endl;
    connection->send_close(1000);
  };
  // client handler on connection open
  client.on_open = [](shared_ptr<WsClient::Connection> connection) {
    cout << "Client: Opened connection" << endl;
    // send message once connected
    auto send_stream = make_shared<WsClient::SendStream>();
    send_stream->write(CMD_TO_SEND, sizeof(CMD_TO_SEND));
    printf("Client: sending out %lu bytes \n", send_stream->size());
    connection->send(send_stream);
  };
  // client handler on connection close
  client.on_close = [](shared_ptr<WsClient::Connection> /*connection*/, int status, const string & /*reason*/) {
    cout << "Client: Closed connection with status code " << status << endl;
  };
  // client handler on connection error - See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
  client.on_error = [](shared_ptr<WsClient::Connection> /*connection*/, const SimpleWeb::error_code &ec) {
    cout << "Client: Error: " << ec << ", error message: " << ec.message() << endl;
  };
  // start client in this thread NOTE check this...
  client.start();

  #ifdef USE_DEMO_SERVER
    // join server thread to current thread (the used for client)
    server_thread.join();
  #endif
}
