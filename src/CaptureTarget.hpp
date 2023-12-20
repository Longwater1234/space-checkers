#pragma once
namespace chk
{

struct CaptureTarget
{
    int preyPieceId{-1}; // the piece that MUST be captured
    int preyCellIdx{-1}; // the cell hosting this cell
};

} // namespace chk
