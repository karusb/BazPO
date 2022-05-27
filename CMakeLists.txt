# CMakeList.txt : CMake project for CMake, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.12)
project(BazPO VERSION 0.2.0 DESCRIPTION "BazWare Program Options Argument Parser" HOMEPAGE_URL "https://github.com/karusb/BazPO")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Specify BuildType if its not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE MinSizeRel)
endif()

include(GNUInstallDirs)

add_library(BazPO INTERFACE)

target_include_directories(
  BazPO
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_features(BazPO INTERFACE cxx_std_14)

enable_testing()
add_subdirectory(test)
add_subdirectory(manualtest)

install(TARGETS BazPO
        EXPORT BazPO_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

include(CMakePackageConfigHelpers)
write_basic_package_version_file("BazPOConfigVersion.cmake"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/BazPOConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/BazPOConfig.cmake"
  INSTALL_DESTINATION
  ${CMAKE_INSTALL_DATAROOTDIR}/BazPO/cmake)

install(EXPORT BazPO_Targets
        FILE BazPOTargets.cmake
        NAMESPACE BazPO::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/BazPO/cmake)

install(FILES "${PROJECT_BINARY_DIR}/BazPOConfig.cmake"
              "${PROJECT_BINARY_DIR}/BazPOConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/BazPO/cmake)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include DESTINATION include)