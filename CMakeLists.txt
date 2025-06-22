cmake_minimum_required(VERSION 3.8)
project(ExpressPPApp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)


set(OPENSSL_SSL_LIBRARY "")
set(OPENSSL_CRYPTO_LIBRARY "")

set(ZLIB_BUILD_SHARED OFF)
set(ZLIB_BUILD_STATIC ON)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/zlib)

if(WIN32)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  
  set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
  set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(OPENSSL_LIB_DIR "${OPENSSL_ROOT_DIR}/lib/VC/x64/MDd")
  else()
    set(OPENSSL_LIB_DIR "${OPENSSL_ROOT_DIR}/lib/VC/x64/MD")
  endif()

  set(OPENSSL_SSL_LIBRARY "${OPENSSL_LIB_DIR}/libssl.lib")
  set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_LIB_DIR}/libcrypto.lib")

  message(STATUS "OPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR}")
  message(STATUS "OPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR}")
  message(STATUS "OPENSSL_SSL_LIBRARY=${OPENSSL_SSL_LIBRARY}")
  message(STATUS "OPENSSL_CRYPTO_LIBRARY=${OPENSSL_CRYPTO_LIBRARY}")

endif()

if(UNIX AND NOT APPLE)
  set(OPENSSL_INCLUDE_DIR "/usr/include/")
  set(OPENSSL_SSL_LIBRARY "/usr/lib/x86_64-linux-gnu/libssl.so")
  set(OPENSSL_CRYPTO_LIBRARY "/usr/lib/x86_64-linux-gnu/libcrypto.so")
endif()

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(server ${SOURCES})
target_include_directories(server PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(server PUBLIC ${PROJECT_SOURCE_DIR}/vendor/nodepp/include)
target_include_directories(server PRIVATE ${PROJECT_SOURCE_DIR}/vendor/zlib)

target_link_libraries(server PRIVATE ZLIB::ZLIBSTATIC)

if (WIN32)
  target_include_directories(server PRIVATE ${OPENSSL_INCLUDE_DIR})
  
  target_link_libraries(server PRIVATE ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY}) 
  target_link_libraries(server PRIVATE Ws2_32)
elseif(UNIX AND NOT APPLE)
  target_include_directories(server PRIVATE ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(server PRIVATE ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
endif()
