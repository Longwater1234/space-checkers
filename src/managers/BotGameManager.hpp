#pragma once

#include "../StartMenu.hpp" // chk::FONT_PATH
#include "../bot/BotPersona.hpp"
#include "../bot/CheckersEngine.hpp"
#include "../utils/ResourcePath.hpp"
#include "LocalGameManager.hpp"

#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <future>
#include <string>

namespace chk
{

/**
 * Offline play against "Ibox Offline".
 *
 * The human is RED (bottom, moves first); the bot is BLACK. Everything about the
 * rules stays in GameManager — this class only decides *which* move BLACK plays
 * and then feeds it through the same handleMovePiece/handleCapturePiece path a
 * human click would take, so animation, crowning and turn handling are identical.
 *
 * Design notes:
 *  - The search runs on a worker thread. A 1.1 s search on the render thread
 *    would drop ~66 frames; instead the UI keeps animating and shows a thinking
 *    indicator while it waits.
 *  - Multi-jumps are executed ONE hop per tick, and after each hop the bot
 *    re-reads the actual board rather than trusting a cached plan. That keeps it
 *    self-correcting: whatever GameManager thinks the position is, the bot agrees.
 *
 * @since 2026-09
 */
class BotGameManager final : public chk::LocalGameManager
{
  public:
    explicit BotGameManager(sf::RenderWindow *windowPtr, chk::bot::Strength strength = chk::bot::Strength::IBOX);
    BotGameManager() = delete;

    void drawBoard() override;
    void handleEvents(chk::CircularBuffer<int> &buffer) override;

    /// How hard Ibox is trying. Takes effect from the next move onward.
    void setStrength(chk::bot::Strength s);
    [[nodiscard]] chk::bot::Strength getStrength() const;

    /**
     * Advance the bot's own state machine by one tick. Called every frame from
     * handleEvents(); kept separate from input polling so it can be driven
     * without a window (tests, replays).
     */
    void updateBot();

    /**
     * Play a RED (human) move by cell index, through the same rule path a pair
     * of mouse clicks would take. For a multi-jump, call once per hop.
     *
     * @return TRUE if the board actually changed
     */
    bool playHumanMove(int fromCell, int toCell);

    /// TRUE while it is the human's (RED's) move.
    [[nodiscard]] bool isHumanTurn() const;
    /// TRUE once the match has finished.
    [[nodiscard]] bool isMatchOver() const;
    [[nodiscard]] size_t humanPieceCount() const;
    [[nodiscard]] size_t botPieceCount() const;
    /// Override the search budget (milliseconds and depth cap).
    void setThinkLimits(const chk::bot::SearchLimits &limits);
    /**
     * Floor on how long a bot move takes, in milliseconds. Purely cosmetic: it
     * stops forced replies from snapping out instantly. Set 0 to disable.
     */
    void setMinMoveDelayMs(int ms);
    /// Current board as an engine position, with `toMove` on move.
    [[nodiscard]] chk::bot::Position snapshotPosition(chk::bot::Side toMove) const;
    /// The line Ibox is currently saying, or empty if it has nothing to add.
    [[nodiscard]] const std::string &currentQuip() const;

  private:
    enum class BotState : uint8_t
    {
        WAITING_FOR_HUMAN,
        THINKING,
        COOLDOWN // brief pause between hops of a multi-jump, so it reads well
    };

    // --- engine / persona ---
    chk::bot::Engine m_engine{24};
    chk::bot::BotPersona m_persona{};
    chk::bot::Strength m_strength;

    // --- turn tracking ---
    BotState m_state{BotState::WAITING_FOR_HUMAN};
    bool m_wasRedTurn{true};
    bool m_inChain{false}; // mid multi-jump, so don't re-comment
    bool m_greeted{false};
    bool m_resultAnnounced{false};
    int m_failedAttempts{0};

    std::future<chk::bot::SearchResult> m_pending;
    sf::Clock m_stateClock; // time in the current bot state
    sf::Clock m_humanClock; // how long the human has been thinking
    sf::Clock m_quipClock;  // how long the current quip has been on screen
    int m_cooldownMs{0};
    int m_minMoveDelayMs{420}; // cosmetic floor, so replies never feel twitchy

    // --- prediction / blunder detection ---
    bool m_hasExpectation{false};
    int m_expectedScore{0};            // score after our move, BLACK's view
    chk::bot::Move m_predictedReply{}; // the human reply we planned around
    int m_lastScoreCp{0};

    // snapshot of the board as it stood before the human moved
    chk::bot::Position m_beforeHuman{};
    bool m_haveBeforeHuman{false};

