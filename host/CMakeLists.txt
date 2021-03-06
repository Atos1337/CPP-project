cmake_minimum_required(VERSION 3.15)
project(Native-messaging-host)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)
include_directories(src)

add_executable(Native-messaging-host
        include/json.hpp
        include/portable-file-dialogs.h
        include/signer.h
        include/ZIP_file_headers.h
        include/ZIP_file_reading.h
        include/ZIP_file_signing.h
        include/ZIP_file_writing.h
        include/zipSigner.hpp
        src/Native-messaging-host.cpp
        src/signer.cpp
	src/ZIP_file_headers.cpp
        src/ZIP_file_reading.cpp
        src/ZIP_file_signing.cpp
        src/ZIP_file_writing.cpp
        src/ZIP_file_headers.cpp
        src/zipSigner.cpp)

set_property(TARGET Native-messaging-host PROPERTY CXX_STANDARD 17)
find_package(OpenSSL REQUIRED)
target_link_libraries(Native-messaging-host OpenSSL::SSL)
find_package(ZLIB)
target_link_libraries(Native-messaging-host ZLIB::ZLIB)
target_link_libraries(Native-messaging-host stdc++fs)
