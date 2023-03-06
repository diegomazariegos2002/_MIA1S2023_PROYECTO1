#ifndef PROYECTO_MONTAR_H
#define PROYECTO_MONTAR_H

#include <string>
#include "MountList.h"

using namespace std;

class Montar {
    public:
        Montar();
        void mount();
        void unmount();
        void mkfs();
        string p;
        string name;
        string id;
        string fs;
        string type;
        MountList *mountList;
};


#endif //PROYECTO_MONTAR_H
