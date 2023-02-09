//
// Created by diego-mazariegos on 5/10/22.
//

#ifndef PROYECTO1ARCHIVOSPRUEBA2_ANALIZADOR_H
#define PROYECTO1ARCHIVOSPRUEBA2_ANALIZADOR_H


#include <string>
#include "Disco.h"
#include "Particion.h"
#include "Montar.h"

using namespace std;

class Analizador {

public:
    string entrada;
    Analizador(string entrada, MountList *mountList);
    void analizarEntrada();
    string toLowerr(string cadena);
    string removeSpace(string entrada);
    Disco *disco;
    Particion *particion;
    Montar *montar;
    MountList *mountList;

    void exec(string path);
};


#endif //PROYECTO1ARCHIVOSPRUEBA2_ANALIZADOR_H
