# mbedtls v2.28 (Apache-2.0 License)

CPMAddPackage(
    NAME mbedtls
    URL "https://github.com/Mbed-TLS/mbedtls/releases/download/mbedtls-3.6.5/mbedtls-3.6.5.tar.bz2"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    DOWNLOAD_ONLY ON
) 
 
if(mbedtls_ADDED)
    file(GLOB mbedtls_SOURCES ${mbedtls_SOURCE_DIR}/library/*.c)
    add_library(mbedtls STATIC ${mbedtls_SOURCES}) 
    target_include_directories(mbedtls PUBLIC ${mbedtls_SOURCE_DIR}/include)
endif() 