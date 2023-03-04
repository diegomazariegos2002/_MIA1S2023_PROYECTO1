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
    this->flagGlobal=false;
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

            string contenido= this->getStringAlmacenadoInodo(this->sb.s_inode_start + sizeof(TablaInodo)); // segundo inodo el de users.txt
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
string AdminUsuarios::getStringAlmacenadoInodo(int startInodo) {
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

string AdminUsuarios::getUID(vector<string> usuarios) {
    return std::string();
}

// Array de bloques según una cadena.
vector<string> AdminUsuarios::getArrayBlks(string cadena) {
    vector<string> arrayBlks;
    string aux="",linea;
    int contador=0;
    for (int i = 0; i < cadena.length(); ++i) {
        if (aux.length()==64){
            arrayBlks.push_back(aux);
            aux="";
            contador=0;
        }
        if (contador < 64){
            aux+=cadena[i];
            contador++;
        }
    }
    if (contador != 0) {
        arrayBlks.push_back(aux);
    }
    return arrayBlks;
}

void AdminUsuarios::mkgrp() {
    if (this->usuario->idU==1 && this->usuario->idG==1){
        if (this->name==" "){
            cout<<"EL NOMBRE DE GRUPO ES UN PARAMETRO OBLIGATORIO Y NO PUEDE IR VACIO";
            return;
        }
        Nodo_M *nodo=this->mountList->buscar(this->usuario->idMount);
        if (nodo!=NULL) {
            if ((this->file= fopen(nodo->path.c_str(),"rb+"))){
                if (nodo->type=='p'){
                    fseek(this->file,nodo->start,SEEK_SET);
                    fread(&this->sb, sizeof(SuperBloque),1,this->file);
                }else if (nodo->type=='l'){
                    fseek(this->file,nodo->start+ sizeof(EBR),SEEK_SET);
                    fread(&this->sb, sizeof(SuperBloque),1,this->file);
                }

                string grupoNuevo="";
                string contenido= this->getStringAlmacenadoInodo(this->sb.s_inode_start + sizeof(TablaInodo)); //Segundo Inodo (inodo users.txt)
                int blksActuales= this->getArrayBlks(contenido).size(); // obteniendo el número de bloques utilizados en ese inodo
                vector<string>listaGrupos=this->getGrupos(contenido);

                if (!this->verificarGrupoExistencia(this->name, listaGrupos)){
                    grupoNuevo= generarNuevoIdGrupos(listaGrupos) + ",G," + this->name + "\n";
                    contenido+=grupoNuevo;
                    vector<string> usersBlks= this->getArrayBlks(contenido);
                    int blksActualizado=usersBlks.size();

                    if ((blksActualizado - blksActuales) > this->sb.s_free_blocks_count){ // Si todavía quedan bloques libres disponibles en el sistema
                        cout << "NO EXISTEN BLOQUES LIBRES EN EL SISTEMA" << endl;
                        return;
                    }

                    if (usersBlks.size() > 4380){ // el 4380 es el número maximo de bloques por inodo
                        cout<<"SE HA ALCANZADO EL NUMERO MAXIMO DE BLOQUES EN EL INODO";
                        return;
                    }

                    //Posicionar nuevo bloque en el BitMap de Bloques
                    int bitMapBlks_Start = this->sb.s_bm_block_start;
                    int bitMapBlks_End = bitMapBlks_Start + this->sb.s_block_start;
                    int blksLibresSeguidos = 0;
                    int inicioBM = -1;
                    int startB = -1;
                    int contAux = 0;
                    char bit;
                    if ((blksActualizado - blksActuales) > 0){
                        for (int i = bitMapBlks_Start; i < bitMapBlks_End; ++i) { // Recorrer el BitMap
                            fseek(this->file, i, SEEK_SET); // leer la posicion
                            fread(&bit, sizeof(char), 1, this->file);

                            if (bit == '1') {// Bloque no disponible
                                blksLibresSeguidos = 0;
                                inicioBM = -1;
                                startB = -1;
                            }
                            else {// Bloque disponible
                                if (blksLibresSeguidos == 0) {
                                    inicioBM = i;
                                    startB = contAux;
                                }
                                blksLibresSeguidos++;
                            }

                            if (blksLibresSeguidos >= (blksActualizado - blksActuales)) break;
                            contAux++;
                        }

                        // Validar que si se encontro espacio libre
                        if (inicioBM==-1 || (blksLibresSeguidos != (blksActualizado - blksActuales))){
                            cout << "NO EXISTEN LOS SUFICIENTES BLOQUES O BLOQUES SEGUIDOS EN EL SISTEMA PARA ACTUALIZAR EL ARCHIVO" << endl;
                            return;
                        }

                        for (int i = inicioBM; i < (inicioBM+blksLibresSeguidos); ++i) {
                            char uno='1';
                            fseek(this->file,i,SEEK_SET);
                            fwrite(&uno, sizeof(char),1,this->file);

                        }
                        this->sb.s_free_blocks_count-=blksLibresSeguidos; // restar la cantidad de bloques disponibles

                        //ACTUALIZAR PRIMER BLOQUE DISPONIBLE
                        int primeraPosicionDisponible_BitMap_Blks = 0;
                        for (int k = bitMapBlks_Start; k < bitMapBlks_End; k++) {
                            fseek(this->file, k, SEEK_SET);
                            fread(&bit, sizeof(char), 1, this->file);
                            if (bit == '0') break;
                            primeraPosicionDisponible_BitMap_Blks++;
                        }
                        this->sb.s_first_blo = primeraPosicionDisponible_BitMap_Blks;
                    }

                    // ACTUALIZAR DATOS del inodo 2 (Inodo Users.txt)
                    TablaInodo inodo;
                    int direccionInodo= this->sb.s_inode_start + sizeof(TablaInodo);
                    fseek(this->file, direccionInodo, SEEK_SET);
                    fread(&inodo, sizeof(TablaInodo),1,this->file);

                    int size = 0;
                    for (int tm = 0; tm < usersBlks.size(); tm++) {
                        size += usersBlks[tm].length();
                    }
                    inodo.i_s= size;
                    inodo.i_mtime = time(nullptr);

                    int j=0,cont=0;
                    while (j < usersBlks.size()){
                        flagGlobal=false;
                        inodo= this->addFile(j, (startB + cont), usersBlks[j], inodo);
                        if (flagGlobal){
                            cont++;
                        }
                        j++;
                    }

                    fseek(this->file, direccionInodo, SEEK_SET);
                    fwrite(&inodo, sizeof(TablaInodo),1,this->file);
                    if (nodo->type=='p'){
                        fseek(this->file,nodo->start,SEEK_SET);
                        fwrite(&this->sb, sizeof(SuperBloque),1,this->file);
                    }else if (nodo->type=='l'){
                        fseek(this->file,nodo->start+ sizeof(EBR),SEEK_SET);
                        fwrite(&this->sb, sizeof(SuperBloque),1,this->file);
                    }

                    if (this->sb.s_filesystem_type==3){ // EXT3 -> JOURNAL
                        this->registrarJournal("mkgrp", '1', "users.txt", grupoNuevo, nodo);
                    }

                    fclose(this->file);
                    cout << "GRUPO "<<this->name<< " GUARDADO CON EXITO" << endl;

                }else{
                    cout<<"IMPOSIBLE REALIZAR ESTO, EL GRUPO INDICADO YA EXISTE"<<endl;
                }
            }else{
                cout <<"NO EXISTE UN DISCO EN LA RUTA ESPECIFICADA"<<endl;
                return;
            }
        }else{
            cout <<"MONTURA CON EL ID: "<< this->usuario->idMount<< " NO ENCONTRADO" <<endl;
            return;
        }
    }else{
        cout <<"NECESITA SER ADMIN PARA ESTO :D"<<endl;
    }
}

// podría mejorar esto (creo)
TablaInodo AdminUsuarios::addFile(int blckActual, int noBlckBitMap, std::string cadena, TablaInodo inodo) {
    TablaInodo tablaInodo;
    BloqueApuntador puntero1, puntero2, puntero3, nuevoPuntero1, nuevoPuntero2, nuevoPuntero3;
    for (int i = 0; i < 15; ++i) { // recorrer los 15 punteros del inodo
        //Recordar si no son utilizados su valor es -1
        // PUNTEROS DIRECTOS
        // si ya existía el bloque se modifica con la nueva info.
        if (inodo.i_block[i]!=-1 && i<12 && i == blckActual){
            BloqueArchivo bloqueArchivo;
            fseek(this->file,inodo.i_block[i],SEEK_SET);
            fread(&bloqueArchivo, sizeof(BloqueArchivo), 1, this->file);
            strcpy(bloqueArchivo.b_content, cadena.c_str());
            fseek(this->file,inodo.i_block[i],SEEK_SET);
            fwrite(&bloqueArchivo, sizeof(BloqueArchivo), 1, this->file);
            return inodo;
        }
        // si no existía se crea el bloque en la dirección indicada.
        else if (inodo.i_block[i]==-1 && i<12 && noBlckBitMap != -1){
            BloqueArchivo bloqueArchivo;
            int direc= this->sb.s_block_start + (noBlckBitMap * sizeof(BloqueArchivo));
            strcpy(bloqueArchivo.b_content, cadena.c_str());
            fseek(this->file, direc, SEEK_SET);
            fwrite(&bloqueArchivo, sizeof(BloqueArchivo), 1, this->file);
            inodo.i_block[i]=direc;
            this->flagGlobal=true;
            return inodo;
        }
        // PUNTEROS SIMPLES
        // si ya existía el bloque se modifica con la nueva info.
        else if (i == 12 && inodo.i_block[i] == -1 && noBlckBitMap != -1) {
            if (this->sb.s_free_blocks_count > 0) {
                char bit;
                int bit_2 = 0;
                char uno = '1';
                int blck_Start = this->sb.s_bm_block_start;
                int blck_End = blck_Start + this->sb.s_blocks_count;
                for (int i = blck_Start; i < blck_End; i++) {
                    fseek(this->file, i, SEEK_SET);
                    fread(&bit, sizeof(char), 1, this->file);
                    if (bit == '0') {
                        fseek(this->file, i, SEEK_SET);
                        fwrite(&uno, sizeof(char), 1, this->file);
                        break;
                    }
                    bit_2++;
                }
                BloqueArchivo bloqueArchivo;
                inodo.i_block[i] = this->sb.s_block_start + (bit_2 * sizeof(BloqueApuntador));

                int bloquePosicion= this->sb.s_block_start + (noBlckBitMap * sizeof(BloqueArchivo));
                strcpy(bloqueArchivo.b_content, cadena.c_str());
                fseek(this->file, bloquePosicion, SEEK_SET);
                fwrite(&bloqueArchivo, sizeof(BloqueArchivo), 1, this->file);
                nuevoPuntero1.b_pointers[0] = bloquePosicion;
                this->flagGlobal=true;
                for (int j = 1; j < 16; j++) {
                    nuevoPuntero1.b_pointers[j] = -1;
                }
                fseek(this->file, inodo.i_block[i], SEEK_SET);
                fwrite(&nuevoPuntero1, sizeof(BloqueApuntador), 1, this->file);

                bit_2 = 0;
                for (int bmI = blck_Start; bmI < blck_End; bmI++) {
                    fseek(this->file, bmI, SEEK_SET);
                    fread(&bit, sizeof(char), 1, this->file);
                    if (bit == '0') break;
                    bit_2++;
                }
                this->sb.s_free_blocks_count -= 1;
                this->sb.s_first_blo = bit_2;
                return inodo;
            } else {
                cout << "BLOQUES INSUFICIENTES PARA REALIZAR ESTE COMANDO" << endl;
                return tablaInodo;
            }
        }
        // si no existía se crea el bloque en la dirección indicada.
        else if (i == 12 && inodo.i_block[i] != -1) {
            fseek(this->file, inodo.i_block[i], SEEK_SET);
            fread(&puntero1, sizeof(BloqueApuntador), 1, this->file);
            for (int p_1 = 0; p_1 < 16; p_1++) {
                if (puntero1.b_pointers[p_1] == -1 && noBlckBitMap != -1) {
                    BloqueArchivo archivo;
                    int posBloque=this->sb.s_block_start+(noBlckBitMap * sizeof(BloqueArchivo));
                    strcpy(archivo.b_content, cadena.c_str());
                    fseek(this->file, posBloque, SEEK_SET);
                    fwrite(&archivo, sizeof(BloqueArchivo), 1, this->file);
                    this->flagGlobal=true;
                    puntero1.b_pointers[p_1] = posBloque;
                    fseek(this->file, inodo.i_block[i], SEEK_SET);
                    fwrite(&puntero1, sizeof(BloqueApuntador), 1, this->file);
                    return inodo;
                } else if ((puntero1.b_pointers[p_1] != -1) && (blckActual == (12 + p_1))){
                    BloqueArchivo archivo;
                    fseek(this->file, puntero1.b_pointers[p_1], SEEK_SET);
                    fread(&archivo, sizeof(BloqueArchivo),1,this->file);
                    strcpy(archivo.b_content, cadena.c_str());
                    fseek(this->file, puntero1.b_pointers[p_1], SEEK_SET);
                    fwrite(&archivo, sizeof(BloqueArchivo),1,this->file);
                    return inodo;
                }
            }
        }
        // PUNTEROS DOBLES
        // si ya existía el bloque se modifica con la nueva info.
        else if (i == 13 && inodo.i_block[i] == -1 && noBlckBitMap != -1) {
            if (this->sb.s_free_blocks_count > 1) {
                //PASO PARA EL PRIMER APUNTADOR
                char uno = '1';
                char bit_1;
                int bit_2 = 0;
                int blck_Start = this->sb.s_bm_block_start;
                int blck_End = blck_Start + this->sb.s_blocks_count;
                for (int i = blck_Start; i < blck_End; i++) {
                    fseek(this->file, i, SEEK_SET);
                    fread(&bit_1, sizeof(char), 1, this->file);
                    if (bit_1 == '0') {
                        fseek(this->file, i, SEEK_SET);
                        fwrite(&uno, sizeof(char), 1, this->file);
                        break;
                    }
                    bit_2++;
                }

                inodo.i_block[i] = this->sb.s_block_start + (bit_2 * sizeof(BloqueApuntador));

                //PASO PARA EL SEGUNDO APUNTADOR
                bit_2=0;
                for (int i = blck_Start; i < blck_End; i++) {
                    fseek(this->file, i, SEEK_SET);
                    fread(&bit_1, sizeof(char), 1, this->file);
                    if (bit_1 == '0') {
                        fseek(this->file, i, SEEK_SET);
                        fwrite(&uno, sizeof(char), 1, this->file);
                        break;
                    }
                    bit_2++;
                }

                nuevoPuntero1.b_pointers[0] = this->sb.s_block_start + (bit_2 * sizeof(BloqueApuntador));

                BloqueArchivo bloqueArchivo;
                int bloquePosicion= this->sb.s_block_start + (noBlckBitMap * sizeof(BloqueArchivo));
                strcpy(bloqueArchivo.b_content, cadena.c_str());
                fseek(this->file, bloquePosicion, SEEK_SET);
                fwrite(&bloqueArchivo, sizeof(BloqueArchivo), 1, this->file);
                this->flagGlobal=true;
                nuevoPuntero2.b_pointers[0] = bloquePosicion;

                for (int j = 1; j < 16; j++) {
                    nuevoPuntero1.b_pointers[j] = -1;
                    nuevoPuntero2.b_pointers[j] = -1;
                }
                fseek(this->file, inodo.i_block[i], SEEK_SET);
                fwrite(&nuevoPuntero1, sizeof(BloqueApuntador), 1, this->file);
                fseek(this->file, nuevoPuntero1.b_pointers[0], SEEK_SET);
                fwrite(&nuevoPuntero2, sizeof(BloqueApuntador), 1, this->file);

                bit_2 = 0;
                for (int BMPos = blck_Start; BMPos < blck_End; BMPos++) {
                    fseek(this->file, BMPos, SEEK_SET);
                    fread(&bit_1, sizeof(char), 1, this->file);
                    if (bit_1 == '0') break;
                    bit_2++;
                }
                this->sb.s_free_blocks_count -= 2;
                this->sb.s_first_blo = bit_2;
                return inodo;
            } else {
                cout << "BLOQUES INSUFICIENTES PARA REALIZAR ESTE COMANDO" << endl;
                return tablaInodo;
            }

        }
        // si no existía se crea el bloque en la dirección indicada.
        else if (i == 13 && inodo.i_block[i] != -1) {
            fseek(this->file, inodo.i_block[i], SEEK_SET);
            fread(&puntero1, sizeof(BloqueApuntador), 1, this->file);
            for (int p1 = 0; p1 < 16; p1++) {
                if (puntero1.b_pointers[p1] != -1){
                    fseek(this->file, puntero1.b_pointers[p1], SEEK_SET);
                    fread(&puntero2, sizeof(BloqueApuntador), 1, this->file);
                    for (int p2 = 0; p2 < 16; p2++) {
                        if (puntero2.b_pointers[p2] == -1 && noBlckBitMap != -1) {
                            BloqueArchivo archivo;
                            int posBloque=this->sb.s_block_start+(noBlckBitMap * sizeof(BloqueArchivo));
                            strcpy(archivo.b_content, cadena.c_str());
                            fseek(this->file, posBloque, SEEK_SET);
                            fwrite(&archivo, sizeof(BloqueArchivo), 1, this->file);
                            this->flagGlobal=true;
                            puntero2.b_pointers[p2] = posBloque;
                            fseek(this->file, puntero1.b_pointers[p1], SEEK_SET);
                            fwrite(&puntero2, sizeof(BloqueApuntador), 1, this->file);
                            return inodo;
                        } else if ((puntero2.b_pointers[p2] != -1) && (blckActual == (28 + p2 + (16 * p1)))){
                            BloqueArchivo archivo;
                            fseek(this->file, puntero2.b_pointers[p2], SEEK_SET);
                            fread(&archivo, sizeof(BloqueArchivo),1,this->file);
                            strcpy(archivo.b_content, cadena.c_str());
                            fseek(this->file, puntero2.b_pointers[p2], SEEK_SET);
                            fwrite(&archivo, sizeof(BloqueArchivo),1,this->file);
                            return inodo;
                        }
                    }
                }else if (puntero1.b_pointers[p1] == -1 && noBlckBitMap != -1) {
                    if (this->sb.s_free_blocks_count > 0) {
                        puntero1.b_pointers[p1] = this->sb.s_block_start + (this->sb.s_first_blo * sizeof(BloqueApuntador));

                        BloqueArchivo archivo;
                        int posBloque=this->sb.s_block_start+(noBlckBitMap * sizeof(BloqueArchivo));
                        strcpy(archivo.b_content, cadena.c_str());
                        fseek(this->file, posBloque, SEEK_SET);
                        fwrite(&archivo, sizeof(BloqueArchivo), 1, this->file);
                        this->flagGlobal=true;
                        nuevoPuntero2.b_pointers[0] = posBloque;
                        for (int inicializer = 1; inicializer < 16; inicializer++) {
                            nuevoPuntero2.b_pointers[inicializer] = -1;
                        }

                        fseek(this->file, puntero1.b_pointers[p1], SEEK_SET);
                        fwrite(&nuevoPuntero2, sizeof(BloqueApuntador), 1, this->file);

                        fseek(this->file, inodo.i_block[i], SEEK_SET);
                        fwrite(&puntero1, sizeof(BloqueApuntador), 1, this->file);

                        int start = this->sb.s_bm_block_start;
                        int end = start + this->sb.s_blocks_count;
                        int bit2 = 0;
                        char bit;
                        bool bandera = false;
                        char one = '1';
                        //actualizacion sb
                        for (int bmI = start; bmI < end; bmI++) {
                            fseek(this->file, bmI, SEEK_SET);
                            fread(&bit, sizeof(char), 1, this->file);
                            if (bit == '0' && bandera)break;
                            if (bit == '0' && !bandera) {
                                fseek(this->file, bmI, SEEK_SET);
                                fwrite(&one, sizeof(char), 1, this->file);
                                bandera = true;
                            }
                            bit2++;
                        }
                        this->sb.s_free_blocks_count -= 1;
                        this->sb.s_first_blo = bit2;
                        return inodo;

                    } else {
                        cout << "NO HAY SUFICIENTES BLOQUES" << endl;
                        cout << "" << endl;
                        return tablaInodo;
                    }
                }
            }
        }
        // PUNTEROS TRIPLES
        // si ya existía el bloque se modifica con la nueva info.
        else if ((i == 14) && (inodo.i_block[i] == -1) && noBlckBitMap != -1) {
            if (this->sb.s_free_blocks_count > 2) {
                // Paso para el primer apuntador
                int bit2 = 0;
                char bit;
                char one = '1';
                int start = this->sb.s_bm_block_start;
                int end = start + this->sb.s_blocks_count;
                for (int i = start; i < end; i++) {
                    fseek(this->file, i, SEEK_SET);
                    fread(&bit, sizeof(char), 1, this->file);
                    if (bit == '0') {
                        fseek(this->file, i, SEEK_SET);
                        fwrite(&one, sizeof(char), 1, this->file);
                        break;
                    }
                    bit2++;
                }

                inodo.i_block[i] = this->sb.s_block_start + (bit2 * sizeof(BloqueApuntador));

                // Paso para el segundo apuntador
                bit2=0;
                for (int i = start; i < end; i++) {
                    fseek(this->file, i, SEEK_SET);
                    fread(&bit, sizeof(char), 1, this->file);
                    if (bit == '0') {
                        fseek(this->file, i, SEEK_SET);
                        fwrite(&one, sizeof(char), 1, this->file);
                        break;
                    }
                    bit2++;
                }

                nuevoPuntero1.b_pointers[0] = this->sb.s_block_start + (bit2 * sizeof(BloqueApuntador));

                // Paso para el tercer apuntador
                bit2=0;
                for (int i = start; i < end; i++) {
                    fseek(this->file, i, SEEK_SET);
                    fread(&bit, sizeof(char), 1, this->file);
                    if (bit == '0') {
                        fseek(this->file, i, SEEK_SET);
                        fwrite(&one, sizeof(char), 1, this->file);
                        break;
                    }
                    bit2++;
                }
                nuevoPuntero2.b_pointers[0] = this->sb.s_block_start + (bit2 * sizeof(BloqueApuntador));

                BloqueArchivo archivo;
                int posBloque=this->sb.s_block_start+(noBlckBitMap * sizeof(BloqueArchivo));
                strcpy(archivo.b_content, cadena.c_str());
                fseek(this->file, posBloque, SEEK_SET);
                fwrite(&archivo, sizeof(BloqueArchivo), 1, this->file);
                this->flagGlobal=true;
                nuevoPuntero3.b_pointers[0] = posBloque;

                for (int j = 1; j < 16; j++) {
                    nuevoPuntero1.b_pointers[j] = -1;
                    nuevoPuntero2.b_pointers[j] = -1;
                    nuevoPuntero3.b_pointers[j] = -1;
                }
                fseek(this->file, inodo.i_block[i], SEEK_SET);
                fwrite(&nuevoPuntero1, sizeof(BloqueApuntador), 1, this->file);
                fseek(this->file, nuevoPuntero1.b_pointers[0], SEEK_SET);
                fwrite(&nuevoPuntero2, sizeof(BloqueApuntador), 1, this->file);
                fseek(this->file, nuevoPuntero2.b_pointers[0], SEEK_SET);
                fwrite(&nuevoPuntero3, sizeof(BloqueApuntador), 1, this->file);

                bit2 = 0;
                for (int bmI = start; bmI < end; bmI++) {
                    fseek(this->file, bmI, SEEK_SET);
                    fread(&bit, sizeof(char), 1, this->file);
                    if (bit == '0') break;
                    bit2++;
                }
                this->sb.s_free_blocks_count -= 3;
                this->sb.s_first_blo = bit2;
                return inodo;
            } else {
                cout << "BLOQUES INSUFICIENTES PARA REALIZAR ESTE COMANDO" << endl;
                return tablaInodo;
            }
        }
        // si no existía se crea el bloque en la dirección indicada.
        else if ((i == 14) && (inodo.i_block[i] != -1)) {
            fseek(this->file, inodo.i_block[i], SEEK_SET);
            fread(&puntero1, sizeof(BloqueApuntador), 1, this->file);
            for (int pun1 = 0; pun1 < 16; pun1++) {
                if (puntero1.b_pointers[pun1] != -1){
                    fseek(this->file, puntero1.b_pointers[pun1], SEEK_SET);
                    fread(&puntero2, sizeof(BloqueApuntador), 1, this->file);
                    for (int pun2 = 0; pun2 < 16; pun2++) {
                        if (puntero2.b_pointers[pun2] != -1){
                            fseek(this->file, puntero2.b_pointers[pun2], SEEK_SET);
                            fread(&puntero3, sizeof(BloqueApuntador), 1, this->file);
                            for (int pun3 = 0; pun3 < 16; ++pun3) {
                                if (puntero3.b_pointers[pun3] == -1 && noBlckBitMap != -1){
                                    BloqueArchivo bloqueArchivo;
                                    int bloquePosicion= this->sb.s_block_start + (noBlckBitMap * sizeof(BloqueArchivo));
                                    strcpy(bloqueArchivo.b_content, cadena.c_str());
                                    fseek(this->file, bloquePosicion, SEEK_SET);
                                    fwrite(&bloqueArchivo, sizeof(BloqueArchivo), 1, this->file);
                                    this->flagGlobal=true;
                                    puntero3.b_pointers[pun3] = bloquePosicion;
                                    fseek(this->file, puntero2.b_pointers[pun2], SEEK_SET);
                                    fwrite(&puntero3, sizeof(BloqueApuntador), 1, this->file);
                                    return inodo;
                                // recordar lo de aquí
                                }else if ((puntero3.b_pointers[pun3] != -1) && (blckActual == (284 + pun3 + (16 * pun2) + (256 * pun1)))){
                                    BloqueArchivo bloqueArchivo;
                                    fseek(this->file, puntero3.b_pointers[pun3], SEEK_SET);
                                    fread(&bloqueArchivo, sizeof(BloqueArchivo), 1, this->file);
                                    strcpy(bloqueArchivo.b_content, cadena.c_str());
                                    fseek(this->file, puntero3.b_pointers[pun3], SEEK_SET);
                                    fwrite(&bloqueArchivo, sizeof(BloqueArchivo), 1, this->file);
                                    return inodo;
                                }
                            }
                        }else if (puntero2.b_pointers[pun2] == -1 && noBlckBitMap != -1) {
                            if (this->sb.s_free_blocks_count > 0) {
                                puntero2.b_pointers[pun2] = this->sb.s_block_start + (this->sb.s_first_blo * sizeof(BloqueApuntador));

                                BloqueArchivo archivo;
                                int posBloque=this->sb.s_block_start+(noBlckBitMap * sizeof(BloqueArchivo));
                                strcpy(archivo.b_content, cadena.c_str());
                                fseek(this->file, posBloque, SEEK_SET);
                                fwrite(&archivo, sizeof(BloqueArchivo), 1, this->file);
                                this->flagGlobal=true;
                                nuevoPuntero3.b_pointers[0] = posBloque;
                                for (int inicializer = 1; inicializer < 16; inicializer++) {
                                    nuevoPuntero3.b_pointers[inicializer] = -1;
                                }

                                fseek(this->file, puntero2.b_pointers[pun2], SEEK_SET);
                                fwrite(&nuevoPuntero3, sizeof(BloqueApuntador), 1, this->file);

                                fseek(this->file, puntero1.b_pointers[pun1], SEEK_SET);
                                fwrite(&puntero2, sizeof(BloqueApuntador), 1, this->file);

                                int start = this->sb.s_bm_block_start;
                                int end = start + this->sb.s_blocks_count;
                                int bit2 = 0;
                                char bit;
                                bool bandera = false;
                                char one = '1';
                                //actualizacion sb
                                for (int bmI = start; bmI < end; bmI++) {
                                    fseek(this->file, bmI, SEEK_SET);
                                    fread(&bit, sizeof(char), 1, this->file);
                                    if (bit == '0' && bandera)break;
                                    if (bit == '0' && !bandera) {
                                        fseek(this->file, bmI, SEEK_SET);
                                        fwrite(&one, sizeof(char), 1, this->file);
                                        bandera = true;
                                    }
                                    bit2++;
                                }
                                this->sb.s_free_blocks_count -= 1;
                                this->sb.s_first_blo = bit2;
                                return inodo;

                            } else {
                                cout << "NO HAY SUFICIENTES BLOQUES" << endl;
                                cout << "" << endl;
                                return tablaInodo;
                            }
                        }
                    }
                }else if (puntero1.b_pointers[pun1] == -1 && noBlckBitMap != -1) {
                    if (this->sb.s_free_blocks_count > 1) {
                        puntero1.b_pointers[pun1] = this->sb.s_block_start + (this->sb.s_first_blo * sizeof(BloqueApuntador));
                        int start = this->sb.s_bm_block_start;
                        int end = start + this->sb.s_blocks_count;
                        int bit2 = 0;
                        char bit;
                        bool bandera = false;
                        char one = '1';
                        //actualizacion sb
                        for (int bmI = start; bmI < end; bmI++) {
                            fseek(this->file, bmI, SEEK_SET);
                            fread(&bit, sizeof(char), 1, this->file);
                            if (bit == '0' && bandera)break;
                            if (bit == '0' && !bandera) {
                                fseek(this->file, bmI, SEEK_SET);
                                fwrite(&one, sizeof(char), 1, this->file);
                                bandera = true;
                            }
                            bit2++;
                        }
                        this->sb.s_free_blocks_count -= 1;
                        this->sb.s_first_blo = bit2;

                        nuevoPuntero2.b_pointers[0] = this->sb.s_block_start + (this->sb.s_first_blo * sizeof(BloqueApuntador));

                        BloqueArchivo archivo;
                        int posBloque=this->sb.s_block_start+(noBlckBitMap * sizeof(BloqueArchivo));
                        strcpy(archivo.b_content, cadena.c_str());
                        fseek(this->file, posBloque, SEEK_SET);
                        fwrite(&archivo, sizeof(BloqueArchivo), 1, this->file);
                        this->flagGlobal=true;
                        nuevoPuntero3.b_pointers[0] = posBloque;
                        for (int inicializer = 1; inicializer < 16; inicializer++) {
                            nuevoPuntero2.b_pointers[inicializer] = -1;
                            nuevoPuntero3.b_pointers[inicializer] = -1;
                        }

                        fseek(this->file, puntero1.b_pointers[pun1], SEEK_SET);
                        fwrite(&nuevoPuntero2, sizeof(BloqueApuntador), 1, this->file);

                        fseek(this->file, nuevoPuntero2.b_pointers[0], SEEK_SET);
                        fwrite(&nuevoPuntero3, sizeof(BloqueApuntador), 1, this->file);

                        fseek(this->file, inodo.i_block[i], SEEK_SET);
                        fwrite(&puntero1, sizeof(BloqueApuntador), 1, this->file);

                        bit2 = 0;
                        bandera = false;
                        //actualizacion sb
                        for (int bmI = start; bmI < end; bmI++) {
                            fseek(this->file, bmI, SEEK_SET);
                            fread(&bit, sizeof(char), 1, this->file);
                            if (bit == '0' && bandera)break;
                            if (bit == '0' && !bandera) {
                                fseek(this->file, bmI, SEEK_SET);
                                fwrite(&one, sizeof(char), 1, this->file);
                                bandera = true;
                            }
                            bit2++;
                        }
                        this->sb.s_free_blocks_count -= 1;
                        this->sb.s_first_blo = bit2;
                        return inodo;

                    } else {
                        cout << "NO HAY SUFICIENTES BLOQUES" << endl;
                        cout << "" << endl;
                        return tablaInodo;
                    }

                }
            }
        }
    }
    return tablaInodo;
}

bool AdminUsuarios::verificarGrupoExistencia( string name, vector<string> listaGrupos) {
    vector<string> camposGrupo;
    for (int i = 0; i < listaGrupos.size(); ++i) {
        camposGrupo = getCampos(listaGrupos[i]);
        if (camposGrupo[2] == name && camposGrupo[0] != "0"){
            return true;
        }
    }
    return false;
}

string AdminUsuarios::generarNuevoIdGrupos(vector<string> grupos) {
    int idGrupo=0;
    vector<string> camposGrupo;
    for (int i = 0; i < grupos.size(); ++i) {
        camposGrupo= getCampos(grupos[i]);
        int id= stoi(camposGrupo[0]);
        if (idGrupo < id){
            idGrupo++;
        }
    }
    return to_string(idGrupo + 1);
}

void AdminUsuarios::registrarJournal(string tipo_Op, char tipo, string nombre, string contenido, Nodo_M *nodo) {
    Journal ultimoJournal,journalNuevo;
    // Posicionarnos después del SuperBloque, que es donde viene el Journal
    if (nodo->type=='l'){
        fseek(this->file,nodo->start+ sizeof(EBR)+ sizeof(SuperBloque),SEEK_SET);
    }else if (nodo->type=='p'){
        fseek(this->file,nodo->start+ sizeof(SuperBloque),SEEK_SET);
    }
    // Leer el primer Journal
    fread(&ultimoJournal, sizeof(Journal), 1, this->file);
    // Recorrer Journals hasta el último ingresado
    while (ultimoJournal.journal_Sig != -1){
        fseek(this->file, ultimoJournal.journal_Start + sizeof(Journal), SEEK_SET);
        fread(&ultimoJournal, sizeof(Journal), 1, this->file);
    }
    // Validar que todavía entre un Journal
    if (this->sb.s_bm_inode_start < (ultimoJournal.journal_Start + sizeof(Journal))){
        return;
    }
    // Ingresar el Journal nuevo
    ultimoJournal.journal_Sig= ultimoJournal.journal_Start + sizeof(Journal);
    journalNuevo.journal_Start=ultimoJournal.journal_Sig;
    strcpy(journalNuevo.journal_Tipo_Operacion, tipo_Op.c_str());
    journalNuevo.journal_Tipo=tipo;
    strcpy(journalNuevo.journal_Path, nombre.c_str());
    strcpy(journalNuevo.journal_Contenido, contenido.c_str());
    journalNuevo.journal_Fecha = time(nullptr);
    journalNuevo.journal_Sig=-1;
    // Escribiendolo después del último Journal
    fseek(this->file, ultimoJournal.journal_Start, SEEK_SET);
    fwrite(&ultimoJournal, sizeof(Journal), 1, this->file);
    fseek(this->file, journalNuevo.journal_Start, SEEK_SET);
    fwrite(&journalNuevo, sizeof(Journal), 1, this->file);
    return;
}

void AdminUsuarios::rmgrp() {
    if (this->usuario->idU==1 && this->usuario->idG==1){
        if (this->name==" "){
            cout<<"EL NOMBRE DEL GRUPO A BORRAR ES OBLIGATORIO";
            return;
        }
        Nodo_M *nodoParticionMontada=this->mountList->buscar(this->usuario->idMount);
        if (nodoParticionMontada != NULL) {
            if ((this->file= fopen(nodoParticionMontada->path.c_str(), "rb+"))){
                // Obteniendo el superbloque para poder acceder a los inodos y luego a los bloques del archivo users.txt
                if (nodoParticionMontada->type == 'p'){
                    fseek(this->file, nodoParticionMontada->start, SEEK_SET);
                    fread(&this->sb, sizeof(SuperBloque),1,this->file);
                }
                else if (nodoParticionMontada->type == 'l'){
                    fseek(this->file, nodoParticionMontada->start + sizeof(EBR), SEEK_SET);
                    fread(&this->sb, sizeof(SuperBloque),1,this->file);
                }
                // Listando grupos
                string contenidoArchivo = this->getStringAlmacenadoInodo(this->sb.s_inode_start + sizeof(TablaInodo));
                vector<string>listadoGrupos = this->getGrupos(contenidoArchivo);
                // Verificando si existe el grupo
                if (this->verificarGrupoExistencia(this->name, listadoGrupos)){
                    for (int i = 0; i < listadoGrupos.size(); ++i) { // iterar cada grupo y buscar el grupo que es
                        vector<string> camposGrupoIteracion = this->getCampos(listadoGrupos[i]); //id, tipo, nombre
                        if (camposGrupoIteracion[2] == this->name){ // si el nombre coincide
                            int posGrupoEncontrado=contenidoArchivo.find(listadoGrupos[i]); // guardar la posGrupoEncontrado
                            int idGrupoEncontrado=listadoGrupos[i].find(',');
                            contenidoArchivo.replace(posGrupoEncontrado, idGrupoEncontrado, "0"); // poniendo "0" en el grupo para identificar que esta eliminado
                        }
                    }
                    vector<string> arrayBlks=this->getArrayBlks(contenidoArchivo);

                    TablaInodo tablaInodo;
                    int direccionInodo = this->sb.s_inode_start + sizeof(TablaInodo);
                    fseek(this->file, direccionInodo, SEEK_SET);
                    fread(&tablaInodo, sizeof(TablaInodo), 1, this->file);

                    int size = 0;
                    for (int k = 0; k < arrayBlks.size(); k++) {
                        size += arrayBlks[k].length();
                    }
                    tablaInodo.i_s=size;
                    tablaInodo.i_atime = time(nullptr);
                    tablaInodo.i_mtime = time(nullptr);

                    int j=0;
                    while (j < arrayBlks.size()){
                        tablaInodo=this->addFile(j, -1, arrayBlks[j], tablaInodo);
                        j++;
                    }

                    fseek(this->file, direccionInodo, SEEK_SET);
                    fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);
                    if (nodoParticionMontada->type == 'p'){
                        fseek(this->file, nodoParticionMontada->start, SEEK_SET);
                        fwrite(&this->sb, sizeof(SuperBloque),1,this->file);
                    }else if (nodoParticionMontada->type == 'l'){
                        fseek(this->file, nodoParticionMontada->start + sizeof(EBR), SEEK_SET);
                        fwrite(&this->sb, sizeof(SuperBloque),1,this->file);
                    }

                    if (this->sb.s_filesystem_type==3){
                        this->registrarJournal("rmgrp", '1', "users.txt", this->name, nodoParticionMontada);
                    }

                    fclose(this->file);
                    cout << "EL COMANDO SE HA REALIZADO CON EXITO Y SE HA REMOVIDO EL GRUPO"<<this->name<< endl;

                }
                else{
                    cout<<"NO EXISTE EL GRUPO CON NOMBRE "<<this->name<< " EN EL LISTADO DE GRUPOS"<<endl;
                }
            }else{
                cout <<"EL DISCO NO EXISTE O NO SE ENCONTRO"<<endl;
                return;
            }
        }else{
            cout <<"VERIFICAR MONTURAS, PORQUE NO SE ENCONTRO "<< this->usuario->idMount<< " EN LOS ID'S DE MONTURAS EXISTENTES"<<endl;
            return;
        }
    }else{
        cout <<"SOLAMENTO EL USUARIO ROOT PUEDE EJECUTAR ESTO"<<endl;
    }
}

