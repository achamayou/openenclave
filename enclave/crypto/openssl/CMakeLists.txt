# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

add_enclave_library(
  oecryptoopenssl
  STATIC
  ${PROJECT_SOURCE_DIR}/3rdparty/mbedtls/mbedtls/library/oid.c
  ${PROJECT_SOURCE_DIR}/common/asn1.c
  ${PROJECT_SOURCE_DIR}/common/cert.c
  ${PROJECT_SOURCE_DIR}/common/kdf.c
  ${PROJECT_SOURCE_DIR}/common/crypto/openssl/asn1.c
  ${PROJECT_SOURCE_DIR}/common/crypto/openssl/cert.c
  ${PROJECT_SOURCE_DIR}/common/crypto/openssl/cmac.c
  ${PROJECT_SOURCE_DIR}/common/crypto/openssl/crl.c
  ${PROJECT_SOURCE_DIR}/common/crypto/openssl/ec.c
  ${PROJECT_SOURCE_DIR}/common/crypto/openssl/hmac.c
  ${PROJECT_SOURCE_DIR}/common/crypto/openssl/key.c
  ${PROJECT_SOURCE_DIR}/common/crypto/openssl/rsa.c
  ${PROJECT_SOURCE_DIR}/common/crypto/openssl/sha.c
  cert.c
  gcm.c
  init.c)

maybe_build_using_clangw(oecryptoopenssl)

enclave_enable_code_coverage(oecryptoopenssl)

enclave_include_directories(
  oecryptoopenssl PRIVATE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/3rdparty/mbedtls/mbedtls/include>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/3rdparty/mbedtls>)

enclave_link_libraries(oecryptoopenssl PUBLIC openssl)

# Enforce the correct build dependency such that OpenSSL will be built first.
add_enclave_dependencies(oecryptoopenssl openssl)

set_enclave_property(TARGET oecryptoopenssl PROPERTY ARCHIVE_OUTPUT_DIRECTORY
                     ${OE_LIBDIR}/openenclave/enclave)

install_enclaves(
  TARGETS
  oecryptoopenssl
  EXPORT
  openenclave-targets
  ARCHIVE
  DESTINATION
  ${CMAKE_INSTALL_LIBDIR}/openenclave/enclave)
