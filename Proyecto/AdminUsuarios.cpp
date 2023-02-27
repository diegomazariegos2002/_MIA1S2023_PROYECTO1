#include "AdminUsuarios.h"
#include "Structs.h"
#include "Nodo_M.h"
#include <string>
#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <sstream>


AdminUsuarios::AdminUsuarios() {
    this->id=" ";
    this->name=" ";
    this->pass=" ";
    this->group=" ";
    this->cambioCont=false;
    this->usuario=new Usuario();
    this->mountList=new MountList();
}

void AdminUsuarios::login() {
    if (this->usuario->idU!=0){
        cout<<"YA EXISTE UN USUARIO LOGUEADO"<<endl;
        return;
    }
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL) {
        if ((this->file= fopen(nodo->path.c_str(),"rb+"))){
            if (nodo->type=='p'){
                MBR mbr;
                fseek(this->file,0,SEEK_SET);
                fread(&mbr, sizeof(MBR),1,this->file);
                if (mbr.mbr_partition[nodo->pos].part_status!='2'){
                    cout<<"PARTICION SIN FORMATEAR, IMPOSIBLE LOGEARSE"<<endl;
                    return;
                }
                fseek(this->file,nodo->start,SEEK_SET);
                fread(&this->sb, sizeof(SuperBloque),1,this->file);
            }
            else if (nodo->type=='l'){
                EBR ebr;
                fseek(this->file,nodo->start,SEEK_SET);
                fread(&ebr, sizeof(EBR),1,this->file);
                if (ebr.part_status!='2'){
                    cout<<"PARTICION SIN FORMATEAR, IMPOSIBLE LOGEARSE"<<endl;
                    return;
                }
                fseek(this->file,nodo->start+ sizeof(EBR),SEEK_SET);
                fread(&this->sb, sizeof(SuperBloque),1,this->file);
            }

            string contenido= this->getContenido(this->sb.s_inode_start + sizeof(TablaInodo));
            vector<string>listaUsuarios= this->getUsers(contenido);
            vector<string> camposUsuario;
            vector<string>listaGrupos= this->getGrupos(contenido);
            vector<string> camposGrupo;
            for (int i = 0; i < listaUsuarios.size(); ++i) {
                camposUsuario= this->getCampos(listaUsuarios[i]);
                if (camposUsuario[3] == this->name){ // si el usuario login es encontrado
                    if(camposUsuario[4] == this->pass){
                        for (int j = 0; j < listaGrupos.size(); ++j) {
                            camposGrupo= this->getCampos(listaGrupos[j]);
                            if (camposGrupo[2] == camposUsuario[2]){ // usuarioLogin.grupo = listado.grupo
                                this->usuario->nombreU=camposUsuario[3];
                                this->usuario->idMount=this->id;
                                this->usuario->idU= stoi(camposUsuario[0]);
                                this->usuario->idG= stoi(camposGrupo[0]);
                                fclose(this->file);
                                cout<<"SESIÓN INICIADA "<<this->usuario->nombreU<<endl;
                                return;
                            }
                        }
                        fclose(this->file);
                        cout<<"NO SE ENCONTRO UN GRUPO QUE COINCIDA"<<endl;
                        return;
                    }
                    fclose(this->file);
                    cout << "PASSWORD INCORRECTA" << endl;
                    return;
                }
            }
            fclose(this->file);
            cout<<"NO SE ENCONTRO NINGUN USUARIO QUE COINCIDA"<<endl;
            return;
        }
        else{
            cout <<"EL DISCO ESPECIFICADO NO EXISTE"<<endl;
            return;
        }
    }
    else{
        cout <<"EL ID DADO NO REPRESENTA NINGUNA MONTURA: "<< this->id<<endl;
        return;
    }
}

void AdminUsuarios::logout() {
    if (this->usuario->idU==0){
        cout<<"DEBE LOGEAR UN USUARIO PRIMERO PARA APLICAR LOGOUT"<<endl;
        return;
    }
    this->usuario=new Usuario();
    this->usuario->idG=0;
    this->usuario->idU=0;
    this->usuario->idMount=" ";
    this->usuario->nombreU=" ";
    cout<<"SE CERRO LA SESIÓN CON ÉXITO"<<endl;
    return;
}

