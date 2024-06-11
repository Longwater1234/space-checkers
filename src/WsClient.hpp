#pragma once
#include "CircularBuffer.hpp"
#include "payloads/base_payload.pb.hpp"
#include <atomic>
#include <iostream>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <mutex>
#include <spdlog/spdlog.h>
#include <string>

#include "imgui-SFML.h"
#include "imgui.h"

namespace chk
{

// callback when connected to server success
using onConnectedServer = std::function<void(chk::payload::WelcomePayload &, std::string_view notice)>;
// callback when both players joined match
using onReadyStartGame = std::function<void(chk::payload::StartPayload &, std::string_view notice)>;
// when connection to server dies non-gracefully
using onDeathCallback = std::function<void(bool)>;
// when opponent makes a simple move
using onMovePieceCallback = std::function<void(chk::payload::MovePayload &)>;

/**
 * This handles all websocket exchanges with Server
 */
class WsClient final
{
  public:
    WsClient();
    void runMainLoop();
    void setOnReadyConnectedCallback(const onConnectedServer &callback);
    void setOnReadyStartGameCallback(const onReadyStartGame &callback);
    void setOnDeathCallback(const onDeathCallback &callback);
    void setOnMovePieceCallback(const onMovePieceCallback &callback);
    bool replyServer(chk::payload::BasePayload *payload) const;

  private:
    std::string final_address;                     // IP or URL of server
    std::atomic_bool isDead{false};                // if connection closed
    std::atomic_bool isConnected{false};           // if done connected to server (else, show loading)
    chk::CircularBuffer<std::string> msgBuffer{1}; // keep only recent 20 messages
    mutable std::string errorMsg{};                // for any websocket errors
    std::atomic_bool conn_clicked = false;         // if 'connect' button clicked
    mutable std::string protoBucket{};             // reusable buffer used to serialize payloads to server

    onConnectedServer _onReadyConnected;
    onReadyStartGame _onReadyStartGame;
    onDeathCallback _onDeathCallback;
    onMovePieceCallback _onMovePieceCallback;

