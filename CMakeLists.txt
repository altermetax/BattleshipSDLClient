cmake_minimum_required(VERSION 3.15)
project(BattleshipSDLClient C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_executable(BattleshipSDLClient
        src/globals.c
        src/game.c
        src/load.c
        src/main.c
        src/network.c
        src/scenes.c
        src/ship.c)

target_link_libraries(BattleshipSDLClient SDL2 SDL2_net SDL2_ttf)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
