set(COMPONENT_ADD_INCLUDEDIRS include)
set(COMPONENT_SRCS "spiffs.c")
set(COMPONENT_REQUIRES "spiffs" "esp32-utility")
register_component()
