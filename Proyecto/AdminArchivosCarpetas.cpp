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
            direccionInodoArchivo = this->getDireccionInodo(rutaDividida, 0, rutaDividida.size() - 1,
                                                            this->sb.s_inode_start, this->file);
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
int AdminArchivosCarpetas::getDireccionInodo(vector<string> rutaDividida, int direccionActual, int numCarpetas, int rutaActual, FILE *discoActual) {
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
                            return this->getDireccionInodo(rutaDividida,
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
                                    return this->getDireccionInodo(rutaDividida,
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
                                            return this->getDireccionInodo(rutaDividida, direccionActual + 1,
                                                                           numCarpetas,
                                                                           blck_Carpeta.b_content[z].b_inodo,
                                                                           discoActual);
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
                                                    return this->getDireccionInodo(rutaDividida, direccionActual + 1,
                                                                                   numCarpetas,
                                                                                   blck_Carpeta.b_content[y].b_inodo,
                                                                                   discoActual);
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

void AdminArchivosCarpetas::mkdir() {
    Nodo_M *nodo=this->mountList->buscar(usuario->idMount);
    if (nodo==NULL){
        cout <<"NO EXISTE MONTURA CON EL ID: "<< this->usuario->idMount<< "EN EL SISTEMA, FAVOR REVISAR"<<endl;
        return;
    }
    if (this->path==" "){
        cout<<"EL PARAMETRO PATH ES OBLIGATORIO Y NO DEBE ESTAR VACIO"<<endl;
        return;
    }

    if ((this->file= fopen(nodo->path.c_str(),"rb+"))){
        if (nodo->type=='l'){
            EBR ebr;
            fseek(file,nodo->start,SEEK_SET);
            fread(&ebr, sizeof(EBR),1,file);
            if (ebr.part_status != '2') {
                cout<<"MONTURA SIN FORMATEAR "<<nodo->name<< " ,VERIFICAR ESO" <<endl;
                fclose(file); return;
            }
            fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
        }
        else if (nodo->type=='p'){
            MBR mbr;
            fseek(this->file,0,SEEK_SET);
            fread(&mbr, sizeof(MBR),1,file);
            if (mbr.mbr_partition[nodo->pos].part_status!='2'){
                cout<<"MONTURA SIN FORMATEAR "<<nodo->name<< " ,VERIFICAR ESO" <<endl;
                fclose(file); return;
            }
            fseek(this->file,nodo->start,SEEK_SET);
        }

        fread(&this->sb, sizeof(SuperBloque),1,file);

        vector<string> rutaDividida= this->getRutaDividida(this->path);
        if (rutaDividida.empty()){
            cout<<"ERROR LA RUTA INGRESADA NO ES VALIDA, VERIFICAR"<<endl;
            fclose(this->file); return;
        }

        int direccionInodoRuta=this->getDireccionInodo(rutaDividida, 0, rutaDividida.size() - 1, this->sb.s_inode_start, this->file);
        if (direccionInodoRuta != -1){
            cout << "ERROR IMPOSIBLE CREAR ALGO QUE YA EXISTE: " << this->path << endl;
            fclose(this->file);
            return;
        }

        // FIN VALIDACIONES GENERALES

        // Crear carpeta
        bool existeCarpeta=true;
        int direccionInodoActual=this->sb.s_inode_start;
        // Si la ruta de la carpeta va más allá de '/' (carpeta raiz).
        if (rutaDividida.size() > 1){
            for (int i = 0; i < rutaDividida.size() - 1; ++i) {
                if (existeCarpeta){
                    int aux=direccionInodoActual;
                    direccionInodoActual= this->existeCarpeta(rutaDividida, i, direccionInodoActual);
                    if (direccionInodoActual == aux){
                        existeCarpeta= false;
                    }
                }
                if(!existeCarpeta){
                    if (this->r){
                        direccionInodoActual=this->crearCarpeta(rutaDividida, i, direccionInodoActual);
                        if (nodo->type=='p'){
                            fseek(this->file,nodo->start,SEEK_SET);
                        }else if (nodo->type=='l'){
                            fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
                        }
                        fwrite(&this->sb, sizeof(SuperBloque),1,file);
                        if (direccionInodoActual == -1){
                            return;
                        }
                    }else{
                        cout<<"NO SE PUEDE CREAR EL ARCHIVO"<<endl;
                        return;
                    }
                }
            }
        }

        if (direccionInodoActual == -1){
            cout<<"ERROR AL CREAR LA CARPETA, REVISAR TODO"<<endl;
            return;
        }
        // Si la ruta es en '/' (carpeta raiz)
        this->crearCarpeta(rutaDividida, rutaDividida.size() - 1, direccionInodoActual);
        if (nodo->type=='p'){
            fseek(this->file,nodo->start,SEEK_SET);
        }else if (nodo->type=='l'){
            fseek(this->file,nodo->start+ sizeof(EBR),SEEK_SET);
        }
        fwrite(&this->sb, sizeof(SuperBloque),1,this->file);
        if (this->sb.s_filesystem_type==3){
            this->escribirJorunal("mkdir",'1',this->path,"",nodo);
        }
        fclose(this->file);
        cout<<"SE CREO LA CARPETA "<<this->path<<endl;
        return;
    }else{
        cout <<"EL DISCO SE CAMBIO DE LUGAR O NO SE ENCUENTRA"<<endl;
    }
}

int AdminArchivosCarpetas::existeCarpeta(vector<std::string> rutaDivida, int carpetaActual, int direccionInodo) {

    BloqueApuntador puntero1,puntero2,puntero3;
    BloqueCarpeta blckCarpeta;
    TablaInodo tablaInodo;

    fseek(file, direccionInodo, SEEK_SET);
    fread(&tablaInodo, sizeof(TablaInodo), 1, file);

    if (tablaInodo.i_type == '1'){
        cout<<"ERROR SE ESTA BUSCANDO UNA CARPETA EN UN ARCHIVO"<<endl;
        return direccionInodo;
    }

    for (int i = 0; i < 15; ++i) {
        if (tablaInodo.i_block[i] != -1){
            if (i<12){
                fseek(file, tablaInodo.i_block[i], SEEK_SET);
                fread(&blckCarpeta, sizeof(BloqueCarpeta), 1, file);
                for (int j = 0; j < 4; ++j) {
                    if (blckCarpeta.b_content[j].b_name == rutaDivida[carpetaActual]){
                        return blckCarpeta.b_content[j].b_inodo;
                    }
                }

            }
            else if (i==12){
                fseek(file, tablaInodo.i_block[i], SEEK_SET);
                fread(&puntero1, sizeof(BloqueApuntador), 1, file);
                for (int j = 0; j < 16; ++j) {
                    if (puntero1.b_pointers[j] != -1){
                        fseek(file, puntero1.b_pointers[j], SEEK_SET);
                        fread(&blckCarpeta, sizeof(BloqueCarpeta), 1, file);
                        for (int k = 0; k < 4; ++k) {
                            if (blckCarpeta.b_content[k].b_name == rutaDivida[carpetaActual]){
                                return blckCarpeta.b_content[k].b_inodo;
                            }
                        }
                    }
                }
            }
            else if (i==13){
                fseek(file, tablaInodo.i_block[i], SEEK_SET);
                fread(&puntero1, sizeof(BloqueApuntador), 1, file);
                for (int j = 0; j < 16; ++j) {
                    if (puntero1.b_pointers[j] != -1){
                        fseek(file, puntero1.b_pointers[j], SEEK_SET);
                        fread(&puntero2, sizeof(BloqueApuntador), 1, file);
                        for (int k = 0; k < 16; ++k) {
                            if (puntero2.b_pointers[k] != -1){
                                fseek(file, puntero2.b_pointers[k], SEEK_SET);
                                fread(&blckCarpeta, sizeof(BloqueCarpeta), 1, file);
                                for (int z = 0; z < 4; ++z) {
                                    if (blckCarpeta.b_content[z].b_name == rutaDivida[carpetaActual]){
                                        return blckCarpeta.b_content[z].b_inodo;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else if (i==14){
                fseek(file, tablaInodo.i_block[i], SEEK_SET);
                fread(&puntero1, sizeof(BloqueApuntador), 1, file);
                for (int j = 0; j < 16; ++j) {
                    if (puntero1.b_pointers[j] != -1){
                        fseek(file, puntero1.b_pointers[j], SEEK_SET);
                        fread(&puntero2, sizeof(BloqueApuntador), 1, file);
                        for (int k = 0; k < 16; ++k) {
                            if (puntero2.b_pointers[k] != -1){
                                fseek(file, puntero2.b_pointers[k], SEEK_SET);
                                fread(&puntero3, sizeof(BloqueApuntador), 1, file);
                                for (int z = 0; z < 16; ++z) {
                                    if (puntero3.b_pointers[z] != -1) {
                                        fseek(file, puntero3.b_pointers[z], SEEK_SET);
                                        fread(&blckCarpeta, sizeof(BloqueCarpeta), 1, file);
                                        for (int y = 0; y < 4; ++y) {
                                            if (blckCarpeta.b_content[y].b_name == rutaDivida[carpetaActual]){
                                                return blckCarpeta.b_content[y].b_inodo;
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
    return direccionInodo;
}
/**
 * Procesos:
 *          - Buscar Rutas recursivamente ( Obtener la direccion final )
 *          - Validar permisos de escritura en el inodo final
 *          - Crear Inodo para la carpeta ( modificar bitmap de inodos, actualizar sb.s_firts_ino )
 *          - Crear bloque carpeta en el Inodo Anteriro ( modificar bitmap de bloques, actualizar sb.s_firsts_blck)
 * @param rutaDividida
 * @param carpetaActual
 * @param direccionInodo
 * @return
 */
int AdminArchivosCarpetas::crearCarpeta(vector<std::string> rutaDividida, int carpetaActual, int direccionInodo) {
    if (!this->verificarPermisoInodo_Escritura(direccionInodo)){
        cout << "IMPOSIBLE DE CREAR CARPETA POR FALTA DE PERMISOS EN LA CARPETA: " << rutaDividida[carpetaActual] << endl;
        return -1;
    }

    BloqueCarpeta carpeta, carpetaNueva;
    BloqueApuntador puntero_1, puntero_2, puntero_3, punteroNuevo;
    TablaInodo tablaInodo, InodoCarpetaNueva;

    int direccionCarpetaNueva=0;

    fseek(this->file, direccionInodo, SEEK_SET);
    fread(&tablaInodo, sizeof(TablaInodo), 1, this->file);

    if (tablaInodo.i_type == '1'){
        cout<<"IMPOSIBLE CREAR UNA CARPETA EN UN ARCHIVO"<<endl;
        return -1;
    }
    // Recorrido de los punteros en el inodo
    for (int i = 0; i < 15; ++i) {
        // Recordar que el -1 hace referencia a que si el puntero esta sin utilizar.
        // Punteros directos
        if (tablaInodo.i_block[i] != -1 && i < 12){ // si esta ocupado el puntero por un bloque carpeta
            fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
            fread(&carpeta, sizeof(BloqueCarpeta), 1, this->file);
            for (int j = 0; j < 4; ++j) { // Recorrer el bloque carpeta para ver si tiene espacio para guardar algo más.
                if (carpeta.b_content[j].b_inodo == -1) { // si, tiene espacio
                    if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>0) {
                        int posInodo= this->getPosicionInodoNuevo();
                        int posCarpetaI=this->crearBloqueCarpetaInicial(posInodo, direccionInodo);
                        this->crearInodoCarpeta(posInodo,posCarpetaI);
                        carpeta.b_content[j].b_inodo=posInodo;
                        strcpy(carpeta.b_content[j].b_name, rutaDividida[carpetaActual].c_str());
                        fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
                        fwrite(&carpeta, sizeof(BloqueCarpeta), 1, this->file);
                        return posInodo;
                    }else{
                        cout << "IMPOSBILE CREAR LA CARPETA " << rutaDividida[carpetaActual] << " DEBIDO A QUE YA NO HAY ESPACIO EN EL SISTEMA" << endl;
                        return -1;
                    }
                }
            }
        }
        else if (tablaInodo.i_block[i] == -1 && i < 12){
            if (this->sb.s_free_blocks_count > 1 && this->sb.s_free_inodes_count>0) {
                int posInodo= this->getPosicionInodoNuevo();
                int posCarpetaI=this->crearBloqueCarpetaInicial(posInodo, direccionInodo);
                this->crearInodoCarpeta(posInodo,posCarpetaI);
                int posCarpetaO=this->crearBloqueCarpetaOtra(posInodo, rutaDividida[carpetaActual]);
                tablaInodo.i_block[i]=posCarpetaO;
                fseek(this->file, direccionInodo, SEEK_SET);
                fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);
                return posInodo;
            }else{
                cout << "NO HAY SUFICIENTES BLOQUES" << endl;
                return -1;
            }
        }
        // Puntero simple
        else if (i == 12 && tablaInodo.i_block[i] == -1) {
            if (this->sb.s_free_blocks_count > 2 && this->sb.s_free_inodes_count>0) {
                int posInodo= this->getPosicionInodoNuevo();
                int posCarpetaI=this->crearBloqueCarpetaInicial(posInodo, direccionInodo);
                this->crearInodoCarpeta(posInodo,posCarpetaI);
                int posCarpetaO=this->crearBloqueCarpetaOtra(posInodo, rutaDividida[carpetaActual]);
                int posApuntador=this->crearBloqueApuntador(posCarpetaO);
                tablaInodo.i_block[i]=posApuntador;
                fseek(this->file, direccionInodo, SEEK_SET);
                fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);
                return posInodo;
            }else{
                cout << "NO HAY SUFICIENTES BLOQUES" << endl;
                return -1;
            }
        }
        else if (i == 12 && tablaInodo.i_block[i] != -1) {
            fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
            fread(&puntero_1, sizeof(BloqueApuntador), 1, this->file);
            for (int p1 = 0; p1 < 16; p1++) {
                if (puntero_1.b_pointers[p1] == -1) {
                    if (this->sb.s_free_blocks_count > 1 && this->sb.s_free_inodes_count>0) {
                        int posInodo= this->getPosicionInodoNuevo();
                        int posCarpetaI=this->crearBloqueCarpetaInicial(posInodo, direccionInodo);
                        this->crearInodoCarpeta(posInodo,posCarpetaI);
                        int posCarpetaO=this->crearBloqueCarpetaOtra(posInodo, rutaDividida[carpetaActual]);
                        puntero_1.b_pointers[p1]=posCarpetaO;
                        fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
                        fwrite(&puntero_1, sizeof(BloqueApuntador), 1, this->file);
                        return posInodo;
                    }else{
                        cout << "NO HAY SUFICIENTES BLOQUES" << endl;
                        return -1;
                    }
                } else if ((puntero_1.b_pointers[p1] != -1)){
                    fseek(this->file, puntero_1.b_pointers[p1], SEEK_SET);
                    fread(&carpeta, sizeof(BloqueCarpeta), 1, this->file);
                    for (int c = 0; c < 4; ++c) {
                        if (carpeta.b_content[c].b_inodo == -1) {
                            if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>0) {
                                int posInodo= this->getPosicionInodoNuevo();
                                int posCarpetaI=this->crearBloqueCarpetaInicial(posInodo, direccionInodo);
                                this->crearInodoCarpeta(posInodo,posCarpetaI);
                                carpeta.b_content[c].b_inodo=posInodo;
                                strcpy(carpeta.b_content[c].b_name, rutaDividida[carpetaActual].c_str());
                                fseek(this->file, puntero_1.b_pointers[p1], SEEK_SET);
                                fwrite(&carpeta, sizeof(BloqueCarpeta), 1, this->file);
                                return posInodo;
                            }else{
                                cout << "NO SE PUEDE CREAR LA CARPETA " << rutaDividida[carpetaActual] << endl;
                                return -1;
                            }
                        }
                    }
                }
            }
        }
        // Puntero doble
        else if (i == 13 && tablaInodo.i_block[i] == -1) {
            if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>3) {
                int posInodo= this->getPosicionInodoNuevo();
                int posCarpetaI=this->crearBloqueCarpetaInicial(posInodo, direccionInodo);
                this->crearInodoCarpeta(posInodo,posCarpetaI);
                int posCarpetaO=this->crearBloqueCarpetaOtra(posInodo, rutaDividida[carpetaActual]);
                int point1=this->crearBloqueApuntador(posCarpetaO);
                int point2=this->crearBloqueApuntador(point1);
                tablaInodo.i_block[i]=point2;
                fseek(this->file, direccionInodo, SEEK_SET);
                fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);
                return posInodo;
            }else{
                cout << "NO SE PUEDE CREAR LA CARPETA " << rutaDividida[carpetaActual] << endl;
                return -1;
            }
        }
        else if (i == 13 && tablaInodo.i_block[i] != -1) {
            fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
            fread(&puntero_1, sizeof(BloqueApuntador), 1, this->file);
            for (int p1 = 0; p1 < 16; ++p1) {
                if (puntero_1.b_pointers[p1] == -1){
                    if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>2) {
                        int posInodo= this->getPosicionInodoNuevo();
                        int posCarpetaI=this->crearBloqueCarpetaInicial(posInodo, direccionInodo);
                        this->crearInodoCarpeta(posInodo,posCarpetaI);
                        int posCarpetaO=this->crearBloqueCarpetaOtra(posInodo, rutaDividida[carpetaActual]);
                        int point1=this->crearBloqueApuntador(posCarpetaO);
                        puntero_1.b_pointers[p1]=point1;
                        fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
                        fwrite(&puntero_1, sizeof(TablaInodo), 1, this->file);
                        return posInodo;
                    }else{
                        cout << "NO SE PUEDE CREAR LA CARPETA " << rutaDividida[carpetaActual] << endl;
                        return -1;
                    }
                }else if(puntero_1.b_pointers[p1] != -1){
                    fseek(this->file, puntero_1.b_pointers[p1], SEEK_SET);
                    fread(&puntero_2, sizeof(BloqueApuntador), 1, this->file);
                    for (int p2 = 0; p2 < 16; ++p2) {
                        if (puntero_2.b_pointers[p2] == -1){
                            if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>1) {
                                int posInodo= this->getPosicionInodoNuevo();
                                int posCarpetaI=this->crearBloqueCarpetaInicial(posInodo, direccionInodo);
                                this->crearInodoCarpeta(posInodo,posCarpetaI);
                                int posCarpetaO=this->crearBloqueCarpetaOtra(posInodo, rutaDividida[carpetaActual]);
                                puntero_2.b_pointers[p2]=posCarpetaO;
                                fseek(this->file, puntero_1.b_pointers[p1], SEEK_SET);
                                fwrite(&puntero_2, sizeof(TablaInodo), 1, this->file);
                                return posInodo;
                            }else{
                                cout << "NO SE PUEDE CREAR LA CARPETA " << rutaDividida[carpetaActual] << endl;
                                return -1;
                            }
                        }else if (puntero_2.b_pointers[p2] != -1){
                            fseek(this->file, puntero_2.b_pointers[p2], SEEK_SET);
                            fread(&carpeta, sizeof(BloqueCarpeta), 1, this->file);
                            for (int c = 0; c < 4; ++c) {
                                if (carpeta.b_content[c].b_inodo == -1) {
                                    if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>0) {
                                        int posInodo= this->getPosicionInodoNuevo();
                                        int posCarpetaI=this->crearBloqueCarpetaInicial(posInodo, direccionInodo);
                                        this->crearInodoCarpeta(posInodo,posCarpetaI);
                                        carpeta.b_content[c].b_inodo=posInodo;
                                        strcpy(carpeta.b_content[c].b_name, rutaDividida[carpetaActual].c_str());
                                        fseek(this->file, puntero_2.b_pointers[p2], SEEK_SET);
                                        fwrite(&carpeta, sizeof(BloqueCarpeta), 1, this->file);
                                        return posInodo;
                                    }else{
                                        cout << "NO SE PUEDE CREAR LA CARPETA " << rutaDividida[carpetaActual] << endl;
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        // Puntero triple
        else if ((i == 14) && (tablaInodo.i_block[i] == -1)) {
            if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>4) {
                int posInodo= this->getPosicionInodoNuevo();
                int posCarpetaI=this->crearBloqueCarpetaInicial(posInodo, direccionInodo);
                this->crearInodoCarpeta(posInodo,posCarpetaI);
                int posCarpetaO=this->crearBloqueCarpetaOtra(posInodo, rutaDividida[carpetaActual]);
                int point1=this->crearBloqueApuntador(posCarpetaO);
                int point2=this->crearBloqueApuntador(point1);
                int point3=this->crearBloqueApuntador(point2);
                tablaInodo.i_block[i]=point3;
                fseek(this->file, direccionInodo, SEEK_SET);
                fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);
                return posInodo;
            }else{
                cout << "NO SE PUEDE CREAR LA CARPETA " << rutaDividida[carpetaActual] << endl;
                return -1;
            }
        }
        else if ((i == 14) && (tablaInodo.i_block[i] != -1)) {
            fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
            fread(&puntero_1, sizeof(BloqueApuntador), 1, this->file);
            for (int p1 = 0; p1 < 16; ++p1) {
                if (puntero_1.b_pointers[p1] == -1){
                    if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>3) {
                        int posInodo= this->getPosicionInodoNuevo();
                        int posCarpetaI=this->crearBloqueCarpetaInicial(posInodo, direccionInodo);
                        this->crearInodoCarpeta(posInodo,posCarpetaI);
                        int posCarpetaO=this->crearBloqueCarpetaOtra(posInodo, rutaDividida[carpetaActual]);
                        int point1=this->crearBloqueApuntador(posCarpetaO);
                        int point2=this->crearBloqueApuntador(point1);
                        puntero_1.b_pointers[p1]=point2;
                        fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
                        fwrite(&puntero_1, sizeof(TablaInodo), 1, this->file);
                        return posInodo;
                    }else{
                        cout << "NO SE PUEDE CREAR LA CARPETA " << rutaDividida[carpetaActual] << endl;
                        return -1;
                    }
                }else if (puntero_1.b_pointers[p1] != -1){
                    fseek(this->file, puntero_1.b_pointers[p1], SEEK_SET);
                    fread(&puntero_2, sizeof(BloqueApuntador), 1, this->file);
                    for (int p2 = 0; p2 < 16; ++p2) {
                        if (puntero_2.b_pointers[p2] == -1){
                            if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>2) {
                                int posInodo= this->getPosicionInodoNuevo();
                                int posCarpetaI=this->crearBloqueCarpetaInicial(posInodo, direccionInodo);
                                this->crearInodoCarpeta(posInodo,posCarpetaI);
                                int posCarpetaO=this->crearBloqueCarpetaOtra(posInodo, rutaDividida[carpetaActual]);
                                int point1=this->crearBloqueApuntador(posCarpetaO);
                                puntero_2.b_pointers[p2]=point1;
                                fseek(this->file, puntero_1.b_pointers[p1], SEEK_SET);
                                fwrite(&puntero_2, sizeof(TablaInodo), 1, this->file);
                                return posInodo;
                            }else{
                                cout << "NO SE PUEDE CREAR LA CARPETA " << rutaDividida[carpetaActual] << endl;
                                return -1;
                            }
                        }else if (puntero_2.b_pointers[p2] != -1){
                            fseek(this->file, puntero_2.b_pointers[p2], SEEK_SET);
                            fread(&puntero_3, sizeof(BloqueApuntador), 1, this->file);
                            for (int p3 = 0; p3 < 16; ++p3) {
                                if (puntero_3.b_pointers[p3] == -1){
                                    if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>1) {
                                        int posInodo= this->getPosicionInodoNuevo();
                                        int posCarpetaI=this->crearBloqueCarpetaInicial(posInodo, direccionInodo);
                                        this->crearInodoCarpeta(posInodo,posCarpetaI);
                                        int posCarpetaO=this->crearBloqueCarpetaOtra(posInodo, rutaDividida[carpetaActual]);
                                        puntero_3.b_pointers[p3]=posCarpetaO;
                                        fseek(this->file, puntero_2.b_pointers[p2], SEEK_SET);
                                        fwrite(&puntero_3, sizeof(TablaInodo), 1, this->file);
                                        return posInodo;
                                    }else{
                                        cout << "NO SE PUEDE CREAR LA CARPETA " << rutaDividida[carpetaActual] << endl;
                                        return -1;
                                    }
                                }else if (puntero_3.b_pointers[p3] != -1){
                                    fseek(this->file, puntero_3.b_pointers[p3], SEEK_SET);
                                    fread(&carpeta, sizeof(BloqueCarpeta), 1, this->file);
                                    for (int c = 0; c < 4; ++c) {
                                        if (carpeta.b_content[c].b_inodo == -1) {
                                            if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>0) {
                                                int posInodo= this->getPosicionInodoNuevo();
                                                int posCarpetaI=this->crearBloqueCarpetaInicial(posInodo, direccionInodo);
                                                this->crearInodoCarpeta(posInodo,posCarpetaI);
                                                carpeta.b_content[c].b_inodo=posInodo;
                                                strcpy(carpeta.b_content[c].b_name, rutaDividida[carpetaActual].c_str());
                                                fseek(this->file, puntero_3.b_pointers[p3], SEEK_SET);
                                                fwrite(&carpeta, sizeof(BloqueCarpeta), 1, this->file);
                                                return posInodo;
                                            }else{
                                                cout << "NO SE PUEDE CREAR LA CARPETA " << rutaDividida[carpetaActual] << endl;
                                                return -1;
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
    cout << "IMPOSIBLE CREAR CARPETA " << rutaDividida[carpetaActual] << "" << endl;
    return -1;
}

bool AdminArchivosCarpetas::verificarPermisoInodo_Escritura(int direccionInodo) {
    TablaInodo tablaInodo;
    fseek(this->file, direccionInodo, SEEK_SET);
    fread(&tablaInodo, sizeof(TablaInodo), 1, this->file);
    string permisos= to_string(tablaInodo.i_perm);

    if (permisos.length() == 2){
        permisos= "0" + permisos;
    }else if (permisos.length() == 1){
        permisos= "00" + permisos;
    }

    // si es admin
    if (this->usuario->idU==1 && this->usuario->idG==1){
        return true;
    }
    // permisos de grupos
    else if (tablaInodo.i_gid == usuario->idG){
        if ((permisos[1] == '2') || (permisos[1] == '3') || (permisos[1] == '6') || (permisos[1] == '7')){
            return true;
        }
    }
    // permisos de propietario
    else if ((tablaInodo.i_uid == usuario->idU) && (tablaInodo.i_gid == usuario->idG)){
        if ((permisos[0] == '2') || (permisos[0] == '3') || (permisos[0] == '6') || (permisos[0] == '7')){
            return true;
        }
    }
    // permisos de otros
    else{
        if ((permisos[2] == '2') || (permisos[2] == '3') || (permisos[2] == '6') || (permisos[2] == '7')){
            return true;
        }
    }

    return false;
}
/**
 * Este método también actualiza el Bit Map de Inodos.
 * @return
 */
int AdminArchivosCarpetas::getPosicionInodoNuevo() {

    int direccionInodos_Start = this->sb.s_bm_inode_start;
    int direccionInodos_End = direccionInodos_Start + this->sb.s_inodes_count;
    char uno = '1';
    int contador_PosicionBitMapInodos = 0;
    char bit;
    // Primero Leer el bitmap de inodos ( PUEDE MEJORARSE EL MÉTODO SOLO LEYENDO EL PRIMER BIT LIBRE)
    for (int j = direccionInodos_Start; j < direccionInodos_End; j++) {
        fseek(this->file, j, SEEK_SET);
        fread(&bit, sizeof(char), 1, this->file);
        if (bit == '0') { // primer espacio disponible
            fseek(this->file, j, SEEK_SET);
            fwrite(&uno, sizeof(char), 1, this->file);
            break;
        }
        contador_PosicionBitMapInodos++;
    }
    this->sb.s_free_inodes_count-=1; // actualizar el cambio en los inodos
    int direccionInodo= this->sb.s_inode_start + (sizeof(TablaInodo) * contador_PosicionBitMapInodos);

    // Actualizar el último Inodo Disponible en el sistema
    this->actualizarUltimoInodoDisponible();

    return direccionInodo;
}
int AdminArchivosCarpetas::actualizarUltimoInodoDisponible() {
    int direccionInodos_Start = this->sb.s_bm_inode_start;
    int contador_BitMapInodo = 0;
    int direccionInodos_End = direccionInodos_Start + this->sb.s_inodes_count;
    char bit;
    for (int j = direccionInodos_Start; j < direccionInodos_End; j++) {
        fseek(this->file, j, SEEK_SET);
        fread(&bit, sizeof(char), 1, this->file);
        if (bit == '0') { // Primer Inodo Libre
            contador_BitMapInodo++;
            break;
        }
        contador_BitMapInodo++;
    }
    this->sb.s_firts_ino = contador_BitMapInodo;
}

int AdminArchivosCarpetas::crearBloqueCarpetaInicial(int posActual, int posPadre) {
    int posBloque=this->buscarPosicionNewBLoque();

    BloqueCarpeta newCarpeta;
    strcpy(newCarpeta.b_content[0].b_name,".");
    newCarpeta.b_content[0].b_inodo=posActual;
    strcpy(newCarpeta.b_content[1].b_name, "..");
    newCarpeta.b_content[1].b_inodo = posPadre;
    strcpy(newCarpeta.b_content[2].b_name, "");
    newCarpeta.b_content[2].b_inodo = -1;
    strcpy(newCarpeta.b_content[3].b_name, "");
    newCarpeta.b_content[3].b_inodo = -1;

    fseek(this->file,posBloque,SEEK_SET);
    fwrite(&newCarpeta, sizeof(BloqueCarpeta),1,this->file);

    return posBloque;
}


