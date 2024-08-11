- [ ] Dont allow immediate capture, if Hunter Piece just became King (wasnt king before capture)
- [ ] reset isMyTurn after game is over.
- [ ] reset all states to FALSE inside OnlineManager after game is over

  192.168.2.110:9876/game

```cpp
#include <iostream>
using namespace std;

bool xnor(bool p, bool q) {
    return !(p ^ q);
}

int main() {
    bool p = true;
    bool q = false;

    bool result = xnor(p, q);

    if (result) {
        cout << "XNOR is true" << endl;
    } else {
        cout << "XNOR is false" << endl;
    }

    return 0;
}

```

## GAME LOGS

### first game

- start time: 19:31:12
- end time: 19:35:17

### Standard CJK font paths

- Windows: `C:/Windows/Fonts/ARIALUNI.ttf`
- MacOS: `/System/Library/Fonts/PingFang.ttc`
- How to load cjk font in imgui

```cpp
#ifdef WIN32
constexpr auto CHINESE_FONT = "C:/Windows/Fonts/ARIALUNI.ttf";
#elseif __APPLE__
constexpr auto CHINESE_FONT = "/System/Library/Fonts/PingFang.ttc";
#endif

ImFont* font = io.Fonts->AddFontFromFileTTF(CHINESE_FONT, 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
```

i am writing a game using SFML 2.6. i want to support 
displaying Asian characters. Sfml sf::Text::setString( ) support 
Asian characters with proper display only if used with wide string
literal as such: setString(L"你好"). but my string parameter is
coming from a function call gameManager->getCurrentMsg() which returns std::string (utf8). 
how do i convert the returned message to widestring literal? 

 sfText.setString(gameManager->getCurrentMsg());
 window.draw(sfText);


