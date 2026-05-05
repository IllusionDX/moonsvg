cmake_minimum_required(VERSION 3.10)

project(MoonSVG C)

# CMake needs *.c files to do something useful
configure_file(src/moonsvg.h ${CMAKE_CURRENT_BINARY_DIR}/moonsvg.c)
configure_file(src/moonsvgrast.h ${CMAKE_CURRENT_BINARY_DIR}/moonsvgrast.c)

add_library(moonsvg ${CMAKE_CURRENT_BINARY_DIR}/moonsvg.c)

find_library(MATH_LIBRARY m) # Business as usual
if(MATH_LIBRARY)
    target_link_libraries(moonsvg PUBLIC ${MATH_LIBRARY})
endif()

target_include_directories(moonsvg PUBLIC $<INSTALL_INTERFACE:include/moonsvg>)
target_compile_definitions(moonsvg PRIVATE MOONSVG_IMPLEMENTATION)

# Same for moonsvgrast
add_library(moonsvgrast ${CMAKE_CURRENT_BINARY_DIR}/moonsvgrast.c)
target_link_libraries(moonsvgrast PUBLIC moonsvg)
target_include_directories(moonsvgrast PRIVATE src)
target_compile_definitions(moonsvgrast PRIVATE MOONSVGRAST_IMPLEMENTATION)

# Installation and export:

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION 1.0
    COMPATIBILITY AnyNewerVersion
)

install(TARGETS moonsvg moonsvgrast
        EXPORT ${PROJECT_NAME}Targets
)

export(EXPORT ${PROJECT_NAME}Targets 
       FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake" 
       NAMESPACE ${PROJECT_NAME}::
)

set(ConfigPackageLocation lib/cmake/${PROJECT_NAME})

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION ${ConfigPackageLocation}
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(
    FILES
      src/moonsvg.h
      src/moonsvgrast.h
    DESTINATION
      include/moonsvg
  )

install(EXPORT ${PROJECT_NAME}Targets
  FILE
    ${PROJECT_NAME}Targets.cmake
  NAMESPACE
    ${PROJECT_NAME}::
  DESTINATION
    ${ConfigPackageLocation}
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION
    ${ConfigPackageLocation}
)
