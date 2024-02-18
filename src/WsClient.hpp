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
    std::atomic_bool connectionReady;
    std::atomic_bool popupShown{false};
    std::vector<std::string> messages_{};
    std::mutex mut_;
    void showErrorPopup(const std::string &msg);
    // void handleChat();
};

/**
 * show the imgui connection window
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
            ImGui::Text("Connecting to %s...", this->final_address.c_str());
            w_open = false;
        }
        ImGui::EndDisabled();
        ImGui::End();
    }
    return w_open;
}

/**
 * infinite loop of socket chat with imgui
 */
inline void WsClient::tryConnect()
{
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
            std::cout << "Received message: " << msg->str << std::endl;
            std::cout << "> " << std::flush;
        }
        else if (msg->type == ix::WebSocketMessageType::Open)
        {
            std::lock_guard<std::mutex> lg{this->mut_};
            // this->messages_.push_back("Connection established");
            std::cout << "Connection established" << std::endl;
            this->connectionReady = true;
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            this->messages_.push_back("Connection error: " + msg->errorInfo.reason);
            std::cout << "Connection error: " << msg->errorInfo.reason << std::endl;
            this->connectionReady = true;
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
        const static std::string err = this->messages_[this->messages_.size() - 1];
        ImGui::OpenPopup("Error", ImGuiPopupFlags_NoOpenOverExistingPopup);
        this->showErrorPopup(err);
        return;
    }

    static bool chatWindow = true;
    ImGui::SetNextWindowSize(ImVec2(400, 400));
    //!! You might want to use these ^^ values in the OS window instead, and add the ImGuiWindowFlags_NoTitleBar flag in
    //! the ImGui window !!
    if (chatWindow)
    {
        ImGui::Begin("chat window", &chatWindow);
        if (!this->connectionReady)
        {
            /* code */
            ImGui::Text("Connecting to %s...", this->final_address.c_str());
        }

        ImGui::BeginChild("chatmessages", ImVec2(300, 200), false);
        for (const auto &msg : this->messages_)
        {
            std::lock_guard<std::mutex> lg(mut_);
            if (!msg.empty())
            {
                ImGui::Text(msg.c_str());
            }
        }
        ImGui::EndChild();

        ImGui::SetCursorPos(ImVec2(0, 300));
        ImGui::PushItemWidth(300); // NOTE: (Push/Pop)ItemWidth is optional
        ImGui::PopItemWidth();
        static char msgpack[256] = "";
        ImGui::InputText("message", msgpack, IM_ARRAYSIZE(msgpack), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        if (ImGui::Button("Send", ImVec2(70.0, 0)))
        {
            std::lock_guard<std::mutex> lg{this->mut_};
            this->messages_.push_back("You: " + std::string(msgpack));
            webSocket.send(msgpack);
            memset(msgpack, NULL, sizeof(msgpack));
        }
        ImGui::End();
    }
    else
    {
        webSocket.stop();
    }
}

inline void WsClient::showErrorPopup(const std::string &msg)
{
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    static bool popen = true;
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (popen)
    {
        if (ImGui::BeginPopupModal("Error", &popen, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text(msg.c_str());
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
} // namespace chk
