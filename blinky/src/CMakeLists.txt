# Private CMakeLists for blinky_starter_app example code

# Copyright 2022 Eaton Corporation. All Rights Reserved.

cmake_minimum_required(VERSION 3.13.1)

target_sources(app PRIVATE
    "main.c"
    "UserTask.c"
    "Comm.c"
)

target_include_directories(app PUBLIC "./")