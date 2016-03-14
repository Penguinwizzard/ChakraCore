add_executable (jsbc
  jsbc.cpp
  stdafx.cpp
    )

include_directories(..)

target_include_directories (jsbc  
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  $<BUILD_INTERFACE:${ROOT_SOURCE_DIR}/lib/Common>
  $<BUILD_INTERFACE:${ROOT_SOURCE_DIR}/lib/Common/Memory>  
  $<BUILD_INTERFACE:${ROOT_SOURCE_DIR}/lib/Parser>
  )

target_link_libraries (jsbc
  PRIVATE Chakra.Common.Parser
  PRIVATE Chakra.Common.Memory
  PRIVATE Chakra.Common.Common  
  PRIVATE Chakra.Common.Core
  PRIVATE Chakra.Common.DataStructures
  PRIVATE Chakra.Common.Exceptions
  PRIVATE Chakra.Pal
  )

