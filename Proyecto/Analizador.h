//
// Created by diego-mazariegos on 5/10/22.
//

#ifndef PROYECTO_ANALIZADOR_H
#define PROYECTO_ANALIZADOR_H


#include <string>
#include "Disco.h"
#include "Particion.h"
#include "Montar.h"
#include "Rep.h"
#include "Usuario.h"
#include "AdminUsuarios.h"
#include "AdminArchivosCarpetas.h"

using namespace std;

class Analizador {

public:
    string entrada;
    Disco *disco;
    Particion *particion;
    Montar *montar;
    MountList *mountList;
    Rep *rep;
    Usuario *usuario;
    AdminUsuarios *adminU;
    AdminArchivosCarpetas *adminArcCarpt;


    Analizador(string entrada, MountList *mountList, Usuario *usuario);
    void analizarEntrada();
    string toLowerr(string cadena);
    string removeSpace(string entrada);
    void exec(string path);
};


#endif //PROYECTO_ANALIZADOR_H
