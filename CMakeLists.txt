cmake_minimum_required(VERSION 3.12)
project(ORB_SLAM3 LANGUAGES CXX C)

# Build type (for single-config generators)
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_definitions(-DCOMPILEDWITHC14)

# Compiler flags per platform
if(MSVC)
  add_compile_definitions(NOMINMAX _USE_MATH_DEFINES)
  add_compile_options(/permissive- /W4 /MP /EHsc /bigobj)
  string(APPEND CMAKE_CXX_FLAGS_RELEASE " /O2")
  string(APPEND CMAKE_C_FLAGS_RELEASE " /O2")
else()
  add_compile_options(-Wall)
  string(APPEND CMAKE_C_FLAGS " -Wall -O3")
  string(APPEND CMAKE_CXX_FLAGS " -Wall -O3")
  string(APPEND CMAKE_C_FLAGS_RELEASE " -march=native")
  string(APPEND CMAKE_CXX_FLAGS_RELEASE " -march=native")
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules")

# Dependencies
find_package(OpenCV 4.4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
message(STATUS "OpenCV version: ${OpenCV_VERSION}")

# Eigen: prefer imported target if available
find_package(Eigen3 3.1.0 REQUIRED NO_MODULE)

find_package(Pangolin REQUIRED)

# Optional RealSense
find_package(realsense2 QUIET)

# Boost serialization & OpenSSL crypto (replace -lboost_serialization -lcrypto)
find_package(Boost REQUIRED COMPONENTS serialization)
find_package(OpenSSL REQUIRED)

# Include dirs
include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include/CameraModels
  ${PROJECT_SOURCE_DIR}/Thirdparty/Sophus
)
# Eigen include for legacy FindEigen3
if(EIGEN3_INCLUDE_DIR)
  include_directories(${EIGEN3_INCLUDE_DIR})
elseif(Eigen3_FOUND)
  # when using imported target, no include needed here
endif()
if(Pangolin_INCLUDE_DIRS)
  include_directories(${Pangolin_INCLUDE_DIRS})
endif()

# Outputs
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
# Examples set their own RUNTIME_OUTPUT_DIRECTORY further below.

# Third-party libs built from source if available
# g2o
if(EXISTS "${PROJECT_SOURCE_DIR}/Thirdparty/g2o/CMakeLists.txt")
  add_subdirectory(Thirdparty/g2o)
endif()
# DBoW2
if(EXISTS "${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/CMakeLists.txt")
  add_subdirectory(Thirdparty/DBoW2)
endif()

# Core library
add_library(${PROJECT_NAME} SHARED
  src/System.cc
  src/Tracking.cc
  src/LocalMapping.cc
  src/LoopClosing.cc
  src/ORBextractor.cc
  src/ORBmatcher.cc
  src/FrameDrawer.cc
  src/Converter.cc
  src/MapPoint.cc
  src/KeyFrame.cc
  src/Atlas.cc
  src/Map.cc
  src/MapDrawer.cc
  src/Optimizer.cc
  src/Frame.cc
  src/KeyFrameDatabase.cc
  src/Sim3Solver.cc
  src/Viewer.cc
  src/ImuTypes.cc
  src/G2oTypes.cc
  src/CameraModels/Pinhole.cpp
  src/CameraModels/KannalaBrandt8.cpp
  src/OptimizableTypes.cpp
  src/MLPnPsolver.cpp
  src/GeometricTools.cc
  src/TwoViewReconstruction.cc
  src/Config.cc
  src/Settings.cc
  include/System.h
  include/Tracking.h
  include/LocalMapping.h
  include/LoopClosing.h
  include/ORBextractor.h
  include/ORBmatcher.h
  include/FrameDrawer.h
  include/Converter.h
  include/MapPoint.h
  include/KeyFrame.h
  include/Atlas.h
  include/Map.h
  include/MapDrawer.h
  include/Optimizer.h
  include/Frame.h
  include/KeyFrameDatabase.h
  include/Sim3Solver.h
  include/Viewer.h
  include/ImuTypes.h
  include/G2oTypes.h
  include/CameraModels/GeometricCamera.h
  include/CameraModels/Pinhole.h
  include/CameraModels/KannalaBrandt8.h
  include/OptimizableTypes.h
  include/MLPnPsolver.h
  include/GeometricTools.h
  include/TwoViewReconstruction.h
  include/SerializationUtils.h
  include/Config.h
  include/Settings.h
)

# On Windows, auto-export all symbols from the DLL (no __declspec(dllexport) in sources)
if(WIN32)
  set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Link third-party libs portably
