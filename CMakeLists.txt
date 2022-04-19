# CMakeList.txt : CMake project for CMake, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)
project(BazPO VERSION 1.0.0 DESCRIPTION "BazWare Program Options Argument Parser")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Specify BuildType if its not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE MinSizeRel)
endif()

include(GNUInstallDirs)
add_library(BazPO STATIC
    ProgramOptions/BazPO.cpp
    ProgramOptions/BazPO.hpp
)


set_target_properties(BazPO PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER ProgramOptions/BazPO.hpp)
configure_file(BazPO.pc.in BazPO.pc @ONLY)

install(TARGETS BazPO
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_BINARY_DIR}/BazPO.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

set(CMAKE_SKIP_BUILD_RPATH FALSE)

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
add_executable(BazPOManual ProgramOptionsManual/ProgramOptionsManual.cpp)

target_include_directories(BazPOManual PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/ProgramOptions"
                          "${CMAKE_INSTALL_LIBDIR}"
                          )
#TODO: Remove
install(TARGETS BazPOManual
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

#Tests
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

add_executable(
  ProgramOptionsTest
  ProgramOptionsTest/test.cpp
)
target_link_libraries(
  ProgramOptionsTest
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(ProgramOptionsTest)
