cmake_minimum_required(VERSION 3.10)
project(server_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCES
    src/main.cpp
    src/RSAKey.cpp
    src/DocSignInfo.cpp
    src/DocToSignature.cpp
    src/Server.cpp
)

find_package(OpenSSL REQUIRED)

find_package(Poco REQUIRED COMPONENTS Net Util JSON Foundation)

add_executable(server ${SOURCES})

target_link_libraries(server
    Poco::Net
    Poco::Util
    Poco::JSON
    Poco::Foundation
    OpenSSL::SSL
    OpenSSL::Crypto
)