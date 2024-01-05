#pragma once
#include <map>
#include <ostream>
namespace chk
{
/**
 * Keeps details about the Piece about to be captured
 */
struct CaptureTarget
{
    int preyPieceId{-1};                     // the piece that MUST be captured
    int preyCellIdx{-1};                     // the cell hosting this piece
    std::map<uint16_t, int> hunterMoves{{}}; // map of PieceId -> destCell

    // override output operator
    friend std::ostream &operator<<(std::ostream &os, const CaptureTarget &target)
    {
        os << "preyPieceId: " << target.preyPieceId << " preyCellIdx: " << target.preyCellIdx
           << " size hunterMoves: " << target.hunterMoves.size();
        return os;
    }
};

} // namespace chk
