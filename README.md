# space-checkers

Online Multiplayer Checkers game in C++ built with SFML 2.6 and ixWebsockets. With very minimal dependencies (no BOOST lib), and
a simple build process. All dependencies are auto-downloaded and built for you using **CPM.cmake** (see folder `dependencies`). The only dependency you need
pre-installed on your OS is SFML 2.6.


### Main Libraries Used
- SFML 2.6.1
- imGui-SFML
- ixWebsockets
- spdlog
- Google Protobuf 27.1

## Requirements for Building

- C++17 (or newer) build tools.
- Pre-built [SFML 2.6.x](https://www.sfml-dev.org/download/sfml/2.6.1/) binaries. (Must match your Compiler and OS)
- [CMake 3.20+](https://cmake.org/download/) or newer (GUI recommended)

### For Windows

- MS Visual Studio 2019 or newer (NOT vscode), with default "**Desktop C++ Development**" workload.
- Please download "Visual C++ 64bit" edition of SFML; ignore others.
- Move your unzipped `SFML-2.6.x` folder to its own home, example: `C:/SFML/SFML-2.6.1`.
- Edit **line 15** in [CMakeLists.txt](CMakeLists.txt), to set value `SFML_HOME` to folder path you moved SFML into (see previous step)

### For MacOS

- XCode latest from AppStore (with MacOS SDK)
- Apple Developer tools. After Xcode is installed, run this in your Terminal:
```bash
  sudo xcode-select --install
```
- Please install SFML 2.6 as **Frameworks** as shown in [official macOS guide](https://www.sfml-dev.org/tutorials/2.6/start-osx.php).

### For Linux

- Use your OS package manager (`apt-get` or `yum`) to install SFML 2.5 or newer.
- Alternatively, you may build SFML from source, see [official docs SFML](https://www.sfml-dev.org/tutorials/2.6/start-linux.php).

## Building Instructions

Here is the summary for all 3 major desktop platforms.

### On Windows

- Open this folder directly in Visual Studio 2019 IDE or newer. (with C++ Workload installed)
- Select build mode "x64 Release" from top toolbar.
- Click the menu "Build" > "Build All". That's it. Your game will be in new folder `out/build/x64Release/` inside project directory.

### On MacOS

- Using Cmake GUI or `cmake` CLI, generate new **XCode** project.
- For example, when using cmake on Terminal, run this command:

```bash
mkdir build
cd build
cmake . . -G "XCode" -DCMAKE_BUILD_TYPE="Release"
```

- Open the generated `.xcodeproj` inside XCode.
- From top toolbar, click "Product" > "Build".

### On Linux

- You may use Cmake GUI to generate Unix Makefiles. Then run `make build`.
- Alternatively, open terminal at this project directory, run these commands:

```bash
mkdir build
cd build
cmake . . -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="Release"
cmake --build ./
```

- Your game will be built and found in `build/Release/` directory