//Método para leer el contenido de un Inodo.
string AdminUsuarios::getContenido(int startInodo) {
    string contenido="";
    BloqueApuntador b_Apuntador1,b_Apuntador2,b_Apuntador3;
    BloqueArchivo b_Archivo;
    TablaInodo tablaInodo;
    fseek(this->file, startInodo, SEEK_SET);
    fread(&tablaInodo, sizeof(TablaInodo), 1, this->file);
    for (int i = 0; i < 15; ++i) {
        if (tablaInodo.i_block[i] != -1){ // si, si poseen información
            if (i<12){ // apuntadores directos - leer el bloque de archivos
                fseek(this->file, tablaInodo.i_block[i], SEEK_SET); // nos posicionamos en la dirección de cada bloque de archivos
                fread(&b_Archivo, sizeof(BloqueArchivo), 1, this->file);
                for (int j = 0; j < 64; ++j) {
                    if (b_Archivo.b_content[j] == '\000'){ // si encuentra un dato nulo
                        break;
                    }
                    contenido+=b_Archivo.b_content[j]; // ir concatenando la información
                }
            }
            else if (i==12){ // apuntador simple - leer el bloque de apuntadores - leer el bloque de archivos
                // Leer el bloque de apuntadores
                fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
                fread(&b_Apuntador1, sizeof(BloqueApuntador), 1, this->file);
                for (int j = 0; j < 16; ++j) { // leer apuntadores directos
                    if (b_Apuntador1.b_pointers[j] != -1){
                        fseek(this->file, b_Apuntador1.b_pointers[j], SEEK_SET);
                        fread(&b_Archivo, sizeof(BloqueArchivo), 1, this->file);
                        for (int k = 0; k < 64; ++k) { // leer bloques de archivos
                            if (b_Archivo.b_content[k] == '\000'){
                                break;
                            }
                            contenido+=b_Archivo.b_content[k];
                        }
                    }
                }
            }
            else if (i==13){ // apuntador doble - leer el bloque de apuntadores - leer el bloque de apuntadores - leer el bloque de archivos
                fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
                fread(&b_Apuntador1, sizeof(BloqueApuntador), 1, this->file);
                for (int j = 0; j < 16; ++j) { // leer apuntador simple
                    if (b_Apuntador1.b_pointers[j] != -1){
                        fseek(this->file, b_Apuntador1.b_pointers[j], SEEK_SET);
                        fread(&b_Apuntador2, sizeof(BloqueApuntador), 1, this->file);
                        for (int k = 0; k < 16; ++k) { // leer apuntador directo
                            if (b_Apuntador2.b_pointers[k] != -1){
                                fseek(this->file, b_Apuntador2.b_pointers[k], SEEK_SET);
                                fread(&b_Archivo, sizeof(BloqueArchivo), 1, this->file);
                                for (int z = 0; z < 64; ++z) { // leer bloque de archivos
                                    if (b_Archivo.b_content[z] == '\000'){
                                        break;
                                    }
                                    contenido+=b_Archivo.b_content[z];
                                }
                            }
                        }
                    }
                }
            }
            else if (i==14){ // apuntador triple - leer el bloque de apuntadores - leer el bloque de apuntadores - leer el bloque de apuntadores - leer el bloque de archivos
                fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
                fread(&b_Apuntador1, sizeof(BloqueApuntador), 1, this->file);
                for (int j = 0; j < 16; ++j) { // leer apuntador doble
                    if (b_Apuntador1.b_pointers[j] != -1){
                        fseek(this->file, b_Apuntador1.b_pointers[j], SEEK_SET);
                        fread(&b_Apuntador2, sizeof(BloqueApuntador), 1, this->file);
                        for (int k = 0; k < 16; ++k) { // leer apuntador simple
                            if (b_Apuntador2.b_pointers[k] != -1){
                                fseek(this->file, b_Apuntador2.b_pointers[k], SEEK_SET);
                                fread(&b_Apuntador3, sizeof(BloqueApuntador), 1, this->file);
                                for (int z = 0; z < 16; ++z) { // leer apuntador directo
                                    fseek(this->file, b_Apuntador3.b_pointers[z], SEEK_SET);
                                    fread(&b_Archivo, sizeof(BloqueArchivo), 1, this->file);
                                    for (int y = 0; y < 64; ++y) { // leer bloque archivo
                                        if (b_Archivo.b_content[y] == '\000'){
                                            break;
                                        }
                                        contenido+=b_Archivo.b_content[y];
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return contenido;
}

vector<string> AdminUsuarios::getUsers(string entrada) {
    vector<string> users;
    string copiaEntrada=entrada;
    stringstream ss(entrada);
    string linea;

    while (getline(ss, linea, '\n')){
        if (linea != ""){
            vector<string> campos= this->getCampos(linea);
            if (campos[1] == "U" && campos[0] != "0"){
                users.push_back(linea);
            }
        }
    }
    return users;
}

vector<string> AdminUsuarios::getGrupos(string entrada) {
    vector<string> grupos;
    string copiaEntrada=entrada;
    string linea;
    stringstream ss(entrada);

    while (getline(ss, linea, '\n')){
        if (linea != ""){
            vector<string> campos=this->getCampos(linea);
            if (campos[1] == "G" && campos[0] != "0"){
                grupos.push_back(linea);
            }
        }
    }
    return grupos;
}

vector<string> AdminUsuarios::getCampos(std::string entrada) {
    vector<string> campos;
    string copiaEntrada=entrada;
    stringstream ss(entrada);
    string campo;

    while (getline(ss, campo, ',')){
        if (campo != "")campos.push_back(campo);
    }
    return campos;
}


string AdminUsuarios::getGID(vector<string> grupos) {
    return std::string();
}

string AdminUsuarios::getUID(vector<string> usuarios) {
    return std::string();
}

vector<string> AdminUsuarios::splitContent(string cadena) {
    return vector<string>();
}

void AdminUsuarios::mkgrp() {

}

void AdminUsuarios::rmgrp() {

}

void AdminUsuarios::mkusr() {

}

void AdminUsuarios::rmusr() {

}

void AdminUsuarios::chgrp() {

}

bool AdminUsuarios::grupoExist(vector<string> grupos, string name) {
    return false;
}

bool AdminUsuarios::usrExist(vector<string> usuarios, string name) {
    return false;
}

TablaInodo AdminUsuarios::agregarArchivo(string cadena, TablaInodo inodo, int j, int aux) {
    return TablaInodo();
}

void AdminUsuarios::escribirJorunal(string tipo_Op, char tipo, string nombre, string contenido, Nodo_M *nodo) {

}