# Try target names first (when built via add_subdirectory), then fall back to find_library.
set(_dbow2_target "")
if(TARGET DBoW2)
  set(_dbow2_target DBoW2)
else()
  find_library(DBOW2_LIB NAMES DBoW2 dbow2
               PATHS "${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib" "${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/Release" "${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/Debug"
               NO_DEFAULT_PATH)
  if(DBOW2_LIB)
    set(_dbow2_target "${DBOW2_LIB}")
  endif()
endif()

set(_g2o_target "")
if(TARGET g2o)
  set(_g2o_target g2o)
else()
  find_library(G2O_LIB NAMES g2o
               PATHS "${PROJECT_SOURCE_DIR}/Thirdparty/g2o/lib" "${PROJECT_SOURCE_DIR}/Thirdparty/g2o/lib/Release" "${PROJECT_SOURCE_DIR}/Thirdparty/g2o/lib/Debug"
               NO_DEFAULT_PATH)
  if(G2O_LIB)
    set(_g2o_target "${G2O_LIB}")
  endif()
endif()

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    ${OpenCV_LIBS}
    Boost::serialization
    OpenSSL::Crypto
)
# Pangolin (variable or target)
if(TARGET Pangolin::pango)
  target_link_libraries(${PROJECT_NAME} PUBLIC Pangolin::pango)
elseif(Pangolin_LIBRARIES)
  target_link_libraries(${PROJECT_NAME} PUBLIC ${Pangolin_LIBRARIES})
endif()
# Eigen (header-only target if available)
if(TARGET Eigen3::Eigen)
  target_link_libraries(${PROJECT_NAME} PUBLIC Eigen3::Eigen)
endif()
# DBoW2 / g2o (if found)
if(_dbow2_target)
  target_link_libraries(${PROJECT_NAME} PUBLIC ${_dbow2_target})
endif()
if(_g2o_target)
  target_link_libraries(${PROJECT_NAME} PUBLIC ${_g2o_target})
endif()

# RealSense (optional)
if(realsense2_FOUND OR TARGET realsense2::realsense2)
  if(TARGET realsense2::realsense2)
    target_link_libraries(${PROJECT_NAME} PUBLIC realsense2::realsense2)
  elseif(realsense2_LIBRARY)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${realsense2_LIBRARY})
    if(realsense2_INCLUDE_DIR)
      target_include_directories(${PROJECT_NAME} PUBLIC ${realsense2_INCLUDE_DIR})
    endif()
  endif()
endif()

if(MSVC)
  target_compile_definitions(${PROJECT_NAME} PUBLIC _HAS_STD_BYTE=0 WIN32_LEAN_AND_MEAN NOMINMAX _USE_MATH_DEFINES)
endif()

# ===== Examples =====

# RGB-D examples
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples/RGB-D)
add_executable(rgbd_tum Examples/RGB-D/rgbd_tum.cc)
target_link_libraries(rgbd_tum ${PROJECT_NAME})

if(realsense2_FOUND OR TARGET realsense2::realsense2)
  add_executable(rgbd_realsense_D435i Examples/RGB-D/rgbd_realsense_D435i.cc)
  target_link_libraries(rgbd_realsense_D435i ${PROJECT_NAME})
endif()

# RGB-D inertial examples
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples/RGB-D-Inertial)
if(realsense2_FOUND OR TARGET realsense2::realsense2)
  add_executable(rgbd_inertial_realsense_D435i Examples/RGB-D-Inertial/rgbd_inertial_realsense_D435i.cc)
  target_link_libraries(rgbd_inertial_realsense_D435i ${PROJECT_NAME})
endif()

# Stereo examples
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples/Stereo)
add_executable(stereo_kitti Examples/Stereo/stereo_kitti.cc)
target_link_libraries(stereo_kitti ${PROJECT_NAME})

add_executable(stereo_euroc Examples/Stereo/stereo_euroc.cc)
target_link_libraries(stereo_euroc ${PROJECT_NAME})

add_executable(stereo_tum_vi Examples/Stereo/stereo_tum_vi.cc)
target_link_libraries(stereo_tum_vi ${PROJECT_NAME})

if(realsense2_FOUND OR TARGET realsense2::realsense2)
  add_executable(stereo_realsense_t265 Examples/Stereo/stereo_realsense_t265.cc)
  target_link_libraries(stereo_realsense_t265 ${PROJECT_NAME})

  add_executable(stereo_realsense_D435i Examples/Stereo/stereo_realsense_D435i.cc)
  target_link_libraries(stereo_realsense_D435i ${PROJECT_NAME})
