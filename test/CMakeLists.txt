


include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)




# Convenience function to add tests
function(add_json_parser_test test_name)
  add_executable(${test_name} ${test_name}.cpp)
  target_link_libraries(${test_name} json_parser_lib gtest gtest_main)
  add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# Add tests
add_json_parser_test(test1)
#add_json_parser_test(test2)

enable_testing()
