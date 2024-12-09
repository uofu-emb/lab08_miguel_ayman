add_library(can2040 ${CAN_PATH}/src/can2040.c)
target_include_directories(can2040 PUBLIC ${CAN_PATH}/src)
target_link_libraries(can2040 PRIVATE
  cmsis_core
  hardware_structs
  pico_stdlib
)
