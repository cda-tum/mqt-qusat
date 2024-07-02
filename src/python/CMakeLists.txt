if(APPLE)
  set(BASEPOINT @loader_path)
else()
  set(BASEPOINT $ORIGIN)
endif()
list(APPEND CMAKE_INSTALL_RPATH ${BASEPOINT} ${BASEPOINT}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

pybind11_add_module(
  py${PROJECT_NAME}
  # Prefer thin LTO if available
  THIN_LTO
  # Optimize the bindings for size
  OPT_SIZE
  # Source code goes here
  bindings.cpp)
target_compile_definitions(py${PROJECT_NAME} PRIVATE Z3_FOUND)
target_link_libraries(py${PROJECT_NAME} PRIVATE ${PROJECT_NAME} MQT::CorePython MQT::ProjectOptions
                                                MQT::ProjectWarnings)

# Install directive for scikit-build-core
install(
  TARGETS py${PROJECT_NAME}
  DESTINATION .
  COMPONENT mqt-${PROJECT_NAME}_Python)
