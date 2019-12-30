cmake_minimum_required(VERSION 3.4.3)
set(CMAKE_CXX_STANDARD 14)

project(SceneTextOCR)

set(CNTK_DIR ../ExternalLibs/CNTK)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

include_directories(${CNTK_DIR}/Source/CNTKv2LibraryDll/API/ /usr/local/opencv-3.1.0/include)
link_directories(${CNTK_DIR}/build/release/lib)

set(CNTK_REQUIRED_LIBS Cntk.Core-2.6.so Cntk.Eval-2.6.so Cntk.Math-2.6.so ${OpenCV_LIBS})

add_library(SceneTextOCR ResNet.cpp TextTranscriptionModel.cpp)

add_executable(SceneTextOCRTrainer main.cpp)
target_link_libraries(SceneTextOCRTrainer SceneTextOCR ${CNTK_REQUIRED_LIBS})

add_executable(SceneTextOCREvaluator evaluator.cpp)
target_link_libraries(SceneTextOCREvaluator SceneTextOCR ${CNTK_REQUIRED_LIBS} ${CMAKE_THREAD_LIBS_INIT})
