# space - checkers

Online Multiplayer Checkers game in C++ built with SFML 2.6 and ixWebsockets. With very minimal dependencies (no Boost required), and
a simple build process. All dependencies are auto-downloaded and built for you using **CPM.cmake** (similar to CMake FetchContent). The only dependency you need
pre-installed on your OS is SFML 2.6.

### Main Libraries Used

- SFML 2.6.1
- imGui-SFML
- ixWebsockets
- spdlog
- Google Protobuf 27.2
- simdjson

## Requirements for Building

- C++17 (or newer) build tools.
- Pre-built [SFML 2.6.x](https://www.sfml-dev.org/download/sfml/2.6.1/) binaries. (Must match your Compiler and OS)
- [CMake 3.20+](https://cmake.org/download/) or newer (GUI recommended)

### For Windows

- At least Windows 10 64-bit
- MS Visual Studio 2019 or newer (NOT vscode), with default "**Desktop C++ Development**" bundle.
- Please download "Visual C++ 64bit" edition of SFML; ignore others.
- Move your unzipped `SFML-2.6.x` folder to its own home, example: `C:/SFML/SFML-2.6.1`.
- Edit **line 16** in [CMakeLists.txt](CMakeLists.txt), to set value `SFML_HOME` to folder path you moved SFML into (see previous step)

### For MacOS (Intel & ARM)

- Please install SFML 2.6 as **Frameworks**, not as "dylibs", as shown in [official macOS guide](https://www.sfml-dev.org/tutorials/2.6/start-osx.php).
- XCode 14 or newer from AppStore (with MacOS SDK)
- Apple Developer tools. After Xcode is installed, run this in your Terminal:

```bash
  sudo xcode-select --install
```

### For Linux

- Use your OS package manager (`apt-get` or `yum`) to install SFML 2.6 or newer.
- Alternatively, you may build SFML 2.6 from source, see [official docs SFML](https://www.sfml-dev.org/tutorials/2.6/start-linux.php).

## Building Instructions

Please see [BUILDING.md](BUILDING.md) for detailed instructions.

### License

[MIT License](LICENSE) (c) 2024, Davis T.

