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
    bool r;
    AdminArchivosCarpetas();
    void cat();
    void mkdir();
    vector<string> getRutaDividida(string cadena);
    int getDireccionInodo(vector<string> rutaDividida, int direccionActual, int numCarpetas, int rutaActual, FILE *discoActual);
    bool verificarPermisoInodo_Lectura(int direccionInodo);
    string getStringAlmacenadoInodo(int startInodo);
    void registrarJournal(string tipo_Op, char tipo, string nombre, string contenido, Nodo_M *nodo);

    int existeCarpeta(vector<string> rutaDivida, int carpetaActual, int direccionInodo);
    int crearCarpeta(vector<string> rutaDividida, int carpetaActual, int direccionInodo);

    bool verificarPermisoInodo_Escritura(int direccionInodo);

    int getPosicionInodoNuevo();

    int actualizarUltimoInodoDisponible();

    int crearBloqueCarpetaInicial(int posActual, int posPadre);
};


#endif //PROYECTO_ADMINARCHIVOSCARPETAS_H
