//
// Tests for the "Ibox Offline" search engine.
//
// CheckersEngine.hpp deliberately has no SFML dependency, so all of this runs
// without a window or a GL context.
//
#include "bot/BotPersona.hpp"
#include "bot/CheckersEngine.hpp"
#include <gtest/gtest.h>

using namespace chk::bot;

namespace
{

uint64_t perft(const Position &pos, int depth)
{
    MoveList moves;
    const int n = generateMoves(pos, moves);
    if (depth == 1)
    {
        return static_cast<uint64_t>(n);
    }
    uint64_t total = 0;
    for (int i = 0; i < n; ++i)
    {
        Position next = pos;
        makeMove(next, moves[i]);
        total += perft(next, depth - 1);
    }
    return total;
}

} // namespace

// ---------------------------------------------------------------------------
// Geometry — must agree with GameManager::drawCheckerboard()
// ---------------------------------------------------------------------------

TEST(EngineGeometry, SquareIndicesMapToExpectedRowAndColumn)
{
    // Dark cells are numbered 32 down to 1, scanning rows top to bottom.
    EXPECT_EQ(TBL.row[32], 0);
    EXPECT_EQ(TBL.col[32], 1);
    EXPECT_EQ(TBL.row[29], 0);
    EXPECT_EQ(TBL.col[29], 7);
    EXPECT_EQ(TBL.row[4], 7);
    EXPECT_EQ(TBL.col[4], 0);
    EXPECT_EQ(TBL.row[1], 7);
    EXPECT_EQ(TBL.col[1], 6);
}

TEST(EngineGeometry, DiagonalsMatchGameManagerDeltas)
{
    // GameManager computes these with hand-rolled index deltas; the engine
    // derives them from (row, col). They must agree.
    EXPECT_EQ(TBL.step[2][DIR_UP_LEFT], 7);
    EXPECT_EQ(TBL.step[2][DIR_UP_RIGHT], 6);
    EXPECT_EQ(TBL.jump[2][DIR_UP_LEFT], 11);
}

TEST(EngineGeometry, EdgeSquaresHaveNoOffBoardNeighbours)
{
    // Square 5 sits on the right-hand edge (row 6, col 7).
    EXPECT_EQ(TBL.step[5][DIR_UP_RIGHT], 0);
    EXPECT_EQ(TBL.step[5][DIR_UP_LEFT], 9);
}

