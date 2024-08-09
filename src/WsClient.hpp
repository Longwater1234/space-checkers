#pragma once
#include "CircularBuffer.hpp"
#include "ServerLocation.hpp"
#include "payloads/base_payload.pb.hpp"
#include <atomic>
#include <cstdlib>
#include <filesystem>
#include <ixwebsocket/IXHttpClient.h>
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

// callback when connected to server success
using onConnectedServer = std::function<void(const chk::payload::WelcomePayload &, std::string_view notice)>;
// callback when both players joined match
using onReadyStartGame = std::function<void(const chk::payload::StartPayload &, std::string_view notice)>;
// when server kills our connection
using onDeathCallback = std::function<void(std::string_view notice)>;
// when opponent makes a simple move
using onMovePieceCallback = std::function<void(const chk::payload::MovePayload &)>;
// when opponent captures my piece
using onCaptureCallback = std::function<void(const chk::payload::CapturePayload &)>;
// when we got a winner or loser
using onWinLoseCallback = std::function<void(std::string_view notice)>;

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
    void setOnCapturePieceCallback(const onCaptureCallback &callback);
    void setOnWinLoseCallback(const onWinLoseCallback &callback);
    bool replyServer(const chk::payload::BasePayload &payload) const;

  private:
    std::string final_address;                      // IP or URL of private server (input by User)
    std::atomic_bool isDead{false};                 // if connection closed
    std::atomic_bool isConnected{false};            // if done connected to server (else, show loading)
    chk::CircularBuffer<std::string> msgBuffer{1};  // keep only recent 1 message
    mutable std::string errorMsg{};                 // for any websocket errors
    std::atomic_bool connClicked = false;           // if 'connect' button clicked
    std::vector<chk::ServerLocation> publicServers; // list of public servers (fetched from CDN)

    onConnectedServer _onReadyConnected;
    onReadyStartGame _onReadyStartGame;
    onDeathCallback _onDeathCallback;
    onMovePieceCallback _onMovePieceCallback;
    onCaptureCallback _onCaptureCallback;
    onWinLoseCallback _onWinLoseCallback;

    std::mutex mut;
    std::unique_ptr<ix::WebSocket> webSocketPtr = nullptr; // our Websocket object
    void showErrorPopup();                                 // whenver there is an error (from server)
    void showWinnerPopup(std::string_view notice);         // when server notifies about winner
    void runGameLoop();
    static void showHint(const char *tip);
    void tryConnect(std::string_view address);
    void showConnectWindow();
    void showPublicServerWindow(bool &showPublic);
    void prefetchPublicServers();
    void resetAllStates();
};

