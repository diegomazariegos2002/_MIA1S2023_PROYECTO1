//
// Created by diego-mazariegos on 5/10/22.
//

#ifndef PROYECTO1ARCHIVOSPRUEBA2_ANALIZADOR_H
#define PROYECTO1ARCHIVOSPRUEBA2_ANALIZADOR_H


#include <string>
#include "Disco.h"
#include "Particion.h"
#include "Montar.h"
#include "Rep.h"
#include "Usuario.h"
#include "AdminUsuarios.h"

using namespace std;

class Analizador {

public:
    string entrada;
    Analizador(string entrada, MountList *mountList, Usuario *usuario);
    void analizarEntrada();
    string toLowerr(string cadena);
    string removeSpace(string entrada);
    Disco *disco;
    Particion *particion;
    Montar *montar;
    MountList *mountList;
    Rep *rep;
    Usuario *usuario;
    AdminUsuarios *adminU;

    void exec(string path);
};


#endif //PROYECTO1ARCHIVOSPRUEBA2_ANALIZADOR_H
