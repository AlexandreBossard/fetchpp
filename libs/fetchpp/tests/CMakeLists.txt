add_executable(test_functional
  tests.cpp
  main.cpp
)
target_link_libraries(test_functional fetchpp CONAN_PKG::Catch2)

add_test(NAME test_functional COMMAND test_functional)
