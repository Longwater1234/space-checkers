#pragma once
#include "CircularBuffer.hpp"
#include "ServerLocation.hpp"
#include "payloads/base_payload.pb.hpp"
#include <atomic>
#include <cpr/cpr.h>
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
// CDN address
constexpr auto cloudfront = "https://d1txhef4jwuosv.cloudfront.net/ws_server_locations.json";

/**
 * This handles all websocket exchanges with Server
 */
class WsClient final
{
  public:
    WsClient();
    WsClient(const WsClient &) = delete;
    WsClient &operator=(const WsClient &) = delete;
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
    std::atomic_bool haveWinner{false};             // whether server returned Winner or Loser
    std::atomic_bool isConnected{false};            // if done connected to server (else, show loading)
    chk::CircularBuffer<std::string> msgBuffer{1};  // keep only recent 1 incoming message
    mutable std::string deathNote;                  // reason from server for disconnecting
    mutable std::string protoBucket;                // REUSABLE container to store OUTGOING protobuf
    std::atomic_bool connClicked = false;           // if 'connect' button clicked
    std::vector<chk::ServerLocation> publicServers; // list of public servers (fetched from CDN)

    /* callbacks for different events from server */
    onConnectedServer _onReadyConnected;
    onReadyStartGame _onReadyStartGame;
    onDeathCallback _onDeathCallback;
    onMovePieceCallback _onMovePieceCallback;
    onCaptureCallback _onCaptureCallback;
    onWinLoseCallback _onWinLoseCallback;

    std::mutex mut;
    std::unique_ptr<ix::WebSocket> webSocketPtr = nullptr; // our Websocket object
    void showErrorPopup();                                 // whenver there is an error (from server)
    void runServerLoop();                                  // while connected, keep exchanging messages with server
    static void showHint(const char *tip);
    void tryConnect(std::string_view address);
    void showConnectWindow();
    void showWinnerPopup();
    void showPublicServerWindow(bool &showPublic);
    void asyncFetchPublicServers();
    void parseServerList(const cpr::Response &response);
    void resetAllStates();
};

} // namespace chk
