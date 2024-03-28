# space-checkers

Online Multiplayer Checkers game in C++ built with SFML 2.6 and ixWebsockets. With very minimal dependencies (no BOOST lib), and
a simple build process. All dependencies are auto-downloaded and built for you using CPM. The only dependency you need
pre-installed is SFML.

## Requirements for Building

- C++17 (or newer) compiler.
- Pre-built [SFML 2.6.x](https://www.sfml-dev.org/download/sfml/2.6.1/) binaries. (Must match your Compiler and OS)
- [CMake 3.20+](https://cmake.org/download/) or newer (GUI recommended)

### For Windows

- MS Visual Studio 2022 or newer (NOT vscode), with default "**Desktop C++ Development**" workload.
- Please choose "Visual C++ 2022" edition of SFML; ignore others.
- Move your unzipped `SFML-2.6.x` folder to its own home, example: `C:/SFML/SFML-2.6.1`

### For MacOS

- XCode latest from AppStore (with MacOS SDK)
- Apple Developer tools. After Xcode is installed, run this in your Terminal:
  ```bash
  sudo xcode-select --install
  ```
- Please install SFML 2.6 Framework as shown in [official macOS guide](https://www.sfml-dev.org/tutorials/2.6/start-osx.php).
