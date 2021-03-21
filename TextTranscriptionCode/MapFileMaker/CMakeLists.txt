find_package(Boost
	COMPONENTS filesystem)

add_executable(MapFileMaker DirectoryFileFetcher.cpp ImageValidator.cpp Main.cpp)
target_include_directories(MapFileMaker PUBLIC ${Boost_INCLUDE_DIR} /usr/local/opencv-3.1.0/include)
target_link_directories(MapFileMaker PUBLIC ${Boost_LIBRARY_DIR} ${OpenCV_LIBS})
target_link_libraries(MapFileMaker ${Boost_LIBRARIES} ${OpenCV_LIBS})

