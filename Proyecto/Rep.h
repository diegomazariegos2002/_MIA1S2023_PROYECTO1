#ifndef PROYECTO_REP_H
#define PROYECTO_REP_H

#include <string>
#include <vector>
#include "MountList.h"
#include "Structs.h"

using namespace std;

class Rep {
    public:
        Rep();
        string path;
        string name;
        string id;
        string ruta;
        string directorio;
        string extension;
        MountList *mountList;
        SuperBloque superBloqueGlobal;
        int contadorBloques_ReporteBloques;
        void generate();
        string getExtensionFile(string path);
        string getCarpetas(string ruta);
        string getNameFileInRoute(string path);
        void ejecutarReporte_mbr();
        void ejecutarReporte_disk();
        void ejecutarReporte_inode();
        void ejecutarReporte_Journaling();
        void ejecutarReporte_block();
        void ejecutarReporte_bm_inode();
        void ejecutarReporte_bm_block();
        void ejecutarReporte_tree();
        void ejecutarReporte_sb();
        void ejecutarReporte_file();
        void ejecutarReporte_ls();
        string getStringAlmacenadoInodo(int direccionInodoStart, FILE *fileDisco);
        int getDireccionInodo(vector<string> rutaDividida, int direccionActual, int numCarpetas, int rutaActual, FILE *discoActual);
        string graficarBlkCarpeta(int direccionBlk, FILE *disco);
        string graficarBlkArchivo(int direccionBlk, FILE *disco);
        string graficarBlkApuntador(int direccionBlk, FILE *disco);
        string dibujarBlkReporteTree(int pos, int type, FILE *file);
        string dibujarInodoReporteTree(int direccionInodo, FILE *disco);
        string graficarInodoRecursivoLs(int direccionInodo, string nombreFile, FILE *fileDisco);
        string getUsuarioInodo(int idUsuario, FILE *fileDisco);
        string getGrupoInodo(int idGrupo, FILE *fileDisco);
        vector<string> getCampos(string entrada);
        string getPermisosInodo(int permisoInodo);
        string dibujarEnlacesTree(int inicio, int final);
        vector<string> getRutaDividida(string cadena);
};
#endif //PROYECTO_REP_H