TEST(EngineGeometry, EveryStepIsReciprocal)
{
    constexpr int opposite[4] = {3, 2, 1, 0};
    for (int sq = 1; sq <= NUM_SQ; ++sq)
    {
        for (int d = 0; d < 4; ++d)
        {
            const int to = TBL.step[sq][d];
            if (to != 0)
            {
                EXPECT_EQ(TBL.step[to][opposite[d]], sq) << "sq " << sq << " dir " << d;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Move generation — perft against published English draughts node counts
// ---------------------------------------------------------------------------

TEST(EngineMoveGen, PerftMatchesPublishedCounts)
{
    // Standard English-draughts perft from the opening position. Getting all of
    // these right pins down compulsory captures, multi-jump chains and the
    // "crowning ends the move" rule in one go.
    const uint64_t expected[] = {0, 7, 49, 302, 1469, 7361, 36768, 179740};
    Position pos;
    pos.setStartPosition();
    for (int depth = 1; depth <= 7; ++depth)
    {
        EXPECT_EQ(perft(pos, depth), expected[depth]) << "perft(" << depth << ")";
    }
}

TEST(EngineMoveGen, CapturesAreCompulsory)
{
    Position pos;
    pos.clear();
    pos.stm = Side::BLACK;
    pos.sq[22] = BLACK_MAN;
    pos.sq[TBL.step[22][DIR_DOWN_LEFT]] = RED_MAN;
    pos.sq[3] = RED_MAN; // an unrelated red piece
    pos.rehash();

    MoveList moves;
    const int n = generateMoves(pos, moves);
    ASSERT_GT(n, 0);
    for (int i = 0; i < n; ++i)
    {
        EXPECT_TRUE(moves[i].isCapture()) << "quiet move offered while a capture exists";
    }
}

TEST(EngineMoveGen, CrowningTerminatesAJumpChain)
{
    // A man that lands on the back rank is crowned and its move ends there,
    // even if another jump would otherwise be available. GameManager relies on
    // this (it only looks for a continuation when king status is unchanged).
    Position pos;
    pos.clear();
    pos.stm = Side::BLACK;
    const int over = TBL.step[9][DIR_DOWN_LEFT];
    const int land = TBL.jump[9][DIR_DOWN_LEFT];
    ASSERT_NE(land, 0);
    ASSERT_EQ(TBL.row[land], 7); // lands on BLACK's promotion row
    pos.sq[9] = BLACK_MAN;
    pos.sq[over] = RED_MAN;
    const int over2 = TBL.step[land][DIR_DOWN_RIGHT];
    if (over2 != 0)
    {
        pos.sq[over2] = RED_MAN;
    }
    pos.sq[32] = RED_MAN;
    pos.rehash();

    MoveList moves;
    const int n = generateMoves(pos, moves);
    ASSERT_GT(n, 0);
    for (int i = 0; i < n; ++i)
    {
        if (moves[i].promotes)
        {
            EXPECT_EQ(moves[i].nCaptures, 1) << "chain continued past crowning";
        }
    }
}

TEST(EngineMoveGen, MenCannotCaptureBackwards)
{
    Position pos;
    pos.clear();
    pos.stm = Side::BLACK;
    pos.sq[14] = BLACK_MAN;
    const int behind = TBL.step[14][DIR_UP_LEFT];
    ASSERT_NE(behind, 0);
    ASSERT_NE(TBL.jump[14][DIR_UP_LEFT], 0);
    pos.sq[behind] = RED_MAN;
    pos.rehash();

    MoveList moves;
    const int n = generateMoves(pos, moves);
    for (int i = 0; i < n; ++i)
    {
        EXPECT_FALSE(moves[i].isCapture()) << "a man captured backwards";
    }
}

TEST(EngineMoveGen, KingsCanCaptureBackwards)
{
    Position pos;
    pos.clear();
    pos.stm = Side::BLACK;
    pos.sq[14] = BLACK_KING;
    const int behind = TBL.step[14][DIR_UP_LEFT];
    pos.sq[behind] = RED_MAN;
    pos.rehash();

    MoveList moves;
    const int n = generateMoves(pos, moves);
    ASSERT_GT(n, 0);
    EXPECT_TRUE(moves[0].isCapture());
}

// ---------------------------------------------------------------------------
// Hashing
// ---------------------------------------------------------------------------

TEST(EngineHashing, IncrementalKeyMatchesFullRecompute)
{
    Position pos;
    pos.setStartPosition();
    uint64_t rng = 12345;
    for (int i = 0; i < 200; ++i)
    {
        MoveList moves;
        const int n = generateMoves(pos, moves);
        if (n == 0)
        {
            break;
        }
        rng ^= rng << 13;
        rng ^= rng >> 7;
        rng ^= rng << 17;
        makeMove(pos, moves[rng % static_cast<uint64_t>(n)]);

        Position copy = pos;
        copy.rehash();
        ASSERT_EQ(copy.key, pos.key) << "incremental hash diverged at ply " << i;
    }
}

// ---------------------------------------------------------------------------
// Search
// ---------------------------------------------------------------------------

TEST(EngineSearch, TakesTheLongerCaptureChainWhenItIsBetter)
{
    Position pos;
    pos.clear();
    pos.stm = Side::BLACK;

    // Build a genuine two-hop chain that does not end in a crowning.
    int start = 0, over1 = 0, land1 = 0, over2 = 0;
    for (int s = 21; s <= 24 && start == 0; ++s)
    {
        for (int d1 = DIR_DOWN_LEFT; d1 <= DIR_DOWN_RIGHT && start == 0; ++d1)
        {
            const int o1 = TBL.step[s][d1], l1 = TBL.jump[s][d1];
            if (o1 == 0 || l1 == 0 || TBL.row[l1] == 7)
            {
                continue;
            }
            for (int d2 = DIR_DOWN_LEFT; d2 <= DIR_DOWN_RIGHT; ++d2)
            {
                const int o2 = TBL.step[l1][d2], l2 = TBL.jump[l1][d2];
                if (o2 != 0 && l2 != 0 && o2 != o1)
                {
                    start = s;
                    over1 = o1;
                    land1 = l1;
                    over2 = o2;
                    break;
                }
            }
        }
    }
    ASSERT_NE(start, 0) << "could not build a test chain";

    pos.sq[start] = BLACK_MAN;
    pos.sq[over1] = RED_MAN;
    pos.sq[over2] = RED_MAN;
    pos.sq[3] = RED_MAN;
    pos.rehash();

    Engine engine{4};
    engine.setLimits(SearchLimits{10, 200});
    const SearchResult result = engine.search(pos);
    EXPECT_EQ(result.best.nCaptures, 2);
    EXPECT_EQ(result.best.from, start);
    EXPECT_EQ(result.best.path[0], land1);
}

TEST(EngineSearch, AlwaysReturnsALegalMove)
{
    Engine engine{8};
    engine.setLimits(SearchLimits{8, 25});
    Position pos;
    pos.setStartPosition();

    for (int ply = 0; ply < 120; ++ply)
    {
        MoveList moves;
        const int n = generateMoves(pos, moves);
        if (n == 0 || pos.quietPlies >= 80)
        {
            break;
        }
        const SearchResult result = engine.search(pos);
        bool legal = false;
        for (int i = 0; i < n; ++i)
        {
            if (moves[i] == result.best)
            {
                legal = true;
                break;
            }
        }
        ASSERT_TRUE(legal) << "illegal move " << moveToString(result.best) << " at ply " << ply;
        makeMove(pos, result.best);
    }
}

TEST(EngineSearch, ReportsAPrincipalVariation)
{
    Engine engine{8};
    engine.setLimits(SearchLimits{8, 200});
    Position pos;
    pos.setStartPosition();
    const SearchResult result = engine.search(pos);

    ASSERT_FALSE(result.pv.empty());
    EXPECT_TRUE(result.pv[0] == result.best);
    // pv[1] is the reply Ibox plans around, and drives the "I predicted that"
    // and blunder-detection commentary, so it needs to be there.
    EXPECT_GE(result.pv.size(), 2u);
}

TEST(EngineSearch, BeatsAMaterialDownOpponentDecisively)
{
    // Down two men with no compensation, the search must see it is losing.
    Position pos;
    pos.setStartPosition();
    pos.sq[21] = EMPTY;
    pos.sq[22] = EMPTY;
    pos.stm = Side::BLACK;
    pos.rehash();

    Engine engine{8};
    engine.setLimits(SearchLimits{8, 200});
    const SearchResult result = engine.search(pos);
    EXPECT_LT(result.score, -100) << "engine did not recognise it was two men down";
}

// ---------------------------------------------------------------------------
// Persona wiring
// ---------------------------------------------------------------------------

TEST(BotPersonaTests, BlunderOutranksAPlainCapture)
{
    TurnReport r;
    r.botCaptures = 1;
    r.humanScoreSwing = 250;
    EXPECT_EQ(BotPersona::decide(r), Mood::BLUNDER);
}

TEST(BotPersonaTests, ResultBeatsEverythingElse)
{
    TurnReport r;
    r.gameOver = true;
    r.botWon = true;
    r.botCaptures = 3;
    EXPECT_EQ(BotPersona::decide(r), Mood::WIN);
}

TEST(BotPersonaTests, LevelEndgameTriggersTheGrind)
{
    TurnReport r;
    r.piecesLeft = 6;
    r.scoreCp = 10;
    EXPECT_EQ(BotPersona::decide(r), Mood::ENDGAME_GRIND);
}

TEST(BotPersonaTests, LinesDoNotRepeatBackToBack)
{
    BotPersona persona{4242};
    std::string previous;
    for (int i = 0; i < 60; ++i)
    {
        const std::string &line = persona.speak(Mood::QUIET);
        EXPECT_NE(line, previous) << "repeated a line at draw " << i;
        EXPECT_FALSE(line.empty());
        previous = line;
    }
}
