#include "Nodo_M.h"
#include <string>

using namespace std;

Nodo_M::Nodo_M(std::string path,std::string name, char type, int num, string letra,int pos, int start) {
    this->path=path;
    this->num=num;
    this->letra=letra;
    this->name=name;
    this->type=type;
    this->id="75"+to_string(num)+this->letra;
    this->start=start;
    this->pos=pos;
    this->sig=NULL;
}