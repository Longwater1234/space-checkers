#include "Cell.hpp"
#include "GameManager.hpp"
#include "managers/LocalGameManager.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <gtest/gtest.h>

namespace chk
{

class TestableLocalGameManager : public LocalGameManager
{
  public:
    explicit TestableLocalGameManager(sf::RenderWindow *w) : LocalGameManager(w)
    {
    }

    using GameManager::blockList;
    using GameManager::forcedMoves;
    using GameManager::gameMap;
    using GameManager::getBlockList;
    using GameManager::identifyTargets;
    using GameManager::isGameOver;
    using GameManager::isPlayerRedTurn;
    using GameManager::playerBlack;
    using GameManager::playerRed;
    using GameManager::setPlayerRedTurn;
    using GameManager::setSourceCell;
    using LocalGameManager::handleMovePiece;
    using LocalGameManager::updateMatchStatus;

    void callBaseUpdateMatchStatus(const PlayerPtr &p1, const PlayerPtr &p2)
    {
        GameManager::updateMatchStatus(p1, p2);
    }

    PlayerPtr &getRedPlayer()
    {
        return this->playerRed;
    }

    PlayerPtr &getBlackPlayer()
    {
        return this->playerBlack;
    }

    std::unordered_map<int, int> &getGameMap()
    {
        return this->gameMap;
    }

    void placePiece(PieceType type, int pieceId, int cellIdx, bool isKing = false)
    {
        const auto &cells = this->getBlockList();
        const auto it = std::find_if(cells.begin(), cells.end(),
                                     [cellIdx](const chk::CellPtr &c) { return c->getIndex() == cellIdx; });
        if (it == cells.end())
        {
            return;
        }

        sf::CircleShape circle{0.5f * chk::SIZE_CELL};
        circle.setPosition((*it)->getPos());
        auto piece = std::make_unique<chk::Piece>(circle, type, pieceId);
        if (isKing)
        {
            piece->activateKing();
        }

        if (type == chk::PieceType::Red)
        {
            this->playerRed->receivePiece(piece);
        }
        else
        {
            this->playerBlack->receivePiece(piece);
        }
        this->gameMap.emplace(cellIdx, pieceId);
    }
};

} // namespace chk

class GameManagerTests : public ::testing::Test
{
  protected:
    sf::RenderWindow window;
    sf::Font font;
    std::unique_ptr<chk::TestableLocalGameManager> mgr;

    void SetUp() override
    {
        window.create(sf::VideoMode(600, 650), "Test Window");
        mgr = std::make_unique<chk::TestableLocalGameManager>(&window);
        mgr->drawCheckerboard(font);
    }

    void TearDown() override
    {
        if (window.isOpen())
        {
            window.close();
        }
    }
};

TEST_F(GameManagerTests, NoPieces_HasNoPossibleMovesReturnsTrue)
{
    EXPECT_TRUE(mgr->hasNoPossibleMoves(mgr->getRedPlayer()));
}

TEST_F(GameManagerTests, SingleNormalPieceTrappedAtEdge_HasNoPossibleMovesReturnsTrue)
{
    // Normal RED piece at cell 32 (row 0, col 1, y = 0). Normal RED can only move UP (y < 0, off board).
    mgr->placePiece(chk::PieceType::Red, 1, 32, false);
    EXPECT_TRUE(mgr->hasNoPossibleMoves(mgr->getRedPlayer()));
}

TEST_F(GameManagerTests, SingleNormalPieceCanMove_HasNoPossibleMovesReturnsFalse)
{
    // Normal RED piece at cell 1 (row 7, col 6, pos = (450, 525)). Can move up-left to cell 6 or up-right to cell 5.
    mgr->placePiece(chk::PieceType::Red, 1, 1, false);
    EXPECT_FALSE(mgr->hasNoPossibleMoves(mgr->getRedPlayer()));
}

TEST_F(GameManagerTests, KingTrappedByOpponentPieces_HasNoPossibleMovesReturnsTrue)
{
    // RED King at cell 32 (row 0, col 1, pos = (75, 0))
    mgr->placePiece(chk::PieceType::Red, 1, 32, true);

    // Opponent BLACK pieces blocking adjacent diagonal cells:
    // cell 28 (row 1, col 0, pos = (0, 75))
    // cell 27 (row 1, col 2, pos = (150, 75))
    mgr->placePiece(chk::PieceType::Black, 2, 28, false);
    mgr->placePiece(chk::PieceType::Black, 3, 27, false);

    // Opponent piece blocking jump over cell 27: cell 23 (row 2, col 3, pos = (225, 150))
    // Note: jump over cell 28 would land at (-75, 150) which is off board.
    mgr->placePiece(chk::PieceType::Black, 4, 23, false);

    // Identify targets for RED (none should be found since all jumps are blocked/off-board)
    mgr->identifyTargets(mgr->getRedPlayer());
    EXPECT_TRUE(mgr->getForcedMoves().empty());

    // RED King has no legal simple moves and no jumps
    EXPECT_TRUE(mgr->hasNoPossibleMoves(mgr->getRedPlayer()));
}

