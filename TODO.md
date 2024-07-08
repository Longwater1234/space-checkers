- [ ] Dont allow immediate capture, if Hunter Piece just became King (wasnt king before capture)
- [ ] too many deadlocks
- [ ] too much logging, on join! "I am player X"

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