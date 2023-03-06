#ifndef PROYECTO_ADMINDISK_H
#define PROYECTO_ADMINDISK_H
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
        string pathFull;
        string getDirectorio(string path);
};


#endif //PROYECTO_ADMINDISK_H
