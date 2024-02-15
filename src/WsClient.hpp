#pragma once
#include "GameManager.hpp"
#include <atomic>
#include <iostream>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <string>
namespace chk
{
class WsClient
{
  public:
    WsClient(const std::string &ipAddress, chk::GameManager *mgr) : ip_address(ipAddress), manager_(mgr){};
    WsClient() = delete;
    void operator()();

  private:
    std::string ip_address;
    chk::GameManager *manager_;
};

inline void WsClient::operator()()
{
    ix::initNetSystem();

    // Our websocket object
    ix::WebSocket webSocket;

    // TLS options
    ix::SocketTLSOptions tlsOptions;
#ifndef _WIN32
    // Currently system CAs are not supported on non-Windows platforms with mbedtls
    tlsOptions.caFile = "NONE";
#endif // _WIN32
    webSocket.setTLSOptions(tlsOptions);

    webSocket.setUrl(this->ip_address);
    std::cout << "Connecting to " << this->ip_address << "..." << std::endl;

    // To synchrously wait for connection to be established, use an atomic boolean
    std::atomic_bool connectionReady;

    // Setup a callback to be fired (in a background thread, watch out for race conditions !)
    // when a message or an event (open, close, error) is received
    webSocket.setOnMessageCallback([&webSocket, &connectionReady](const ix::WebSocketMessagePtr &msg) {
        if (msg->type == ix::WebSocketMessageType::Message)
        {
            std::cout << "Received message: " << msg->str << std::endl;
            std::cout << "> " << std::flush;
        }
        else if (msg->type == ix::WebSocketMessageType::Open)
        {
            std::cout << "Connection established" << std::endl;
            connectionReady = true;
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            std::cout << "Connection error: " << msg->errorInfo.reason << std::endl;
            connectionReady = true;
        }
    });

    // Now that our callback is setup, we can start our background thread and receive messages
    webSocket.start();

    // Wait for the connection to be ready (either successfully or with an error)
    while (!connectionReady)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Handle connection error/timeout
    if (webSocket.getReadyState() != ix::ReadyState::Open)
    {
        return;
    }

    // Send an initial message to the server (default to TEXT mode)
    webSocket.send("Hello from IXWebSocket!");

    // Allow the user to play around
    while (true)
    {
        std::string text;
        std::getline(std::cin, text);

        if (text.empty())
            break;

        webSocket.send(text);
    }
}
} // namespace chk