    std::mutex mut;
    std::unique_ptr<ix::WebSocket> webSocketPtr = nullptr; // our Websocket object
    void showErrorPopup();
    void initGameLoop();
    static void showHint(const char *tip);
    void tryConnect(std::string_view address);
    void showConnectWindow();
    void resetAllStates();
};

inline chk::WsClient::WsClient()
{
    // Required on Windows
    ix::initNetSystem();
    // Our websocket object
    this->webSocketPtr = std::make_unique<ix::WebSocket>();
    // set inital connection timeout
    this->webSocketPtr->setHandshakeTimeout(10);
    // once dead, dont revive
    this->webSocketPtr->disableAutomaticReconnection();
    ix::SocketTLSOptions tlsOptions;
#ifndef _WIN32
    // Currently system CAs are not supported on non-Windows platforms with mbedtls
    tlsOptions.caFile = "NONE";
#endif // _WIN32
    this->webSocketPtr->setTLSOptions(tlsOptions);
}

/**
 * Show help tooltip with given message
 * @param tip the help message
 */
inline void WsClient::showHint(const char *tip)
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
inline void WsClient::showConnectWindow()
{
    static bool is_secure = false;
    ImGui::SetNextWindowSize(ImVec2(sf::Vector2f(300.0, 300.0)));
    static char inputUrl[256] = "127.0.0.1:9876/game";
    if (ImGui::Begin("Connect Window", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        ImGui::InputText("Server IP", inputUrl, IM_ARRAYSIZE(inputUrl), ImGuiInputTextFlags_CharsNoBlank);
        ImGui::SameLine();
        WsClient::showHint("eg: 127.0.0.1:8080 OR myserver.example.org");
        ImGui::Checkbox("Secure", &is_secure);
        if (!std::string_view(inputUrl).empty() && ImGui::Button("Connect", ImVec2(100.0f, 0)))
        {
            const char *suffix = is_secure ? "wss://" : "ws://";
            this->final_address = suffix + std::string(inputUrl);
            this->conn_clicked = true;
            memset(inputUrl, 0, sizeof(inputUrl));
        }
        ImGui::End();
    }
}

/**
 * Reset all local states to FALSE or empty string
 */
inline void WsClient::resetAllStates()
{
    this->isConnected = false;
    this->conn_clicked = false;
    this->isDead = false;
    this->errorMsg.clear();
    this->webSocketPtr->stop();
}

/**
 * Run main loop of showing connection window, tryConnect, and handle exchanges
 */
inline void WsClient::runMainLoop()
{
    // clang-format off
    if (!isConnected) {
        if (!conn_clicked) {
           this->showConnectWindow();
        } else {
            this->tryConnect(final_address);
        }
    }
    // already connected
    else {
        this->initGameLoop();
    }
    // connection failure 🙁
    if (this->isDead) {
       this->showErrorPopup();
       if (this->_onDeathCallback != nullptr) {
        _onDeathCallback(true);
       }
    }
    // clang-format on
}

/**
 * Try to connect to Server
 * @param address server IP or URI
 */
inline void WsClient::tryConnect(std::string_view address)
{
    this->webSocketPtr->setUrl(address.data());
    if (!this->isConnected)
    {
        ImGui::SetNextWindowSize(ImVec2(sf::Vector2f{400.0, 100.0}));
        ImGui::Begin("Loading", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Connecting to %s", this->final_address.c_str());
        ImGui::End();
    }
    // Setup a callback to be fired when an Async event is received
    this->webSocketPtr->setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg) {
        if (msg->type == ix::WebSocketMessageType::Message)
        {
            std::scoped_lock<std::mutex> lg{this->mut};
            this->msgBuffer.addItem(msg->str);
        }
        else if (msg->type == ix::WebSocketMessageType::Open)
        {
            spdlog::info("Connection established");
            this->isConnected = true;
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
        this->webSocketPtr->start();
    }

    // Handle any connection error/timeout
    if (this->webSocketPtr->getReadyState() != ix::ReadyState::Open && this->isDead)
    {
        this->webSocketPtr->stop();
        return;
    }
}

/**
 * Set the callback to handle created pieces (from server)
 * @param callback - the callback function
 */
inline void WsClient::setOnReadyConnectedCallback(const onConnectedServer &callback)
{
    this->_onReadyConnected = callback;
}

/**
 * Set the callback to handle starting game after signal from server
 * @param callback the callback function
 */
inline void WsClient::setOnReadyStartGameCallback(const onReadyStartGame &callback)
{
    this->_onReadyStartGame = callback;
}

/**
 * Set the callback to handle connection failures or sudden cut-off
 * @param callback the callback function
 */
inline void WsClient::setOnDeathCallback(const onDeathCallback &callback)
{
    this->_onDeathCallback = callback;
}

/**
 * Set the callback for handling Opponent moving their piece
 * @param callback the callback function
 */
inline void WsClient::setOnMovePieceCallback(const onMovePieceCallback &callback)
{
    this->_onMovePieceCallback = callback;
}

/**
 * Send JSON response back to server
 * @param payload the request body
 */
inline bool WsClient::replyServer(chk::payload::BasePayload *payload) const
{
    if (this->isDead || !this->isConnected)
    {
        return false;
    }
    payload->SerializeToString(&this->protoBucket);
    const auto &result = this->webSocketPtr->sendBinary(this->protoBucket);
    return result.success;
}

/**
 * Exchange messages with the server and update the game accordingly. if any error happen, close connection
 */
inline void WsClient::initGameLoop()
{
    if (!this->isConnected)
    {
        if (ImGui::Begin("Loading", nullptr, ImGuiWindowFlags_NoResize))
        {
            /* code */
            ImGui::TextWrapped("Connecting to %s", this->final_address.c_str());
            ImGui::End();
            return;
        }
    }

    for (const auto &msg : this->msgBuffer.getAll())
    {
        if (!msg.empty())
        {
            chk::payload::BasePayload basePayload;
            basePayload.ParseFromString(msg);
            if (basePayload.has_welcome())
            {
                /* code */
                chk::payload::WelcomePayload welcome = basePayload.welcome();
                if (this->_onReadyConnected != nullptr)
                {
                    this->_onReadyConnected(welcome, basePayload.notice());
                }
            }
            else if (basePayload.has_start())
            {
                chk::payload::StartPayload startPayload = basePayload.start();
                if (this->_onReadyStartGame != nullptr)
                {
                    this->_onReadyStartGame(startPayload, basePayload.notice());
                }
            }
            else if (basePayload.has_exit_payload())
            {
                std::scoped_lock lg(this->mut);
                this->errorMsg = basePayload.notice();
                spdlog::error(basePayload.notice());
                this->isDead = true;
            }
            else if (basePayload.has_move_payload())
            {
                chk::payload::MovePayload movePayload = basePayload.move_payload();
                spdlog::info(movePayload.DebugString());
                if (this->_onMovePieceCallback != nullptr)
                {
                    this->_onMovePieceCallback(movePayload);
                }
            }
            std::scoped_lock lg(this->mut);
            msgBuffer.clean();
        }
    }
}

/**
 * Show error message from websockets as a popup window
 */
inline void WsClient::showErrorPopup()
{
    if (this->errorMsg.empty())
    {
        return;
    }
    // Always center this next dialog
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_FirstUseEver, ImVec2(0.5, 0.5));
    ImGui::OpenPopup("Error", ImGuiPopupFlags_NoOpenOverExistingPopup);
    if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text(u8"%s", this->errorMsg.c_str());
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            this->resetAllStates();
        }
        ImGui::EndPopup();
    }
}

} // namespace chk
