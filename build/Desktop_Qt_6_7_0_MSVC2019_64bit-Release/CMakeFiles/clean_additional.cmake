# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\seam-carving-cpp_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\seam-carving-cpp_autogen.dir\\ParseCache.txt"
  "seam-carving-cpp_autogen"
  )
endif()
