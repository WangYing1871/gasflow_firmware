
zephyr_library()

zephyr_library_sources_ifdef(CONFIG_MFC_ACU20FD acu20fd.c)
