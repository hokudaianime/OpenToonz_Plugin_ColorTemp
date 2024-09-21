cmake_minimum_required(VERSION 3.29)
project(ColorTemperature)

set(PLUGIN_NAME ColorTemperature)
set(PLUGIN_VENDOR HokudaiAnime)

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(PLATFORM1 32)
        set(PLATFORM2)
    else()
        set(PLATFORM1 64)
        set(PLATFORM2 _64)
    endif()

    if(PLATFORM1 EQUAL 64)
        add_definitions(-Dx64)
    endif()

    find_package(OpenMP REQUIRED)
    if(OpenMP_FOUND)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    endif()
    set(PLUGIN_UTILITY_LIB "${CMAKE_CURRENT_SOURCE_DIR}/opentoonz_plugin_utility/lib/${CMAKE_CFG_INTDIR}/libopentoonz_plugin_utility.lib")
endif(WIN32)

if(APPLE)
    set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR TRUE)
    set(CMAKE_INSTALL_NAME_DIR "@loader_path")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks")
    set(PLUGIN_UTILITY_LIB "${CMAKE_CURRENT_SOURCE_DIR}/opentoonz_plugin_utility/lib/${CMAKE_CFG_INTDIR}/libopentoonz_plugin_utility.a")
endif(APPLE)

find_package(OpenCV REQUIRED)
set(LIBS ${OpenCV_LIBS} ${PLUGIN_UTILITY_LIB})

add_subdirectory(opentoonz_plugin_utility)

set(SOURCES src/main.cpp)

add_library(${PLUGIN_NAME} SHARED ${HEADERS} ${SOURCES})
add_dependencies(${PLUGIN_NAME} opentoonz_plugin_utility)
target_include_directories(${PLUGIN_NAME} PRIVATE
    opentoonz_plugin_utility/include 
    opentoonz_plugin_utility/plugin_sdk/core 
    "${OpenCV_INCLUDE_DIRS}")

set_target_properties(${PLUGIN_NAME} PROPERTIES
    PREFIX "${PLUGIN_VENDOR}_"
    SUFFIX "_${CMAKE_SYSTEM_NAME}.plugin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

target_compile_definitions(${PLUGIN_NAME} PRIVATE PLUGIN_NAME=${PLUGIN_NAME} PRIVATE PLUGIN_VENDOR=${PLUGIN_VENDOR})

target_link_libraries(${PLUGIN_NAME} ${LIBS})