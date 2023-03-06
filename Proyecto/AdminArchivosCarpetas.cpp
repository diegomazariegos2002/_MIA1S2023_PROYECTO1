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
            int direccionInodoArchivo;

            // (int numCarpetas, int rutaActual, FILE *discoActual, vector<string> rutaDividida, int direccionActual)
            direccionInodoArchivo = this->getDireccionInodoFile(rutaDividida, 0, rutaDividida.size() - 1, this->sb.s_inode_start, this->file);
            if (direccionInodoArchivo == -1){
                cout << "ARCHIVO NO ENCONTRADO " << endl;
                fclose(this->file);
                return;
            }


            if (!this->verificarPermisoInodo_Lectura(direccionInodoArchivo)){
                cout<<"ERROR NO SE POSEEN LOS PERMISOS DE LECTURA PARA "<< rutaDividida[rutaDividida.size()-1]<<endl;
                fclose(this->file);
                return;
            }

            // Si hay permisos

            fseek(this->file, direccionInodoArchivo, SEEK_SET);
            fread(&tablaInodo, sizeof(TablaInodo), 1, this->file);

            if (tablaInodo.i_type == '0'){
                cout<<"ERROR LA RUTA NO HACE REFERENCIA A UN ARCHIVO: "<< this->path <<endl;
                fclose(this->file);
                return;
            }

            tablaInodo.i_atime= time(nullptr);
            string stringInodoArchivo=this->getStringAlmacenadoInodo(direccionInodoArchivo);
            // Imprimir el contenido del Inodo.
            cout << stringInodoArchivo << endl;

            fseek(this->file, direccionInodoArchivo, SEEK_SET);
            fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);

            if (this->sb.s_filesystem_type==3){
                this->registrarJournal("cat",'1',this->path,"",nodo);
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

    // si se encuentra un inodo Archivo
    if (tablaInodo.i_type == '1'){
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

bool AdminArchivosCarpetas::verificarPermisoInodo_Lectura(int direccionInodo) {
    TablaInodo tablaInodo;
    fseek(this->file,direccionInodo,SEEK_SET);
    fread(&tablaInodo, sizeof(TablaInodo), 1, this->file);
    string permisos= to_string(tablaInodo.i_perm);

    if (permisos.length() == 2){
        permisos= "0" + permisos;
    }
    else if (permisos.length() == 1){
        permisos= "00" + permisos;
    }

    // si es el admin
    if (this->usuario->idU==1 && this->usuario->idG==1){
        return true;
    }
    // si el usuario pertenece al grupo
    else if (tablaInodo.i_gid == usuario->idG){
        // 4 = lectura, 5 = ......
        if ((permisos[1] == '4') || (permisos[1] == '5') || (permisos[1] == '6') || (permisos[1] == '7')){
            return true;
        }
    }
    // si es el propietario
    else if ((tablaInodo.i_uid == usuario->idU) && (tablaInodo.i_gid == usuario->idG)){
        if ((permisos[0] == '4') || (permisos[0] == '5') || (permisos[0] == '6') || (permisos[0] == '7')){
            return true;
        }
    }
    // otros
    else{
        if ((permisos[2] == '4') || (permisos[2] == '5') || (permisos[2] == '6') || (permisos[2] == '7')){
            return true;
        }
    }
    return false;
}
string AdminArchivosCarpetas::getStringAlmacenadoInodo(int startInodo) {
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
void AdminArchivosCarpetas::registrarJournal(string tipo_Op, char tipo, string nombre, string contenido, Nodo_M *nodo) {
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

