//
// CheckersEngine.hpp — a self-contained English-draughts engine.
//
// Deliberately has ZERO dependency on SFML, imGui or anything else in this
// project, so it can be unit-tested (and perft-verified) on its own.
// The GUI bridge lives in managers/BotGameManager.hpp.
//
// Board numbering matches GameManager::drawCheckerboard(): dark cells are
// numbered 32 down to 1, scanning row 0..7 top-to-bottom, col 0..7 left-to-right.
//
//   index = 32 - (4 * row + j),  where j is the 0-based dark-cell slot in that row
//   col   = (row even) ? 2j + 1 : 2j
//
// So RED starts on 1..12 (rows 5-7) and promotes on row 0,
//    BLACK starts on 21..32 (rows 0-2) and promotes on row 7.
//
// Created for the "Ibox Offline" opponent.
//
#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace chk::bot
{

// ---------------------------------------------------------------------------
// Basic types
// ---------------------------------------------------------------------------

inline constexpr int NUM_SQ = 32;    // playable (dark) squares
inline constexpr int MAX_MOVES = 48; // generous upper bound for one position
inline constexpr int MAX_CAPTURES = 12;
inline constexpr int MAX_PLY = 64;
inline constexpr int MATE_SCORE = 30000;
inline constexpr int INF_SCORE = 32000;

/// Contents of a square. Index 0 of a Position is unused so squares read 1..32.
enum Sq : uint8_t
{
    EMPTY = 0,
    RED_MAN = 1,
    RED_KING = 2,
    BLACK_MAN = 3,
    BLACK_KING = 4
};

enum class Side : uint8_t
{
    RED = 0,
    BLACK = 1
};

inline bool isRed(uint8_t s)
{
    return s == RED_MAN || s == RED_KING;
}
inline bool isBlack(uint8_t s)
{
    return s == BLACK_MAN || s == BLACK_KING;
}
inline bool isKing(uint8_t s)
{
    return s == RED_KING || s == BLACK_KING;
}
inline bool isOwnedBy(uint8_t s, Side side)
{
    return side == Side::RED ? isRed(s) : isBlack(s);
}

// Diagonal directions. 0/1 head toward row 0 (RED's forward), 2/3 toward row 7.
inline constexpr int DIR_UP_LEFT = 0;
inline constexpr int DIR_UP_RIGHT = 1;
inline constexpr int DIR_DOWN_LEFT = 2;
inline constexpr int DIR_DOWN_RIGHT = 3;

// ---------------------------------------------------------------------------
// Precomputed geometry + Zobrist keys
// ---------------------------------------------------------------------------

/**
 * Static lookup tables. Built once at static-init time.
 */
struct Tables
{
    int8_t row[NUM_SQ + 1]{};
    int8_t col[NUM_SQ + 1]{};
    /// step[sq][dir] = adjacent square in that direction, or 0 if off-board
    int8_t step[NUM_SQ + 1][4]{};
    /// jump[sq][dir] = square two diagonal steps away, or 0 if off-board
    int8_t jump[NUM_SQ + 1][4]{};
    uint64_t zPiece[5][NUM_SQ + 1]{};
    uint64_t zSide{};

    Tables()
    {
        // Map every playable index to its (row, col)
        for (int idx = 1; idx <= NUM_SQ; ++idx)
        {
            const int m = 32 - idx;
            const int r = m / 4;
            const int j = m % 4;
            row[idx] = static_cast<int8_t>(r);
            col[idx] = static_cast<int8_t>((r % 2 == 0) ? (2 * j + 1) : (2 * j));
        }

        // Reverse lookup so we can walk diagonals in (row, col) space
        int8_t byRC[8][8];
        std::memset(byRC, 0, sizeof(byRC));
        for (int idx = 1; idx <= NUM_SQ; ++idx)
        {
            byRC[row[idx]][col[idx]] = static_cast<int8_t>(idx);
        }

        constexpr int dr[4] = {-1, -1, +1, +1};
        constexpr int dc[4] = {-1, +1, -1, +1};

        for (int idx = 1; idx <= NUM_SQ; ++idx)
        {
            for (int d = 0; d < 4; ++d)
            {
                const int r1 = row[idx] + dr[d];
                const int c1 = col[idx] + dc[d];
                step[idx][d] = (r1 >= 0 && r1 < 8 && c1 >= 0 && c1 < 8) ? byRC[r1][c1] : 0;

                const int r2 = row[idx] + 2 * dr[d];
                const int c2 = col[idx] + 2 * dc[d];
                jump[idx][d] = (r2 >= 0 && r2 < 8 && c2 >= 0 && c2 < 8) ? byRC[r2][c2] : 0;
            }
        }

        // splitmix64 — deterministic, so replays and the transposition table are stable
        uint64_t s = 0x9E3779B97F4A7C15ULL;
        auto next = [&s]() {
            s += 0x9E3779B97F4A7C15ULL;
            uint64_t z = s;
            z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
            z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
            return z ^ (z >> 31);
        };
        for (int p = 0; p < 5; ++p)
        {
            for (int i = 0; i <= NUM_SQ; ++i)
            {
                zPiece[p][i] = next();
            }
        }
        zSide = next();
    }
};

inline const Tables TBL{};

/// Promotion row for a side: RED crowns on row 0, BLACK crowns on row 7.
inline bool isPromotionSquare(int sq, Side side)
{
    return side == Side::RED ? TBL.row[sq] == 0 : TBL.row[sq] == 7;
}

// ---------------------------------------------------------------------------
// Move
// ---------------------------------------------------------------------------

/**
 * A complete move. A multi-jump chain is ONE Move; `path` holds each successive
 * landing square so the GUI can animate the hops one at a time.
 */
struct Move
{
    uint8_t from{0};
    uint8_t to{0};
    uint8_t nCaptures{0};
    bool promotes{false};
    /// squares of the captured pieces, in the order they are jumped
    std::array<uint8_t, MAX_CAPTURES> captured{};
    /// landing square after each hop; path[nCaptures-1] == to
    std::array<uint8_t, MAX_CAPTURES> path{};

    [[nodiscard]] bool isCapture() const
    {
        return nCaptures > 0;
    }
    bool operator==(const Move &o) const
    {
        if (from != o.from || to != o.to || nCaptures != o.nCaptures)
        {
            return false;
        }
        for (int i = 0; i < nCaptures; ++i)
        {
            if (captured[i] != o.captured[i])
            {
                return false;
            }
        }
        return true;
    }
    [[nodiscard]] bool isNull() const
    {
        return from == 0 && to == 0;
    }
};

using MoveList = std::array<Move, MAX_MOVES>;

// ---------------------------------------------------------------------------
// Position
// ---------------------------------------------------------------------------

/**
 * Full game state. Small enough (about 48 bytes) that copy-make beats
 * make/unmake bookkeeping.
 */
struct Position
{
    std::array<uint8_t, NUM_SQ + 1> sq{}; // sq[0] unused
    Side stm{Side::RED};                  // side to move
    uint16_t quietPlies{0};               // plies since last capture or man move (draw counter)
    uint64_t key{0};

    void clear()
    {
        sq.fill(EMPTY);
        stm = Side::RED;
        quietPlies = 0;
        key = 0;
    }

    void rehash()
    {
        key = 0;
        for (int i = 1; i <= NUM_SQ; ++i)
        {
            if (sq[i] != EMPTY)
            {
                key ^= TBL.zPiece[sq[i]][i];
            }
        }
        if (stm == Side::BLACK)
        {
            key ^= TBL.zSide;
        }
    }

    /// Standard opening setup, RED to move (matches LocalGameManager).
    void setStartPosition()
    {
        clear();
        for (int i = 1; i <= 12; ++i)
        {
            sq[i] = RED_MAN;
        }
        for (int i = 21; i <= 32; ++i)
        {
            sq[i] = BLACK_MAN;
        }
        stm = Side::RED;
        rehash();
    }

    [[nodiscard]] int count(uint8_t what) const
    {
        int n = 0;
        for (int i = 1; i <= NUM_SQ; ++i)
        {
            if (sq[i] == what)
            {
                ++n;
            }
        }
        return n;
    }
};

// ---------------------------------------------------------------------------
// Move generation
// ---------------------------------------------------------------------------

namespace detail
{

/// Directions a piece may travel: kings get all four, men only their forward pair.
inline void directionsFor(uint8_t piece, int &d0, int &d1, int &d2, int &d3, int &n)
{
    if (isKing(piece))
    {
        d0 = 0;
        d1 = 1;
        d2 = 2;
        d3 = 3;
        n = 4;
    }
    else if (piece == RED_MAN)
    {
        d0 = DIR_UP_LEFT;
        d1 = DIR_UP_RIGHT;
        d2 = d3 = 0;
        n = 2;
    }
    else // BLACK_MAN
    {
        d0 = DIR_DOWN_LEFT;
        d1 = DIR_DOWN_RIGHT;
        d2 = d3 = 0;
        n = 2;
    }
}

/**
 * Depth-first expansion of a jump chain.
 *
 * `board` is a scratch copy in which already-jumped pieces have been lifted, so a
 * piece cannot be captured twice in the same chain and the vacated squares are
 * available as landing squares (both are standard).
 *
 * Crowning terminates the move: this mirrors GameManager::handleCapturePiece(),
 * which only looks for a continuation when the hunter's king status is unchanged.
 */
inline void expandJumps(std::array<uint8_t, NUM_SQ + 1> &board, Side side, uint8_t piece, int at, Move &partial,
                        MoveList &out, int &count)
{
    int d[4], n;
    directionsFor(piece, d[0], d[1], d[2], d[3], n);

    bool extended = false;
    for (int k = 0; k < n; ++k)
    {
        const int dir = d[k];
        const int over = TBL.step[at][dir];
        const int land = TBL.jump[at][dir];
        if (over == 0 || land == 0)
        {
            continue;
        }
        const uint8_t victim = board[over];
        if (victim == EMPTY || isOwnedBy(victim, side) || board[land] != EMPTY)
        {
            continue;
        }
        if (partial.nCaptures >= MAX_CAPTURES)
        {
            continue;
        }

        // Play the hop on the scratch board
        board[over] = EMPTY;
        board[at] = EMPTY;
        board[land] = piece;

        const int slot = partial.nCaptures;
        partial.captured[slot] = static_cast<uint8_t>(over);
        partial.path[slot] = static_cast<uint8_t>(land);
        partial.nCaptures = static_cast<uint8_t>(slot + 1);
        partial.to = static_cast<uint8_t>(land);

        const bool crowns = !isKing(piece) && isPromotionSquare(land, side);
        if (crowns)
        {
            // Reaching the back rank ends the move, even if more jumps exist.
            partial.promotes = true;
            if (count < MAX_MOVES)
            {
                out[count++] = partial;
            }
            partial.promotes = false;
            extended = true;
        }
        else
        {
            expandJumps(board, side, piece, land, partial, out, count);
            extended = true;
        }

        // Undo the hop
        board[land] = EMPTY;
        board[at] = piece;
        board[over] = victim;
        partial.nCaptures = static_cast<uint8_t>(slot);
        partial.to = static_cast<uint8_t>(at);
    }

    // No continuation from here: the chain so far is a complete move.
    if (!extended && partial.nCaptures > 0)
    {
        partial.to = static_cast<uint8_t>(at);
        if (count < MAX_MOVES)
        {
            out[count++] = partial;
        }
    }
}

} // namespace detail

/**
 * Generate every legal move. Captures are compulsory in English draughts, so if
 * any capture exists only captures are returned.
 *
 * @return number of moves written into `out`
 */
inline int generateMoves(const Position &pos, MoveList &out)
{
    int count = 0;
    const Side side = pos.stm;

    // --- captures first -----------------------------------------------------
    std::array<uint8_t, NUM_SQ + 1> scratch = pos.sq;
    for (int i = 1; i <= NUM_SQ; ++i)
    {
        const uint8_t piece = pos.sq[i];
        if (piece == EMPTY || !isOwnedBy(piece, side))
        {
            continue;
        }
        Move partial;
        partial.from = static_cast<uint8_t>(i);
        partial.to = static_cast<uint8_t>(i);
        detail::expandJumps(scratch, side, piece, i, partial, out, count);
    }
    if (count > 0)
    {
        return count;
    }

    // --- otherwise quiet steps ---------------------------------------------
    for (int i = 1; i <= NUM_SQ; ++i)
    {
        const uint8_t piece = pos.sq[i];
        if (piece == EMPTY || !isOwnedBy(piece, side))
        {
            continue;
        }
        int d[4], n;
        detail::directionsFor(piece, d[0], d[1], d[2], d[3], n);
        for (int k = 0; k < n; ++k)
        {
            const int to = TBL.step[i][d[k]];
            if (to == 0 || pos.sq[to] != EMPTY)
            {
                continue;
            }
            if (count >= MAX_MOVES)
            {
                break;
            }
            Move mv;
            mv.from = static_cast<uint8_t>(i);
            mv.to = static_cast<uint8_t>(to);
            mv.promotes = !isKing(piece) && isPromotionSquare(to, side);
            out[count++] = mv;
        }
    }
    return count;
}

/**
 * Apply a move, updating the Zobrist key and the draw counter incrementally.
 */
inline void makeMove(Position &pos, const Move &mv)
{
    const Side side = pos.stm;
    uint8_t piece = pos.sq[mv.from];

    pos.key ^= TBL.zPiece[piece][mv.from];
    pos.sq[mv.from] = EMPTY;

    for (int i = 0; i < mv.nCaptures; ++i)
    {
        const int victimSq = mv.captured[i];
        pos.key ^= TBL.zPiece[pos.sq[victimSq]][victimSq];
        pos.sq[victimSq] = EMPTY;
    }

    if (mv.promotes)
    {
        piece = (side == Side::RED) ? RED_KING : BLACK_KING;
    }
    pos.sq[mv.to] = piece;
    pos.key ^= TBL.zPiece[piece][mv.to];

    // A capture or a man move is irreversible; anything else ticks the draw clock.
    const bool wasMan = (mv.promotes || piece == RED_MAN || piece == BLACK_MAN);
    pos.quietPlies = (mv.isCapture() || wasMan) ? 0 : static_cast<uint16_t>(pos.quietPlies + 1);

    pos.stm = (side == Side::RED) ? Side::BLACK : Side::RED;
    pos.key ^= TBL.zSide;
}

/// True when the side to move has at least one capture available.
inline bool hasCapture(const Position &pos)
{
    const Side side = pos.stm;
    for (int i = 1; i <= NUM_SQ; ++i)
    {
        const uint8_t piece = pos.sq[i];
        if (piece == EMPTY || !isOwnedBy(piece, side))
        {
            continue;
        }
        int d[4], n;
        detail::directionsFor(piece, d[0], d[1], d[2], d[3], n);
        for (int k = 0; k < n; ++k)
        {
            const int dir = d[k];
            const int over = TBL.step[i][dir];
            const int land = TBL.jump[i][dir];
            if (over && land && pos.sq[land] == EMPTY && pos.sq[over] != EMPTY && !isOwnedBy(pos.sq[over], side))
            {
                return true;
            }
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Evaluation
// ---------------------------------------------------------------------------

namespace eval
{

inline constexpr int MAN_VALUE = 100;
inline constexpr int KING_VALUE = 155;

/// Advancement bonus by "distance travelled" (0 = own back rank, 6 = about to crown).
inline constexpr int ADVANCE[8] = {0, 1, 3, 6, 10, 16, 24, 24};

/// Column bonus — the two edge files are dead wood, the middle two are the fight.
inline constexpr int FILE_BONUS[8] = {-6, -1, 2, 4, 4, 2, -1, -6};

/// King centralisation, indexed by row.
inline constexpr int KING_ROW[8] = {-8, -2, 3, 6, 6, 3, -2, -8};

/// Cheap step-mobility count (chains not expanded — this is a leaf heuristic).
inline int stepMobility(const Position &pos, Side side)
{
    int n = 0;
    for (int i = 1; i <= NUM_SQ; ++i)
    {
        const uint8_t piece = pos.sq[i];
        if (piece == EMPTY || !isOwnedBy(piece, side))
        {
            continue;
        }
        int d[4], cnt;
        detail::directionsFor(piece, d[0], d[1], d[2], d[3], cnt);
        for (int k = 0; k < cnt; ++k)
        {
            const int to = TBL.step[i][d[k]];
            if (to && pos.sq[to] == EMPTY)
            {
                ++n;
            }
        }
    }
    return n;
}

} // namespace eval

/**
 * Static evaluation, always from BLACK's point of view (positive = good for BLACK,
 * which is the side "Ibox Offline" plays).
 */
inline int evaluate(const Position &pos)
{
    int redMen = 0, blackMen = 0, redKings = 0, blackKings = 0;
    int score = 0;

    for (int i = 1; i <= NUM_SQ; ++i)
    {
        const uint8_t piece = pos.sq[i];
        if (piece == EMPTY)
        {
            continue;
        }
        const int r = TBL.row[i];
        const int c = TBL.col[i];

        switch (piece)
        {
        case RED_MAN:
            ++redMen;
            // RED travels toward row 0
            score -= eval::ADVANCE[7 - r];
            score -= eval::FILE_BONUS[c];
            break;
        case BLACK_MAN:
            ++blackMen;
            score += eval::ADVANCE[r];
            score += eval::FILE_BONUS[c];
            break;
        case RED_KING:
            ++redKings;
            score -= eval::KING_ROW[r];
            score -= eval::FILE_BONUS[c];
            break;
        case BLACK_KING:
            ++blackKings;
            score += eval::KING_ROW[r];
            score += eval::FILE_BONUS[c];
            break;
        default:
            break;
        }
    }

    const int material = (blackMen - redMen) * eval::MAN_VALUE + (blackKings - redKings) * eval::KING_VALUE;
    score += material;

    const int totalPieces = redMen + blackMen + redKings + blackKings;

    // --- back rank ---------------------------------------------------------
    // Holding your own back rank stops the opponent crowning. Worthless once
    // the opponent has no men left to promote, so it is phased out.
    if (redMen > 0)
    {
        int guards = 0;
        for (int i = 29; i <= 32; ++i) // BLACK's back rank == row 0
        {
            if (pos.sq[i] == BLACK_MAN)
            {
                ++guards;
            }
        }
        score += guards * 7;
    }
    if (blackMen > 0)
    {
        int guards = 0;
        for (int i = 1; i <= 4; ++i) // RED's back rank == row 7
        {
            if (pos.sq[i] == RED_MAN)
            {
                ++guards;
            }
        }
        score -= guards * 7;
    }

    // --- mobility ----------------------------------------------------------
    score += (eval::stepMobility(pos, Side::BLACK) - eval::stepMobility(pos, Side::RED)) * 2;

    // --- trade-when-ahead --------------------------------------------------
    // Exchanging pieces while up material converts an edge into a win, so the
    // same material lead is scored higher as the board empties.
    if (material != 0)
    {
        score += (material * (24 - totalPieces)) / 48;
    }

    // --- tempo -------------------------------------------------------------
    score += (pos.stm == Side::BLACK) ? 2 : -2;

    return score;
}

// ---------------------------------------------------------------------------
// Transposition table
// ---------------------------------------------------------------------------

enum class Bound : uint8_t
{
    NONE = 0,
    EXACT,
    LOWER,
    UPPER
};

struct TTEntry
{
    uint64_t key{0};
    int32_t score{0};
    Move best{};
    int16_t depth{-1};
    Bound bound{Bound::NONE};
};

// ---------------------------------------------------------------------------
// Search
// ---------------------------------------------------------------------------

/// How hard "Ibox Offline" tries. IBOX is the real thing.
enum class Strength : uint8_t
{
    CASUAL, // shallow + deliberately fallible
    CLUB,   // solid
    IBOX    // full strength, time-limited
};

struct SearchLimits
{
    int maxDepth{24};
    int timeMs{1100};
};

struct SearchResult
{
    Move best{};
    /// Score from BLACK's point of view, in centi-men (100 == one man).
    int score{0};
    int depth{0};
    uint64_t nodes{0};
    int elapsedMs{0};
    /// Principal variation. pv[0] is `best`, pv[1] is the reply we expect.
    std::vector<Move> pv;
};

/**
 * Negamax + alpha-beta search with iterative deepening, a transposition table,
 * killer/history move ordering, forced-move extensions and a capture-resolving
 * quiescence search.
 */
class Engine
{
  public:
    explicit Engine(size_t ttSizeMb = 16)
    {
        size_t entries = (ttSizeMb * 1024ULL * 1024ULL) / sizeof(TTEntry);
        size_t pow2 = 1;
        while (pow2 * 2 <= entries)
        {
            pow2 *= 2;
        }
        m_ttMask = pow2 - 1;
        m_tt.assign(pow2, TTEntry{});
    }

    void setStrength(Strength s)
    {
        m_strength = s;
        switch (s)
        {
        case Strength::CASUAL:
            m_limits = SearchLimits{4, 150};
            break;
        case Strength::CLUB:
            m_limits = SearchLimits{9, 450};
            break;
        case Strength::IBOX:
        default:
            m_limits = SearchLimits{24, 1100};
            break;
        }
    }

    [[nodiscard]] Strength getStrength() const
    {
        return m_strength;
    }

    void setLimits(const SearchLimits &l)
    {
        m_limits = l;
    }

    void newGame()
    {
        std::fill(m_tt.begin(), m_tt.end(), TTEntry{});
        std::memset(m_history, 0, sizeof(m_history));
        m_gameHistory.clear();
        m_rng = 0x2545F4914F6CDD1DULL;
    }

    /// Feed positions the real game has already visited, for repetition detection.
    void pushGameHistory(uint64_t key)
    {
        m_gameHistory.push_back(key);
    }

    /**
     * Find the best move for `pos.stm`.
     * @return best move plus score from BLACK's perspective.
     */
    SearchResult search(const Position &pos)
    {
        m_start = std::chrono::steady_clock::now();
        m_nodes = 0;
        m_stop = false;
        std::memset(m_killers, 0, sizeof(m_killers));

        SearchResult result;
        MoveList rootMoves;
        const int nRoot = generateMoves(pos, rootMoves);
        if (nRoot == 0)
        {
            return result; // no legal move: caller decides the game is over
        }
        result.best = rootMoves[0];

        // Only one legal move? Play it without burning a second on it.
        if (nRoot == 1)
        {
            Position next = pos;
            makeMove(next, rootMoves[0]);
            result.score = evaluate(next);
            result.depth = 1;
            result.pv.push_back(rootMoves[0]);
            return result;
        }

        Move bestSoFar = rootMoves[0];
        int scoreSoFar = 0;

        for (int depth = 1; depth <= m_limits.maxDepth; ++depth)
        {
            m_pathLen = 0;
            const int score = searchRoot(pos, depth, bestSoFar, scoreSoFar);
            if (m_stop && depth > 1)
            {
                break; // discard the half-finished iteration
            }
            bestSoFar = m_rootBest;
            scoreSoFar = score;
            result.best = bestSoFar;
            result.score = (pos.stm == Side::BLACK) ? score : -score;
            result.depth = depth;
            extractPv(result.pv);

            if (std::abs(score) > MATE_SCORE - MAX_PLY)
            {
                break; // forced result found, deeper search adds nothing
            }
            if (timeUp())
            {
                break;
            }
        }

        // CASUAL deliberately fumbles: pick a slightly worse root move sometimes.
        if (m_strength == Strength::CASUAL && nRoot > 1 && (nextRandom() % 100) < 25)
        {
            result.best = rootMoves[nextRandom() % static_cast<uint64_t>(nRoot)];
            result.pv.clear();
            result.pv.push_back(result.best);
        }

        result.nodes = m_nodes;
        result.elapsedMs = elapsedMs();
        return result;
    }

    /// Quick static-ish score, used for blunder detection between moves.
    int quickEval(const Position &pos, int depthOverride = 6)
    {
        const SearchLimits saved = m_limits;
        m_limits = SearchLimits{depthOverride, 120};
        const SearchResult r = search(pos);
        m_limits = saved;
        return r.score;
    }

  private:
    std::vector<TTEntry> m_tt;
    size_t m_ttMask{0};
    Strength m_strength{Strength::IBOX};
    SearchLimits m_limits{};

    uint64_t m_nodes{0};
    bool m_stop{false};
    std::chrono::steady_clock::time_point m_start{};

    Move m_rootBest{};
    Move m_killers[MAX_PLY][2]{};
    int m_history[NUM_SQ + 1][NUM_SQ + 1]{};

    /// Triangular principal-variation table. Walking the TT for the PV is
    /// unreliable once entries start getting evicted, so collect it directly.
    Move m_pv[MAX_PLY][MAX_PLY]{};
    int m_pvLen[MAX_PLY]{};

    void setPvAt(int ply, const Move &mv)
    {
        m_pv[ply][0] = mv;
        const int childLen = (ply + 1 < MAX_PLY) ? m_pvLen[ply + 1] : 0;
        const int copyLen = std::min(childLen, MAX_PLY - ply - 2);
        for (int i = 0; i < copyLen; ++i)
        {
            m_pv[ply][i + 1] = m_pv[ply + 1][i];
        }
        m_pvLen[ply] = copyLen + 1;
    }

    /// Zobrist keys along the current search path, for repetition detection.
    uint64_t m_path[MAX_PLY]{};
    int m_pathLen{0};
    std::vector<uint64_t> m_gameHistory;

    uint64_t m_rng{0x2545F4914F6CDD1DULL};

    uint64_t nextRandom()
    {
        m_rng ^= m_rng << 13;
        m_rng ^= m_rng >> 7;
        m_rng ^= m_rng << 17;
        return m_rng;
    }

    [[nodiscard]] int elapsedMs() const
    {
        using namespace std::chrono;
        return static_cast<int>(duration_cast<milliseconds>(steady_clock::now() - m_start).count());
    }

    bool timeUp()
    {
        return elapsedMs() >= m_limits.timeMs;
    }

    /// Check the clock every 2048 nodes rather than every node.
    void tickClock()
    {
        if ((++m_nodes & 2047ULL) == 0 && timeUp())
        {
            m_stop = true;
        }
    }

    bool isRepetition(uint64_t key) const
    {
        for (int i = 0; i < m_pathLen; ++i)
        {
            if (m_path[i] == key)
            {
                return true;
            }
        }
        // Two prior sightings in the real game means a third is a draw.
        int seen = 0;
        for (auto k : m_gameHistory)
        {
            if (k == key && ++seen >= 2)
            {
                return true;
            }
        }
        return false;
    }

    // -- move ordering ------------------------------------------------------

    int scoreMove(const Move &mv, const Move &ttMove, int ply) const
    {
        if (!ttMove.isNull() && mv == ttMove)
        {
            return 1'000'000;
        }
        if (mv.isCapture())
        {
            return 500'000 + mv.nCaptures * 1000 + (mv.promotes ? 500 : 0);
        }
        if (mv == m_killers[ply][0])
        {
            return 400'000;
        }
        if (mv == m_killers[ply][1])
        {
            return 300'000;
        }
        return (mv.promotes ? 200'000 : 0) + m_history[mv.from][mv.to];
    }

    void orderMoves(MoveList &moves, int n, const Move &ttMove, int ply) const
    {
        std::array<int, MAX_MOVES> keys{};
        for (int i = 0; i < n; ++i)
        {
            keys[i] = scoreMove(moves[i], ttMove, ply);
        }
        // Insertion sort: n is small and lists are usually near-ordered already.
        for (int i = 1; i < n; ++i)
        {
            const Move mv = moves[i];
            const int k = keys[i];
            int j = i - 1;
            while (j >= 0 && keys[j] < k)
            {
                moves[j + 1] = moves[j];
                keys[j + 1] = keys[j];
                --j;
            }
            moves[j + 1] = mv;
            keys[j + 1] = k;
        }
    }

    // -- transposition table ------------------------------------------------

    TTEntry *probe(uint64_t key)
    {
        TTEntry &e = m_tt[key & m_ttMask];
        return e.key == key ? &e : nullptr;
    }

    void store(uint64_t key, int depth, int score, Bound bound, const Move &best)
    {
        TTEntry &e = m_tt[key & m_ttMask];
        if (e.key == key && e.depth > depth && e.bound == Bound::EXACT)
        {
            return; // keep the deeper exact entry
        }
        e.key = key;
        e.depth = static_cast<int16_t>(depth);
        e.score = score;
        e.bound = bound;
        e.best = best;
    }

    // -- the search ---------------------------------------------------------

    int searchRoot(const Position &pos, int depth, const Move &prevBest, int prevScore)
    {
        MoveList moves;
        const int n = generateMoves(pos, moves);
        orderMoves(moves, n, prevBest, 0);

        int alpha = -INF_SCORE;
        const int beta = INF_SCORE;
        Move best = moves[0];
        int bestScore = -INF_SCORE;
        m_pvLen[0] = 0;

        m_path[m_pathLen++] = pos.key;
        for (int i = 0; i < n; ++i)
        {
            Position next = pos;
            makeMove(next, moves[i]);
            const int score = -negamax(next, depth - 1, -beta, -alpha, 1, true);
            if (m_stop && i > 0)
            {
                break;
            }
            if (score > bestScore)
            {
                bestScore = score;
                best = moves[i];
                setPvAt(0, moves[i]);
                if (score > alpha)
                {
                    alpha = score;
                }
            }
        }
        --m_pathLen;

        if (bestScore == -INF_SCORE)
        {
            bestScore = prevScore;
            best = prevBest;
        }
        m_rootBest = best;
        store(pos.key, depth, bestScore, Bound::EXACT, best);
        return bestScore;
    }

    int negamax(const Position &pos, int depth, int alpha, int beta, int ply, bool pvNode)
    {
        tickClock();
        if (m_stop)
        {
            return 0;
        }
        if (ply < MAX_PLY)
        {
            m_pvLen[ply] = 0;
        }

        if (ply > 0 && (isRepetition(pos.key) || pos.quietPlies >= 80))
        {
            return 0; // draw
        }
        if (ply >= MAX_PLY - 1)
        {
            return stmScore(pos);
        }

        const int alphaOrig = alpha;
        Move ttMove{};
        if (const TTEntry *e = probe(pos.key))
        {
            ttMove = e->best;
            // Never cut off on the PV: it would truncate the variation we report.
            if (e->depth >= depth && !pvNode)
            {
                if (e->bound == Bound::EXACT)
                {
                    return e->score;
                }
                if (e->bound == Bound::LOWER && e->score > alpha)
                {
                    alpha = e->score;
                }
                else if (e->bound == Bound::UPPER && e->score < beta)
                {
                    beta = e->score;
                }
                if (alpha >= beta)
                {
                    return e->score;
                }
            }
        }

        // Captures are compulsory, so a position with a capture available is not a
        // real branching point — search it one ply deeper instead of cutting off.
        const bool forced = hasCapture(pos);
        if (forced && depth < 12)
        {
            ++depth;
        }

        if (depth <= 0)
        {
            return quiescence(pos, alpha, beta, ply);
        }

        MoveList moves;
        const int n = generateMoves(pos, moves);
        if (n == 0)
        {
            // No move available: in draughts that is a loss for the side to move.
            return -MATE_SCORE + ply;
        }
        orderMoves(moves, n, ttMove, ply);

        Move best = moves[0];
        int bestScore = -INF_SCORE;
        m_path[m_pathLen++] = pos.key;

        for (int i = 0; i < n; ++i)
        {
            Position next = pos;
            makeMove(next, moves[i]);

            int score;
            if (i == 0)
            {
                score = -negamax(next, depth - 1, -beta, -alpha, ply + 1, pvNode);
            }
            else
            {
                // Late-move reduction for quiet moves late in an ordered list.
                int reduction = 0;
                if (depth >= 3 && i >= 4 && !moves[i].isCapture() && !moves[i].promotes && !forced)
                {
                    reduction = 1;
                }
                score = -negamax(next, depth - 1 - reduction, -alpha - 1, -alpha, ply + 1, false);
                if (score > alpha && (reduction > 0 || score < beta))
                {
                    score = -negamax(next, depth - 1, -beta, -alpha, ply + 1, pvNode);
                }
            }

            if (m_stop)
            {
                --m_pathLen;
                return 0;
            }

            if (score > bestScore)
            {
                bestScore = score;
                best = moves[i];
            }
            if (score > alpha)
            {
                alpha = score;
                setPvAt(ply, moves[i]);
            }
            if (alpha >= beta)
            {
                if (!moves[i].isCapture())
                {
                    if (!(m_killers[ply][0] == moves[i]))
                    {
                        m_killers[ply][1] = m_killers[ply][0];
                        m_killers[ply][0] = moves[i];
                    }
                    m_history[moves[i].from][moves[i].to] += depth * depth;
                }
                break;
            }
        }
        --m_pathLen;

        const Bound bound = (bestScore <= alphaOrig) ? Bound::UPPER : (bestScore >= beta ? Bound::LOWER : Bound::EXACT);
        store(pos.key, depth, bestScore, bound, best);
        return bestScore;
    }

    /**
     * Resolve pending capture sequences so we never evaluate a position with a
     * piece hanging in mid-exchange.
     */
    int quiescence(const Position &pos, int alpha, int beta, int ply)
    {
        tickClock();
        if (m_stop || ply >= MAX_PLY - 1)
        {
            return stmScore(pos);
        }

        if (!hasCapture(pos))
        {
            return stmScore(pos);
        }

        MoveList moves;
        const int n = generateMoves(pos, moves);
        if (n == 0)
        {
            return -MATE_SCORE + ply;
        }

        int bestScore = -INF_SCORE;
        for (int i = 0; i < n; ++i)
        {
            Position next = pos;
            makeMove(next, moves[i]);
            const int score = -quiescence(next, -beta, -alpha, ply + 1);
            if (score > bestScore)
            {
                bestScore = score;
            }
            if (score > alpha)
            {
                alpha = score;
            }
            if (alpha >= beta)
            {
                break;
            }
        }
        return bestScore;
    }

    /// evaluate() is BLACK-relative; negamax needs it side-to-move-relative.
    static int stmScore(const Position &pos)
    {
        const int e = evaluate(pos);
        return pos.stm == Side::BLACK ? e : -e;
    }

    /// Copy out the principal variation collected during the last iteration.
    void extractPv(std::vector<Move> &pv) const
    {
        pv.clear();
        for (int i = 0; i < m_pvLen[0] && i < 16; ++i)
        {
            pv.push_back(m_pv[0][i]);
        }
    }
};

// ---------------------------------------------------------------------------
// Small helpers for the GUI layer
// ---------------------------------------------------------------------------

/// Human-readable move, e.g. "11-15" or "23x14x7".
inline std::string moveToString(const Move &mv)
{
    std::string s = std::to_string(static_cast<int>(mv.from));
    if (!mv.isCapture())
    {
        s += "-" + std::to_string(static_cast<int>(mv.to));
        return s;
    }
    for (int i = 0; i < mv.nCaptures; ++i)
    {
        s += "x" + std::to_string(static_cast<int>(mv.path[i]));
    }
    return s;
}

/// Total pieces still on the board.
inline int pieceCount(const Position &pos)
{
    int n = 0;
    for (int i = 1; i <= NUM_SQ; ++i)
    {
        if (pos.sq[i] != EMPTY)
        {
            ++n;
        }
    }
    return n;
}

} // namespace chk::bot
