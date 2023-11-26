add_executable(everest_server "common.cpp" "server.cpp")
add_executable(everest_cli "common.cpp" "cli.cpp")

set_target_properties(everest_server PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

set_target_properties(everest_cli PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

CPMAddPackage(
    NAME libien
    GIT_REPOSITORY https://github.com/ien646/libien
    GIT_TAG master
)

target_link_libraries(everest_server PRIVATE libien)
target_link_libraries(everest_cli PRIVATE libien)