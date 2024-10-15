# space - checkers

Offline & Online Multiplayer Checkers game in C++ built with SFML 2.6, Protobuf and ixWebsockets. With very minimal dependencies
and a simple build process. All dependencies are auto-downloaded (as `.tar.gz`) and built for you using [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake).

This game can connect to both Private and Public game servers. The Server project for this game is on a separate git repo, [available here](#). 
The only dependency you need pre-installed on your OS is SFML 2.6.x (or newer).

### Main Libraries Used

- SFML 2.6
- imGui-SFML
- ixWebsockets
- spdlog
- Google Protobuf 27.2 (Used entirely during gameplay)
- simdjson (Used once, for parsing list of public servers)

## Requirements for Building

- C++17 (or newer) build tools.
- Pre-built [SFML 2.6.x](https://www.sfml-dev.org/download/sfml/2.6.1/) binaries. (Must match your Compiler and OS)
- [CMake 3.20+](https://cmake.org/download/) or newer (GUI recommended)

### For Windows

- At least Windows 10
- MS Visual Studio 2022 or newer (NOT vscode), with "**Desktop C++ Development**" bundle.
- Please download "Visual C++ 64bit" edition of SFML; ignore others.
- Move your unzipped `SFML-2.6.x` folder to its own home, example: `C:/SFML/SFML-2.6.1`.
- Edit **line 16** in [CMakeLists.txt](CMakeLists.txt), to set value `SFML_HOME` to folder path you moved SFML into (see
  previous step)

### For MacOS

- Please install SFML 2.6 as **Frameworks**, not as "dylibs", as shown in [official macOS guide](https://www.sfml-dev.org/tutorials/2.6/start-osx.php).
- XCode 14 or newer from AppStore (with MacOS SDK)
- Apple Developer tools. After Xcode is installed, run this in your Terminal:

```bash
  sudo xcode-select --install
```

### For Linux Destkop

- Latest display drivers
- Use your OS package manager (`apt` or `yum`) to install SFML 2.6 or newer.
- Alternatively, you may build SFML 2.6 from source, see [official SFML docs](https://www.sfml-dev.org/tutorials/2.6/start-linux.php).
- You are required to install latest **OpenSSL Dev** library. See example on Ubuntu / Debian below.

```bash
  sudo apt install libsfml-dev
  sudo apt install libssl-dev
```

- Luckily, Windows and macOS **come with their native SSL libs built-in**, so nothing more to do :-)

## Building Instructions

Please see [BUILDING.md](BUILDING.md) for detailed instructions.

### License

[MIT License](LICENSE) &copy; 2024, Davis T.
