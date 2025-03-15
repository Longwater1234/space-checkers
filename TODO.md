# TODO list

- [ ] (Online mode) Display countdown timer (40 s), waiting for player's move. Reset it when they make a move.
- [ ] Record and list all previous moves and captures (for current match), in a scroll window.

### (Extras) Standard CJK font paths

- Windows 10/11: `C:/Windows/Fonts/msyc.ttc`
- MacOS: `/System/Library/Fonts/PingFang.ttc`
- How to load CJK font in imGui:

```cpp
ImFont* font = io.Fonts->AddFontFromFileTTF("/path/to/font.ext", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
```
