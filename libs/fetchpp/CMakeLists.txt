project(fetchpp CXX)
cmake_minimum_required(VERSION 3.4)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_library(fetchpp
  include/fetchpp/fetchpp.hpp

  src/fetchpp.cpp
  src/detail/parse_url.cpp
)

target_include_directories(fetchpp
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(fetchpp PUBLIC CONAN_PKG::fmt CONAN_PKG::boost CONAN_PKG::openssl)


if(ENABLE_TESTING)
  add_subdirectory(tests)
endif()
