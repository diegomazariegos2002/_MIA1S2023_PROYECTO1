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
        void generate();
        string getExtensionFile(string path);
        string getCarpetas(string ruta);
        string getName(string path);
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
        vector<string>splitRuta(string cadena);
        string getContent(int inodoStart, FILE *file);
        int getInodoF(vector<string> rutaS, int posAct, int rutaSize, int start, FILE *file);
        string graphBlockCarpeta(int pos,FILE *file);
        string graphBlockArchivo(int pos,FILE *file);
        string graphBlockApuntador(int pos,FILE *file);
        string dibujarBlkReporteTree(int pos, int type, FILE *file);
        string dibujarInodoReporteTree(int direccionInodo, FILE *disco);
        string lsInodo(int pos,string name,FILE *file);
        string getUsuario(int id,FILE *file);
        string getGrupo(int id,FILE *file);
        vector<string> splitDatos(string cadena);
        string getPermiso(int permiso);
        string enlaces(int inicio, int final);
        string getDireccionCarpetas(int direccionInodoCarpeta);
};
#endif //PROYECTO_REP_H
