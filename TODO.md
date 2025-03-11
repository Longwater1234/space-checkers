
## GAME LOGS

### first game

- start time: 19:31:12
- end time: 19:35:17

### Standard CJK font paths

- Windows: `C:/Windows/Fonts/ARIALUNI.TTF.ttf`
- MacOS: `/System/Library/Fonts/PingFang.ttc`
- How to load cjk font in imgui

```cpp
ImFont* font = io.Fonts->AddFontFromFileTTF("/path/to/font.ext", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
```
