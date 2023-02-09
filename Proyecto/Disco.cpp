#include "Disco.h"
#include <string>
#include <cstring>
#include <iostream>
#include <stdlib.h>
#include "Structs.h"

using namespace std;

Disco::Disco() {
    this->s=0;
    this->f="bf";
    this->u="m";
    this->p=" ";
    this->pathFull="";
}

bool Disco::validar() {
    bool bandera= false;
    if (s>0){
        if ((strncmp(this->f.c_str(),"bf",2)==0) || (strncmp(this->f.c_str(),"ff",2)==0) || (strncmp(this->f.c_str(),"wf",2)==0)){
            if ((strncmp(this->u.c_str(),"k",1)==0) || (strncmp(this->u.c_str(),"m",1)==0)){
                if (this->p != " "){
                    int i= this->p.find('.');
                    string extension=this->p.substr(i+1,this->p.length());
                    if ((strncmp(extension.c_str(),"dsk",extension.length())==0)){
                        bandera= true;
                    }else{
                        cout<<"EXTENSION INCORRECTA"<<endl;
                    }
                }else{
                    cout<<"RUTA INCORRECTA"<<endl;
                }
            }else{
                cout<<"UNIDADES DEL TAMAÑO DE MEMORIA INVALIDO"<<endl;
            }
        }else{
            cout<<"AJUSTE INVALIDO"<<endl;
        }
    }else{
        cout<<"EL TAMAÑO DEL DISCO TIENE QUE SER MAYOR A 0"<<endl;
    }
    return bandera;
}

string Disco::getDirectorio(std::string path) {
    string aux=path;
    size_t p=0;
    string directorio="";
    while ((p=aux.find("/"))!= string::npos){
        directorio += aux.substr(0,p)+"/";
        aux.erase(0,p+1);
    }
    return directorio;
}

void Disco::mkdisk() {
    if (validar()){
        FILE *file;
        if ((file= fopen(this->p.c_str(),"r"))){
            cout<<"EL DISCO YA EXISTE"<<endl;
            fclose(file);
        }else{
            this->pathFull=this->getDirectorio(this->p);
            system(("sudo -S mkdir -p \'" + this->pathFull + "\'").c_str());
            system(("sudo -S chmod -R 777 \'" + this->pathFull + "\'").c_str());
            FILE *file;
            
            char buffer[1024];
            for (int i = 0; i < 1024; ++i) {
               buffer[i]='\0';
            }
            
            int size= this->s;
            if (this->u=="m"){
                size=size*1024;
            }

            file= fopen(this->p.c_str(),"wb");
            int i=0;
            while (i<size){
                fwrite(&buffer,1024,1,file);
                i++;
            }
            fclose(file);

            file= fopen(this->p.c_str(),"rb+");

            MBR mbr;
            mbr.mbr_fecha_creacion= time(nullptr);
            mbr.mbr_dsk_signature=static_cast<int>(time(nullptr));
            mbr.mbr_tamano=size*1024;
            mbr.disk_fit= this->f[0];
            for (int j = 0; j < 4; ++j) {
                mbr.mbr_partition[j].part_start=-1;
                mbr.mbr_partition[j].part_type='p';
            }
            fseek(file,0,SEEK_SET);
            fwrite(&mbr,sizeof(MBR),1,file);
            fclose(file);

            cout<<"SE CREO EL DISCO EXITOSAMENTE"<<endl;
        }
    }
}

void Disco::rmdisk() {
    if (this->p != " "){
        int i= this->p.find('.');
        string extension=this->p.substr(i+1,this->p.length());
        if (extension =="dsk"){
            FILE *file;
            if ((file= fopen(this->p.c_str(),"r"))){
                fclose(file);
                remove(this->p.c_str());
                cout<<"DISCO ELIMINADO"<<endl;
            }else{
                cout<<"DISCO INEXISTENTE"<<endl;
            }
        }else{
            cout<<"EXTENSION INCORRECTA"<<endl;
        }
    }else{
        cout<<"ASEGURESE DE ESCRIBIR UN RUTA"<<endl;
    }
}