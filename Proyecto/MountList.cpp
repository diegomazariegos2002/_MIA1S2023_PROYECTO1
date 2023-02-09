#include "MountList.h"
#include <string>
#include <iostream>
#include "Nodo_M.h"

MountList::MountList() {
    this->primero=NULL;
    this->ultimo=NULL;
}

void MountList::add(std::string path, std::string name, char type, int start,int pos) {
    if (!existMount(path,name)){
        int num=this->getNum(path);
        string letra=this->getName(path);

        Nodo_M *nuevo=new Nodo_M(path,name,type,num,letra,pos,start);
        if (this->primero==NULL){
            this->primero=nuevo;
            this->ultimo=nuevo;
        }else{
            this->ultimo->sig=nuevo;
            this->ultimo= nuevo;
        }
        cout<<"LA PARTICION MONTADA "<<name<<" CON ID "<<nuevo->id<<endl;
    }else{
        cout<<"LA PARTICION "<<name<<" YA HA SIDO MONTADA"<<endl;
        return;
    }
}

bool MountList::existMount(std::string path, std::string name) {
    Nodo_M *aux=this->primero;
    while (aux!=NULL){
        if (aux->path==path && aux->name==name){
            return true;
        }
        aux=aux->sig;
    }
    return false;
}

int MountList::getNum(std::string path) {
    int mayor=0;
    Nodo_M *aux=this->primero;
    while (aux!=NULL){
        if (aux->path==path && aux->num>mayor){
            mayor=aux->num;
        }
        aux=aux->sig;
    }
    return mayor+1;
}

string MountList::getName(std::string path) {
    string aux=path;
    size_t p=0;
    string name="";
    while ((p=aux.find("/"))!= string::npos){
        name += aux.substr(0,p)+"/";
        aux.erase(0,p+1);
    }

    if ((p=aux.find("."))!= string::npos){
        name=aux.substr(0,p);
    }
    return name;
}

Nodo_M *MountList::buscar(std::string id) {
    Nodo_M *aux=this->primero;
    while (aux != NULL) {
        if (aux->id == id) {
            return aux;
        }
        aux = aux->sig;
    }
    return NULL;
}

bool MountList::eliminar(std::string id) {
    if (this->primero!=NULL){
        if (this->primero==this->ultimo && this->primero->id==id){
            this->primero=this->ultimo=NULL;
            return true;
        }else if (this->primero->id==id){
            this->primero= this->primero->sig;
            return true;
        }else{
            Nodo_M *aux=this->primero->sig,
                    *ant=this->primero;
            while (aux!=NULL){
                if (aux->id==id){
                    ant->sig=aux->sig;
                    aux->sig=NULL;
                    return true;
                }
                ant=aux;
                aux=aux->sig;
            }
            cout<<"NO SE ENCONTRO UNA PARTICION CON EL ID "<<id<<endl;
            return false;
        }
    }else{
        cout<<"TODAVIA NO EXISTEN PARTICIONES MONTANDAS PARA EJECUTAR ELIMINAR"<<endl;
        return false;
    }
}