    // --- presentation ---
    sf::Font m_font;
    bool m_fontOk{false};
    sf::Text m_nameplate;
    sf::Text m_quipText;
    std::string m_quip;
    std::string m_thinkingLabel;

    // --- helpers ---
    [[nodiscard]] chk::bot::Position readBoard(chk::bot::Side toMove) const;
    [[nodiscard]] const chk::Block *findCell(int cellIndex) const;
    void beginThinking();
    void collectSearchResult();
    bool executeOneHop(const chk::bot::Move &mv);
    void onHumanTurnStarted();
    void onBotTurnStarted();
    void speak(const chk::bot::TurnReport &report);
    void checkForStalemate();
    void setupText();
};

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

inline BotGameManager::BotGameManager(sf::RenderWindow *windowPtr, chk::bot::Strength strength)
    : LocalGameManager(windowPtr), m_strength(strength)
{
    m_engine.setStrength(strength);
    m_engine.newGame();
    setupText();
}

inline void BotGameManager::setStrength(chk::bot::Strength s)
{
    m_strength = s;
    m_engine.setStrength(s);
}

inline chk::bot::Strength BotGameManager::getStrength() const
{
    return m_strength;
}

inline bool BotGameManager::isHumanTurn() const
{
    return this->isPlayerRedTurn();
}

inline bool BotGameManager::isMatchOver() const
{
    return this->isGameOver();
}

inline size_t BotGameManager::humanPieceCount() const
{
    return this->playerRed->getPieceCount();
}

inline size_t BotGameManager::botPieceCount() const
{
    return this->playerBlack->getPieceCount();
}

inline void BotGameManager::setThinkLimits(const chk::bot::SearchLimits &limits)
{
    m_engine.setLimits(limits);
}

inline void BotGameManager::setMinMoveDelayMs(int ms)
{
    m_minMoveDelayMs = ms < 0 ? 0 : ms;
}

inline chk::bot::Position BotGameManager::snapshotPosition(chk::bot::Side toMove) const
{
    return readBoard(toMove);
}

inline const std::string &BotGameManager::currentQuip() const
{
    return m_quip;
}

inline void BotGameManager::setupText()
{
    m_fontOk = m_font.loadFromFile(chk::getResourcePath(chk::FONT_PATH));
    if (!m_fontOk)
    {
        spdlog::warn("BotGameManager: could not load font, commentary will be hidden");
        return;
    }
    m_nameplate.setFont(m_font);
    m_nameplate.setCharacterSize(14);
    m_nameplate.setFillColor(sf::Color{150, 190, 255});
    m_nameplate.setPosition(sf::Vector2f{10.0f, 8.05f * chk::SIZE_CELL});

    m_quipText.setFont(m_font);
    m_quipText.setCharacterSize(15);
    m_quipText.setFillColor(sf::Color{235, 220, 190});
    m_quipText.setPosition(sf::Vector2f{10.0f, 8.95f * chk::SIZE_CELL});
}

// ---------------------------------------------------------------------------
// Board <-> engine conversion
// ---------------------------------------------------------------------------

/**
 * Snapshot the live board into an engine position.
 *
 * @param toMove which side is on move in the resulting position
 */
inline chk::bot::Position BotGameManager::readBoard(chk::bot::Side toMove) const
{
    chk::bot::Position pos;
    pos.clear();
    pos.stm = toMove;

    for (const auto &cell : this->getBlockList())
    {
        const int idx = cell->getIndex();
        if (idx < 1 || idx > chk::bot::NUM_SQ)
        {
            continue;
        }
        const int pieceId = this->getPieceFromCell(idx);
        if (pieceId == -1)
        {
            continue;
        }
        if (this->playerRed->hasThisPiece(pieceId))
        {
            const bool king = this->playerRed->getOwnPieces().at(pieceId)->getIsKing();
            pos.sq[idx] = king ? chk::bot::RED_KING : chk::bot::RED_MAN;
        }
        else if (this->playerBlack->hasThisPiece(pieceId))
        {
            const bool king = this->playerBlack->getOwnPieces().at(pieceId)->getIsKing();
            pos.sq[idx] = king ? chk::bot::BLACK_KING : chk::bot::BLACK_MAN;
        }
    }
    pos.rehash();
    return pos;
}

/// Look up a board cell by its 1..32 index.
inline const chk::Block *BotGameManager::findCell(int cellIndex) const
{
    for (const auto &cell : this->getBlockList())
    {
        if (cell->getIndex() == cellIndex)
        {
            return &cell;
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Turn transitions
// ---------------------------------------------------------------------------

/// Called the moment control passes back to the human.
inline void BotGameManager::onHumanTurnStarted()
{
    m_humanClock.restart();
    m_beforeHuman = readBoard(chk::bot::Side::RED);
    m_haveBeforeHuman = true;
    m_inChain = false;
}

/// Called the moment control passes to the bot (not for mid-chain continuations).
inline void BotGameManager::onBotTurnStarted()
{
    m_inChain = false;
}

// ---------------------------------------------------------------------------
// Thinking
// ---------------------------------------------------------------------------

inline void BotGameManager::beginThinking()
{
    const chk::bot::Position pos = readBoard(chk::bot::Side::BLACK);

    chk::bot::MoveList probe;
    if (chk::bot::generateMoves(pos, probe) == 0)
    {
        checkForStalemate();
        return;
    }

    m_pending = std::async(std::launch::async, [this, pos]() { return m_engine.search(pos); });
    m_state = BotState::THINKING;
    m_stateClock.restart();
    m_thinkingLabel = "thinking";
}

/**
 * Pick up the finished search, play one hop of it, and (on the first hop of the
 * turn) work out what Ibox should say about the position.
 */
inline void BotGameManager::collectSearchResult()
{
    if (!m_pending.valid())
    {
        m_state = BotState::WAITING_FOR_HUMAN;
        return;
    }
    if (m_pending.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
    {
        return; // still working — keep rendering
    }

    // A little floor on the delay so instant replies don't feel twitchy.
    if (m_stateClock.getElapsedTime().asMilliseconds() < m_minMoveDelayMs)
    {
        return;
    }

    const chk::bot::SearchResult result = m_pending.get();
    if (result.best.isNull())
    {
        checkForStalemate();
        m_state = BotState::WAITING_FOR_HUMAN;
        return;
    }

    const bool firstHopOfTurn = !m_inChain;

    // ---- assess the human's last move before we reply to it ----------------
    chk::bot::TurnReport report;
    report.scoreCp = result.score;
    report.piecesLeft = chk::bot::pieceCount(readBoard(chk::bot::Side::BLACK));
    report.humanThinkMs = m_humanClock.getElapsedTime().asMilliseconds();

    if (firstHopOfTurn && m_hasExpectation)
    {
        // Both scores are from BLACK's point of view with best play assumed, so
        // the difference is what the human's choice actually cost them.
        report.humanScoreSwing = result.score - m_expectedScore;
    }
    if (firstHopOfTurn && m_haveBeforeHuman && !m_predictedReply.isNull())
    {
        // Did they play the move we planned around? Compare the board we saw
        // before their move with the board now.
        const chk::bot::Position now = readBoard(chk::bot::Side::BLACK);
        chk::bot::Position predicted = m_beforeHuman;
        chk::bot::MoveList legal;
        const int n = chk::bot::generateMoves(predicted, legal);
        for (int i = 0; i < n; ++i)
        {
            if (legal[i] == m_predictedReply)
            {
                chk::bot::makeMove(predicted, legal[i]);
                report.humanPlayedPredicted = (predicted.key == now.key);
                break;
            }
        }
        // Did the human crown anything?
        const int kingsBefore = m_beforeHuman.count(chk::bot::RED_KING);
        report.humanPromoted = now.count(chk::bot::RED_KING) > kingsBefore;
    }

    report.botCaptures = result.best.nCaptures;
    report.botPromoted = result.best.promotes;
    report.gameJustStarted = !m_greeted;

    // ---- play the move -----------------------------------------------------
    const bool ok = executeOneHop(result.best);
    if (!ok)
    {
        if (++m_failedAttempts >= 3)
        {
            spdlog::error("BotGameManager: could not play {} — conceding the turn",
                          chk::bot::moveToString(result.best));
            this->updateMessage("Ibox Offline forfeits its turn (internal error)");
            m_failedAttempts = 0;
            m_state = BotState::WAITING_FOR_HUMAN;
            return;
        }
        m_state = BotState::WAITING_FOR_HUMAN;
        return;
    }
    m_failedAttempts = 0;

    // ---- remember what we expect next --------------------------------------
    m_lastScoreCp = result.score;
    m_expectedScore = result.score;
    m_hasExpectation = true;
    m_predictedReply = (result.pv.size() >= 2) ? result.pv[1] : chk::bot::Move{};

    // If the turn did not flip, we are mid-chain and must hop again.
    if (!this->isPlayerRedTurn() && !this->isGameOver())
    {
        m_inChain = true;
        m_state = BotState::COOLDOWN;
        m_cooldownMs = (m_minMoveDelayMs > 0) ? 260 : 0;
        m_stateClock.restart();
    }
    else
    {
        m_state = BotState::WAITING_FOR_HUMAN;
        report.humanMustCapture = !this->getForcedMoves().empty();
    }

    if (firstHopOfTurn)
    {
        speak(report);
    }
}

/**
 * Play a single step of `mv` on the real board, through the ordinary game paths.
 *
 * For captures we install exactly the one CaptureTarget we intend into
 * forcedMoves, so GameManager::handleCapturePiece plays our jump and not some
 * other one it happened to find first.
 *
 * @return TRUE if the board actually changed
 */
inline bool BotGameManager::executeOneHop(const chk::bot::Move &mv)
{
    const int fromIdx = mv.from;
    const int pieceId = this->getPieceFromCell(fromIdx);
    if (pieceId == -1 || !this->playerBlack->hasThisPiece(pieceId))
    {
        spdlog::error("BotGameManager: no BLACK piece on cell {}", fromIdx);
        return false;
    }

    if (!mv.isCapture())
    {
        const chk::Block *dest = findCell(mv.to);
        if (dest == nullptr)
        {
            return false;
        }
        this->setSourceCell(fromIdx);
        this->handleMovePiece(this->playerBlack, this->playerRed, *dest, pieceId);
        this->updateMatchStatus(this->playerBlack, this->playerRed);
        return true;
    }

    // --- one hop of a (possibly longer) jump chain --------------------------
    const int overIdx = mv.captured[0];
    const int landIdx = mv.path[0];
    const chk::Block *landCell = findCell(landIdx);
    if (landCell == nullptr)
    {
        return false;
    }
    const int preyId = this->getPieceFromCell(overIdx);
    if (preyId == -1 || !this->playerRed->hasThisPiece(preyId))
    {
        spdlog::error("BotGameManager: expected a RED piece on cell {}", overIdx);
        return false;
    }

    chk::CaptureTarget target;
    target.preyPieceId = preyId;
    target.preyCellIdx = overIdx;
    target.hunterNextCell = landIdx;

    this->forcedMoves.clear();
    this->forcedMoves.emplace(pieceId, target);
    this->setSourceCell(fromIdx);
    this->handleCapturePiece(this->playerBlack, this->playerRed, *landCell);
    this->updateMatchStatus(this->playerBlack, this->playerRed);
    return true;
}

// ---------------------------------------------------------------------------
// End conditions
// ---------------------------------------------------------------------------

/**
 * In draughts, a player with no legal move loses. The base manager only ends the
 * game when a side runs out of pieces, so handle the blocked case here.
 */
inline void BotGameManager::checkForStalemate()
{
    if (this->isGameOver())
    {
        return;
    }
    const chk::bot::Side toMove = this->isPlayerRedTurn() ? chk::bot::Side::RED : chk::bot::Side::BLACK;
    const chk::bot::Position pos = readBoard(toMove);
    chk::bot::MoveList moves;
    if (chk::bot::generateMoves(pos, moves) != 0)
    {
        return;
    }
    this->setGameOver(true);
    if (toMove == chk::bot::Side::RED)
    {
        this->updateMessage("GAME OVER! You have no legal moves. Ibox Offline wins.");
    }
    else
    {
        this->updateMessage("GAME OVER! Ibox Offline has no legal moves. You win!");
    }
}

// ---------------------------------------------------------------------------
// Commentary
// ---------------------------------------------------------------------------

inline void BotGameManager::speak(const chk::bot::TurnReport &report)
{
    chk::bot::TurnReport r = report;
    if (this->isGameOver())
    {
        r.gameOver = true;
        const size_t redLeft = this->playerRed->getPieceCount();
        const size_t blackLeft = this->playerBlack->getPieceCount();
        if (redLeft == 0)
        {
            r.botWon = true; // human wiped out
        }
        else if (blackLeft == 0)
        {
            r.botWon = false; // bot wiped out
        }
        else
        {
            // Ended by stalemate: whoever is on move is the one with no moves.
            r.botWon = this->isPlayerRedTurn();
        }
        m_resultAnnounced = true;
    }

    const chk::bot::Mood mood = chk::bot::BotPersona::decide(r);
    if (mood == chk::bot::Mood::GREETING)
    {
        m_greeted = true;
    }
    if (!m_persona.shouldSpeak(mood))
    {
        return;
    }
    m_quip = m_persona.speak(mood);
    m_quipClock.restart();
}

// ---------------------------------------------------------------------------
// Main loop hooks
// ---------------------------------------------------------------------------

inline void BotGameManager::handleEvents(chk::CircularBuffer<int> &buffer)
{
    const bool humanTurn = this->isPlayerRedTurn();

    for (auto event = sf::Event{}; window->pollEvent(event);)
    {
        if (event.type == sf::Event::Closed)
        {
            window->close();
        }
        if (event.type != sf::Event::MouseButtonPressed || !sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            continue;
        }
        // Clicks are ignored while Ibox is on move — you cannot touch its pieces.
        if (!humanTurn || this->isGameOver())
        {
            continue;
        }
        const auto clickedPos = sf::Mouse::getPosition(*window);
        if (clickedPos.y > chk::SIZE_CELL * 8)
        {
            continue;
        }
        for (auto &cell : this->getBlockList())
        {
            if (cell->containsPoint(clickedPos) && cell->getIndex() != -1)
            {
                GameManager::handleCellTap(this->playerRed, this->playerBlack, buffer, cell);
                break;
            }
        }
    }

    updateBot();
}

/**
 * The bot's per-frame state machine: notice turn handovers, kick off a search,
 * collect it when ready, and pace multi-jumps.
 */
inline void BotGameManager::updateBot()
{
    if (this->isGameOver())
    {
        if (!m_resultAnnounced)
        {
            chk::bot::TurnReport r;
            r.gameOver = true;
            r.scoreCp = m_lastScoreCp;
            speak(r);
        }
        return;
    }

    // --- detect turn handover ----------------------------------------------
    const bool redTurnNow = this->isPlayerRedTurn();
    if (redTurnNow != m_wasRedTurn)
    {
        if (redTurnNow)
        {
            onHumanTurnStarted();
        }
        else
        {
            onBotTurnStarted();
        }
        m_wasRedTurn = redTurnNow;
    }

    if (redTurnNow)
    {
        // The human may have just been blocked out of every move.
        checkForStalemate();
        return;
    }

    // --- bot's turn ---------------------------------------------------------
    switch (m_state)
    {
    case BotState::WAITING_FOR_HUMAN:
        beginThinking();
        break;
    case BotState::THINKING:
        collectSearchResult();
        break;
    case BotState::COOLDOWN:
        if (m_stateClock.getElapsedTime().asMilliseconds() >= m_cooldownMs)
        {
            beginThinking();
        }
        break;
    }
}

inline bool BotGameManager::playHumanMove(int fromCell, int toCell)
{
    if (this->isGameOver() || !this->isPlayerRedTurn())
    {
        return false;
    }
    const chk::Block *src = findCell(fromCell);
    const chk::Block *dst = findCell(toCell);
    if (src == nullptr || dst == nullptr)
    {
        return false;
    }
    // Two taps: select the piece, then the destination — exactly what a click
    // pair does, so all the forced-capture rules still apply.
    chk::CircularBuffer<int> buffer{1};
    GameManager::handleCellTap(this->playerRed, this->playerBlack, buffer, *src);
    GameManager::handleCellTap(this->playerRed, this->playerBlack, buffer, *dst);
    return true;
}

inline void BotGameManager::drawBoard()
{
    LocalGameManager::drawBoard();

    if (!m_fontOk)
    {
        return;
    }

    // --- nameplate ----------------------------------------------------------
    std::string plate = std::string(chk::bot::BOT_NAME) + "  (" + chk::bot::BOT_TAGLINE + ")";
    if (m_state == BotState::THINKING || m_state == BotState::COOLDOWN)
    {
        // simple animated ellipsis so the wait reads as deliberate, not frozen
        const int dots = (m_stateClock.getElapsedTime().asMilliseconds() / 320) % 4;
        plate += "  -  thinking";
        plate.append(static_cast<size_t>(dots), '.');
    }
    else if (!this->isGameOver())
    {
        plate += "  -  your move";
    }
    m_nameplate.setString(plate);
    window->draw(m_nameplate);

    // --- current quip, fading out after a few seconds ------------------------
    if (!m_quip.empty())
    {
        constexpr float HOLD_S = 6.0f;
        const float age = m_quipClock.getElapsedTime().asSeconds();
        if (age > HOLD_S)
        {
            m_quip.clear();
        }
        else
        {
            const float fade = (age > HOLD_S - 1.0f) ? (HOLD_S - age) : 1.0f;
            auto colour = sf::Color{235, 220, 190};
            colour.a = static_cast<sf::Uint8>(255.0f * std::max(0.0f, std::min(1.0f, fade)));
            m_quipText.setFillColor(colour);
            m_quipText.setString("\"" + m_quip + "\"");
            window->draw(m_quipText);
        }
    }
}

} // namespace chk
