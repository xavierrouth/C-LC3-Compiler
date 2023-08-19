cmake_minimum_required(VERSION 3.10)

set(name "LC3 Compiler")
set(version 1.0) # Version, where minor=semester_year, patch=semester_end_month, tweak=revision
set(entrypoints "main") # Entrypoints to run the program
set(assignment_clean_rm) # Generated files that should be removed with "make clean"

# Add color support to our messages.
# See: https://stackoverflow.com/questions/18968979/how-to-get-colorized-output-with-cmake
if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColorReset   "${Esc}[m")
  set(ColorBold    "${Esc}[1m")
  set(ColorRed     "${Esc}[31m")
  set(ColorGreen   "${Esc}[32m")
  set(ColorYellow  "${Esc}[33m")
  set(ColorBlue    "${Esc}[34m")
  set(ColorMagenta "${Esc}[35m")
  set(ColorCyan    "${Esc}[36m")
  set(ColorWhite   "${Esc}[37m")
endif()

# Disable in-source builds (running "cmake ." in the project directory)
# This is bad practice as it pollutes the project directory.
# See: https://dpiepgrass.medium.com/cmake-fights-against-disabling-in-source-builds-ab1d71c1d26f
if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}") 
  message(FATAL_ERROR "${ColorRed}In-source builds are disabled.\n"
    "Create a subdirectory `build/` and use `cmake ..` inside it.\n"
    "${ColorBold}Delete `CMakeCache.txt` and `CMakeFiles/` before you continue.${ColorReset}")
endif()

# Specify compiler and linker.

#set(CMAKE_C_COMPILER "/usr/bin/clang")
#set(CMAKE_LINKER "/usr/bin/clang")

# Define project.   
project(${name} VERSION ${version})

# Specify Debug build.
set(CMAKE_BUILD_TYPE Debug)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wfatal-errors -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wunused-but-set-variable")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -O0 -gdwarf-4")


# Path definitions.
#set(lib_dir ${CMAKE_SOURCE_DIR}/lib)
set(src_dir ${CMAKE_SOURCE_DIR}/src)
set(tests_dir ${CMAKE_SOURCE_DIR}/tests)
set(entry_dir ${CMAKE_SOURCE_DIR}/entry)

# Run CMakeLists in lib_dir to build our required libraries.
#add_subdirectory(${lib_dir})

# Run CMakeLists in src_dir to build the sources.
add_subdirectory(${src_dir})

# Run CMakeLists in test_dir to build the sources.
add_subdirectory(${tests_dir})

# Test executable
add_executable(test ${tests_dir}/main.c)
target_link_libraries(test PRIVATE test_src) # Link source
target_link_libraries(test PRIVATE src)

# Add the entrypoints using our libs and src code.

add_executable(main ${entry_dir}/main.c)
target_link_libraries(main PRIVATE src) # "libs" removed
set_target_properties(main PROPERTIES OUTPUT_NAME "lc3-compile")