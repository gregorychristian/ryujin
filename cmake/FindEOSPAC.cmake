##
## SPDX-License-Identifier: MIT
## Copyright (C) 2020 - 2023 by the ryujin authors
##


# FIXME: this should be made much more robust. But *meh*

set(EOSPAC_LIBRARY "")
foreach(_prefix ${CMAKE_PREFIX_PATH})
  message(STATUS "${_prefix}")
  file(GLOB_RECURSE _candidates ${_prefix}/*/libeospac6.a)
  if(NOT "${_candidates}" STREQUAL "")
    list(GET _candidates 0 EOSPAC_LIBRARY)
    break()
  endif()
endforeach()
if("${EOSPAC_LIBRARY}" STREQUAL "")
  set(EOSPAC_LIBRARY "EOSPAC_LIBRARY-NOTFOUND")
endif()

set(EOSPAC_INCLUDE_DIR "")
foreach(_prefix ${CMAKE_PREFIX_PATH})
  message(STATUS "${_prefix}")
  file(GLOB_RECURSE _candidates ${_prefix}/*/eos_Interface.h)
  if(NOT "${_candidates}" STREQUAL "")
    list(GET _candidates 0 EOSPAC_INCLUDE_DIR)
    get_filename_component(EOSPAC_INCLUDE_DIR "${EOSPAC_INCLUDE_DIR}" DIRECTORY)
    break()
  endif()
endforeach()
if("${EOSPAC_INCLUDE_DIR}" STREQUAL "")
  set(EOSPAC_INCLUDE_DIR "EOSPAC_INCLUDE_DIR-NOTFOUND")
endif()

find_package_handle_standard_args(EOSPAC DEFAULT_MSG
  EOSPAC_LIBRARY EOSPAC_INCLUDE_DIR
  )

if(EOSPAC_FOUND AND NOT TARGET eospac6)
  add_library(eospac6 INTERFACE)
  target_link_libraries(eospac6 INTERFACE ${EOSPAC_LIBRARY})
  target_include_directories(eospac6 SYSTEM INTERFACE ${EOSPAC_INCLUDE_DIR})
endif()
