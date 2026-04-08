# mbedtls v3.6.5 (Apache-2.0 License)

CPMAddPackage(
    NAME mbedtls
    URL "https://github.com/Mbed-TLS/mbedtls/releases/download/mbedtls-3.6.5/mbedtls-3.6.5.tar.bz2"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    OPTIONS "USE_STATIC_MBEDTLS_LIBRARY TRUE" "ENABLE_PROGRAMS OFF" "ENABLE_TESTING OFF"
)

if(mbedtls_ADDED)
    set(MBEDTLS_INCLUDE_DIRS ${mbedtls_SOURCE_DIR}/include)
    set(MBEDTLS_LIBRARIES mbedtls mbedx509 mbedcrypto)
endif()
