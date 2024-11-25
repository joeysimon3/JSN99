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
    src/shared_memory.c
)

# Executable 1: init_store_pool
add_executable(writeshmem
    src/writeshmem.c
    ${COMMON_SOURCES}
)
target_link_libraries(writeshmem pthread)

# If you have another script with a `main` function, e.g., access_update_pool.c
add_executable(readshmem
    src/readshmem.c
    ${COMMON_SOURCES}
)
target_link_libraries(readshmem pthread)

add_executable(destroyshmem
    src/destroyshmem.c
    ${COMMON_SOURCES}
)
target_link_libraries(destroyshmem pthread)