endif()

# Monocular examples
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples/Monocular)
add_executable(mono_tum Examples/Monocular/mono_tum.cc)
target_link_libraries(mono_tum ${PROJECT_NAME})

add_executable(mono_kitti Examples/Monocular/mono_kitti.cc)
target_link_libraries(mono_kitti ${PROJECT_NAME})

add_executable(mono_euroc Examples/Monocular/mono_euroc.cc)
target_link_libraries(mono_euroc ${PROJECT_NAME})

add_executable(mono_tum_vi Examples/Monocular/mono_tum_vi.cc)
target_link_libraries(mono_tum_vi ${PROJECT_NAME})

if(realsense2_FOUND OR TARGET realsense2::realsense2)
  add_executable(mono_realsense_t265 Examples/Monocular/mono_realsense_t265.cc)
  target_link_libraries(mono_realsense_t265 ${PROJECT_NAME})

  add_executable(mono_realsense_D435i Examples/Monocular/mono_realsense_D435i.cc)
  target_link_libraries(mono_realsense_D435i ${PROJECT_NAME})
endif()

# Monocular inertial examples
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples/Monocular-Inertial)
add_executable(mono_inertial_euroc Examples/Monocular-Inertial/mono_inertial_euroc.cc)
target_link_libraries(mono_inertial_euroc ${PROJECT_NAME})

add_executable(mono_inertial_tum_vi Examples/Monocular-Inertial/mono_inertial_tum_vi.cc)
target_link_libraries(mono_inertial_tum_vi ${PROJECT_NAME})

if(realsense2_FOUND OR TARGET realsense2::realsense2)
  add_executable(mono_inertial_realsense_t265 Examples/Monocular-Inertial/mono_inertial_realsense_t265.cc)
  target_link_libraries(mono_inertial_realsense_t265 ${PROJECT_NAME})

  add_executable(mono_inertial_realsense_D435i Examples/Monocular-Inertial/mono_inertial_realsense_D435i.cc)
  target_link_libraries(mono_inertial_realsense_D435i ${PROJECT_NAME})
endif()

# Stereo Inertial examples
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples/Stereo-Inertial)
add_executable(stereo_inertial_euroc Examples/Stereo-Inertial/stereo_inertial_euroc.cc)
target_link_libraries(stereo_inertial_euroc ${PROJECT_NAME})

add_executable(stereo_inertial_tum_vi Examples/Stereo-Inertial/stereo_inertial_tum_vi.cc)
target_link_libraries(stereo_inertial_tum_vi ${PROJECT_NAME})

if(realsense2_FOUND OR TARGET realsense2::realsense2)
  add_executable(stereo_inertial_realsense_t265 Examples/Stereo-Inertial/stereo_inertial_realsense_t265.cc)
  target_link_libraries(stereo_inertial_realsense_t265 ${PROJECT_NAME})

  add_executable(stereo_inertial_realsense_D435i Examples/Stereo-Inertial/stereo_inertial_realsense_D435i.cc)
  target_link_libraries(stereo_inertial_realsense_D435i ${PROJECT_NAME})
endif()

# Calibration examples
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples/Calibration)
if(realsense2_FOUND OR TARGET realsense2::realsense2)
  add_executable(recorder_realsense_D435i Examples/Calibration/recorder_realsense_D435i.cc)
  target_link_libraries(recorder_realsense_D435i ${PROJECT_NAME})

  add_executable(recorder_realsense_T265 Examples/Calibration/recorder_realsense_T265.cc)
  target_link_libraries(recorder_realsense_T265 ${PROJECT_NAME})
endif()

# ===== Old examples (kept as-is) =====

# RGB-D old
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples_old/RGB-D)
add_executable(rgbd_tum_old Examples_old/RGB-D/rgbd_tum.cc)
target_link_libraries(rgbd_tum_old ${PROJECT_NAME})

if(realsense2_FOUND OR TARGET realsense2::realsense2)
  add_executable(rgbd_realsense_D435i_old Examples_old/RGB-D/rgbd_realsense_D435i.cc)
  target_link_libraries(rgbd_realsense_D435i_old ${PROJECT_NAME})
endif()

# RGB-D inertial old
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples_old/RGB-D-Inertial)
if(realsense2_FOUND OR TARGET realsense2::realsense2)
  add_executable(rgbd_inertial_realsense_D435i_old Examples_old/RGB-D-Inertial/rgbd_inertial_realsense_D435i.cc)
  target_link_libraries(rgbd_inertial_realsense_D435i_old ${PROJECT_NAME})
