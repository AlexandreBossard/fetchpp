add_executable(test_functional
  helpers/format.cpp
  tests.cpp
  test_process_one.cpp
  main.cpp
)
target_link_libraries(test_functional fetchpp CONAN_PKG::Catch2 CONAN_PKG::jsonformoderncpp)

add_test(test_functional test_functional)
