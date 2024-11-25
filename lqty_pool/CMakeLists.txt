cmake_minimum_required(VERSION 3.10)

# Define the project
project(lqty_pool LANGUAGES C)

# Specify the C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Include the src directory for headers
include_directories(${CMAKE_SOURCE_DIR}/src)

# Collect source files for shared functionality
set(COMMON_SOURCES
    src/liquidity_pool.c
    src/uniswapv2.c
)

# Executable 1: init_store_pool
add_executable(init_store_pool
    src/init_store_pool.c
    ${COMMON_SOURCES}
)
target_link_libraries(init_store_pool pthread)

# If you have another script with a `main` function, e.g., access_update_pool.c
add_executable(access_update_pool
    src/access_update_pool.c
    ${COMMON_SOURCES}
)
target_link_libraries(access_update_pool pthread)

add_executable(cleanup
    src/cleanup.c
    ${COMMON_SOURCES}
)
target_link_libraries(cleanup pthread)
