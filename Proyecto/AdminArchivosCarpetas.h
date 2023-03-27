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
    string cont;
    int size;
    bool flagGlobal;
    string name;
    int ugo;
    AdminArchivosCarpetas();
    void cat();
    void mkdir();
    void mkfile();
    void rename();
    void chmod();
    void chown();
    void find();
    void edit();
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
    string leerArchivoComputadora();
    string getCadenaSize();
    vector<string> getArrayBlks(string cadena);
    void crearInodoArchivo(int direccionInodo);
    int enlazarArchivoCarpeta(int direccionArchivo, int direccionCarpeta, string nombreArchivo);
    TablaInodo addFile(int blckActual, int noBlckBitMap, std::string cadena, TablaInodo inodo);
    void cambiarNombre(TablaInodo tablaInodoCarpeta, std::string nombreOriginal);
    void cambiarPermisosRecursivo(int direccionInodo, int ugo, bool r);
    int getIdUser();
    int getIdGrupoUsuario();
    vector<string> getCampos(string entrada);
    int getIdGrupo(string nameGrupo);
    void cambiarPropietarioRecursivo_pt1(int direccionInodo, int idUser, int idGrupo, bool r);
    void cambiarPropietarioRecursivo_pt2(TablaInodo tablaInodo, int idUser, int idGrupo, bool r);

    int getInicioBloqueLibresSeguidos(int bits_a_Buscar);


    TablaInodo editarInodoRecursivamente(string contenidoBlk, TablaInodo tablaInodoActual, int indexBlk, bool eliminarBlk);

    int getIndiceBitMapBlks(int direccionBlkBusqueda);

    void update_First_BlkFree();

    string find_ImprimirBusqueda(int direccionInodo, string nombreBusqueda, int contadorProfundidad, bool mostrarTodo);
};


#endif //PROYECTO_ADMINARCHIVOSCARPETAS_H
