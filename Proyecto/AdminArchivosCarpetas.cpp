#include <iostream>
#include <cstring>
#include <sstream>
#include "AdminArchivosCarpetas.h"


AdminArchivosCarpetas::AdminArchivosCarpetas() {

}

void AdminArchivosCarpetas::cat() {
    if (this->rutasCat.size()==0){
        cout<<"ES OBLIGATORIO INGRESAR AL MENOS UNA RUTA"<<endl;
        return;
    }
    Nodo_M *nodo=this->mountList->buscar(usuario->idMount);
    if (nodo==NULL){
        cout <<"MONTURA CON EL ID: "<< this->usuario->idMount<< " IMPOSIBLE DE ENCONTRAR"<<endl;
        return;
    }
    // Leer status de la respectiva partición.
    if ((this->file= fopen(nodo->path.c_str(),"rb+"))){
        if (nodo->type=='l'){
            EBR ebr;
            fseek(file,nodo->start,SEEK_SET);
            fread(&ebr, sizeof(EBR),1,file);
            if (ebr.part_status != '2') {
                fclose(file);
                cout<<"LA PARTICION "<<nodo->name<<" NO HA SIDO FORMATEADA AL PARECER" << endl;
                return;
            }
            fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
        }
        else if (nodo->type=='p'){
            MBR mbr;
            fseek(this->file,0,SEEK_SET);
            fread(&mbr, sizeof(MBR),1,file);
            if (mbr.mbr_partition[nodo->pos].part_status!='2'){
                cout<<"LA PARTICION "<<nodo->name<<" NO HA SIDO FORMATEADA AL PARECER" << endl;
                fclose(this->file);
                return;
            }
            fseek(this->file,nodo->start,SEEK_SET);
        }

        fread(&this->sb, sizeof(SuperBloque),1,file);

        // Realizar para cada file
        for (int i = 0; i < this->rutasCat.size(); ++i) {
            this->path = this->rutasCat[i];

            vector<string> rutaDividida= this->getRutaDividida(this->path);
            if (rutaDividida.empty()){
                cout<<"UNA DE LAS RUTAS NO ES VALIDA"<<endl;
                fclose(this->file);
                return;
            }

            TablaInodo tablaInodo;
            int posicionInodoArchivo;

            if (rutaDividida.size() > 2){
                // (int numCarpetas, int rutaActual, FILE *discoActual, vector<string> rutaDividida, int direccionActual)
                posicionInodoArchivo = this->getDireccionInodoFile(rutaDividida, 0, rutaDividida.size() - 1, this->sb.s_inode_start, this->file);
                if (posicionInodoArchivo == -1){
                    cout << "ARCHIVO NO ENCONTRADO " << endl;
                    fclose(this->file);
                    return;
                }
            }else{
                posicionInodoArchivo=this->sb.s_inode_start;
            }


        }

        fclose(this->file);
    }else{
        cout <<"EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA"<<endl;
    }
}

