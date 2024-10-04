target_sources(${CMAKE_PROJECT_NAME} PRIVATE st7789.c)

# Generate PIO headers
pico_generate_pio_header(${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/st7789.pio)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        )
