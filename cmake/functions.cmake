# Copyright 2018 The Cartographer Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include(CMakeParseArguments)

function(google_test NAME ARG_SRC)
  add_executable(${NAME} ${ARG_SRC})
  set(TARGET_COMPILE_FLAGS "${TARGET_COMPILE_FLAGS} ${GOOG_CXX_FLAGS}")

  set_target_properties(${NAME} PROPERTIES
    COMPILE_FLAGS ${TARGET_COMPILE_FLAGS})

  target_include_directories(${NAME} PUBLIC ${PROJECT_NAME})
  target_link_libraries(${NAME} PUBLIC ${PROJECT_NAME})

  # Make sure that gmock always includes the correct gtest/gtest.h.
  target_include_directories("${NAME}" SYSTEM PRIVATE
    "${GMOCK_INCLUDE_DIRS}")
  target_link_libraries("${NAME}" PUBLIC ${GMOCK_LIBRARIES})

  add_test(${NAME} ${NAME})
endfunction()

# Create a variable 'VAR_NAME'='FLAG'. If VAR_NAME is already set, FLAG is
# appended.
function(google_add_flag VAR_NAME FLAG)
  if (${VAR_NAME})
    set(${VAR_NAME} "${${VAR_NAME}} ${FLAG}" PARENT_SCOPE)
  else()
    set(${VAR_NAME} "${FLAG}" PARENT_SCOPE)
  endif()
endfunction()

macro(google_initialize_async_grpc_project)
  if(ASYNC_GRPC_CMAKE_DIR)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
        ${ASYNC_GRPC_CMAKE_DIR}/modules)
  else()
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
  endif()
  set(GOOG_CXX_FLAGS "-pthread -std=c++11 -fPIC ${GOOG_CXX_FLAGS}")

  google_add_flag(GOOG_CXX_FLAGS "-Wall")
  google_add_flag(GOOG_CXX_FLAGS "-Wpedantic")

  # Turn some warnings into errors.
  google_add_flag(GOOG_CXX_FLAGS "-Werror=format-security")
  google_add_flag(GOOG_CXX_FLAGS "-Werror=missing-braces")
  google_add_flag(GOOG_CXX_FLAGS "-Werror=reorder")
  google_add_flag(GOOG_CXX_FLAGS "-Werror=return-type")
  google_add_flag(GOOG_CXX_FLAGS "-Werror=switch")
  google_add_flag(GOOG_CXX_FLAGS "-Werror=uninitialized")

  if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
  endif()
endmacro()
