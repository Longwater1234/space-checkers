# zlib v1.3.1 (zlib License)

CPMAddPackage(
    NAME zlib
    URL "https://github.com/madler/zlib/archive/refs/tags/v1.3.2.tar.gz"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    OPTIONS "ZLIB_BUILD_EXAMPLES OFF" "ZLIB_BUILD_TESTING OFF" "ZLIB_INSTALL OFF" "ZLIB_BUILD_SHARED OFF"
)

if(TARGET zlibstatic AND NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()

if(MSVC)
    target_compile_definitions(zlibstatic PRIVATE _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
endif()
