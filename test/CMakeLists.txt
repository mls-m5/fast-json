
add_executable(
  json_tests
  mls-unit-test/test_main.cpp
  test1.cpp
  parser_test.cpp
  serialization_macro_test.cpp
)

target_link_libraries(json_tests json_parser_lib)
add_test(NAME json_tests COMMAND json_tests)
