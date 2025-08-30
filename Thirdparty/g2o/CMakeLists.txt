cmake_minimum_required(VERSION 3.12)
project(g2o LANGUAGES C CXX)

# ---- Build type (for single-config generators) ----
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
message(STATUS "BUILD TYPE: ${CMAKE_BUILD_TYPE}")

# ---- Library type (static on MSVC recommended) ----
if(MSVC)
  set(G2O_LIB_TYPE STATIC)
else()
  set(G2O_LIB_TYPE SHARED)
endif()

# ---- Output dirs ----
set(g2o_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib" CACHE PATH "Target for the libraries")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${g2o_LIBRARY_OUTPUT_DIRECTORY}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${g2o_LIBRARY_OUTPUT_DIRECTORY}")
# (Executables not produced here; RUNTIME dir unused)

# ---- Compiler flags ----
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  # Remove any GCC leftovers that parent projects might have put in
  foreach(flag_var CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELEASE)
    if(DEFINED ${flag_var})
      string(REPLACE "-Wall" "" ${flag_var} "${${flag_var}}")
      string(REPLACE "-Wextra" "" ${flag_var} "${${flag_var}}")
      string(REPLACE "-W " "" ${flag_var} "${${flag_var}}") # bare -W becomes /W (invalid without level)
      string(REPLACE " -W" " " ${flag_var} "${${flag_var}}")
      string(REPLACE "-O3" "" ${flag_var} "${${flag_var}}")
      string(REPLACE "-march=native" "" ${flag_var} "${${flag_var}}")
    endif()
  endforeach()
  add_compile_options(/permissive- /W4 /EHsc /bigobj)
  string(APPEND CMAKE_C_FLAGS_RELEASE " /O2")
  string(APPEND CMAKE_CXX_FLAGS_RELEASE " /O2")
else()
  add_compile_options(-Wall)
  string(APPEND CMAKE_C_FLAGS_RELEASE " -O3 -march=native")
  string(APPEND CMAKE_CXX_FLAGS_RELEASE " -O3 -march=native")
endif()

# ---- Optional OpenMP (off by default) ----
option(G2O_USE_OPENMP "Build g2o with OpenMP support (EXPERIMENTAL)" OFF)
find_package(OpenMP)
set(G2O_OPENMP OFF)
if(G2O_USE_OPENMP AND OpenMP_CXX_FOUND)
  set(G2O_OPENMP ON)
  message(STATUS "Compiling with OpenMP support")
endif()

# ---- Eigen3 ----
# Prefer the imported target from a Config package (vcpkg) but accept FindEigen3 variables too.
find_package(Eigen3 3.1.0 REQUIRED)
if(TARGET Eigen3::Eigen)
  set(G2O_EIGEN_TARGET Eigen3::Eigen)
  get_target_property(_eigen_inc Eigen3::Eigen INTERFACE_INCLUDE_DIRECTORIES)
  if(_eigen_inc)
    set(EIGEN3_INCLUDE_DIR "${_eigen_inc}")
  endif()
elseif(NOT EIGEN3_INCLUDE_DIR)
  message(FATAL_ERROR "Eigen3 include directory not found.")
endif()

# ---- Generate config.h from config.h.in (keeps original behavior) ----
set(G2O_CXX_COMPILER "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER}")
configure_file(config.h.in "${CMAKE_CURRENT_SOURCE_DIR}/config.h" @ONLY)

# ---- Include paths ----
# Keep original layout: core, types, stuff, and the root (for generated config.h)
include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}/core"
  "${CMAKE_CURRENT_SOURCE_DIR}/types"
  "${CMAKE_CURRENT_SOURCE_DIR}/stuff"
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${EIGEN3_INCLUDE_DIR}"
)

