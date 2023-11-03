cmake_minimum_required(VERSION 3.7)

project(Sonic2Mania)

set(MOD_NAME Sonic2Mania CACHE STRING "The mod directory to look into")
set(OUTPUT_NAME "S2M" CACHE STRING "The name of the built library")

set(RETRO_REVISION 3 CACHE STRING "What revision to compile for. Defaults to Origins = 3")
set(RETRO_MOD_LOADER_VER 2 CACHE STRING "Sets the mod loader version. Defaults to latest")

option(MOD_INCLUDE_EDITOR "Whether or not to include editor functions. Defaults to true" ON)

option(DISCORD_RPC "Compile with Discord RPC or not" OFF)

file(GLOB OBJECTS RELATIVE ${CMAKE_SOURCE_DIR} src/S2M/Objects/*/*.cpp)

add_library(Sonic2Mania SHARED
    src/GameAPI/CPP/GameAPI/Game.cpp
    src/S2M/S2M.cpp
    ${OBJECTS}
)

target_include_directories(Sonic2Mania PRIVATE
    src/S2M/
    src/S2M/Objects/
    src/GameAPI/CPP/GameAPI/
)

if(DISCORD_RPC AND WIN32)
    file(GLOB DISCORDSDK src/DiscordGameSDK/cpp/*.cpp)
    target_sources(Sonic2Mania PRIVATE ${DISCORDSDK})
    target_include_directories(Sonic2Mania PRIVATE src/DiscordGameSDK/cpp)

    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        target_link_libraries(Sonic2Mania PRIVATE
            ${CMAKE_SOURCE_DIR}/src/DiscordGameSDK/lib/x86_64/discord_game_sdk.dll.lib
        )
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "X86")
        target_link_libraries(Sonic2Mania PRIVATE
            ${CMAKE_SOURCE_DIR}/src/DiscordGameSDK/lib/x86/discord_game_sdk.dll.lib
        )
    endif()
endif()

if(MSVC)
    target_link_options(Sonic2Mania PRIVATE /OPT:NOICF)
else()
    target_link_options(Sonic2Mania PRIVATE --icf=none)
endif()

set_target_properties(Sonic2Mania PROPERTIES OUTPUT_NAME ${OUTPUT_NAME})

target_compile_definitions(Sonic2Mania PRIVATE
    RETRO_REVISION=${RETRO_REVISION}
    RETRO_USE_MOD_LOADER=1
    RETRO_MOD_LOADER_VER=${RETRO_MOD_LOADER_VER}
    RETRO_INCLUDE_EDITOR=$<BOOL:${MOD_INCLUDE_EDITOR}>
    GAME_TYPE=0
    GAME_NO_GLOBALS=1
    _CRT_SECURE_NO_WARNINGS=1
    DISCORD_RPC=$<BOOL:${DISCORD_RPC}>
)

add_custom_command(TARGET Sonic2Mania POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:${MOD_NAME}>
    ${CMAKE_SOURCE_DIR})

unset(MOD_NAME CACHE)
unset(OUTPUT_NAME CACHE)