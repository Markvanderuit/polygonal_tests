cmake_minimum_required(VERSION 3.22)

# Helper function to add a library target
function(prg_add_library target_name include_names)
  # Gather source files recursively
  file(GLOB_RECURSE src_files ${CMAKE_CURRENT_SOURCE_DIR}/src/${target_name}/*.cpp)
  
  # Specify target library
  add_library(${target_name} ${src_files})

  # Specify target includes/features/defines
  target_include_directories(${target_name} PUBLIC include ${include_names})
  target_compile_features(${target_name}    PUBLIC cxx_std_23)
  target_compile_definitions(${target_name} PUBLIC _USE_MATH_DEFINES)
endfunction()

function(prg_add_pch target_name)
  file(GLOB_RECURSE header_files ${CMAKE_CURRENT_SOURCE_DIR}/include/${target_name}/*hpp)
  target_precompile_headers(${target_name} PUBLIC ${header_files})
endfunction()

function(prg_reuse_pch target_name other_names)
  foreach(other_name ${other_names})
    target_precompile_headers(${target_name} REUSE_FROM ${other_name})
  endforeach()
endfunction()