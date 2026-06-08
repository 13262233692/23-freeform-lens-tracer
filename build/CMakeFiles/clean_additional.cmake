# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\FreeformLensTracer_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\FreeformLensTracer_autogen.dir\\ParseCache.txt"
  "FreeformLensTracer_autogen"
  )
endif()
