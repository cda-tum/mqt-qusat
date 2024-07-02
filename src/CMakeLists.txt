# main project library
add_library(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/include/SatEncoder.hpp
                            ${PROJECT_SOURCE_DIR}/include/Statistics.hpp SatEncoder.cpp)

# set include directories
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include
                                                  ${PROJECT_BINARY_DIR}/include)

# link to the MQT::Core libraries
target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC MQT::Core nlohmann_json::nlohmann_json
  PRIVATE MQT::ProjectOptions MQT::ProjectWarnings)

# add z3 SMT solver
target_link_libraries(${PROJECT_NAME} PUBLIC z3::z3lib)

# add MQT alias
add_library(MQT::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if(BUILD_MQT_QUSAT_BINDINGS)
  add_subdirectory(python)
endif()