endif()

# Stereo old
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples_old/Stereo)
add_executable(stereo_kitti_old Examples_old/Stereo/stereo_kitti.cc)
target_link_libraries(stereo_kitti_old ${PROJECT_NAME})

add_executable(stereo_euroc_old Examples_old/Stereo/stereo_euroc.cc)
target_link_libraries(stereo_euroc_old ${PROJECT_NAME})

add_executable(stereo_tum_vi_old Examples_old/Stereo/stereo_tum_vi.cc)
target_link_libraries(stereo_tum_vi_old ${PROJECT_NAME})

if(realsense2_FOUND OR TARGET realsense2::realsense2)
  add_executable(stereo_realsense_t265_old Examples_old/Stereo/stereo_realsense_t265.cc)
  target_link_libraries(stereo_realsense_t265_old ${PROJECT_NAME})

  add_executable(stereo_realsense_D435i_old Examples_old/Stereo/stereo_realsense_D435i.cc)
  target_link_libraries(stereo_realsense_D435i_old ${PROJECT_NAME})
endif()

# Monocular old
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples_old/Monocular)
add_executable(mono_tum_old Examples_old/Monocular/mono_tum.cc)
target_link_libraries(mono_tum_old ${PROJECT_NAME})

add_executable(mono_kitti_old Examples_old/Monocular/mono_kitti.cc)
target_link_libraries(mono_kitti_old ${PROJECT_NAME})

add_executable(mono_euroc_old Examples_old/Monocular/mono_euroc.cc)
target_link_libraries(mono_euroc_old ${PROJECT_NAME})

add_executable(mono_tum_vi_old Examples_old/Monocular/mono_tum_vi.cc)
target_link_libraries(mono_tum_vi_old ${PROJECT_NAME})

if(realsense2_FOUND OR TARGET realsense2::realsense2)
  add_executable(mono_realsense_t265_old Examples_old/Monocular/mono_realsense_t265.cc)
  target_link_libraries(mono_realsense_t265_old ${PROJECT_NAME})

  add_executable(mono_realsense_D435i_old Examples_old/Monocular/mono_realsense_D435i.cc)
  target_link_libraries(mono_realsense_D435i_old ${PROJECT_NAME})
endif()

# Monocular inertial old
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples_old/Monocular-Inertial)
add_executable(mono_inertial_euroc_old Examples_old/Monocular-Inertial/mono_inertial_euroc.cc)
target_link_libraries(mono_inertial_euroc_old ${PROJECT_NAME})

add_executable(mono_inertial_tum_vi_old Examples_old/Monocular-Inertial/mono_inertial_tum_vi.cc)
target_link_libraries(mono_inertial_tum_vi_old ${PROJECT_NAME})

if(realsense2_FOUND OR TARGET realsense2::realsense2)
  add_executable(mono_inertial_realsense_t265_old Examples_old/Monocular-Inertial/mono_inertial_realsense_t265.cc)
  target_link_libraries(mono_inertial_realsense_t265_old ${PROJECT_NAME})

  add_executable(mono_inertial_realsense_D435i_old Examples_old/Monocular-Inertial/mono_inertial_realsense_D435i.cc)
  target_link_libraries(mono_inertial_realsense_D435i_old ${PROJECT_NAME})
endif()

# Stereo Inertial old
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples_old/Stereo-Inertial)
add_executable(stereo_inertial_euroc_old Examples_old/Stereo-Inertial/stereo_inertial_euroc.cc)
target_link_libraries(stereo_inertial_euroc_old ${PROJECT_NAME})

add_executable(stereo_inertial_tum_vi_old Examples_old/Stereo-Inertial/stereo_inertial_tum_vi.cc)
target_link_libraries(stereo_inertial_tum_vi_old ${PROJECT_NAME})

if(realsense2_FOUND OR TARGET realsense2::realsense2)
  add_executable(stereo_inertial_realsense_t265_old Examples_old/Stereo-Inertial/stereo_inertial_realsense_t265.cc)
  target_link_libraries(stereo_inertial_realsense_t265_old ${PROJECT_NAME})

  add_executable(stereo_inertial_realsense_D435i_old Examples_old/Stereo-Inertial/stereo_inertial_realsense_D435i_old.cc)
  # ^ if filename is actually ...D435i.cc in your repo, adjust accordingly.
  # (Kept original name if it was ...D435i.cc)
endif()
