#ifndef CODIGO_FUENTE_PARTICION_H
#define CODIGO_FUENTE_PARTICION_H

#include <string>

using namespace std;

class Particion {
    public:
        int s,add;
        char u,t,f,flag;
        string d;
        string p,name;
        Particion();
        void fdisk();
        void primaryPartition();
        void extendPartition();
        void LogicPartition();
        void deleteFullPartition();
        void reducePartition();
        void incrementPartition();
        void deleteMegaByte(string p, long pos, int posicionFinal);
        void deleteKiloByte(string p, long posicionInicial, int posicionFinal);
        void deleteByte(string p, long posicionInicial, int posicionFinal);
        bool freeSpace(int s, string p, char u, int address);
        bool existeParticionExtendida(string p);
        bool existeParticion(string p, string name);
};


#endif //CODIGO_FUENTE_PARTICION_H
