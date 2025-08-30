cmake_minimum_required(VERSION 3.12)
project(DBoW2 LANGUAGES CXX)

# Config type for single-config generators
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Flags
if(MSVC)
  add_compile_options(/permissive- /W4 /EHsc /bigobj)
  string(APPEND CMAKE_CXX_FLAGS_RELEASE " /O2")
else()
  add_compile_options(-Wall)
  string(APPEND CMAKE_CXX_FLAGS " -O3 -march=native")
endif()

set(HDRS_DBOW2
  DBoW2/BowVector.h
  DBoW2/FORB.h
  DBoW2/FClass.h
  DBoW2/FeatureVector.h
  DBoW2/ScoringObject.h
  DBoW2/TemplatedVocabulary.h
)
set(SRCS_DBOW2
  DBoW2/BowVector.cpp
  DBoW2/FORB.cpp
  DBoW2/FeatureVector.cpp
  DBoW2/ScoringObject.cpp
)

set(HDRS_DUTILS
  DUtils/Random.h
  DUtils/Timestamp.h
)
set(SRCS_DUTILS
  DUtils/Random.cpp
  DUtils/Timestamp.cpp
)

find_package(OpenCV 4 QUIET)
if(NOT OpenCV_FOUND)
  find_package(OpenCV 3.0 REQUIRED)
endif()

# Boost::serialization for BowVector.h
find_package(Boost REQUIRED COMPONENTS serialization)

add_library(DBoW2 STATIC
  ${SRCS_DBOW2}
  ${SRCS_DUTILS}
)
# public includes so dependents inherit them
target_include_directories(DBoW2
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(DBoW2
  PUBLIC
    ${OpenCV_LIBS}
    Boost::serialization
)

# On non-Windows, keep PIC even for static to satisfy some toolchains
if(NOT WIN32)
  set_target_properties(DBoW2 PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# Old variable some parents expect
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
