#ifndef PROYECTO_MOUNTLIST_H
#define PROYECTO_MOUNTLIST_H

#include <string>
#include "Nodo_M.h"

using namespace std;

class MountList {
    public:
        MountList();
        Nodo_M *primero,*ultimo;
        void add(string path,string name, char type, int start, int pos);
        Nodo_M *buscar(string id);
        bool eliminar(string id);
        int getNum(string path);
        string getName(string path);
        bool existMount(string path,string name);
};


#endif //PROYECTO_MOUNTLIST_H
