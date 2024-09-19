cmake_minimum_required(VERSION 3.10)

project(YOLO)

add_library(${PROJECT_NAME}
    SHARED
    ${PROJECT_SOURCE_DIR}/src/YOLO.cpp
    ${PROJECT_SOURCE_DIR}/src/YOLOImpl.cpp
    ${CMAKE_SOURCE_DIR}/snpetask/SNPETask.cpp
)

target_link_libraries(${PROJECT_NAME}
    SNPE
    ${PTHREAD_DL_LIBS}
    fmt::fmt
    ${OpenCV_LIBS}
    ${spdlog_LIBRARIES}
)

install(
    TARGETS YOLO
    LIBRARY DESTINATION /opt/thundersoft/algs/lib
)
