cmake_minimum_required(VERSION 3.15)
project(BattleshipSDLClient C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_executable(BattleshipSDLClient
        src/game.c
        src/load.c
        src/main.c
        src/network.c
        src/scenes.c
        src/ship.c)

target_link_libraries(BattleshipSDLClient SDL2 SDL2_net SDL2_ttf)

add_custom_command(
        TARGET BattleshipSDLClient PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources/ $<TARGET_FILE_DIR:BattleshipSDLClient>/resources)