#ifndef CODIGO_FUENTE_ADMINDISK_H
#define CODIGO_FUENTE_ADMINDISK_H
#include <string>

using namespace std;

class Disco {
    public:
        Disco();
        bool validar();
        void mkdisk();
        void rmdisk();
        int s;
        string f;
        string u;
        string p;
        string directorio;
        string getDirectorio(string path);
};


#endif //CODIGO_FUENTE_ADMINDISK_H
