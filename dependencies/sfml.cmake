# optional: download SFML 2.6.2

CPMAddPackage(
    NAME sfml
    URL    "https://github.com/SFML/SFML/archive/refs/tags/2.6.2.tar.gz"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    OPTIONS "BUILD_SHARED_LIBS FALSE"
) 
