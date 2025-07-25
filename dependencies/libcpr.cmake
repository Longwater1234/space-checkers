# add libcpr (simpler CURL) v1.11.3

SET(SSL_OPTION "CPR_FORCE_OPENSSL_BACKEND TRUE")
if(WIN32)
   SET(SSL_OPTION "CPR_FORCE_WINSSL_BACKEND TRUE")
endif()

# dont upgrade, will require MESON to build!
CPMAddPackage(
    NAME cpr
    URL    "https://github.com/libcpr/cpr/archive/refs/tags/1.11.3.tar.gz"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    OPTIONS "BUILD_SHARED_LIBS FALSE" "CURL_USE_LIBPSL OFF" "USE_LIBIDN2 OFF" ${SSL_OPTION}
) 