inline chk::WsClient::WsClient()
{
    ix::initNetSystem();
    // Our websocket object
    this->webSocketPtr = std::make_unique<ix::WebSocket>();
    // set inital connection timeout
    this->webSocketPtr->setHandshakeTimeout(10);
    // once dead, DO NOT try reconnect
    this->webSocketPtr->disableAutomaticReconnection();
    // ping server every 30 seconds
    this->webSocketPtr->setPingInterval(30);
    ix::SocketTLSOptions tlsOptions;
#ifndef _WIN32
    // Currently system CAs are not supported on non-Windows platforms with mbedtls
    tlsOptions.caFile = "NONE";
#endif // _WIN32
    this->webSocketPtr->setTLSOptions(tlsOptions);
    // prefetch for public server list
    this->prefetchPublicServers();
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
 * Show the imgui connection window, for private server address input
 */
inline void WsClient::showConnectWindow()
{
    static bool is_secure = false;
    static bool showPublic = true;

    if (showPublic)
    {
        this->showPublicServerWindow(showPublic);
        return;
    }

    // =================== PRIVATE SERVERS ===============================
    ImGui::SetNextWindowSize(ImVec2(sf::Vector2f(300.0, 300.0)));
    static char inputUrl[256] = "127.0.0.1:9876/game";
    if (ImGui::Begin("Private Server", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        ImGui::InputText("Server IP", inputUrl, IM_ARRAYSIZE(inputUrl), ImGuiInputTextFlags_CharsNoBlank);
        ImGui::SameLine();
        WsClient::showHint("eg: 127.0.0.1:8080 OR myserver.example.org");
        ImGui::Checkbox("Secure", &is_secure);
        if (!std::string_view(inputUrl).empty() && ImGui::Button("Connect", ImVec2{100.0f, 0}))
        {
            const char *suffix = is_secure ? "wss://" : "ws://";
            this->final_address = suffix + std::string(inputUrl);
            this->connClicked = true;
            memset(inputUrl, 0, sizeof(inputUrl));
        }
        if (ImGui::Button("< Go Back", ImVec2{100.0f, 0}))
        {
            showPublic = true;
        }
        ImGui::End();
    }
}

/**
 * Show list of public game servers, using imgui ListBox
 * @param showPublic switch for showing/hiding this window
 */
inline void WsClient::showPublicServerWindow(bool &showPublic)
{
    // =================== PUBLIC SERVERS ===============================
    ImGui::SetNextWindowSize(ImVec2(sf::Vector2f(300.0, 300.0)));
    if (ImGui::Begin("Public Servers", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        static int current_idx = 0;
        if (ImGui::BeginListBox("Select One"))
        {
            for (int i = 0; i < publicServers.size(); i++)
            {
                const bool is_selected = (i == current_idx);
                if (ImGui::Selectable(publicServers.at(i).name.c_str(), is_selected))
                {
                    current_idx = i;
                }

                // Set the initial focus
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }
        if (!publicServers.empty() && ImGui::Button("Connect", ImVec2{100.0f, 0}))
        {
            this->final_address = publicServers.at(current_idx).address;
            this->connClicked = true;
        }
        publicServers.empty() ? ImGui::NewLine() : ImGui::SameLine();
        if (ImGui::Button("Refresh", ImVec2{90.0f, 0}))
        {
            this->prefetchPublicServers();
        }
        if (ImGui::Button("My Private Server >", ImVec2{150.0f, 0}))
        {
            showPublic = false;
        }
        ImGui::End();
    }
}

/**
 * Fetch public servers JSON list from central storage (which is updated regularly)
 */
inline void WsClient::prefetchPublicServers()
{
    const std::string url = "https://d1txhef4jwuosv.cloudfront.net/ws_server_locations.json";
    std::filesystem::path tempFile = std::filesystem::temp_directory_path() / "json_result.txt";
    const std::string tempFileStr = tempFile.u8string();

#ifdef WIN32
    ix::HttpClient httpClient;
    auto args = httpClient.createRequest(url, ix::HttpClient::kGet);
    ix::HttpResponsePtr response = httpClient.get(url, args); // blocking call (Async DOES not work!)

    std::FILE *ff{};
    fopen_s(&ff, tempFileStr.c_str(), "w");
    int statusCode = response->statusCode;
    if (statusCode != 200 || ff == nullptr)
    {
        spdlog::error("http request failed. Reason {}", response->errorMsg);
        // std::scoped_lock lg(this->mut);
        this->errorMsg = response->errorMsg;
        this->isDead = true;
        return;
    }
    fputs(response->body.c_str(), ff);
    fclose(ff);

#else
    // CURL is available on Unix OS (MacOS, Linux). ixHttpClient doesnt work on Unix!
    const std::string commandStr = fmt::format("curl -fsSL {} -o {}", url, tempFileStr);
    if (std::system(commandStr.c_str()))
    {
        // comand failed. exit value != 0
        this->isDead = true;
        this->errorMsg = "failed to fetch public server list";
        return;
    }
#endif // WIN32

    simdjson::dom::parser jsonParser;
    try
    {
        simdjson::dom::array jsonArray = jsonParser.load(tempFileStr);
        // std::scoped_lock lg(this->mut);
        this->publicServers.clear();
        for (const simdjson::dom::object &elem : jsonArray)
        {
            chk::ServerLocation location{};
            location.name = elem.at_key("name").get_c_str();
            location.address = elem.at_key("address").get_c_str();
            this->publicServers.emplace_back(location);
        }
    }
    catch (const simdjson::simdjson_error &ex)
    {
        std::scoped_lock lg(this->mut);
        this->errorMsg = ex.what();
        this->isDead = true;
    }
    // });
}

/**
 * Reset all local states to FALSE or empty string
 */
inline void WsClient::resetAllStates()
{
    this->isConnected = false;
    this->connClicked = false;
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
        if (!connClicked) {
           this->showConnectWindow();
        } else {
            this->tryConnect(final_address);
        }
    }
    // already connected
    else {
        this->runGameLoop();
    }
    // some error happened 🙁
    if (this->isDead) {
       this->showErrorPopup();
       if (this->_onDeathCallback != nullptr) {
        _onDeathCallback(this->errorMsg);
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
        // ImGui::Text("Connecting to %s", this->final_address.c_str());
        ImGui::Text("Connecting to online server");
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
        else if (msg->type == ix::WebSocketMessageType::Close)
        {
            std::scoped_lock lg{this->mut};
            this->errorMsg = "Error: disconnected from Server!" + msg->str;
            spdlog::error(this->errorMsg);
            this->isDead = true;
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
 * Set the callback to handle connection failures or server kickouts
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
 * Set the callback for handling Opponent capturing my Piece
 * @param callback the callback function
 */
inline void WsClient::setOnCapturePieceCallback(const onCaptureCallback &callback)
{
    this->_onCaptureCallback = callback;
}

/**
 * Set the callback for handling Winner or Loser of match
 * @param callback the callback function
 */
inline void WsClient::setOnWinLoseCallback(const onWinLoseCallback &callback)
{
    this->_onWinLoseCallback = callback;
}

/**
 * Send Protobuf response back to server
 * @param payload the request body
 * @return TRUE if sent successfully, else FALSE
 */
inline bool WsClient::replyServer(const chk::payload::BasePayload &payload) const
{
    if (this->isDead || !this->isConnected)
    {
        return false;
    }
    spdlog::info("SENDING {}", payload.ShortDebugString());
    const auto &result = this->webSocketPtr->sendBinary(payload.SerializeAsString());
    return result.success;
}

/**
 * Exchange messages with the server and update the game accordingly. if any error happen, close connection
 */
inline void WsClient::runGameLoop()
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
        if (msg.empty())
        {
            continue;
        }
        chk::payload::BasePayload basePayload;
        if (!basePayload.ParseFromString(msg))
        {
            std::scoped_lock lg(this->mut);
            this->errorMsg = "Profobuf: Could not parse payload";
            this->isDead = true;
            return;
        }

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
            std::scoped_lock lg{this->mut};
            this->errorMsg = basePayload.notice();
            spdlog::error(basePayload.notice());
            this->isDead = true;
        }
        else if (basePayload.has_move_payload())
        {
            if (this->_onMovePieceCallback != nullptr)
            {
                spdlog::warn("RECIEVE {}", basePayload.ShortDebugString());
                this->_onMovePieceCallback(basePayload.move_payload());
            }
        }
        else if (basePayload.has_capture_payload())
        {
            if (this->_onCaptureCallback != nullptr)
            {
                spdlog::warn("RECIEVE {}", basePayload.ShortDebugString());
                this->_onCaptureCallback(basePayload.capture_payload());
            }
        }
        else if (basePayload.has_winlose_payload())
        {
            if (this->_onWinLoseCallback != nullptr)
            {
                this->showWinnerPopup(basePayload.notice());
                spdlog::info(basePayload.notice());
                this->_onWinLoseCallback(basePayload.notice());
            }
        }
        std::scoped_lock lg(this->mut);
        this->msgBuffer.clean();
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
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5, 0.5));
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

/**
 * Show winner/loser popup window.
 */
inline void WsClient::showWinnerPopup(std::string_view notice)
{
    // Always center this next dialog
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5, 0.5));
    ImGui::OpenPopup("GameOver", ImGuiPopupFlags_NoOpenOverExistingPopup);
    if (ImGui::BeginPopupModal("GameOver", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", notice.data());
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
