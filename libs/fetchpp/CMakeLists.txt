project(fetchpp CXX)
cmake_minimum_required(VERSION 3.4)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_library(fetchpp
  include/fetchpp/fetchpp.hpp
  include/fetchpp/field_arg.hpp
  include/fetchpp/field.hpp

  include/fetchpp/alias/beast.hpp
  include/fetchpp/alias/http.hpp
  include/fetchpp/alias/net.hpp

  src/detail/parse_url.hpp
  src/detail/ssl_client.hpp

  src/detail/parse_url.cpp
  src/detail/ssl_client.cpp

  src/fetchpp.cpp
  src/field_arg.cpp

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