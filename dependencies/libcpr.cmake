# add libcpr (CURL for modern CPP) v1.14.2

if(WIN32)
    SET(SSL_OPTION "CPR_FORCE_WINSSL_BACKEND TRUE")
else()
    SET(SSL_OPTION "CPR_FORCE_MBEDTLS_BACKEND TRUE")
endif()

CPMAddPackage(
    NAME cpr
    URL    "https://github.com/libcpr/cpr/archive/refs/tags/1.14.2.tar.gz"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    OPTIONS
        "BUILD_SHARED_LIBS OFF"
        "CURL_ENABLE_EXPORT_TARGET OFF"
        "CMAKE_SKIP_INSTALL_RULES TRUE"
        "CURL_DISABLE_INSTALL TRUE"
        "CURL_ZLIB ON"
        "CPR_CURL_USE_LIBPSL OFF"
        "USE_LIBIDN2 OFF"
        ${SSL_OPTION}
        "BUILD_EXAMPLES FALSE"
)
