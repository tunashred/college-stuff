cmake_minimum_required(VERSION 3.0.0)

# Set C and C++ standards
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project name and version
project(security-lab VERSION 0.1.0 LANGUAGES C CXX)

# Include header files
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find all source files
file(GLOB C_SOURCES "src/*.c")

set(MAIN_FILE_PATH "${CMAKE_SOURCE_DIR}/src/main.c")

# Removing main.c because it would link to other executables
list(REMOVE_ITEM C_SOURCES "${MAIN_FILE_PATH}")

# Set executable name
set(EXECUTABLE "runme")

# Add executable target
add_executable(${EXECUTABLE} ${C_SOURCES} ${MAIN_FILE_PATH})

#
# Compile flags for any build type
#
set(COMPILE_FLAGS
    -Wall
    -Wextra
    -pthread
    -Wshadow
    -Wcast-align
    -Wwrite-strings
    -Wcast-qual
    -Wconversion
    -Wunreachable-code
    -fstack-usage
)

# Add compile flags
target_compile_options(${EXECUTABLE} PRIVATE ${COMPILE_FLAGS})

# Link libs
target_link_options(${EXECUTABLE} PRIVATE -lm)

# Release specific flags
set(RELEASE_FLAGS
    -D_FORTIFY_SOURCE=2
    -O3
)

# Debug specific flags
set(DEBUG_FLAGS
    -g
    -O0
    -fverbose-asm
    -finstrument-functions
    -fanalyzer
    -fno-inline
)

function(generate_assembly_for_file SRC_FILE)
    # Get file name without extension
    get_filename_component(FILE_NAME ${SRC_FILE} NAME_WE)
    add_custom_target(
        ${FILE_NAME}_assembly
        COMMAND ${CMAKE_C_COMPILER} ${DEBUG_FLAGS} -I${CMAKE_SOURCE_DIR}/include -S ${SRC_FILE} -o ${CMAKE_CURRENT_BINARY_DIR}/${FILE_NAME}.s
        COMMENT "Generating assembly file for ${FILE_NAME}.c"
    )
endfunction()

# Extra compile flags for release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(${RELEASE_FLAGS})

# Extra compile flags for debug build
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(${DEBUG_FLAGS})

    # Generate assembly files
    foreach(SRC_FILE IN LISTS C_SOURCES)
        generate_assembly_for_file(${SRC_FILE})
    endforeach()
endif()

##################################################################
# Commentary on compile flags
#
# Common flags
# -Wall: Enable all warnings
# -pthread: Enable pthread support
# -Wshadow: Warns if a local variable shadows a function parameter or global variable, or when a lib function is shadowed
# -Wpointer-arith: Warns about questionable pointer operations or pointer arithmetic with void* type or function pointers
# -Wcast-align: Warns for potentially unsafe pointer casts related to strict pointer alignment
# -Wwrite-strings: Warns if a string literal is assigned to a char*
# -Wcast-qual: Warns when casting removes type qualifiers such as const or volatile
# -Wconversion: Warns about implicit type conversions that can lead to data loss
# -Wunreachable-code: Detects code that will never be executed
# -fstack-usage: Generates .su files for each source file which contains stack usage info of each function
#
# Link flags
# -lm: needed for math.h
# -pg: For gprof
# -lprofiler: For gperftools
#
# Release flags
# -D_FORTIFY_SOURCE=2: Runtime buffer overflow detection
# -O3: Max optimization level
#
# Debug flags
# -g: Include debug info
# -O0: No optimizations
# -fverbose-asm: Puts extra comments to generated assembly code to make it more readable
# -finstrument-functions: Inserts hooks for each function in the program
# -fanalyzer: Static analyser at compile time
# -fno-inline: Prevents compiler from inlining any function
