set(DIR_BASE "C:/Users/theka/source/repos")
set(ROM_DIR "C:/Users/theka/emu/roms/nes")
set(INPUT_ROM "${ROM_DIR}/Super Mario Bros. (World).nes")
set(XXD "${DIR_BASE}/xxd/xxd.exe")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rom.nes
  COMMAND ${CMAKE_COMMAND} -E copy ${INPUT_ROM} ${CMAKE_CURRENT_BINARY_DIR}/rom.nes
  DEPENDS ${SOURCE_FILES} ${INPUT_ROM}
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rom.h
  COMMAND ${XXD} -i rom.nes ${CMAKE_CURRENT_BINARY_DIR}/rom.h
  DEPENDS ${SOURCE_FILES} ${CMAKE_CURRENT_BINARY_DIR}/rom.nes
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(rom_header ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/rom.h)
add_dependencies(${CMAKE_PROJECT_NAME} rom_header)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        )
