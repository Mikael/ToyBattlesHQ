project(AuthServer LANGUAGES CXX)

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

target_link_libraries(AuthServer PUBLIC
    Common
    unofficial::mariadb-connector-cpp::mariadbcpp
)

if (WIN32)
    target_link_libraries(AuthServer PUBLIC ws2_32 wsock32)
endif()

find_package(OpenSSL REQUIRED)
target_link_libraries(AuthServer PRIVATE OpenSSL::Crypto OpenSSL::SSL)

set_target_properties(AuthServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)
