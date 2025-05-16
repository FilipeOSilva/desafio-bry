cmake_minimum_required(VERSION 3.10)
project(server_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Adiciona warnings (-Wall)
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall)
endif()

# Inclui a pasta de headers para que o compilador encontre os arquivos .h
include_directories(${PROJECT_SOURCE_DIR}/include)

# Define os arquivos fonte
set(SOURCES
    src/main.cpp
    src/RSAKey.cpp
    src/DocSignInfo.cpp
    src/DocToSignature.cpp
)

# Busca bibliotecas OpenSSL
find_package(OpenSSL REQUIRED)

# Busca bibliotecas POCO necessárias
find_package(Poco REQUIRED COMPONENTS Net Util JSON Foundation)

# Cria executável
add_executable(server ${SOURCES})

# Linka as libs
target_link_libraries(server
    Poco::Net
    Poco::Util
    Poco::JSON
    Poco::Foundation
    OpenSSL::SSL
    OpenSSL::Crypto
)