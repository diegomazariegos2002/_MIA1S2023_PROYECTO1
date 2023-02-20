#ifndef CODIGO_FUENTE_REP_H
#define CODIGO_FUENTE_REP_H

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
        SuperBloque sbb;
        void generate();
        string getExtension(string path);
        string getDirectorio(string path);
        string getName(string path);
        void mbr();
        void disk();
        void inode();
        void journaling();
        void block();
        void bm_inode();
        void bm_block();
        void tree();
        void sb();
        void file();
        void ls();
        vector<string>splitRuta(string cadena);
        string getContent(int inodoStart, FILE *file);
        int getInodoF(vector<string> rutaS, int posAct, int rutaSize, int start, FILE *file);
        string graphBlockCarpeta(int pos,FILE *file);
        string graphBlockArchivo(int pos,FILE *file);
        string graphBlockApuntador(int pos,FILE *file);
        string treeBlock(int pos, int type,FILE *file);
        string treeInodo(int pos,FILE *file);
        string lsInodo(int pos,string name,FILE *file);
        string getUsuario(int id,FILE *file);
        string getGrupo(int id,FILE *file);
        vector<string> splitDatos(string cadena);
        string getPermiso(int permiso);
        string conexiones(int inicio,int final);
};
#endif //CODIGO_FUENTE_REP_H
