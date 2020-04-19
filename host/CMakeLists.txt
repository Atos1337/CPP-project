cmake_minimum_required(VERSION 3.15)
project(host)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)
include_directories(src)

add_executable(host
        include/json.hpp
        include/portable-file-dialogs.h
        include/signer.h
        include/Test_function_for_sign.h
        include/ZIP_file_headers.h
        include/ZIP_file_reading.h
        include/ZIP_file_signing.h
        include/ZIP_file_writing.h
        include/zipSigner.hpp
        src/Native-messaging-host.cpp
        src/Sign_example.cpp
        src/signer.cpp
        src/signerExample.cpp
        src/Test_function_for_sign.cpp
        src/ZIP_file_reading.cpp
        src/ZIP_file_signing.cpp
        src/ZIP_file_writing.cpp
        src/zipSigner.cpp)
