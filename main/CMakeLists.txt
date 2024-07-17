file(GLOB SOURCE_FILES
    src/*.cpp
    main.cpp
    proto/cpp/*.cc
    Arduino-fork/cores/esp32/*.c
    Arduino-fork/cores/esp32/*.cpp
    Arduino-fork/cores/esp32/libb64/*.c
    Arduino-fork/cores/esp32/libb64/*.cpp
    Arduino-fork/cores/esp32/apps/sntp/*.c
    Arduino-fork/cores/esp32/apps/sntp/*.cpp
    Arduino-fork/libraries/SPI/src/*.c
    Arduino-fork/libraries/SPI/src/*.cpp
)

idf_component_register(
            SRCS 
                ${SOURCE_FILES}
            INCLUDE_DIRS 
                "include"
                "proto/cpp"
                "crc/inc"
                "Arduino-fork"
                "Arduino-fork/cores/esp32"
                "Arduino-fork/cores/esp32/libb64"
                "Arduino-fork/cores/esp32/apps/sntp"
                "Arduino-fork/libraries/SPI/src"
                "Arduino-fork/variants/esp32s3"
            )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++23 -Wno-format -Wno-missing-field-initializers -Wno-write-strings -Wno-return-type -Wno-reorder")

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-format -Wno-missing-field-initializers -Wno-write-strings -fexceptions -Wno-return-type -Wno-reorder)