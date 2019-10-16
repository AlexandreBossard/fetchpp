project(fetchpp CXX)
cmake_minimum_required(VERSION 3.4)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_library(fetchpp
  include/fetchpp/fetchpp.hpp

  src/fetchpp.cpp
)

target_include_directories(fetchpp
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(fetchpp PUBLIC CONAN_PKG::fmt CONAN_PKG::boost_beast)


if(ENABLE_TESTING)
  add_subdirectory(tests)
endif()
