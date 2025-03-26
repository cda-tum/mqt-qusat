package_add_test(${PROJECT_NAME}_test ${PROJECT_NAME} test_satencoder.cpp)
target_link_libraries(${PROJECT_NAME}_test PRIVATE MQT::CoreAlgorithms)
