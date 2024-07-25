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