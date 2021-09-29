project(VK)
cmake_minimum_required(VERSION 3.0)

find_package(Kumir2 REQUIRED)
find_package(CURL REQUIRED)
kumir2_use_qt(Core)
link_libraries("${CMAKE_SOURCE_DIR}/src/3rdparty/JSON/jsoncpp.lib")

set(SOURCES
	api.cpp
)

set(HEADERS
	api.h
    vk.h
	Friends.h
	Groups.h
	Messages.h
	Photos.h
	Users.h
	Video.h
)	

set(MOC_HEADERS
	api.h
    vk.h
)

add_library(VK_LIB STATIC ${SOURCES})

kumir2_wrap_cpp(MOC_SOURCES ${MOC_HEADERS})

kumir2_add_library(
	NAME VK
	SOURCES     ${SOURCES} ${MOC_SOURCES}
	LIBRARIES   ${QT_LIBRARIES} ${JSON_LIB}
)