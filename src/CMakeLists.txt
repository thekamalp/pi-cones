add_subdirectory(rom)
add_subdirectory(font)
if(WIN32)
else()
add_subdirectory(st7789)
endif()
add_subdirectory(pi_cones)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        )
