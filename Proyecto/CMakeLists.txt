cmake_minimum_required(VERSION 3.24)
project(Proyecto)

set(CMAKE_CXX_STANDARD 17)

add_executable(Proyecto main.cpp Structs.h Disco.h Disco.cpp Analizador.h Analizador.cpp Particion.cpp Nodo_M.cpp MountList.cpp Montar.cpp Rep.h Rep.cpp AdminUsuarios.h AdminUsuarios.cpp Usuario.cpp)
