cmake_minimum_required(VERSION 3.10.2)

project(epolltest)

aux_source_directory(./ SRC_LISTS)
add_executable(epolltest ${SRC_LISTS})