TEST_F(GameManagerTests, KingWithOpenDiagonal_HasNoPossibleMovesReturnsFalse)
{
    // RED King at cell 32 (row 0, col 1, pos = (75, 0))
    mgr->placePiece(chk::PieceType::Red, 1, 32, true);

    // Only cell 28 is blocked, cell 27 is open
    mgr->placePiece(chk::PieceType::Black, 2, 28, false);

    EXPECT_FALSE(mgr->hasNoPossibleMoves(mgr->getRedPlayer()));
}

TEST_F(GameManagerTests, PieceWithForcedCapture_HasNoPossibleMovesReturnsFalse)
{
    // RED piece at cell 24 (row 2, col 1, pos = (75, 150))
    mgr->placePiece(chk::PieceType::Red, 1, 24, false);

    // BLACK piece at cell 27 (row 1, col 2, pos = (150, 75)) — enemy ahead on RHS
    mgr->placePiece(chk::PieceType::Black, 3, 27, false);
    // Jump destination is row 0, col 3 (pos = (225, 0), cell 31) which is EMPTY!

    mgr->identifyTargets(mgr->getRedPlayer());
    EXPECT_FALSE(mgr->getForcedMoves().empty());
    EXPECT_FALSE(mgr->hasNoPossibleMoves(mgr->getRedPlayer()));
}

TEST_F(GameManagerTests, LocalGameManager_TrappedOpponentTriggersGameOverOnTurn)
{
    // Issue #82: if opponent's piece cannot move and size == 1, game over.
    // Set up RED with 1 trapped piece at cell 32
    mgr->placePiece(chk::PieceType::Red, 1, 32, false); // Normal RED at row 0, cannot move forward

    // Set up BLACK with a piece that can move
    mgr->placePiece(chk::PieceType::Black, 2, 1, false);

    // It's RED's turn
    mgr->setPlayerRedTurn(true);

    EXPECT_FALSE(mgr->isGameOver());
    mgr->updateMatchStatus(mgr->getBlackPlayer(), mgr->getRedPlayer());

    EXPECT_TRUE(mgr->isGameOver());
    EXPECT_EQ(mgr->getCurrentMsg(), "GAME OVER! BLACK wins!");
}

TEST_F(GameManagerTests, LocalGameManager_OpponentWithMovesDoesNotTriggerGameOver)
{
    // Both players have movable pieces
    mgr->placePiece(chk::PieceType::Red, 1, 1, false);
    mgr->placePiece(chk::PieceType::Black, 2, 32, false);

    mgr->setPlayerRedTurn(true);
    mgr->updateMatchStatus(mgr->getBlackPlayer(), mgr->getRedPlayer());

    EXPECT_FALSE(mgr->isGameOver());
}

TEST_F(GameManagerTests, LocalGameManager_MovePieceTrappingOpponent_SetsGameOver)
{
    // RED piece at cell 32 (trapped, cannot move forward)
    mgr->placePiece(chk::PieceType::Red, 1, 32, false);

    // BLACK piece at cell 24 (row 2, col 1, pos = (75, 150))
    mgr->placePiece(chk::PieceType::Black, 2, 24, false);

    // Find destination cell 20 (row 3, col 0, pos = (0, 225))
    const auto &cells = mgr->getBlockList();
    const auto it = std::find_if(cells.begin(), cells.end(), [](const chk::CellPtr &c) { return c->getIndex() == 20; });
    ASSERT_NE(it, cells.end());

    // It is BLACK's turn
    mgr->setPlayerRedTurn(false);
    mgr->setSourceCell(24);

    EXPECT_FALSE(mgr->isGameOver());

    // BLACK moves piece 2 from cell 24 to cell 20
    mgr->handleMovePiece(mgr->getBlackPlayer(), mgr->getRedPlayer(), *it, 2);

    // Now it is RED's turn, RED is trapped, game should be OVER with BLACK winning!
    EXPECT_TRUE(mgr->isGameOver());
    EXPECT_EQ(mgr->getCurrentMsg(), "GAME OVER! BLACK wins!");
}

TEST_F(GameManagerTests, GameManagerBase_UpdateMatchStatus_DoesNotCheckNoMoves)
{
    // Verify base GameManager::updateMatchStatus only checks piece counts == 0,
    // ensuring OnlineGameManager (which calls base updateMatchStatus) is unaffected.
    mgr->placePiece(chk::PieceType::Red, 1, 32, false); // Trapped RED
    mgr->placePiece(chk::PieceType::Black, 2, 1, false);

    mgr->setPlayerRedTurn(true);
    mgr->callBaseUpdateMatchStatus(mgr->getBlackPlayer(), mgr->getRedPlayer());

    // Base GameManager should NOT set game over because both piece counts are > 0
    EXPECT_FALSE(mgr->isGameOver());
}