void AdminUsuarios::mkusr() {
    if (this->usuario->idU==1 && this->usuario->idG==1){
        if (this->name==" " || this->pass==" " || this->group==" "){
            cout<<"ERROR SE NECESITAN QUE ESTEN TODOS LOS CAMPOS DE USUARIO PARA EJECUTAR";
            return;
        }
        Nodo_M *nodo=this->mountList->buscar(this->usuario->idMount);
        if (nodo!=NULL) {
            if ((this->file= fopen(nodo->path.c_str(),"rb+"))){
                // Extrayendo superbloque
                if (nodo->type=='l'){
                    fseek(this->file,nodo->start+ sizeof(EBR),SEEK_SET);
                    fread(&this->sb, sizeof(SuperBloque),1,this->file);
                }else if (nodo->type=='p'){
                    fseek(this->file,nodo->start,SEEK_SET);
                    fread(&this->sb, sizeof(SuperBloque),1,this->file);
                }
                // Extrayendo Inodo y datos de usuarios.
                string textoInodo=this->getStringAlmacenadoInodo(this->sb.s_inode_start + sizeof(TablaInodo));
                // Extrayendo información de los datos almacenados
                vector<string>listadoUsuarios=this->getUsers(textoInodo);
                vector<string>listadoGrupos=this->getGrupos(textoInodo);
                int blksActuales=this->getArrayBlks(textoInodo).size();
                string newUser="";
                // Validar existencia usuario
                if (this->validarUserExistencia(this->name, listadoUsuarios)){
                    cout<<"ERROR EL USUARIO QUE SE QUIERE INGRESAR YA EXISTE"<<endl;
                    return;
                }

                if (this->verificarGrupoExistencia(this->group, listadoGrupos)){
                    newUser = getUID(listadoUsuarios) + ",U," + this->group + "," + this->name + "," + this->pass + "\n";
                    textoInodo+=newUser;
                    vector<string> blcksInodoActualizado =this->getArrayBlks(textoInodo);
                    int blksActualizado=blcksInodoActualizado.size();

                    if (this->sb.s_free_blocks_count<(blksActualizado - blksActuales)){
                        cout << "NO EXISTEN LOS BLOQUES LIBRES EN EL SISTEMA PARA INGRESAR OTRO USUARIO" << endl;
                        return;
                    }

                    if (blcksInodoActualizado.size() > 4380){
                        cout<<"SE HA ALCANZADO EL LIMITE DE BLOQUES EN EL INODO ASIGNADO";
                        return;
                    }

                    // si es necesario actualizar el bitmap de bloques
                    int bitMapBlks_Start = this->sb.s_bm_block_start;
                    int bitMapBlks_End = bitMapBlks_Start + this->sb.s_block_start;
                    int blksLibresSeguidos = 0;
                    int inicioBM = -1;
                    int startB = -1;
                    int contAux = 0;
                    char bit;

                    if ((blksActualizado - blksActuales) > 0){
                        for (int i = bitMapBlks_Start; i < bitMapBlks_End; ++i) {
                            fseek(this->file, i, SEEK_SET);
                            fread(&bit, sizeof(char), 1, this->file);

                            if (bit == '1') {// Bloque no disponible
                                blksLibresSeguidos = 0;
                                inicioBM = -1;
                                startB = -1;
                            }
                            else {// Bloque disponible
                                if (blksLibresSeguidos == 0) {
                                    inicioBM = i;
                                    startB = contAux;
                                }
                                blksLibresSeguidos++;
                            }

                            if (blksLibresSeguidos >= (blksActualizado - blksActuales)) break;
                            contAux++;
                        }
                        if ((blksLibresSeguidos != (blksActualizado - blksActuales)) || inicioBM==-1){
                            cout << "NO EXISTEN LOS SUFICIENTES BLOQUES O BLOQUES SEGUIDOS EN EL SISTEMA PARA ACTUALIZAR EL ARCHIVO" << endl;
                            return;
                        }
                        //Escribiendo en el bloque de bitmaps los bitmaps a utilizar
                        for (int i = inicioBM; i < (inicioBM+(blksActualizado - blksActuales)); ++i) {
                            char uno='1';
                            fseek(this->file,i,SEEK_SET);
                            fwrite(&uno, sizeof(char),1,this->file);

                        }
                        this->sb.s_free_blocks_count-=(blksActualizado - blksActuales); // actualizando el número de bloques libres
                        int bit_2 = 0;
                        //nueva posicion disponible
                        for (int k = bitMapBlks_Start; k < bitMapBlks_End; k++) {
                            fseek(this->file, k, SEEK_SET);
                            fread(&bit, sizeof(char), 1, this->file);
                            if (bit == '0') break;
                            bit_2++;
                        }
                        this->sb.s_first_blo = bit_2;
                    }

                    TablaInodo tablaInodo;
                    int seekInodo=this->sb.s_inode_start+ sizeof(TablaInodo);
                    fseek(this->file,seekInodo,SEEK_SET);
                    fread(&tablaInodo, sizeof(TablaInodo), 1, this->file);

                    // Calculando Size con los nuevos bloques
                    int size = 0;
                    for (int k = 0; k < blcksInodoActualizado.size(); k++) {
                        size += blcksInodoActualizado[k].length();
                    }
                    // Actualizando el nuevo size y tiempo de actualización
                    tablaInodo.i_s=size;
                    tablaInodo.i_mtime = time(nullptr);

                    int j=0,cont=0;
                    while (j < blcksInodoActualizado.size()){
                        flagGlobal=false;
                        tablaInodo = this->addFile(j, (startB + cont), blcksInodoActualizado[j], tablaInodo);
                        if (flagGlobal){
                            cont++;
                        }
                        j++;
                    }

                    fseek(this->file,seekInodo,SEEK_SET);
                    fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);

                    if (nodo->type=='l'){
                        fseek(this->file,nodo->start+ sizeof(EBR),SEEK_SET);
                        fwrite(&this->sb, sizeof(SuperBloque),1,this->file);
                    }else if (nodo->type=='p'){
                        fseek(this->file,nodo->start,SEEK_SET);
                        fwrite(&this->sb, sizeof(SuperBloque),1,this->file);
                    }

                    if (this->sb.s_filesystem_type==3){
                        this->registrarJournal("mkgrp", '1', "users.txt", newUser, nodo);
                    }

                    fclose(this->file);
                    cout << "USUARIO "<<this->name<<" REGISTRADO EN EL SISTEMA"<<endl;

                }else{
                    cout<<"EL GRUPO "<<this->group<<" AL CUAL SE QUIERE ASIGNAR EL USUARIO NO EXISTE"<<endl;
                }
            }else{
                cout <<"ERROR EL DISCO SE MOVIO O DEJO DE EXISTIR"<<endl;
            }
        }else{
            cout <<"ERROR NO EXISTE O NO SE ENCONTRO MONTURA CON ID: "<< this->usuario->idMount<<" EN LA LISTA DE MONTURAS"<<endl;
        }
    }else{
        cout <<"ERROR SOLO COMANDO ROOT CON GRUPO 1 PUEDE EJECUTAR ESTE COMANDO"<<endl;
    }
}

bool AdminUsuarios::validarUserExistencia(string name, vector<string> usuarios) {
    vector<string> camposUsuario;
    for (int i = 0; i < usuarios.size(); ++i) {
        camposUsuario = getCampos(usuarios[i]);
        if (camposUsuario[3] == name && camposUsuario[0] != "0"){
            return true;
        }
    }
    return false;
}

void AdminUsuarios::rmusr() {

}

void AdminUsuarios::chgrp() {

}




