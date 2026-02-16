project(AuthServer LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE AUTH_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/*.h
    ${CMAKE_CURRENT_LIST_DIR}/*.hpp
)

add_executable(AuthServer
    ${AUTH_SOURCES}
)

target_include_directories(AuthServer PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

find_package(CURL REQUIRED)
target_link_libraries(AuthServer PUBLIC
    Common
    unofficial::mariadb-connector-cpp::mariadbcpp
    ${CURL_LIBRARIES}
)

target_include_directories(AuthServer PRIVATE
    ${CURL_INCLUDE_DIRS}
)


if (WIN32)
    target_link_libraries(AuthServer PUBLIC ws2_32 wsock32)
endif()

find_package(OpenSSL REQUIRED)
target_link_libraries(AuthServer PRIVATE OpenSSL::Crypto OpenSSL::SSL)

set_target_properties(AuthServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)