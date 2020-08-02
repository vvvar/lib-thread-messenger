project(lib-thread-messenger)

cmake_minimum_required(VERSION 3.17)

add_library(${PROJECT_NAME} STATIC
  src/Messenger.cpp
  include/${PROJECT_NAME}/ThreadMessenger.hpp
  include/${PROJECT_NAME}/Messenger.hpp
)

target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(${PROJECT_NAME}
	PRIVATE
		cxx_std_11
)
