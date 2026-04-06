cmake_minimum_required(VERSION 3.16)
project(HospitalManagementSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

add_executable(hms
    src/main.cpp
    src/hospital_system.cpp
)

target_include_directories(hms PRIVATE src)

option(HMS_BUILD_WEB "Build web frontend server" ON)

if(HMS_BUILD_WEB)
    set(HTTPLIB_REQUIRE_OPENSSL OFF CACHE BOOL "" FORCE)
    set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
    set(HTTPLIB_COMPILE OFF CACHE BOOL "" FORCE)
    set(HTTPLIB_INSTALL OFF CACHE BOOL "" FORCE)
    set(JSON_BuildTests OFF CACHE INTERNAL "")

    FetchContent_Declare(
        httplib
        URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.17.3.tar.gz
    )

    FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    )

    FetchContent_MakeAvailable(httplib nlohmann_json)

    add_executable(hms_web
        src/web_server.cpp
        src/hospital_system.cpp
    )

    target_include_directories(hms_web PRIVATE src)
    target_link_libraries(hms_web PRIVATE httplib::httplib nlohmann_json::nlohmann_json)

    if(WIN32)
        target_link_libraries(hms_web PRIVATE ws2_32)
    endif()
endif()

