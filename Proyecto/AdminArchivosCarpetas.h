#ifndef PROYECTO_ADMINARCHIVOSCARPETAS_H
#define PROYECTO_ADMINARCHIVOSCARPETAS_H

#include <string>
#include <vector>
#include "MountList.h"
#include "Usuario.h"
#include "Structs.h"

using namespace std;

class AdminArchivosCarpetas {
public:
    string path;
    MountList *mountList;
    Usuario *usuario;
    vector<string> rutasCat;
    FILE *file;
    SuperBloque sb;
    AdminArchivosCarpetas();
    void cat();
    vector<string> getRutaDividida(string cadena);
    int getDireccionInodoFile(vector<string> rutaDividida, int direccionActual, int numCarpetas, int rutaActual, FILE *discoActual);

};


#endif //PROYECTO_ADMINARCHIVOSCARPETAS_H
