cmake_minimum_required(VERSION 3.21)
project(QuizzGame)

set(CMAKE_CXX_STANDARD 17)

add_executable(QuizzGame servQuizzGame.cpp clientQuizzGame.cpp)
