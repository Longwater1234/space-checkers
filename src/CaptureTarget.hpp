#pragma once
#include <ostream>

namespace chk
{
/**
 * Keeps details about the Piece which MUST be captured next
 */
struct CaptureTarget
{
    short preyPieceId{-1};  // ID of the piece that MUST be captured
    int preyCellIdx{-1};    // the cell hosting this piece
    int hunterNextCell{-1}; // landing destination of hunter after capturing enemy

    // override output operator
    friend std::ostream &operator<<(std::ostream &os, const CaptureTarget &target)
    {
        os << "preyPieceId: " << target.preyPieceId << " preyCellIdx: " << target.preyCellIdx
           << "hunterNextCell: " << target.hunterNextCell;
        return os;
    }
};

} // namespace chk
