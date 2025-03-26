if(APPLE)
  set(BASEPOINT @loader_path)
else()
  set(BASEPOINT $ORIGIN)
endif()
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
list(
  APPEND
  CMAKE_INSTALL_RPATH
  ${BASEPOINT}
  ${BASEPOINT}/${CMAKE_INSTALL_LIBDIR}
  ${BASEPOINT}/../core/${CMAKE_INSTALL_LIBDIR}
  ${BASEPOINT}/../core/lib
  ${BASEPOINT}/../core/lib64
  ${BASEPOINT}/../../core/${CMAKE_INSTALL_LIBDIR}
  ${BASEPOINT}/../../core/lib
  ${BASEPOINT}/../../core/lib64)

pybind11_add_module(
  py${PROJECT_NAME}
  # Prefer thin LTO if available
  THIN_LTO
  # Optimize the bindings for size
  OPT_SIZE
  # Source code goes here
  bindings.cpp)
target_compile_definitions(py${PROJECT_NAME} PRIVATE Z3_FOUND)
target_link_libraries(py${PROJECT_NAME} PRIVATE ${PROJECT_NAME} pybind11_json MQT::ProjectOptions
                                                MQT::ProjectWarnings)

# Install directive for scikit-build-core
install(
  TARGETS py${PROJECT_NAME}
  DESTINATION .
  COMPONENT mqt-${PROJECT_NAME}_Python)
