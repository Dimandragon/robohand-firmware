file(GLOB SOURCE_FILES
    src/*.cpp
    main.cpp
    proto/protobuf/*/*.cc
)

idf_component_register(
            SRCS 
                ${SOURCE_FILES}
            INCLUDE_DIRS 
                "include"
                "proto/protobuf"
                "proto/protobuf/notifications"
                "proto/protobuf/potentiometer"
                "proto/protobuf/servo"
                "proto/protobuf/shared"
                "proto/protobuf/straingauge"
                "proto/protobuf/commands"
                "proto/protobuf/imu")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++23 -Wno-format -Wno-missing-field-initializers -Wno-write-strings -Wno-return-type -Wno-reorder")

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-format -Wno-missing-field-initializers -Wno-write-strings -fexceptions -Wno-return-type -Wno-reorder)