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
    bool verificarPermisoInodo_Lectura(int direccionInodo);
    string getStringAlmacenadoInodo(int startInodo);
    void registrarJournal(string tipo_Op, char tipo, string nombre, string contenido, Nodo_M *nodo);
};


#endif //PROYECTO_ADMINARCHIVOSCARPETAS_H
