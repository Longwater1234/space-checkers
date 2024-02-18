#pragma once
#include "GameManager.hpp"
#include <atomic>
#include <iostream>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <mutex>
#include <string>
#include <vector>

#include "imgui-SFML.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

namespace chk
{
class WsClient
{
  public:
    WsClient(chk::GameManager *mgr) : manager_(mgr){};
    WsClient() = delete;
    bool showConnectionWindow();
    void tryConnect();

  private:
    std::string final_address;
    chk::GameManager *manager_;
    std::atomic_bool connectionReady{false};  // wait for connection to open;
    std::atomic_bool connectionClosed{false}; // when connection closed
    std::string errorMsg{};
    std::vector<std::string> messages_{};
    std::mutex mut_;
    void showErrorPopup(std::string_view msg);
    void showChatWindow(ix::WebSocket *webSocket);
};

/**
 * Show the imgui connection window, for server address
 * @return TRUE if CONNECT button is clicked, else FALSE
 */
inline bool WsClient::showConnectionWindow()
{
    static bool is_secure = false;
    static bool w_open = true;
    static bool btn_disabled = false;
    // static std::string ip_address{};
    if (w_open)
    {
        /* code */
        ImGui::SetNextWindowSize(ImVec2(sf::Vector2f(300.0, 300.0)));
        static char inputUrl[256];
        ImGui::Begin("Connect Window", nullptr, ImGuiWindowFlags_NoResize);
        ImGui::InputText("IP Address", inputUrl, IM_ARRAYSIZE(inputUrl));
        ImGui::Checkbox("Secure", &is_secure);
        ImGui::BeginDisabled(btn_disabled);
        if (std::string_view(inputUrl).size() > 0 && ImGui::Button("Connect", ImVec2(100.0f, 0)))
        {
            btn_disabled = true;
            const char *suffix = is_secure ? "wss://" : "ws://";
            this->final_address = suffix + std::string(inputUrl);
            memset(inputUrl, NULL, sizeof(inputUrl));
            w_open = false;
        }
        ImGui::EndDisabled();
        ImGui::End();
    }
    // negate the FALSE value
    return !w_open;
}

/**
 * Try to connect to server
 */
inline void WsClient::tryConnect()
{
    // Initialize WS
    ix::initNetSystem();

    // Our websocket object
    static ix::WebSocket webSocket;

    // TLS options
    static ix::SocketTLSOptions tlsOptions;
#ifndef _WIN32
    // Currently system CAs are not supported on non-Windows platforms with mbedtls
    tlsOptions.caFile = "NONE";
#endif // _WIN32
    webSocket.setTLSOptions(tlsOptions);

    webSocket.setUrl(this->final_address);

    // To synchrously wait for connection to be established, use an atomic boolean

    // Setup a callback to be fired (in a background thread, watch out for race conditions !)
    // when a message or an event (open, close, error) is received
    webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg) {
        if (msg->type == ix::WebSocketMessageType::Message)
        {
            std::lock_guard<std::mutex> lg{this->mut_};
            this->messages_.push_back("Server: " + msg->str);
        }
        else if (msg->type == ix::WebSocketMessageType::Open)
        {
            std::lock_guard<std::mutex> lg{this->mut_};
            this->messages_.push_back("Connection established");
            this->connectionReady = true;
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            std::lock_guard<std::mutex> lg{this->mut_};
            errorMsg = "Connection error: " + msg->errorInfo.reason;
            this->connectionClosed = true;
        }
    });

    // Start our b/ground thread and receive messages (if connection not dead)
    if (!connectionClosed)
    {
        webSocket.start();
    }

    // ping server every 50 seconds
    webSocket.setPingInterval(50);

    // Wait for the connection to be ready (either successfully or with an error)
    while (!connectionReady && webSocket.getReadyState() != ix::ReadyState::Closed)
    {
        std::cout << "i ran ! " << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Handle connection error/timeout
    if (webSocket.getReadyState() != ix::ReadyState::Open)
    {
        connectionClosed = true;
        if (!this->messages_.empty())
        {
            ImGui::OpenPopup("Error", ImGuiPopupFlags_NoOpenOverExistingPopup);
            this->showErrorPopup(errorMsg);
            webSocket.stop();
            return;
        }
    }

    this->showChatWindow(&webSocket);
}

/**
 * Show error message from websockets as a popup window
 * @param msg The error message
 */
inline void WsClient::showErrorPopup(std::string_view msg)
{
    if (msg.empty())
    {
        return;
    }
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    static bool popen = true;
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (popen)
    {
        if (ImGui::BeginPopupModal("Error", &popen, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text(msg.data());
            ImGui::Separator();
            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                popen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        };
    }
}

/**
 * Show the chat window and handle sending messsages
 * @param websocket The websocket pointer
 */
inline void WsClient::showChatWindow(ix::WebSocket *webSocket)
{
    static bool chatWindow = true;
    ImGui::SetNextWindowSize(ImVec2(400, 400));
    if (chatWindow)
    {
        ImGui::Begin("chat window", &chatWindow, ImGuiWindowFlags_NoResize);
        if (!this->connectionReady)
        {
            /* code */
            ImGui::Text("Connecting to %s", this->final_address.c_str());
        }

        ImGui::BeginChild("chatmessages", ImVec2(300, 200), false);
        for (const auto &msg : this->messages_)
        {
            if (!msg.empty())
            {
                ImGui::Text(msg.c_str());
            }
        }
        ImGui::EndChild();

        ImGui::SetCursorPos(ImVec2(0, 300));
        ImGui::PushItemWidth(300);
        ImGui::PopItemWidth();
        static char msgpack[256] = "";
        ImGui::InputText("message", msgpack, IM_ARRAYSIZE(msgpack), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        if (ImGui::Button("Send", ImVec2(70.0, 0)))
        {
            std::lock_guard<std::mutex> lg{this->mut_};
            this->messages_.push_back("You: " + std::string(msgpack));
            webSocket->send(msgpack);
            memset(msgpack, NULL, sizeof(msgpack));
        }
        ImGui::End();
    }
    else
    {
        // IF THIS WINDOW IS CLOSED, SHUTDOWN socket
        webSocket->stop();
        this->connectionClosed = true;
    }
}
} // namespace chk
