#Manual
add_executable(BazPOManual BazPOManual.cpp)

target_include_directories(BazPOManual PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/BazPO"
                          "${CMAKE_INSTALL_LIBDIR}"
                          )
