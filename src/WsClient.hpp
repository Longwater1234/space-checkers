#pragma once
#include "CircularBuffer.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include <atomic>
#include <iostream>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <mutex>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <string>

#include "imgui-SFML.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

namespace chk
{
class WsClient
{
  public:
    explicit WsClient(chk::GameManager *mgr, chk::Player *p1, chk::Player *p2)
        : manager(mgr), player1(p1), player2(p2){};
    WsClient() = delete;
    bool doneConnectWindow();
    void tryConnect();

  private:
    std::string final_address;                      // IP or URL of server
    chk::GameManager *manager;                      // game manager (pointer)
    std::atomic_bool isReady{false};                // if connection ready open
    std::atomic_bool isDead{false};                 // if connection closed
    chk::CircularBuffer<std::string> msgBuffer{20}; // keep only recent 20 messages
    std::string errorMsg{};                         // for any websocket errors
    std::mutex mut;
    bool w_open = true;
    chk::Player *player1;
    chk::Player *player2;
    void showErrorPopup() const;
    void showChatWindow(ix::WebSocket *webSocket);
    void showHint(const char *tip) const;
};

/**
 * Show help tooltip with given message
 * @param tip the help message
 */
inline void WsClient::showHint(const char *tip) const
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(tip);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

/**
 * Show the imgui connection window, for server address
 * @return TRUE if CONNECT button is clicked, else FALSE
 */
inline bool WsClient::doneConnectWindow()
{
    static bool is_secure = false;
    static bool btn_disabled = false;
    static bool btn_clicked = false;
    if (w_open)
    {
        ImGui::SetNextWindowSize(ImVec2(sf::Vector2f(300.0, 300.0)));
        static char inputUrl[256] = "";
        ImGui::Begin("Connect Window", nullptr, ImGuiWindowFlags_NoResize);
        ImGui::InputText("Server IP", inputUrl, IM_ARRAYSIZE(inputUrl), ImGuiInputTextFlags_CharsNoBlank);
        ImGui::SameLine();
        this->showHint("eg: 127.0.0.1:8080 OR myserver.example.org");
        ImGui::Checkbox("Secure", &is_secure);
        ImGui::BeginDisabled(btn_disabled);
        if (!std::string_view(inputUrl).empty() && ImGui::Button("Connect", ImVec2(100.0f, 0)))
        {
            btn_disabled = true;
            const char *suffix = is_secure ? "wss://" : "ws://";
            this->final_address = suffix + std::string(inputUrl);
            this->w_open = false;
            btn_clicked = true;
            memset(inputUrl, 0, sizeof(inputUrl));
        }
        ImGui::EndDisabled();
        ImGui::End();
    }
    return btn_clicked;
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

    // Setup a callback to be fired when an event (open, close, msg, error) is received
    webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg) {
        if (msg->type == ix::WebSocketMessageType::Message)
        {
            std::scoped_lock<std::mutex> lg{this->mut};
            this->msgBuffer.addItem("Server: " + msg->str);
        }
        else if (msg->type == ix::WebSocketMessageType::Open)
        {
            std::scoped_lock lg{this->mut};
            this->msgBuffer.addItem("Connection established");
            this->isReady = true;
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            std::scoped_lock lg{this->mut};
            this->errorMsg = "Connection error: " + msg->errorInfo.reason;
            spdlog::error(this->errorMsg);
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
        // TODO this should be guarded by "if" statement and global bool variable
        this->showErrorPopup();
        webSocket.stop();
        return;
    }

    // LISTEN for UI game updates from manager, send to server
    this->manager->setOnMoveSuccessCallback([this](const short &pieceId, const int &targetCell) {
        std::string pkg = "I moved " + std::to_string(pieceId).append(" to cell index ") + std::to_string(targetCell);
        spdlog::info(pkg);
        webSocket.send(pkg);
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
    ImGui::SetNextWindowSize(ImVec2(sf::Vector2f{400, 400}));
    if (chatWindow)
    {
        ImGui::Begin("Echo Chat", &chatWindow, ImGuiWindowFlags_NoResize);
        if (!this->isReady)
        {
            /* code */
            ImGui::Text("Connecting to %s", this->final_address.c_str());
            ImGui::End();
            return;
        }

        ImGui::BeginChild("ChatMessages", ImVec2(300.0, 290.0), ImGuiChildFlags_None);
        for (const auto &msg : this->msgBuffer.getAll())
        {
            if (!msg.empty())
            {
                ImGui::TextWrapped(u8"%s", msg.c_str());
            }
        }
        ImGui::EndChild();

        ImGui::SetCursorPos(ImVec2(sf::Vector2f{0, 350.0}));
        ImGui::PushItemWidth(300);
        static char msgpack[256] = "";
        if (ImGui::InputTextWithHint(".", "Write message, press Enter", msgpack, IM_ARRAYSIZE(msgpack),
                                     ImGuiInputTextFlags_EnterReturnsTrue))
        {
            if (!std::string_view(msgpack).empty())
            {
                std::scoped_lock<std::mutex> lg{this->mut};
                this->msgBuffer.addItem("You: " + std::string(msgpack));
                webSocket->send(msgpack);
                memset(msgpack, 0, sizeof(msgpack));
            }
        }
        ImGui::PopItemWidth();
        ImGui::End();
    }
    else
    {
        // IF THIS chat WINDOW IS CLOSED, SHUTDOWN socket
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
    ImGui::SetNextWindowPos(center, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
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