# ---- Library target and sources (from your file) ----
add_library(g2o ${G2O_LIB_TYPE}
  # types
  g2o/types/types_sba.h
  g2o/types/types_six_dof_expmap.h
  g2o/types/types_sba.cpp
  g2o/types/types_six_dof_expmap.cpp
  g2o/types/types_seven_dof_expmap.cpp
  g2o/types/types_seven_dof_expmap.h
  g2o/types/se3quat.h
  g2o/types/se3_ops.h
  g2o/types/se3_ops.hpp

  # core
  g2o/core/base_edge.h
  g2o/core/base_binary_edge.h
  g2o/core/hyper_graph_action.cpp
  g2o/core/base_binary_edge.hpp
  g2o/core/hyper_graph_action.h
  g2o/core/base_multi_edge.h
  g2o/core/hyper_graph.cpp
  g2o/core/base_multi_edge.hpp
  g2o/core/hyper_graph.h
  g2o/core/base_unary_edge.h
  g2o/core/linear_solver.h
  g2o/core/base_unary_edge.hpp
  g2o/core/marginal_covariance_cholesky.cpp
  g2o/core/base_vertex.h
  g2o/core/marginal_covariance_cholesky.h
  g2o/core/base_vertex.hpp
  g2o/core/matrix_structure.cpp
  g2o/core/batch_stats.cpp
  g2o/core/matrix_structure.h
  g2o/core/batch_stats.h
  g2o/core/openmp_mutex.h
  g2o/core/block_solver.h
  g2o/core/block_solver.hpp
  g2o/core/parameter.cpp
  g2o/core/parameter.h
  g2o/core/cache.cpp
  g2o/core/cache.h
  g2o/core/optimizable_graph.cpp
  g2o/core/optimizable_graph.h
  g2o/core/solver.cpp
  g2o/core/solver.h
  g2o/core/creators.h
  g2o/core/optimization_algorithm_factory.cpp
  g2o/core/estimate_propagator.cpp
  g2o/core/optimization_algorithm_factory.h
  g2o/core/estimate_propagator.h
  g2o/core/factory.cpp
  g2o/core/optimization_algorithm_property.h
  g2o/core/factory.h
  g2o/core/sparse_block_matrix.h
  g2o/core/sparse_optimizer.cpp
  g2o/core/sparse_block_matrix.hpp
  g2o/core/sparse_optimizer.h
  g2o/core/hyper_dijkstra.cpp
  g2o/core/hyper_dijkstra.h
  g2o/core/parameter_container.cpp
  g2o/core/parameter_container.h
  g2o/core/optimization_algorithm.cpp
  g2o/core/optimization_algorithm.h
  g2o/core/optimization_algorithm_with_hessian.cpp
  g2o/core/optimization_algorithm_with_hessian.h
  g2o/core/optimization_algorithm_levenberg.cpp
  g2o/core/optimization_algorithm_levenberg.h
  g2o/core/optimization_algorithm_gauss_newton.cpp
  g2o/core/optimization_algorithm_gauss_newton.h
  g2o/core/jacobian_workspace.cpp
  g2o/core/jacobian_workspace.h
  g2o/core/robust_kernel.cpp
  g2o/core/robust_kernel.h
  g2o/core/robust_kernel_factory.cpp
  g2o/core/robust_kernel_factory.h
  g2o/core/robust_kernel_impl.cpp
  g2o/core/robust_kernel_impl.h

  # stuff
  g2o/stuff/string_tools.h
  g2o/stuff/color_macros.h
  g2o/stuff/macros.h
  g2o/stuff/timeutil.cpp
  g2o/stuff/misc.h
  g2o/stuff/timeutil.h
  g2o/stuff/os_specific.c
  g2o/stuff/os_specific.h
  g2o/stuff/string_tools.cpp
  g2o/stuff/property.cpp
  g2o/stuff/property.h
)

# Position independent code even for static libs on non-Windows
if(NOT WIN32)
  set_target_properties(g2o PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# ---- Link & definitions ----
if(TARGET Eigen3::Eigen)
  target_link_libraries(g2o PUBLIC Eigen3::Eigen)
endif()

if(G2O_OPENMP)
  target_link_libraries(g2o PUBLIC OpenMP::OpenMP_CXX)
  target_compile_definitions(g2o PUBLIC EIGEN_DONT_PARALLELIZE)
endif()

# Keep a helpful define on UNIX like upstream did
if(UNIX)
  target_compile_definitions(g2o PUBLIC UNIX)
endif()
