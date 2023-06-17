# Path definitions.
set(tests_dir ${CMAKE_CURRENT_SOURCE_DIR})

# Add overall src library.
file(GLOB test_sources CONFIGURE_DEPENDS ${tests_dir}/*.c)
add_library(test_src ${test_sources})
target_link_libraries(test_src PRIVATE src)
target_include_directories(test_src PUBLIC ${tests_dir})