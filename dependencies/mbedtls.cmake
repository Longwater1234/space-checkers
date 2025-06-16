# mbedtls v2.25.0 (Apache-2.0 License)

CPMAddPackage(
    NAME mbedtls
    URL "https://github.com/Mbed-TLS/mbedtls/archive/refs/tags/mbedtls-2.28.10.tar.gz"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    DOWNLOAD_ONLY ON
) 
 
if(mbedtls_ADDED)
    file(GLOB mbedtls_SOURCES ${mbedtls_SOURCE_DIR}/library/*.c)
    add_library(mbedtls STATIC ${mbedtls_SOURCES}) 
    target_include_directories(mbedtls PUBLIC ${mbedtls_SOURCE_DIR}/include)
endif()