#include "WsClient.hpp"

namespace chk
{

chk::WsClient::WsClient()
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
    // Currently system CAs are not supported on non-Windows platforms with
    // mbedtls
    tlsOptions.caFile = "NONE";
#endif // _WIN32
    this->webSocketPtr->setTLSOptions(tlsOptions);
    // prefetch for public server list
    this->asyncFetchPublicServers();
}

/**
 * Show help tooltip with given message
 * @param tip the help message
 */
void chk::WsClient::showHint(const char *tip)
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
void chk::WsClient::showConnectWindow()
{
    static bool is_secure = false;  // switch for enable/disable SSL (PRIVATE servers only)
    static bool show_public = true; // whether to show public server list

    if (show_public)
    {
        this->showPublicServerWindow(show_public);
        return;
    }

    // =================== PRIVATE SERVERS ===============================
    ImGui::SetNextWindowSize(ImVec2{300.0f, 300.0f});
    static char inputUrl[256] = "127.0.0.1:9876/game";
    if (ImGui::Begin("Private Server", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        ImGui::InputText("Host or IP", inputUrl, IM_ARRAYSIZE(inputUrl), ImGuiInputTextFlags_CharsNoBlank);
        ImGui::SameLine();
        WsClient::showHint("eg: 127.0.0.1:8080 OR myserver.example.org");
        ImGui::Checkbox("Secure", &is_secure);
        if (!std::string_view(inputUrl).empty() && ImGui::Button("Connect", ImVec2{100.0f, 0}))
        {
            const char *prefix = is_secure ? "wss://" : "ws://";
            this->final_address = prefix + std::string{inputUrl};
            this->connClicked = true;
            memset(inputUrl, 0, sizeof(inputUrl));
        }
        if (ImGui::Button("< Go Back", ImVec2{100.0f, 0}))
        {
            show_public = true;
        }
        ImGui::End();
    }
}

/**
 * Show list of PUBLIC game servers, using imgui ListBox
 * @param showPublic switch for showing/hiding this window
 */
void WsClient::showPublicServerWindow(bool &showPublic)
{
    // =================== PUBLIC SERVERS ===============================
    ImGui::SetNextWindowSize(ImVec2{300.0f, 300.0f});
    if (ImGui::Begin("Public Servers", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        static size_t current_idx{0};
        if (ImGui::BeginListBox("Select One"))
        {
            for (size_t i = 0; i < publicServers.size(); ++i)
            {
                const bool selected = (i == current_idx);
                if (ImGui::Selectable(publicServers.at(i).name.c_str(), selected))
                {
                    current_idx = i;
                }

                // Set the initial focus
                if (selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
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
            this->asyncFetchPublicServers();
        }
        if (ImGui::Button("My Private Server >", ImVec2{150.0f, 0}))
        {
            showPublic = false;
        }
        ImGui::End();
    }
}

/**
 * Fetch updated public servers from CDN (Timeout 5000ms)
 * @see libcpr official docs: https://docs.libcpr.org/advanced-usage.html
 */
void WsClient::asyncFetchPublicServers()
{
    cpr::GetCallback([this](cpr::Response r) { this->parseServerList(r); }, cpr::Url{chk::cloudfront},
                     cpr::Timeout{5000});
}

/**
 * Parse the JSON response of server list then display them
 * @param response From the previous request
 */
void WsClient::parseServerList(const cpr::Response &response)
{
    if (response.status_code != 200)
    {
        std::scoped_lock lg{this->mut};
        this->deathNote = "httpRequest error: " + response.error.message;
        this->isDead = true;
        return;
    }

    // Parse the JSON response
    simdjson::dom::parser jsonParser;
    try
    {
        simdjson::dom::array jsonArray = jsonParser.parse(simdjson::padded_string_view(response.text));
        std::scoped_lock lg{this->mut};
        this->publicServers.clear();
        this->publicServers.reserve(jsonArray.size());
        for (const simdjson::dom::object &elem : jsonArray)
        {
            chk::ServerLocation location;
            location.name = elem.at_key("name").get_c_str();
            location.address = elem.at_key("address").get_c_str();
            this->publicServers.emplace_back(std::move_if_noexcept(location));
        }
    }
    catch (const simdjson::simdjson_error &ex)
    {
        this->deathNote = ex.what();
        this->isDead = true;
#ifndef NDEBUG
        spdlog::error(ex.what());
#endif // DEBUG
    }
}

/**
 * Reset all local states to FALSE or empty string
 */
void WsClient::resetAllStates()
{
    this->isConnected = false;
    this->connClicked = false;
    this->isDead = false;
    this->haveWinner = false;
    this->deathNote.clear();
}

/**
 * Run main loop of showing connection window, tryConnect, and handle exchanges
 */
void WsClient::runMainLoop()
{
    // clang-format off
    if (!isConnected) {
        if (!connClicked) {
            this->showConnectWindow();
        } else {
            this->tryConnect(final_address);
        }     
    }
    
    else {
        // already connected
        this->readIncomingPayloads();
    }

    // some error happened 🙁
    if (this->isDead) {
        if (this->_onDeathCallback != nullptr) {
            _onDeathCallback(deathNote);
        }
        this->showErrorPopup();
    } else if (this->haveWinner) {
        this->showWinnerPopup();
    }
    // clang-format on
}

/**
 * Try to connect to Server.
 * @param address server IP or URI
 */
void WsClient::tryConnect(std::string_view address)
{
    this->webSocketPtr->setUrl(address.data());
    if (!this->isConnected)
    {
        ImGui::SetNextWindowSize(ImVec2{400.0f, 100.0f});
        ImGui::Begin("Loading", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Connecting to online server");
        ImGui::End();
    }

    // Setup a callback to be fired when an Async event is received
    this->webSocketPtr->setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg) {
        if (msg->type == ix::WebSocketMessageType::Message)
        {
            std::scoped_lock<std::mutex> lg{this->mut};
            this->msgBuffer.addItem(std::move_if_noexcept(msg->str));
        }
        else if (msg->type == ix::WebSocketMessageType::Open)
        {
            spdlog::info("Connection established");
            this->isConnected = true;
        }
        else if (msg->type == ix::WebSocketMessageType::Close)
        {
            std::scoped_lock lg{this->mut};
            this->isDead = true;
            this->deathNote = "Error: Server closed the connection!" + msg->str;
            spdlog::error(this->deathNote);
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            std::scoped_lock lg{this->mut};
            this->isDead = true;
            this->deathNote = "Connection error: " + msg->errorInfo.reason;
            spdlog::error(this->deathNote);
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
void WsClient::setOnReadyConnectedCallback(const onConnectedServer &callback)
{
    this->_onReadyConnected = callback;
}

/**
 * Set the callback to handle starting game after signal from server.
 * @param callback the callback function
 */
void WsClient::setOnReadyStartGameCallback(const onReadyStartGame &callback)
{
    this->_onReadyStartGame = callback;
}

/**
 * Set the callback to handle connection failures or server kickouts.
 * @param callback the callback function
 */
void WsClient::setOnDeathCallback(const onDeathCallback &callback)
{
    this->_onDeathCallback = callback;
}

/**
 * Set the callback for handling Opponent moving their piece.
 * @param callback the callback function
 */
void WsClient::setOnMovePieceCallback(const onMovePieceCallback &callback)
{
    this->_onMovePieceCallback = callback;
}

/**
 * Set the callback for handling Opponent capturing my Piece.
 * @param callback the callback function
 */
void WsClient::setOnCapturePieceCallback(const onCaptureCallback &callback)
{
    this->_onCaptureCallback = callback;
}

/**
 * Set the callback for handling Winner or Loser of match.
 * @param callback the callback function
 */
void WsClient::setOnWinLoseCallback(const onWinLoseCallback &callback)
{
    this->_onWinLoseCallback = callback;
}

/**
 * Send Protobuf response back to server.
 *
 * @param payload the request body
 * @return TRUE if sent successfully, else FALSE
 */
bool WsClient::replyServer(const chk::payload::BasePayload &payload) const
{
    if (this->isDead || !this->isConnected)
    {
        return false;
    }
#ifndef NDEBUG
    spdlog::info("SENDING {}", payload.ShortDebugString());
#endif // DEBUG

    if (!payload.SerializeToString(&protoBucket))
    {
        spdlog::error("Protobuf serialization failed");
        return false;
    }
    const auto &result = this->webSocketPtr->sendBinary(this->protoBucket);
    return result.success;
}

/**
 * Read messages from server and update the game accordingly. If any
 * error happens or match ends, close connection
 */
void WsClient::readIncomingPayloads()
{
    for (const auto &msg : this->msgBuffer.getAll())
    {
        if (msg.empty())
        {
            continue;
        }
        chk::payload::BasePayload basePayload;
        if (!basePayload.ParseFromString(msg))
        {
            this->isDead = true;
            std::scoped_lock lg{this->mut};
            this->deathNote = "Profobuf: Could not parse payload";
            return;
        }

       // =================== begin switch block =====================
        switch (basePayload.inner_case())
        {
        case chk::payload::BasePayload::kWelcome:
            if (this->_onReadyConnected != nullptr)
            {
                this->_onReadyConnected(basePayload.welcome(), basePayload.notice());
            }
            break;

        case chk::payload::BasePayload::kStart:
            if (this->_onReadyStartGame != nullptr)
            {
                this->_onReadyStartGame(basePayload.start(), basePayload.notice());
            }
            break;

        case chk::payload::BasePayload::kExitPayload: {
            this->isDead = true;
            std::scoped_lock lg{this->mut};
            this->deathNote = basePayload.notice();
            spdlog::error(basePayload.notice());
            break;
        }

        case chk::payload::BasePayload::kMovePayload: {
            if (this->_onMovePieceCallback != nullptr)
            {
#ifndef NDEBUG
                spdlog::warn("RECIEVE {}", basePayload.ShortDebugString());
#endif // DEBUG
                this->_onMovePieceCallback(basePayload.move_payload());
            }
            break;
        }

        case chk::payload::BasePayload::kCapturePayload: {
            if (this->_onCaptureCallback != nullptr)
            {
#ifndef NDEBUG
                spdlog::warn("RECIEVE {}", basePayload.ShortDebugString());
#endif // DEBUG
                this->_onCaptureCallback(basePayload.capture_payload());
            }
            break;
        }

        case chk::payload::BasePayload::kWinlosePayload: {
            if (this->_onWinLoseCallback != nullptr)
            {
                this->_onWinLoseCallback(basePayload.notice());
                std::scoped_lock lg{this->mut};
                this->deathNote = basePayload.notice();
                this->haveWinner = true;
            }
            break;
        }

        default:
            break;
        }
    }
    std::scoped_lock lg{this->mut};
    this->msgBuffer.clean();
}

/**
 * Show error message from websockets as a popup window
 */
void WsClient::showErrorPopup()
{
    if (this->deathNote.empty())
    {
        return;
    }
    // Always center this next dialog
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2{0.5f, 0.5f});
    ImGui::OpenPopup("Error", ImGuiPopupFlags_NoOpenOverExistingPopup);
    if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", this->deathNote.c_str());
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2{120.0f, 0}))
        {
            ImGui::CloseCurrentPopup();
            this->resetAllStates();
            this->webSocketPtr->stop();
        }
        ImGui::EndPopup();
    }
}

/**
 * Show winner/loser popup window.
 */
void chk::WsClient::showWinnerPopup()
{
    // Always center this next dialog
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2{0.5f, 0.5f});
    ImGui::OpenPopup("GameOver", ImGuiPopupFlags_NoOpenOverExistingPopup);
    if (ImGui::BeginPopupModal("GameOver", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s", this->deathNote.c_str());
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2{120.0f, 0}))
        {
            ImGui::CloseCurrentPopup();
            this->resetAllStates();
            this->webSocketPtr->stop();
        }
        ImGui::EndPopup();
    }
}
} // namespace chk