vector<string> AdminArchivosCarpetas::getRutaDividida(std::string cadena) {
    string aux;
    stringstream aux2(cadena);
    vector<string> rutaDividida;
    while (getline(aux2, aux, '/')){
        if (aux != ""){
            rutaDividida.push_back(aux);
        }
    }
    return rutaDividida;
}
// (vector<string> rutaDividida, int direccionActual,   int numCarpetas,         int rutaActual,         FILE *discoActual)
// (          rutaDividida,               0,           rutaDividida.size() - 1,  this->sb.s_inode_start,    this->file);
int AdminArchivosCarpetas::getDireccionInodoFile(vector<string> rutaDividida, int direccionActual, int numCarpetas, int rutaActual, FILE *discoActual) {
    TablaInodo tablaInodo;
    BloqueCarpeta blck_Carpeta;
    BloqueApuntador blck_apuntador1,blck_apuntador2,blck_apuntador3;

    fseek(discoActual, rutaActual, SEEK_SET);
    fread(&tablaInodo, sizeof(TablaInodo), 1, discoActual);

    if (tablaInodo.i_type == '1'){ // si se encuentra un inodo Archivo
        cout<<"NO EXISTE UNA CARPETA EN LA RUTA ESPECIFICADA"<<endl;
        return -1;
    }

    // Si es un bloque carpeta
    for (int i = 0; i < 15; ++i) {
        if (tablaInodo.i_block[i] != -1){
            if (i<12){ // punteros directos
                fseek(discoActual, tablaInodo.i_block[i], SEEK_SET);
                fread(&blck_Carpeta, sizeof(BloqueCarpeta), 1, discoActual);
                for (int j = 0; j < 4; ++j) { // Recorrer los valores de la carpeta
                    // si existe una carpeta en bloque de carpetas que coincida con la carpeta que toca
                    if (blck_Carpeta.b_content[j].b_name == rutaDividida[direccionActual]){
                        if (direccionActual < numCarpetas) { // si todavía falta recorrer más carpetas, continuar en la siguiente
                            return this->getDireccionInodoFile(rutaDividida,
                                                   direccionActual + 1,
                                                   numCarpetas,
                                                   blck_Carpeta.b_content[j].b_inodo,
                                                   discoActual);
                        }
                        if (direccionActual == numCarpetas) { // si ya se llego al limite de carpetas, devolver el recursivo
                            return blck_Carpeta.b_content[j].b_inodo;
                        }
                    }
                }

            }
            else if (i==12){ // punteros simples
                fseek(discoActual, tablaInodo.i_block[i], SEEK_SET);
                fread(&blck_apuntador1, sizeof(BloqueApuntador), 1, discoActual);
                for (int j = 0; j < 16; ++j) {  // recorrer punteros directos
                    if (blck_apuntador1.b_pointers[j] != -1){
                        fseek(discoActual, blck_apuntador1.b_pointers[j], SEEK_SET);
                        fread(&blck_Carpeta, sizeof(BloqueCarpeta), 1, discoActual);
                        for (int k = 0; k < 4; ++k) { // recorrer bloque de carpetas
                            // si existe una carpeta en bloque de carpetas que coincida con la carpeta que toca
                            if (blck_Carpeta.b_content[k].b_name == rutaDividida[direccionActual]){
                                if (direccionActual < numCarpetas) { // si todavía falta recorrer más carpetas, continuar en la siguiente
                                    return this->getDireccionInodoFile(rutaDividida,
                                                                       direccionActual + 1,
                                                                       numCarpetas,
                                                                        blck_Carpeta.b_content[k].b_inodo,
                                                                        discoActual);
                                }
                                if (direccionActual == numCarpetas) { // si ya se llego al limite de carpetas, devolver el recursivo
                                    return blck_Carpeta.b_content[k].b_inodo;
                                }
                            }
                        }
                    }
                }
            }
            else if (i==13){ // puntero doble
                fseek(discoActual, tablaInodo.i_block[i], SEEK_SET);
                fread(&blck_apuntador1, sizeof(BloqueApuntador), 1, discoActual);
                for (int j = 0; j < 16; ++j) { // recorrer punteros simples
                    if (blck_apuntador1.b_pointers[j] != -1){
                        fseek(discoActual, blck_apuntador1.b_pointers[j], SEEK_SET);
                        fread(&blck_apuntador2, sizeof(BloqueApuntador), 1, discoActual);
                        for (int k = 0; k < 16; ++k) { // recorrer punteros directos
                            if (blck_apuntador2.b_pointers[k] != -1){
                                fseek(discoActual, blck_apuntador2.b_pointers[k], SEEK_SET);
                                fread(&blck_Carpeta, sizeof(BloqueCarpeta), 1, discoActual);
                                for (int z = 0; z < 4; ++z) { // recorrer bloque de carpetas
                                    // si existe una carpeta en bloque de carpetas que coincida con la carpeta que toca
                                    if (blck_Carpeta.b_content[z].b_name == rutaDividida[direccionActual]){
                                        if (direccionActual < numCarpetas) { // si todavía falta recorrer más carpetas, continuar en la siguiente
                                            return this->getDireccionInodoFile(rutaDividida, direccionActual + 1, numCarpetas,
                                                                               blck_Carpeta.b_content[z].b_inodo, discoActual);
                                        }
                                        if (direccionActual == numCarpetas) { // si ya se llego al limite de carpetas, devolver el recursivo
                                            return blck_Carpeta.b_content[z].b_inodo;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else if (i==14){ // puntero triple
                fseek(discoActual, tablaInodo.i_block[i], SEEK_SET);
                fread(&blck_apuntador1, sizeof(BloqueApuntador), 1, discoActual);
                for (int j = 0; j < 16; ++j) { // recorrer punteros dobles
                    if (blck_apuntador1.b_pointers[j] != -1){
                        fseek(discoActual, blck_apuntador1.b_pointers[j], SEEK_SET);
                        fread(&blck_apuntador2, sizeof(BloqueApuntador), 1, discoActual);
                        for (int k = 0; k < 16; ++k) { // recorrer punteros simples
                            if (blck_apuntador2.b_pointers[k] != -1){
                                fseek(discoActual, blck_apuntador2.b_pointers[k], SEEK_SET);
                                fread(&blck_apuntador3, sizeof(BloqueApuntador), 1, discoActual);
                                for (int z = 0; z < 16; ++z) { // recorrer punteros directos
                                    if (blck_apuntador3.b_pointers[z] != -1) {
                                        fseek(discoActual, blck_apuntador3.b_pointers[z], SEEK_SET);
                                        fread(&blck_Carpeta, sizeof(BloqueCarpeta), 1, discoActual);
                                        for (int y = 0; y < 4; ++y) {
                                            // si existe una carpeta en bloque de carpetas que coincida con la carpeta que toca
                                            if (blck_Carpeta.b_content[y].b_name == rutaDividida[direccionActual]) {
                                                if (direccionActual < numCarpetas) { // si todavía falta recorrer más carpetas, continuar en la siguiente
                                                    return this->getDireccionInodoFile(rutaDividida, direccionActual + 1, numCarpetas,
                                                                                       blck_Carpeta.b_content[y].b_inodo, discoActual);
                                                }
                                                if (direccionActual == numCarpetas) { // si ya se llego al limite de carpetas, devolver el recursivo
                                                    return blck_Carpeta.b_content[y].b_inodo;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return -1;
}
