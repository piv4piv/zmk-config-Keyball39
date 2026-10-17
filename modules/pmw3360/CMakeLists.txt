
add_subdirectory(drivers)
if(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
  zephyr_library_named(keyball_oled_guide)
  zephyr_library_sources(guide_screen.c)
  zephyr_library_include_directories(${APPLICATION_SOURCE_DIR}/include)
endif()

