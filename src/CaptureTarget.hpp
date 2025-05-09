#pragma once

namespace chk
{
/**
 * Keeps details about the Piece which MUST be captured next
 */
struct CaptureTarget
{
    short preyPieceId{-1};  // ID of the piece that MUST be captured
    int preyCellIdx{-1};    // the cell hosting this piece
    int hunterNextCell{-1}; // destination of hunter AFTER capturing enemy
};

} // namespace chk
