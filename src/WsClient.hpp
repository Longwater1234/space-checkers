#pragma once
#include "CircularBuffer.hpp"
#include "payloads/ServerStructs.hpp"
#include <atomic>
#include <iostream>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <mutex>
#include <simdjson.h>
#include <spdlog/spdlog.h>
#include <string>

#include "imgui-SFML.h"
#include "imgui.h"

namespace chk
{

using onReadyCreatePieces = std::function<void(chk::payload::Welcome &)>; // callback after creating pieces

/**
 * This will handle all websocket exchanges with Server
 */
class WsClient final
{
  public:
    WsClient();
    void runMainLoop();
    void setOnReadyPiecesCallback(const onReadyCreatePieces &callback);
    bool replyToServer(const simdjson::dom::object &payload);

  private:
    std::string final_address;                      // IP or URL of server
    std::atomic_bool isDead{false};                 // if connection closed
    std::atomic_bool isConnected{false};            // if done connected to server (else, show loading)
    chk::CircularBuffer<std::string> msgBuffer{20}; // keep only recent 20 messages
    mutable std::string errorMsg{};                 // for any websocket errors
    std::atomic_bool conn_clicked = false;          // if 'connect' button clicked
    std::deque<std::string> serverMessages;         // messages from backend server

    onReadyCreatePieces _onReadyCreatePieces; // callback after creating pieces for both players
    std::mutex mut;
    std::unique_ptr<ix::WebSocket> webSocketPtr = nullptr;
    void showErrorPopup();
    void showChatWindow();
    void runServerLoop();
    static void showHint(const char *tip);
    void tryConnect(std::string_view address);
    void showConnectWindow();
    void resetAllStates();
};

inline chk::WsClient::WsClient()
{
    // Initialize WS
    ix::initNetSystem();
    // Our websocket object
    this->webSocketPtr = std::make_unique<ix::WebSocket>();
    // set inital connection timeout
    this->webSocketPtr->setHandshakeTimeout(10);
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
    static char inputUrl[256] = "";
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
        this->showChatWindow();
    }
    // connection failure 🙁
    if (this->isDead) {
       this->showErrorPopup();
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

    // ping server every 50 seconds
    this->webSocketPtr->setPingInterval(50);

    // Handle any connection error/timeout
    if (this->webSocketPtr->getReadyState() != ix::ReadyState::Open && this->isDead)
    {
        this->webSocketPtr->stop();
        return;
    }

    // ----------->>>>> LISTEN for UI updates from GameManager, forward to server
    // this->manager->setOnMoveSuccessCallback([this](const short &pieceId, const int &targetCell) {
    //    // TODO: SERIALIZE TO JSON, and send it here
    //    std::string pkg = fmt::format("I moved {} to cell index {}", pieceId, targetCell);
    //    spdlog::info(pkg);
    //    webSocket.send(pkg);
    //});

    //  this->showChatWindow();
}

/**
 * Set the callback to handle created pieces (from server)
 * @param callback - the callback function
 */
inline void WsClient::setOnReadyPiecesCallback(const onReadyCreatePieces &callback)
{
    this->_onReadyCreatePieces = callback;
}

/**
 * Send JSON response back to server
 * @param payload the request body
 */
inline bool WsClient::replyToServer(const simdjson::dom::object &payload)
{
    if (this->isDead)
    {
        return;
    }
    const auto &result = this->webSocketPtr->send(simdjson::to_string(payload));
    return result.success;
}

/**
 * Show the chat window and handle sending messsages
 * @param webSocket The websocket pointer
 */
inline void WsClient::showChatWindow()
{
    static bool chatWindow = true;
    ImGui::SetNextWindowSize(ImVec2(sf::Vector2f{400.0, 400.0}));
    if (chatWindow)
    {
        ImGui::Begin("Echo Chat", &chatWindow, ImGuiWindowFlags_NoResize);
        if (!this->isConnected)
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
                this->webSocketPtr->send(msgpack);
                memset(msgpack, 0, sizeof(msgpack));
            }
        }
        ImGui::PopItemWidth();
        ImGui::End();
    }
    else
    {
        // IF THIS chat WINDOW IS CLOSED, SHUTDOWN socket
        this->webSocketPtr->stop();
        this->isDead = true;
    }
}

/**
 * Exchange messages with the server and update the game accordingly. if any error happen, close connection
 */
inline void WsClient::runServerLoop()
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

    static simdjson::dom::parser jparser;
    try
    {
        for (const auto &msg : this->msgBuffer.getAll())
        {
            if (!msg.empty())
            {
                static simdjson::dom::object doc = jparser.parse(msg);
                uint16_t rawMsgType = static_cast<uint16_t>(doc.at_key("messageType").get_int64());
                chk::payload::MessageType msgType{rawMsgType};
                if (msgType == chk::payload::MessageType::WELCOME)
                {
                    // CREATE A 'welcome' object
                    chk::payload::Welcome welcome{};
                    auto rawTeam = static_cast<uint16_t>(doc.at_key("myTeam").get_uint64());
                    welcome.myTeam = chk::PlayerType{rawTeam};
                    for (const auto &val : doc.at_key("piecesRed").get_array())
                    {
                        welcome.piecesRed.emplace_back(static_cast<int16_t>(val.get_int64()));
                    }
                    for (const auto &val : doc.at_key("piecesBlack").get_array())
                    {
                        welcome.piecesBlack.emplace_back(static_cast<int16_t>(val.get_int64()));
                    }
                    // invoke the callback
                    if (this->_onReadyCreatePieces != nullptr)
                    {
                        this->_onReadyCreatePieces(welcome);
                    }
                }
                std::scoped_lock lg(this->mut);
                msgBuffer.clean();
            }
        }
    }
    catch (const simdjson::simdjson_error &ex)
    {
        std::scoped_lock lg(this->mut);
        this->errorMsg = fmt::format("JSON ERROR: {}", ex.what());
        this->isDead = true;
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
    };
}

} // namespace chk
