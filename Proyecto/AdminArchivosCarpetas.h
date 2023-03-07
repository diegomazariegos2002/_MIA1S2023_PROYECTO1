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
    int insertCarpeta(vector<string> rutaDividida, int carpetaActual, int direccionInodoPadre);
    bool verificarPermisoInodo_Escritura(int direccionInodo);
    int getDireccionInodoNuevo();
    int actualizarUltimoInodoDisponible();
    int insertBlckCarpetaInicial(int direccionActual, int direccionPadre);
    int getDireccionBloqueNuevo();

    int actualizarUltimoBloqueDisponible();

    void insertInodoCarpeta(int direccionInodo, int direccionBloqueCarpetaInicial);

    int insertBlckCarpetaExtra(int direccionInodoHijo, string nombreCarpeta);

    int insertBlckApuntador(int direccionInodoHijo);
};


#endif //PROYECTO_ADMINARCHIVOSCARPETAS_H
