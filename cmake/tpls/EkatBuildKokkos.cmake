# Build Kokkos from the submodule, unless already built in this CMake project
if (NOT TARGET Kokkos::kokkos)
  message (STATUS "No Kokkos target already defined. Building locally from submodule")
  set (Kokkos_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../extern/kokkos)
  set (Kokkos_BINARY_DIR ${CMAKE_BINARY_DIR}/externals/kokkos)

  message (STATUS "  Kokkos_SOURCE_DIR: ${Kokkos_SOURCE_DIR}")
  message (STATUS "  Kokkos_BINARY_DIR: ${Kokkos_BINARY_DIR}")

  add_subdirectory(${Kokkos_SOURCE_DIR} ${Kokkos_BINARY_DIR})

  include(EkatSetCompilerFlags)
  if (Kokkos_ENABLE_CUDA)
    SetCudaFlags(kokkoscore)
  endif()
  if (Kokkos_ENABLE_OPENMP)
    SetOmpFlags(kokkoscore)
  endif()
  if (EKAT_DISABLE_TPL_WARNINGS)
    include (EkatUtils)
    EkatDisableAllWarning(kokkoscore)
    EkatDisableAllWarning(kokkoscontainers)
  endif ()
else ()
  get_target_property(PREV_Kokkos_SOURCE_DIR Kokkos::kokkos SOURCE_DIR)
  message (STATUS "Kokkos target already added in ${PREV_Kokkos_SOURCE_DIR}")
endif()
