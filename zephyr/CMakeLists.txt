if(CONFIG_CANSCRIBE)
    set(CANSCRIBE_DIR ${ZEPHYR_CURRENT_MODULE_DIR})
    zephyr_include_directories(${CANSCRIBE_DIR})
    zephyr_library()
    zephyr_library_sources(
	${CANSCRIBE_DIR}/cobs.c
    )
endif()
