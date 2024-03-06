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
    explicit WsClient(chk::GameManager *mgr) : manager_(mgr){};
    WsClient() = delete;
    bool showConnectionWindow();
    void tryConnect();

  private:
    std::string final_address;  // IP or URL of server
    chk::GameManager *manager_;      // game manager
    std::atomic_bool isReady{false}; // wait for connection to open;
    std::atomic_bool isDead{false};  // when connection closed
    std::string errorMsg{};
    std::vector<std::string> messages{};
    std::mutex mut;
    bool w_open = true;
    void showErrorPopup() const;
    void showChatWindow(ix::WebSocket *webSocket);
};

/**
 * Show the imgui connection window, for server address
 * @return TRUE if CONNECT button is clicked, else FALSE
 */
inline bool WsClient::showConnectionWindow()
{
    static bool is_secure = false;
    static bool btn_disabled = false;
    static bool w_closed = false;
    if (w_open)
    {
        /* code */
        ImGui::SetNextWindowSize(ImVec2(sf::Vector2f(300.0, 300.0)));
        static char inputUrl[256];
        ImGui::Begin("Connect Window", nullptr, ImGuiWindowFlags_NoResize);
        ImGui::InputText("Server IP", inputUrl, IM_ARRAYSIZE(inputUrl), ImGuiInputTextFlags_CharsNoBlank);
        ImGui::Checkbox("Secure", &is_secure);
        ImGui::BeginDisabled(btn_disabled);
        if (!std::string_view(inputUrl).empty() && ImGui::Button("Connect", ImVec2(100.0f, 0)))
        {
            btn_disabled = true;
            const char *suffix = is_secure ? "wss://" : "ws://";
            this->final_address = suffix + std::string(inputUrl);
            w_open = false;
            w_closed = true;
            memset(inputUrl, 0, sizeof(inputUrl));
        }
        ImGui::EndDisabled();
        ImGui::End();
    }
    // negate the FALSE value
    return w_closed;
}

/**
 * Try to connect to Server
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

    webSocket.setUrl(final_address);

    // set inital connection timeout
    webSocket.setHandshakeTimeout(10);

    // Setup a callback to be fired (in a background thread, watch out for race conditions !)
    // when a message or an event (open, close, error) is received
    webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg) {
        if (msg->type == ix::WebSocketMessageType::Message)
        {
            std::lock_guard<std::mutex> lg{this->mut};
            this->messages.emplace_back("Server: " + msg->str);
        }
        else if (msg->type == ix::WebSocketMessageType::Open)
        {
            std::lock_guard lg{this->mut};
            this->messages.emplace_back("Connection established");
            this->isReady = true;
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            std::lock_guard lg{this->mut};
            errorMsg = "Connection error: " + msg->errorInfo.reason;
            std::cerr << errorMsg << std::endl;
            this->isDead = true;
        }
    });

    // Start our b/ground thread and receive messages (if websocket not dead)
    if (!this->isDead)
    {
        webSocket.start();
    }

    // ping server every 50 seconds
    webSocket.setPingInterval(50);

    // Handle connection error/timeout
    if (webSocket.getReadyState() != ix::ReadyState::Open && this->isDead)
    {
        ImGui::OpenPopup("Error", ImGuiPopupFlags_NoOpenOverExistingPopup);
        this->showErrorPopup();
        webSocket.stop();
        return;
    }

    //LISTEN for UI game updates from manager
    this->manager_->setOnMoveSuccessCallback([this](const short &pieceId, const int &targetCell) {
        std::cout << " I moved " << pieceId << " to cell index " << targetCell << std::endl;
        webSocket.send("i played " + std::to_string(pieceId) + " to cell " + std::to_string(targetCell));
    });

    this->showChatWindow(&webSocket);
}

/**
 * Show the chat window and handle sending messsages
 * @param webSocket The websocket pointer
 */
inline void WsClient::showChatWindow(ix::WebSocket *webSocket)
{
    static bool chatWindow = true;
    ImGui::SetNextWindowSize(ImVec2(400, 400));
    if (chatWindow)
    {
        ImGui::Begin("Echo Chat", &chatWindow, ImGuiWindowFlags_NoResize);
        if (!this->isReady)
        {
            /* code */
            ImGui::Text("Connecting to %s", this->final_address.data());
        }

        ImGui::BeginChild("chatmessages", ImVec2(300, 200), false);
        for (const auto &msg : this->messages)
        {
            if (!msg.empty())
            {
                ImGui::Text(u8"%s", msg.c_str());
            }
        }
        ImGui::EndChild();

        ImGui::SetCursorPos(ImVec2(0, 300));
        ImGui::PushItemWidth(300);
        static char msgpack[256] = "";
        if (ImGui::InputTextWithHint(".", "Write message, press Enter", msgpack, IM_ARRAYSIZE(msgpack),
                                     ImGuiInputTextFlags_EnterReturnsTrue) &&
            this->isReady)
        {
            if (!std::string_view(msgpack).empty())
            {
                std::lock_guard lg{this->mut};
                this->messages.emplace_back("You " + std::string(msgpack));
                webSocket->send(msgpack);
                memset(msgpack, 0, sizeof(msgpack));
            }
        }
        ImGui::PopItemWidth();
        ImGui::End();
    }
    else
    {
        // IF THIS WINDOW IS CLOSED, SHUTDOWN socket
        webSocket->stop();
        this->isDead = true;
    }
}

/**
 * Show error message from websockets as a popup window
 */
inline void WsClient::showErrorPopup() const
{
    if (this->errorMsg.empty())
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
            ImGui::Text(u8"%s", this->errorMsg.c_str());
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
