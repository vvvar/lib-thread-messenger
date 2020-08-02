project(lib-thread-messenger)

cmake_minimum_required(VERSION 3.17)

add_library(${PROJECT_NAME} STATIC
  src/Channel.cpp
  src/Messenger.cpp
  src/Transport.cpp
  include/${PROJECT_NAME}/Channel.hpp
  include/${PROJECT_NAME}/Logging.hpp
  include/${PROJECT_NAME}/Message.hpp
  include/${PROJECT_NAME}/MessageQ.hpp
  include/${PROJECT_NAME}/Messenger.hpp
  include/${PROJECT_NAME}/ThreadMessenger.hpp
  include/${PROJECT_NAME}/Transport.hpp
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
