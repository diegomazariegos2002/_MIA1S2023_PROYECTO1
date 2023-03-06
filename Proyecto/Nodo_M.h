#ifndef PROYECTO_NODO_M_H
#define PROYECTO_NODO_M_H

#include <string>

using namespace std;

class Nodo_M {
    public:
        string path;
        string name;
        string id;
        int num;
        int pos;
        char type;
        string letra;
        int start;
        Nodo_M *sig;
        Nodo_M(string path,string name,char type,int num,string letra,int pos,int start);
};


#endif //PROYECTO_NODO_M_H
