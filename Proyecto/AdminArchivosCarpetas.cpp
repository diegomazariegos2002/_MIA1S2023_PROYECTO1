#include <iostream>
#include <cstring>
#include <sstream>
#include <fstream>
#include "AdminArchivosCarpetas.h"


AdminArchivosCarpetas::AdminArchivosCarpetas() {
    this->path = " ";
    this->mountList = new MountList();
    this->usuario = new Usuario();
    this->rutasCat = *new vector<string>();
    this->r = false;
    this->cont = " ";
    this->size = 0;
    this->flagGlobal = false;
    this->name=" ";
}

void AdminArchivosCarpetas::cat() {
    Nodo_M *nodoMontura=this->mountList->buscar(usuario->idMount);
    if (this->rutasCat.size()==0){
        cout<<"ES OBLIGATORIO INGRESAR AL MENOS UNA RUTA"<<endl;
        return;
    }
    if (nodoMontura == NULL){
        cout <<"MONTURA CON EL ID: "<< this->usuario->idMount<< " IMPOSIBLE DE ENCONTRAR"<<endl;
        return;
    }
    // Leer status de la respectiva partición.
    if ((this->file= fopen(nodoMontura->path.c_str(), "rb+"))){
        if (nodoMontura->type == 'l'){
            EBR ebr;
            fseek(file, nodoMontura->start, SEEK_SET);
            fread(&ebr, sizeof(EBR),1,file);
            if (ebr.part_status != '2') {
                fclose(file);
                cout << "LA PARTICION " << nodoMontura->name << " NO HA SIDO FORMATEADA AL PARECER" << endl;
                return;
            }
            fseek(file, nodoMontura->start + sizeof(EBR), SEEK_SET);
        }
        else if (nodoMontura->type == 'p'){
            MBR mbr;
            fseek(this->file,0,SEEK_SET);
            fread(&mbr, sizeof(MBR),1,file);
            if (mbr.mbr_partition[nodoMontura->pos].part_status != '2'){
                cout << "LA PARTICION " << nodoMontura->name << " NO HA SIDO FORMATEADA AL PARECER" << endl;
                fclose(this->file);
                return;
            }
            fseek(this->file, nodoMontura->start, SEEK_SET);
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
                this->registrarJournal("cat", '1', this->path, "", nodoMontura);
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
                    if (strcmp(blck_Carpeta.b_content[j].b_name, (rutaDividida[direccionActual]).c_str()) == 0){
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
    // VALIDACIONES GENERALES
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
                        direccionInodoActual= this->insertCarpeta(rutaDividida, i, direccionInodoActual);
                        if (nodo->type=='l'){
                            fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
                        }
                        else if (nodo->type=='p'){
                            fseek(this->file,nodo->start,SEEK_SET);
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
        this->insertCarpeta(rutaDividida, rutaDividida.size() - 1, direccionInodoActual);
        if (nodo->type=='l'){
            fseek(this->file,nodo->start+ sizeof(EBR),SEEK_SET);
        }
        else if (nodo->type=='p'){
            fseek(this->file,nodo->start,SEEK_SET);
        }
        fwrite(&this->sb, sizeof(SuperBloque),1,this->file);

        if (this->sb.s_filesystem_type==3){
            this->registrarJournal("mkdir",'1',this->path,"",nodo);
        }
        fclose(this->file);
        cout<<"COMANDO EJECUTADO CON EXITO, CARPETA "<<this->path<< " CREADA" <<endl;
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
 *          - Crear Inodo para la carpeta ( modificar bitmap de inodos, actualizar ejecutarReporte_sb.s_firts_ino )
 *          - Crear bloque carpeta en el Inodo Anteriro ( modificar bitmap de bloques, actualizar ejecutarReporte_sb.s_firsts_blck)
 * @param rutaDividida
 * @param carpetaActual
 * @param direccionInodoPadre
 * @return
 */
int AdminArchivosCarpetas::insertCarpeta(vector<std::string> rutaDividida, int carpetaActual, int direccionInodoPadre) {
    if (!this->verificarPermisoInodo_Escritura(direccionInodoPadre)){
        cout << "IMPOSIBLE DE CREAR CARPETA POR FALTA DE PERMISOS EN LA CARPETA: " << rutaDividida[carpetaActual] << endl;
        return -1;
    }

    BloqueCarpeta carpeta, carpetaNueva;
    BloqueApuntador puntero_1, puntero_2, puntero_3, punteroNuevo;
    TablaInodo tablaInodo, InodoCarpetaNueva;

    int direccionCarpetaNueva=0;

    fseek(this->file, direccionInodoPadre, SEEK_SET);
    fread(&tablaInodo, sizeof(TablaInodo), 1, this->file);

    if (tablaInodo.i_type == '1'){
        cout<<"IMPOSIBLE CREAR UNA CARPETA EN UN ARCHIVO"<<endl;
        return -1;
    }
    // Recorrido de los punteros en el inodo
    for (int i = 0; i < 15; ++i) {
        // Recordar que el -1 hace referencia a que si el puntero esta sin utilizar.

        // Punteros directos
        // OCUPADO
        if (tablaInodo.i_block[i] != -1 && i < 12){ // si esta ocupado el puntero por un bloque carpeta
            fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
            fread(&carpeta, sizeof(BloqueCarpeta), 1, this->file);
            for (int j = 0; j < 4; ++j) { // Recorrer el bloque carpeta para ver si tiene espacio para guardar algo más.
                if (carpeta.b_content[j].b_inodo == -1) { // si, tiene espacio
                    if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>0) { // validar bloques e inodos libres
                        // Buscar donde se va a posicionar
                        int direccionInodoNuevo= this->getDireccionInodoNuevo();
                        // Posicionar el bloque de carpeta inicial para el inodo
                        int direccionBlckCarpeta= this->insertBlckCarpetaInicial(direccionInodoNuevo,
                                                                                 direccionInodoPadre);
                        // Posicionar el Inodo enlazado a su respectiva carpeta inicial
                        this->insertInodoCarpeta(direccionInodoNuevo, direccionBlckCarpeta);
                        // Enlazar la carpeta padre (ya existente) al inodo nuevo ( hijo ).
                        carpeta.b_content[j].b_inodo=direccionInodoNuevo;
                        // Pasando el nombre de la carpeta al padre.
                        strcpy(carpeta.b_content[j].b_name, rutaDividida[carpetaActual].c_str());
                        // Escribir el bloque de carpeta actualizado
                        fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
                        fwrite(&carpeta, sizeof(BloqueCarpeta), 1, this->file);
                        return direccionInodoNuevo;
                    }else{
                        cout << "IMPOSIBLE CREAR LA CARPETA " << rutaDividida[carpetaActual] << " DEBIDO A QUE YA NO HAY ESPACIO EN EL SISTEMA" << endl;
                        return -1;
                    }
                }
            }
        }
        // LIBRE
        else if (tablaInodo.i_block[i] == -1 && i < 12){ // puntero sin apuntar a BLOQUE CARPETA (CREAR BLOQUE CARPETA EXTRA)
            if (this->sb.s_free_blocks_count > 1 && this->sb.s_free_inodes_count>0) {
                // desde
                int direccionInodoNuevo= this->getDireccionInodoNuevo();
                int direccionBlckCarpetaInicial= this->insertBlckCarpetaInicial(direccionInodoNuevo,
                                                                                direccionInodoPadre);
                this->insertInodoCarpeta(direccionInodoNuevo, direccionBlckCarpetaInicial);
                // hasta es lo mismo de arriba.
                // la diferencia radica en, crear un nuevo carpeta en el inodo padre para contener al inodo hijo.
                // INODO PADRE ( nuevo blck carp ) -> INODO HIJO
                int direccionBlckCarpetaExtra = this->insertBlckCarpetaExtra(direccionInodoNuevo, rutaDividida[carpetaActual]);
                tablaInodo.i_block[i] = direccionBlckCarpetaExtra;
                // Escribir el inodo actualizado
                fseek(this->file, direccionInodoPadre, SEEK_SET);
                fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);
                return direccionInodoNuevo;
            }else{
                cout << "IMPOSIBLE CREAR LA CARPETA " << rutaDividida[carpetaActual] << " DEBIDO A QUE YA NO HAY ESPACIO EN EL SISTEMA" << endl;
                return -1;
            }
        }

        // Puntero simples
        // OCUPADO
        else if (i == 12 && tablaInodo.i_block[i] != -1) {
            // Ya existe un bloque apuntador, o sea solo se lee
            fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
            fread(&puntero_1, sizeof(BloqueApuntador), 1, this->file);
            for (int j = 0; j < 16; j++) { // RECORRER PUNTEROS DIRECTOS
                // OCUPADO
                if ((puntero_1.b_pointers[j] != -1)){
                    fseek(this->file, puntero_1.b_pointers[j], SEEK_SET);
                    fread(&carpeta, sizeof(BloqueCarpeta), 1, this->file);
                    for (int k = 0; k < 4; ++k) {
                        if (carpeta.b_content[k].b_inodo == -1) {
                            if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>0) {
                                int direccionInodoNuevo= this->getDireccionInodoNuevo();
                                int direccionBlckCarpetaInicial= this->insertBlckCarpetaInicial(direccionInodoNuevo,
                                                                                                direccionInodoPadre);
                                this->insertInodoCarpeta(direccionInodoNuevo, direccionBlckCarpetaInicial);
                                carpeta.b_content[k].b_inodo=direccionInodoNuevo;
                                strcpy(carpeta.b_content[k].b_name, rutaDividida[carpetaActual].c_str());
                                fseek(this->file, puntero_1.b_pointers[j], SEEK_SET);
                                fwrite(&carpeta, sizeof(BloqueCarpeta), 1, this->file);
                                return direccionInodoNuevo;
                            }else{
                                cout << "IMPOSIBLE CREAR LA CARPETA " << rutaDividida[carpetaActual] << " DEBIDO A QUE YA NO HAY ESPACIO EN EL SISTEMA" << endl;
                                return -1;
                            }
                        }
                    }
                }
                // LIBRE
                else if (puntero_1.b_pointers[j] == -1) {
                    if (this->sb.s_free_blocks_count > 1 && this->sb.s_free_inodes_count>0) {
                        int direccionInodoNuevo= this->getDireccionInodoNuevo();
                        int direccionBlckCarpetaInicial= this->insertBlckCarpetaInicial(direccionInodoNuevo,
                                                                                        direccionInodoPadre);
                        this->insertInodoCarpeta(direccionInodoNuevo, direccionBlckCarpetaInicial);
                        int direccionBlckCarpetaExtra = this->insertBlckCarpetaExtra(direccionInodoNuevo,
                                                                                     rutaDividida[carpetaActual]);
                        // Se actualiza y escribe el BLOQUE APUNTADOR de punteros directos
                        puntero_1.b_pointers[j]=direccionBlckCarpetaExtra;
                        fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
                        fwrite(&puntero_1, sizeof(BloqueApuntador), 1, this->file);
                        return direccionInodoNuevo;
                    }else{
                        cout << "IMPOSIBLE CREAR LA CARPETA " << rutaDividida[carpetaActual] << " DEBIDO A QUE YA NO HAY ESPACIO EN EL SISTEMA" << endl;
                        return -1;
                    }
                }
            }
        }
        // LIBRE
        else if (i == 12 && tablaInodo.i_block[i] == -1) {
            // No existe un bloque apuntador, o sea, hay que crearlo y asignarlo.
            if (this->sb.s_free_blocks_count > 2 && this->sb.s_free_inodes_count>0) {
                // desde
                int direccionInodoNuevo= this->getDireccionInodoNuevo();
                int direccionBlckCarpetaInicial= this->insertBlckCarpetaInicial(direccionInodoNuevo, direccionInodoPadre);
                this->insertInodoCarpeta(direccionInodoNuevo, direccionBlckCarpetaInicial);
                int direccionBlckCarpetaExtra= this->insertBlckCarpetaExtra(direccionInodoNuevo, rutaDividida[carpetaActual]);
                // hasta -> igual lo que cambia es la asignacion de bloque apuntador

                int posBlckApuntador1= this->insertBlckApuntador(direccionBlckCarpetaExtra);
                tablaInodo.i_block[i]=posBlckApuntador1;
                fseek(this->file, direccionInodoPadre, SEEK_SET);
                fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);
                return direccionInodoNuevo;
            }else{
                cout << "IMPOSIBLE CREAR LA CARPETA " << rutaDividida[carpetaActual] << " DEBIDO A QUE YA NO HAY ESPACIO EN EL SISTEMA" << endl;
                return -1;
            }
        }

        // Puntero dobles
        // OCUPADO
        else if (i == 13 && tablaInodo.i_block[i] != -1) {
            fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
            fread(&puntero_1, sizeof(BloqueApuntador), 1, this->file);
            for (int j = 0; j < 16; ++j) { // RECORRER PUNTEROS SIMPLES
                // OCUPADO
                if(puntero_1.b_pointers[j] != -1){
                    fseek(this->file, puntero_1.b_pointers[j], SEEK_SET);
                    fread(&puntero_2, sizeof(BloqueApuntador), 1, this->file);
                    for (int k = 0; k < 16; ++k) { // RECORRER PUNTEROS DIRECTOS
                        // OCUPADO
                        if (puntero_2.b_pointers[k] != -1){
                            fseek(this->file, puntero_2.b_pointers[k], SEEK_SET);
                            fread(&carpeta, sizeof(BloqueCarpeta), 1, this->file);
                            for (int z = 0; z < 4; ++z) {
                                if (carpeta.b_content[z].b_inodo == -1) {
                                    if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>0) {
                                        int direccionInodoNuevo= this->getDireccionInodoNuevo();
                                        int direccionBlckCarpetaInicial= this->insertBlckCarpetaInicial(direccionInodoNuevo,
                                                                                                        direccionInodoPadre);
                                        this->insertInodoCarpeta(direccionInodoNuevo, direccionBlckCarpetaInicial);
                                        carpeta.b_content[z].b_inodo=direccionInodoNuevo;
                                        strcpy(carpeta.b_content[z].b_name, rutaDividida[carpetaActual].c_str());
                                        fseek(this->file, puntero_2.b_pointers[k], SEEK_SET);
                                        fwrite(&carpeta, sizeof(BloqueCarpeta), 1, this->file);
                                        return direccionInodoNuevo;
                                    }else{
                                        cout << "IMPOSIBLE CREAR LA CARPETA " << rutaDividida[carpetaActual] << " DEBIDO A QUE YA NO HAY ESPACIO EN EL SISTEMA" << endl;
                                        return -1;
                                    }
                                }
                            }
                        }
                        // LIBRE
                        else if (puntero_2.b_pointers[k] == -1){
                            if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>1) {
                                int direccionInodoNuevo= this->getDireccionInodoNuevo();
                                int direccionBlckCarpetaInicial= this->insertBlckCarpetaInicial(direccionInodoNuevo,
                                                                                                direccionInodoPadre);
                                this->insertInodoCarpeta(direccionInodoNuevo, direccionBlckCarpetaInicial);
                                int direccionBlckCarpetaExtra= this->insertBlckCarpetaExtra(direccionInodoNuevo,
                                                                                            rutaDividida[carpetaActual]);
                                puntero_2.b_pointers[k]=direccionBlckCarpetaExtra;
                                fseek(this->file, puntero_1.b_pointers[j], SEEK_SET);
                                fwrite(&puntero_2, sizeof(TablaInodo), 1, this->file);
                                return direccionInodoNuevo;
                            }else{
                                cout << "IMPOSIBLE CREAR LA CARPETA " << rutaDividida[carpetaActual] << " DEBIDO A QUE YA NO HAY ESPACIO EN EL SISTEMA" << endl;
                                return -1;
                            }
                        }
                    }
                }
                // LIBRE
                else if (puntero_1.b_pointers[j] == -1){
                    if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>2) {
                        int direccionInodoNuevo= this->getDireccionInodoNuevo();
                        int direccionBlckCarpetaInicial= this->insertBlckCarpetaInicial(direccionInodoNuevo,
                                                                                        direccionInodoPadre);
                        this->insertInodoCarpeta(direccionInodoNuevo, direccionBlckCarpetaInicial);
                        int direccionBlckCarpetaExtra= this->insertBlckCarpetaExtra(direccionInodoNuevo,
                                                                                    rutaDividida[carpetaActual]);
                        int direccionPunteroSimpleNuevo= this->insertBlckApuntador(direccionBlckCarpetaExtra);
                        puntero_1.b_pointers[j]=direccionPunteroSimpleNuevo;
                        fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
                        fwrite(&puntero_1, sizeof(TablaInodo), 1, this->file);
                        return direccionInodoNuevo;
                    }else{
                        cout << "IMPOSIBLE CREAR LA CARPETA " << rutaDividida[carpetaActual] << " DEBIDO A QUE YA NO HAY ESPACIO EN EL SISTEMA" << endl;
                        return -1;
                    }
                }
            }
        }
        // LIBRE
        else if (i == 13 && tablaInodo.i_block[i] == -1) {
            if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>3) {
                int direccionInodoNuevo= this->getDireccionInodoNuevo();
                int direccionBlckCarpetaInicial= this->insertBlckCarpetaInicial(direccionInodoNuevo, direccionInodoPadre);
                this->insertInodoCarpeta(direccionInodoNuevo, direccionBlckCarpetaInicial);
                int direccionBlckCarpetaExtra= this->insertBlckCarpetaExtra(direccionInodoNuevo, rutaDividida[carpetaActual]);
                int direccionBlckApuntador_1= this->insertBlckApuntador(direccionBlckCarpetaExtra);
                int direccionBlckApuntador_2= this->insertBlckApuntador(direccionBlckApuntador_1);
                tablaInodo.i_block[i]=direccionBlckApuntador_2;
                fseek(this->file, direccionInodoPadre, SEEK_SET);
                fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);
                return direccionInodoNuevo;
            }else{
                cout << "IMPOSIBLE CREAR LA CARPETA " << rutaDividida[carpetaActual] << " DEBIDO A QUE YA NO HAY ESPACIO EN EL SISTEMA" << endl;
                return -1;
            }
        }

        // Puntero triples
        // OCUPADO
        else if ((i == 14) && (tablaInodo.i_block[i] != -1)) {
            fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
            fread(&puntero_1, sizeof(BloqueApuntador), 1, this->file);
            for (int j = 0; j < 16; ++j) { // RECORRER PUNTEROS DOBLES
                // OCUPADO
                if (puntero_1.b_pointers[j] != -1){
                    fseek(this->file, puntero_1.b_pointers[j], SEEK_SET);
                    fread(&puntero_2, sizeof(BloqueApuntador), 1, this->file);
                    for (int k = 0; k < 16; ++k) { // RECORRER PUNTEROS SIMPLES
                        // OCUPADO
                        if (puntero_2.b_pointers[k] != -1){
                            fseek(this->file, puntero_2.b_pointers[k], SEEK_SET);
                            fread(&puntero_3, sizeof(BloqueApuntador), 1, this->file);
                            for (int z = 0; z < 16; ++z) { // RECORRER PUNTEROS DIRECTOS
                                // OCUPADO
                                if (puntero_3.b_pointers[z] != -1){
                                    fseek(this->file, puntero_3.b_pointers[z], SEEK_SET);
                                    fread(&carpeta, sizeof(BloqueCarpeta), 1, this->file);
                                    for (int x = 0; x < 4; ++x) { // RECORRER BLOQUE CARPETA
                                        if (carpeta.b_content[x].b_inodo == -1) {
                                            if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>0) {
                                                int direccionInodoNuevo= this->getDireccionInodoNuevo();
                                                int direccionBlckCarpetaInicial= this->insertBlckCarpetaInicial(
                                                        direccionInodoNuevo, direccionInodoPadre);
                                                this->insertInodoCarpeta(direccionInodoNuevo, direccionBlckCarpetaInicial);
                                                carpeta.b_content[x].b_inodo=direccionInodoNuevo;
                                                strcpy(carpeta.b_content[x].b_name, rutaDividida[carpetaActual].c_str());
                                                fseek(this->file, puntero_3.b_pointers[z], SEEK_SET);
                                                fwrite(&carpeta, sizeof(BloqueCarpeta), 1, this->file);
                                                return direccionInodoNuevo;
                                            }else{
                                                cout << "IMPOSIBLE CREAR LA CARPETA " << rutaDividida[carpetaActual] << " DEBIDO A QUE YA NO HAY ESPACIO EN EL SISTEMA" << endl;
                                                return -1;
                                            }
                                        }
                                    }
                                }
                                // LIBRE
                                else if (puntero_3.b_pointers[z] == -1){
                                    if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>1) {
                                        int direccionInodoNuevo= this->getDireccionInodoNuevo();
                                        int direccionBlckCarpetaInicial= this->insertBlckCarpetaInicial(direccionInodoNuevo,
                                                                                                        direccionInodoPadre);
                                        this->insertInodoCarpeta(direccionInodoNuevo, direccionBlckCarpetaInicial);
                                        int direccionBlckCarpetaExtra= this->insertBlckCarpetaExtra(direccionInodoNuevo,
                                                                                                    rutaDividida[carpetaActual]);
                                        puntero_3.b_pointers[z]=direccionBlckCarpetaExtra;
                                        fseek(this->file, puntero_2.b_pointers[k], SEEK_SET);
                                        fwrite(&puntero_3, sizeof(TablaInodo), 1, this->file);
                                        return direccionInodoNuevo;
                                    }else{
                                        cout << "IMPOSIBLE CREAR LA CARPETA " << rutaDividida[carpetaActual] << " DEBIDO A QUE YA NO HAY ESPACIO EN EL SISTEMA" << endl;
                                        return -1;
                                    }
                                }
                            }
                        }
                        // LIBRE
                        else if (puntero_2.b_pointers[k] == -1){
                            if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>2) {
                                int direccionInodoNuevo= this->getDireccionInodoNuevo();
                                int direccionBlckCarpetaInicial= this->insertBlckCarpetaInicial(direccionInodoNuevo,
                                                                                                direccionInodoPadre);
                                this->insertInodoCarpeta(direccionInodoNuevo, direccionBlckCarpetaInicial);
                                int direccionBlckCarpetaExtra= this->insertBlckCarpetaExtra(direccionInodoNuevo,
                                                                                            rutaDividida[carpetaActual]);
                                int direccionBlckApuntador_1= this->insertBlckApuntador(direccionBlckCarpetaExtra);
                                puntero_2.b_pointers[k]=direccionBlckApuntador_1;
                                fseek(this->file, puntero_1.b_pointers[j], SEEK_SET);
                                fwrite(&puntero_2, sizeof(TablaInodo), 1, this->file);
                                return direccionInodoNuevo;
                            }else{
                                cout << "IMPOSIBLE CREAR LA CARPETA " << rutaDividida[carpetaActual] << " DEBIDO A QUE YA NO HAY ESPACIO EN EL SISTEMA" << endl;
                                return -1;
                            }
                        }
                    }
                }
                // LIBRE
                else if (puntero_1.b_pointers[j] == -1){
                    if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>3) {
                        int direccionInodoNuevo= this->getDireccionInodoNuevo();
                        int direccionBlckCarpetaInicial= this->insertBlckCarpetaInicial(direccionInodoNuevo,
                                                                                        direccionInodoPadre);
                        this->insertInodoCarpeta(direccionInodoNuevo, direccionBlckCarpetaInicial);
                        int direccionBlckCarpetaExtra= this->insertBlckCarpetaExtra(direccionInodoNuevo,
                                                                                    rutaDividida[carpetaActual]);
                        int direccionBlckApuntador_1= this->insertBlckApuntador(direccionBlckCarpetaExtra);
                        int direccionBlckApuntador_2= this->insertBlckApuntador(direccionBlckApuntador_1);
                        puntero_1.b_pointers[j]=direccionBlckApuntador_2;
                        fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
                        fwrite(&puntero_1, sizeof(TablaInodo), 1, this->file);
                        return direccionInodoNuevo;
                    }else{
                        cout << "IMPOSIBLE CREAR LA CARPETA " << rutaDividida[carpetaActual] << " DEBIDO A QUE YA NO HAY ESPACIO EN EL SISTEMA" << endl;
                        return -1;
                    }
                }
            }
        }
        // LIBRE
        else if ((i == 14) && (tablaInodo.i_block[i] == -1)) {
            if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>4) {
                int direccionInodoNuevo= this->getDireccionInodoNuevo();
                int direccionBlckCarpetaInicial= this->insertBlckCarpetaInicial(direccionInodoNuevo, direccionInodoPadre);
                this->insertInodoCarpeta(direccionInodoNuevo, direccionBlckCarpetaInicial);
                int direccionBlckCarpetaExtra= this->insertBlckCarpetaExtra(direccionInodoNuevo, rutaDividida[carpetaActual]);
                int direccionBlckApuntador_1 = this->insertBlckApuntador(direccionBlckCarpetaExtra);
                int direccionBlckApuntador_2 = this->insertBlckApuntador(direccionBlckApuntador_1);
                int direccionBlckApuntador_3 = this->insertBlckApuntador(direccionBlckApuntador_2);
                tablaInodo.i_block[i]=direccionBlckApuntador_3;
                fseek(this->file, direccionInodoPadre, SEEK_SET);
                fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);
                return direccionInodoNuevo;
            }else{
                cout << "IMPOSIBLE CREAR LA CARPETA " << rutaDividida[carpetaActual] << " DEBIDO A QUE YA NO HAY ESPACIO EN EL SISTEMA" << endl;
                return -1;
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
int AdminArchivosCarpetas::getDireccionInodoNuevo() {

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
/**
 * Creacion del bloque carpeta ( tengo dudas con el . y el .. porque según yo hacen referencia al inodo padre y actual )
 * @param direccionActual
 * @param direccionPadre
 * @return DireccionBlckCarpetaInicial
 */
int AdminArchivosCarpetas::insertBlckCarpetaInicial(int direccionActual, int direccionPadre) {
    BloqueCarpeta blckCarpeta_Nuevo;

    int direccionBloqueNuevo= this->getDireccionBloqueNuevo();

    strcpy(blckCarpeta_Nuevo.b_content[0].b_name, ".");
    blckCarpeta_Nuevo.b_content[0].b_inodo=direccionActual;
    strcpy(blckCarpeta_Nuevo.b_content[1].b_name, "..");
    blckCarpeta_Nuevo.b_content[1].b_inodo = direccionPadre;
    strcpy(blckCarpeta_Nuevo.b_content[2].b_name, "");
    blckCarpeta_Nuevo.b_content[2].b_inodo = -1;
    strcpy(blckCarpeta_Nuevo.b_content[3].b_name, "");
    blckCarpeta_Nuevo.b_content[3].b_inodo = -1;

    fseek(this->file, direccionBloqueNuevo, SEEK_SET);
    fwrite(&blckCarpeta_Nuevo, sizeof(BloqueCarpeta), 1, this->file);

    return direccionBloqueNuevo;
}

int AdminArchivosCarpetas::getDireccionBloqueNuevo() {

    char bit;
    int direccionInodos_Start = this->sb.s_bm_block_start;
    int direccionInodos_End = direccionInodos_Start + this->sb.s_blocks_count;
    int contador_BitMap = 0;
    char uno = '1';
    int direccionBloqueLibre;
    for (int i = direccionInodos_Start; i < direccionInodos_End; i++) {
        fseek(this->file, i, SEEK_SET);
        fread(&bit, sizeof(char), 1, this->file);
        if (bit == '0') {
            fseek(this->file, i, SEEK_SET);
            fwrite(&uno, sizeof(char), 1, this->file);
            break;
        }
        contador_BitMap++;
    }
    this->sb.s_free_blocks_count-=1;

    direccionBloqueLibre = this->sb.s_block_start + (contador_BitMap * sizeof(BloqueApuntador));

    this->actualizarUltimoBloqueDisponible();

    return direccionBloqueLibre;
}

int AdminArchivosCarpetas::actualizarUltimoBloqueDisponible() {



    int direccionBloques_Start = this->sb.s_bm_block_start;
    char bit;
    int direccionBloques_End = direccionBloques_Start + this->sb.s_blocks_count;
    int contador_posicionBitMap = 0;

    contador_posicionBitMap=0;
    for (int j = direccionBloques_Start; j < direccionBloques_End; j++) {
        fseek(this->file, j, SEEK_SET);
        fread(&bit, sizeof(char), 1, this->file);
        if (bit == '0') {
            contador_posicionBitMap++;
            break;
        }
        contador_posicionBitMap++;
    }
    this->sb.s_first_blo = contador_posicionBitMap;
}

void AdminArchivosCarpetas::insertInodoCarpeta(int direccionInodo, int direccionBloqueCarpetaInicial) {
    TablaInodo tablaInodo;
    tablaInodo.i_uid = this->usuario->idU;
    tablaInodo.i_gid = this->usuario->idG;
    tablaInodo.i_s = 0;
    tablaInodo.i_atime = time(nullptr);
    tablaInodo.i_ctime = time(nullptr);
    tablaInodo.i_mtime = time(nullptr);
    tablaInodo.i_type='0';
    tablaInodo.i_perm=664;
    tablaInodo.i_block[0]=direccionBloqueCarpetaInicial;
    for (int j = 1; j < 15; ++j) {
        tablaInodo.i_block[j]=-1;
    }

    fseek(this->file, direccionInodo, SEEK_SET);
    fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);
}

/**
 * Crear bloque de carpeta, pero obviamente no el bloque de carpeta inicial de los inodos.
 * Pues el bloque carpeta inicial si necesita apuntar a "." y ".."
 * @param direccionInodoHijo
 * @param nombreCarpeta
 * @return DireccionBlckCarpetaExtra
 */
int AdminArchivosCarpetas::insertBlckCarpetaExtra(int direccionInodoHijo, string nombreCarpeta) {
    BloqueCarpeta blckCarpetaNuevo;
    int direccionBlckCarpeta=this->getDireccionBloqueNuevo();


    strcpy(blckCarpetaNuevo.b_content[0].b_name, nombreCarpeta.c_str());
    blckCarpetaNuevo.b_content[0].b_inodo=direccionInodoHijo;
    for (int i = 1; i < 4; ++i) {
        strcpy(blckCarpetaNuevo.b_content[i].b_name, "");
        blckCarpetaNuevo.b_content[i].b_inodo =-1;
    }

    fseek(this->file, direccionBlckCarpeta, SEEK_SET);
    fwrite(&blckCarpetaNuevo, sizeof(BloqueCarpeta), 1, this->file);

    return direccionBlckCarpeta;
}
/**
 * Crear un bloque apuntador nuevo
 * @param direccionInodoHijo
 * @return dirección del bloque apuntador nuevo
 */
int AdminArchivosCarpetas::insertBlckApuntador(int direccionInodoHijo) {
    BloqueApuntador blckApuntadorNuevo;
    blckApuntadorNuevo.b_pointers[0]=direccionInodoHijo;
    int direccionBlckNuevo=this->getDireccionBloqueNuevo();


    for (int i =1; i < 16; ++i) {
        blckApuntadorNuevo.b_pointers[i]=-1;
    }
    fseek(this->file, direccionBlckNuevo, SEEK_SET);
    fwrite(&blckApuntadorNuevo, sizeof(BloqueApuntador), 1, this->file);
    return direccionBlckNuevo;
}

void AdminArchivosCarpetas::mkfile() {
    if (this->path==" "){
        cout<<"EL PARAMETRO PATH ES OBLIGATORIO"<<endl;
        return;
    }
    Nodo_M *nodo=this->mountList->buscar(usuario->idMount);

    if (nodo==NULL){
        cout <<"NO HAY UNA MONTURA CON EL ID: "<< this->usuario->idMount<<endl;
        return;
    }

    if (this->cont==" "){
        if (this->size<0){
            cout<<"EL PARAMETRO SIZE SOLO PUEDE SER MAYOR QUE 0"<<endl;
        }
    }else{
        this->size=1;
    }

    if ((this->file= fopen(nodo->path.c_str(),"rb+"))){

        if (nodo->type=='l'){
            EBR ebr;
            fseek(file,nodo->start,SEEK_SET);
            fread(&ebr, sizeof(EBR),1,file);
            if (ebr.part_status != '2') {
                fclose(file);
                cout<<"PARTICION NO FORMATEADA, IMPOSIBLE EJECUTAR COMANDO"<<endl;
                return;
            }
            fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
        }
        else if (nodo->type=='p'){
            MBR mbr;
            fseek(this->file,0,SEEK_SET);
            fread(&mbr, sizeof(MBR),1,file);
            if (mbr.mbr_partition[nodo->pos].part_status!='2'){
                cout<<"PARTICION NO FORMATEADA, IMPOSIBLE EJECUTAR COMANDO"<<endl;
                return;
            }
            fseek(this->file,nodo->start,SEEK_SET);
        }

        fread(&this->sb, sizeof(SuperBloque),1,file);

        vector<string> rutaDividida= this->getRutaDividida(this->path);
        if (rutaDividida.empty()){
            cout<<"LA RUTA INGRESADA NO ES VALIDA"<<endl;
            fclose(this->file);
            return;
        }

        int direccionInodo = this->getDireccionInodo(rutaDividida, 0, rutaDividida.size() - 1, this->sb.s_inode_start, this->file);
        // Esto creo que se puede mejorar pues en teoría tiene que modificarse el archivo
        if (direccionInodo != -1){
            cout << "EL ARCHIVO INDICADO YA EXISTE EN LA RUTA: " << this->path << endl;
            fclose(this->file);
            return;
        }
        // Validar que la ruta del archivo si exista
        bool existeCarpeta=true;
        TablaInodo tablaInodo;
        int direccionCarpeta=this->sb.s_inode_start;
        // Flujo:
        /**
         * + Validar para cada ruta.
         *      - ExisteCarpeta?
         *      - Si (siguiente carpeta), No [ entonces validar r?
         *                                  -> r=true ( Crear Carpeta ), r=false ( Error ) ]
         */

        if (rutaDividida.size() > 1){
            for (int i = 0; i < rutaDividida.size() - 1; ++i) {
                if (existeCarpeta){
                    int copia=direccionCarpeta;
                    direccionCarpeta=this->existeCarpeta(rutaDividida, i, direccionCarpeta);
                    if (direccionCarpeta == copia){
                        existeCarpeta= false;
                    }
                }
                if(!existeCarpeta){
                    if (this->r){
                        direccionCarpeta=this->insertCarpeta(rutaDividida, i, direccionCarpeta);
                        if (nodo->type=='l'){
                            fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
                        }
                        else if (nodo->type=='p'){
                            fseek(this->file,nodo->start,SEEK_SET);
                        }
                        fwrite(&this->sb, sizeof(SuperBloque),1,file);
                        if (direccionCarpeta == -1){
                            cout<<"ERROR IMPOSIBLE CREAR EL ARCHIVO"<<endl;
                            return;
                        }
                    }else{
                        cout<<"ERROR IMPOSIBLE CREAR EL ARCHIVO PORQUE LA RUTA INDICADA NO EXISTE"<<endl;
                        return;
                    }
                }
            }
        }
        if (direccionCarpeta == -1){
            cout<<"ERROR IMPOSIBLE AL CREAR LA CARPETA"<<endl;
            return;
        }
        // Ya validado lo de la carpeta seguimos con el archivo.
        // Validar permisos
        if (!this->verificarPermisoInodo_Escritura(direccionCarpeta)){
            cout << "IMPOSIBLE EJECUTAR EL COMANDO, POR FALTA DE PERMISOS EN EL ARCHIVO: " << rutaDividida[rutaDividida.size() - 1] << endl;
            return;
        }

        // Validar el parametro cont
        string texto="";
        if (this->cont!=" "){
            FILE *fileComputadora;
            if ((fileComputadora = fopen(this->cont.c_str(), "r"))) {
                texto= this->leerArchivoComputadora();
                fclose(fileComputadora);
            } else {
                cout << "EL ARCHIVO "<<this->cont <<" INDICADO EN EL PARAMETRO CONT NO EXISTE." <<endl;
                return;
            }
        }else{
            texto= this->getCadenaSize();
        }
        vector<string> blksArchivoString=this->getArrayBlks(texto);
        if (this->sb.s_free_blocks_count < blksArchivoString.size()) {
            fclose(this->file);
            cout << "NO EXISTE EL ESPACIO SUFICIENTE EN EL SISTEMA PARA EJECUTAR EL COMANDO" << endl;
            return;
        }
        if (blksArchivoString.size() >= 4380){
            cout << "IMPOSIBLE EJECUTAR EL COMANDO YA QUE LA CADENA SOLICITADA EXCEDE EL LIMITE DEL SISTEMA" << endl;
            return;
        }
        //Busqueda en la dirección del bitmap bloques correspondiente
        fseek(this->file, direccionCarpeta, SEEK_SET);
        fread(&tablaInodo, sizeof(TablaInodo), 1, this->file);

        //Posicionar nuevo bloque en el BitMap de Bloques

        int blksLibresSeguidos = 0;
        int bitMapBlks_Start = this->sb.s_bm_block_start;
        int bitMapBlks_End = bitMapBlks_Start + this->sb.s_block_start;
        int inicioBM = -1;
        int startB = -1;
        int contAux = 0;
        char bit;

        for (int i = bitMapBlks_Start; i < bitMapBlks_End; ++i) { // Recorrer el BitMap
            fseek(this->file, i, SEEK_SET);
            fread(&bit, sizeof(char), 1, this->file); // leer la posición
            //ocupado
            if (bit == '1') {
                blksLibresSeguidos = 0;
                inicioBM = -1;
                startB = -1;
            }
            //libre
            else {
                if (blksLibresSeguidos == 0) {
                    inicioBM = i;
                    startB = contAux;
                }
                blksLibresSeguidos++;
            }

            if (blksLibresSeguidos >= blksArchivoString.size()) break;
            contAux++;
        }
        if (this->size != 0 && (inicioBM==-1 || (blksLibresSeguidos != blksArchivoString.size()))){
            fclose(this->file);
            cout << "NO HAY SUFICIENTES BLOQUES PARA EJECUTAR EL COMANDO" << endl;
            return;
        }
        // Escribir los Bits a ocupar en el Bitmap de bloques
        for (int i = inicioBM; i < (inicioBM + blksArchivoString.size()); ++i) {
            char uno='1';
            fseek(this->file,i,SEEK_SET);
            fwrite(&uno, sizeof(char),1,this->file);

        }
        // Actualizar el contador de bits libre
        this->sb.s_free_blocks_count-=blksArchivoString.size();

        // Actualizar el dato de la última posición disponible.
        int bit_2 = 0;

        for (int k = bitMapBlks_Start; k < bitMapBlks_End; k++) {
            fseek(this->file, k, SEEK_SET);
            fread(&bit, sizeof(char), 1, this->file);
            if (bit == '0') break;
            bit_2++;
        }
        this->sb.s_first_blo = bit_2;
        if (nodo->type=='l'){
            fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
        }
        else if (nodo->type=='p'){
            fseek(this->file,nodo->start,SEEK_SET);
        }
        // Se escribe la actualización
        fwrite(&this->sb, sizeof(SuperBloque),1,file);

        TablaInodo tablaInodoArchivo;
        int direccionInodoArchivoNuevo=this->getDireccionInodoNuevo();
        this->crearInodoArchivo(direccionInodoArchivoNuevo);
        fseek(this->file, direccionInodoArchivoNuevo, SEEK_SET);
        fread(&tablaInodoArchivo, sizeof(TablaInodo), 1, this->file);
        int respuestaEnlace= this->enlazarArchivoCarpeta(direccionInodoArchivoNuevo, direccionCarpeta,
                                                         rutaDividida[rutaDividida.size() - 1]);
        // Enlazar bloques de archivo al Inodo Archivo
        int size = 0;
        if (respuestaEnlace == -1){ // ERROR
            return;
        }

        for (int i = 0; i < blksArchivoString.size(); i++) {
            size += blksArchivoString[i].length();
        }
        tablaInodoArchivo.i_s = size;
        tablaInodoArchivo.i_atime = time(nullptr);
        tablaInodoArchivo.i_mtime = time(nullptr);

        int j=0,contador=0;
        while (j < blksArchivoString.size()){
            flagGlobal=false;
            tablaInodoArchivo= this->addFile(j, (startB + contador), blksArchivoString[j], tablaInodoArchivo);
            if (flagGlobal){
                contador++;
            }
            j++;
        }
        fseek(this->file, direccionInodoArchivoNuevo, SEEK_SET);
        fwrite(&tablaInodoArchivo, sizeof(TablaInodo), 1, this->file);

        if (nodo->type=='l'){
            fseek(this->file,nodo->start+ sizeof(EBR),SEEK_SET);
            fwrite(&this->sb, sizeof(SuperBloque),1,this->file);
        }
        else if (nodo->type=='p'){
            fseek(this->file,nodo->start,SEEK_SET);
            fwrite(&this->sb, sizeof(SuperBloque),1,this->file);
        }


        if (this->sb.s_filesystem_type==3){
            this->registrarJournal("mkfile",'1',this->path,"",nodo);
        }

        cout<<"COMANDO EJECUTA CON EXITO, EL ARCHIVO "<<this->path<< " FUE CREADO." <<endl;
        fclose(this->file);
    }
    else{
        cout <<"EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA"<<endl;
    }

}
// Revisar bien este metodo
string AdminArchivosCarpetas::getCadenaSize() {
    int cont1=0,cont2=0,i=0;
    string cadena="";
    while (i < this->size){
        cadena+= to_string(cont1);
        cont1++;
        cont2++;
        if (cont1==10){
            cont1=0;
        }
        i++;
        if (cont2==19){
            cadena+="\n";
            cont2=0;
            i++;
        }
    }
    return cadena;
}

string AdminArchivosCarpetas::leerArchivoComputadora() {
    string linea;
    string cadenaArchivo="";
    ifstream archivo(this->cont.c_str());
    while (getline(archivo, linea)) {
        cadenaArchivo+=linea;
    }
    return cadenaArchivo;
}
// Array de bloques según una cadena.
vector<string> AdminArchivosCarpetas::getArrayBlks(string cadena) {
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

void AdminArchivosCarpetas::crearInodoArchivo(int direccionInodo) {
    TablaInodo tablaInodoNuevo;
    tablaInodoNuevo.i_uid = this->usuario->idU;
    tablaInodoNuevo.i_gid = this->usuario->idG;
    tablaInodoNuevo.i_s = 0;
    tablaInodoNuevo.i_atime = time(nullptr);
    tablaInodoNuevo.i_ctime = time(nullptr);
    tablaInodoNuevo.i_mtime = time(nullptr);
    tablaInodoNuevo.i_type='1';
    tablaInodoNuevo.i_perm=664;
    for (int j = 0; j < 15; ++j) {
        tablaInodoNuevo.i_block[j]=-1;
    }

    fseek(this->file, direccionInodo, SEEK_SET);
    fwrite(&tablaInodoNuevo, sizeof(TablaInodo), 1, this->file);
}

int AdminArchivosCarpetas::enlazarArchivoCarpeta(int direccionArchivo, int direccionCarpeta, string nombreArchivo) {
    if (!this->verificarPermisoInodo_Escritura(direccionCarpeta)){
        cout << "ERROR IMPOSIBLE EJECUTAR EL COMANDO, POR FALTA DE PERMISOS NO SE PUEDE CREAR EL ARCHIVO: " << nombreArchivo << endl;
        return -1;
    }

    BloqueApuntador blckApuntador_1, blckApuntador_2, blckApuntador_3;
    TablaInodo tablaInodo;
    BloqueCarpeta blckCarpeta; // enlazador del inodo carpeta y el inodo archivo

    fseek(this->file, direccionCarpeta, SEEK_SET);
    fread(&tablaInodo, sizeof(TablaInodo), 1, this->file);

    if (tablaInodo.i_type == '1'){
        cout<<"ERROR EN EL PROCESO"<<endl;
        return -1;
    }

    for (int i = 0; i < 15; ++i) { // Recorrer los punteros del inodo Carpeta
        // PUNTEROS DIRECTOS
        // OCUPADOS
        if (tablaInodo.i_block[i] != -1 && i < 12){
            fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
            fread(&blckCarpeta, sizeof(BloqueCarpeta), 1, this->file);
            for (int j = 0; j < 4; ++j) { // Asignando valores del bloque carpeta
                if (blckCarpeta.b_content[j].b_inodo == -1) { // Si, se encuentra un espacio disponible.
                    strcpy(blckCarpeta.b_content[j].b_name, nombreArchivo.c_str());
                    blckCarpeta.b_content[j].b_inodo=direccionArchivo;
                    // Guardando datos.
                    fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
                    fwrite(&blckCarpeta, sizeof(BloqueCarpeta), 1, this->file);
                    return 0;
                }
            }
        }
        // LIBRES
        else if (tablaInodo.i_block[i] == -1 && i < 12){
            if (this->sb.s_free_blocks_count>0) { // Verificar si se pueden crear más bloques
                int direccionBlckCarpetaExtra= this->insertBlckCarpetaExtra(direccionArchivo, nombreArchivo);
                tablaInodo.i_block[i] = direccionBlckCarpetaExtra;
                fseek(this->file, direccionCarpeta, SEEK_SET);
                fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);
                return 0;
            }else{
                cout << "ERROR AL EJECUTAR EL COMANDO, EL ARCHIVO " << nombreArchivo << " NO SE PUDO CREAR" << endl;
                return -1;
            }
        }
        // PUNTERO SIMPLE
        // OCUPADO
        else if (i == 12 && tablaInodo.i_block[i] != -1) {
            fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
            fread(&blckApuntador_1, sizeof(BloqueApuntador), 1, this->file);
            for (int j = 0; j < 16; j++) {
                if (blckApuntador_1.b_pointers[j] == -1) {
                    if (this->sb.s_free_blocks_count>0) {
                        int direccionBlckCarpetaExtra = this->insertBlckCarpetaExtra(direccionArchivo, nombreArchivo);
                        blckApuntador_1.b_pointers[j] = direccionBlckCarpetaExtra;
                        fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
                        fwrite(&blckApuntador_1, sizeof(BloqueApuntador), 1, this->file);
                        return 0;
                    }else{
                        cout << "ERROR AL EJECUTAR EL COMANDO, EL ARCHIVO " << nombreArchivo << " NO SE PUDO CREAR" << endl;
                        return -1;
                    }
                } else if ((blckApuntador_1.b_pointers[j] != -1)){
                    fseek(this->file, blckApuntador_1.b_pointers[j], SEEK_SET);
                    fread(&blckCarpeta, sizeof(BloqueCarpeta), 1, this->file);
                    for (int k = 0; k < 4; ++k) {
                        if (blckCarpeta.b_content[k].b_inodo == -1) {
                            strcpy(blckCarpeta.b_content[k].b_name, nombreArchivo.c_str());
                            blckCarpeta.b_content[k].b_inodo=direccionArchivo;
                            fseek(this->file, blckApuntador_1.b_pointers[j], SEEK_SET);
                            fwrite(&blckCarpeta, sizeof(BloqueCarpeta), 1, this->file);
                            return 0;
                        }
                    }
                }
            }
        }
        // LIBRE
        else if (i == 12 && tablaInodo.i_block[i] == -1) {
            if (this->sb.s_free_blocks_count>1) {
                int direccionBlckCarpetaExtra = this->insertBlckCarpetaExtra(direccionArchivo, nombreArchivo);
                int direccionBlckApuntadorNuevo = this->insertBlckApuntador(direccionBlckCarpetaExtra);
                tablaInodo.i_block[i] = direccionBlckApuntadorNuevo;
                fseek(this->file, direccionCarpeta, SEEK_SET);
                fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);
                return 0;
            }else{
                cout << "ERROR AL EJECUTAR EL COMANDO, EL ARCHIVO " << nombreArchivo << " NO SE PUDO CREAR" << endl;
                return -1;
            }
        }
        // PUNTERO DOBLE
        // OCUPADO
        else if (i == 13 && tablaInodo.i_block[i] != -1) {
            fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
            fread(&blckApuntador_1, sizeof(BloqueApuntador), 1, this->file);
            for (int j = 0; j < 16; ++j) {
                if (blckApuntador_1.b_pointers[j] == -1){
                    if (this->sb.s_free_blocks_count>1) {
                        int direccionBlckCarpetaExtra = this->insertBlckCarpetaExtra(direccionArchivo, nombreArchivo);
                        int direccionBlckApuntador_1 = this->insertBlckApuntador(direccionBlckCarpetaExtra);
                        blckApuntador_1.b_pointers[j] = direccionBlckApuntador_1;
                        fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
                        fwrite(&blckApuntador_1, sizeof(TablaInodo), 1, this->file);
                        return 0;
                    }else{
                        cout << "ERROR AL EJECUTAR EL COMANDO, EL ARCHIVO " << nombreArchivo << " NO SE PUDO CREAR" << endl;
                        return -1;
                    }
                }else if(blckApuntador_1.b_pointers[j] != -1){
                    fseek(this->file, blckApuntador_1.b_pointers[j], SEEK_SET);
                    fread(&blckApuntador_2, sizeof(BloqueApuntador), 1, this->file);
                    for (int k = 0; k < 16; ++k) {
                        if (blckApuntador_2.b_pointers[k] == -1){
                            if (this->sb.s_free_blocks_count>0) {
                                int direccionBlckCarpetaExtra = this->insertBlckCarpetaExtra(direccionArchivo, nombreArchivo);
                                blckApuntador_2.b_pointers[k] = direccionBlckCarpetaExtra;
                                fseek(this->file, blckApuntador_1.b_pointers[j], SEEK_SET);
                                fwrite(&blckApuntador_2, sizeof(TablaInodo), 1, this->file);
                                return 0;
                            }else{
                                cout << "ERROR AL EJECUTAR EL COMANDO, EL ARCHIVO " << nombreArchivo << " NO SE PUDO CREAR" << endl;
                                return -1;
                            }
                        }else if (blckApuntador_2.b_pointers[k] != -1){
                            fseek(this->file, blckApuntador_2.b_pointers[k], SEEK_SET);
                            fread(&blckCarpeta, sizeof(BloqueCarpeta), 1, this->file);
                            for (int z = 0; z < 4; ++z) {
                                if (blckCarpeta.b_content[z].b_inodo == -1) {
                                    strcpy(blckCarpeta.b_content[z].b_name, nombreArchivo.c_str());
                                    blckCarpeta.b_content[z].b_inodo=direccionArchivo;
                                    fseek(this->file, blckApuntador_2.b_pointers[k], SEEK_SET);
                                    fwrite(&blckCarpeta, sizeof(BloqueCarpeta), 1, this->file);
                                    return 0;
                                }
                            }
                        }
                    }
                }
            }
        }
        // LIBRE
        else if (i == 13 && tablaInodo.i_block[i] == -1) {
            if (this->sb.s_free_blocks_count>2) {
                int direccionBlckCarpetaExtra = this->insertBlckCarpetaExtra(direccionArchivo, nombreArchivo);
                int direccionBlckApuntador_1 = this->insertBlckApuntador(direccionBlckCarpetaExtra);
                int direccionBlckApuntador_2 = this->insertBlckApuntador(direccionBlckApuntador_1);
                tablaInodo.i_block[i] = direccionBlckApuntador_2;
                fseek(this->file, direccionCarpeta, SEEK_SET);
                fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);
                return 0;
            }else{
                cout << "ERROR AL EJECUTAR EL COMANDO, EL ARCHIVO " << nombreArchivo << " NO SE PUDO CREAR" << endl;
                return -1;
            }
        }
        // PUNTERO TRIPLE
        // OCUPADO
        else if ((i == 14) && (tablaInodo.i_block[i] != -1)) {
            fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
            fread(&blckApuntador_1, sizeof(BloqueApuntador), 1, this->file);
            for (int j = 0; j < 16; ++j) {
                if (blckApuntador_1.b_pointers[j] == -1){
                    if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>2) {
                        int direccionBlckCarpetaExtra=this->insertBlckCarpetaExtra(direccionArchivo, nombreArchivo);
                        int direccionBlckApuntador_1=this->insertBlckApuntador(direccionBlckCarpetaExtra);
                        int direccionBlckApuntador_2=this->insertBlckApuntador(direccionBlckApuntador_1);
                        blckApuntador_1.b_pointers[j]=direccionBlckApuntador_2;
                        fseek(this->file, tablaInodo.i_block[i], SEEK_SET);
                        fwrite(&blckApuntador_1, sizeof(TablaInodo), 1, this->file);
                        return 0;
                    }else{
                        cout << "ERROR AL EJECUTAR EL COMANDO, EL ARCHIVO " << nombreArchivo << " NO SE PUDO CREAR" << endl;
                        return -1;
                    }
                }else if (blckApuntador_1.b_pointers[j] != -1){
                    fseek(this->file, blckApuntador_1.b_pointers[j], SEEK_SET);
                    fread(&blckApuntador_2, sizeof(BloqueApuntador), 1, this->file);
                    for (int k = 0; k < 16; ++k) {
                        if (blckApuntador_2.b_pointers[k] == -1){
                            if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>1) {
                                int direccionBlckCarpetaExtra=this->insertBlckCarpetaExtra(direccionArchivo, nombreArchivo);
                                int direccionBlckApuntador_1=this->insertBlckApuntador(direccionBlckCarpetaExtra);
                                blckApuntador_2.b_pointers[k]=direccionBlckApuntador_1;
                                fseek(this->file, blckApuntador_1.b_pointers[j], SEEK_SET);
                                fwrite(&blckApuntador_2, sizeof(TablaInodo), 1, this->file);
                                return 0;
                            }else{
                                cout << "ERROR AL EJECUTAR EL COMANDO, EL ARCHIVO " << nombreArchivo << " NO SE PUDO CREAR" << endl;
                                return -1;
                            }
                        }else if (blckApuntador_2.b_pointers[k] != -1){
                            fseek(this->file, blckApuntador_2.b_pointers[k], SEEK_SET);
                            fread(&blckApuntador_3, sizeof(BloqueApuntador), 1, this->file);
                            for (int z = 0; z < 16; ++z) {
                                if (blckApuntador_3.b_pointers[z] == -1){
                                    if (this->sb.s_free_inodes_count > 0 && this->sb.s_free_blocks_count>1) {
                                        int direccionBlckCarpetaExtra=this->insertBlckCarpetaExtra(direccionArchivo, nombreArchivo);
                                        blckApuntador_3.b_pointers[z]=direccionBlckCarpetaExtra;
                                        fseek(this->file, blckApuntador_2.b_pointers[k], SEEK_SET);
                                        fwrite(&blckApuntador_3, sizeof(TablaInodo), 1, this->file);
                                        return 0;
                                    }else{
                                        cout << "ERROR AL EJECUTAR EL COMANDO, EL ARCHIVO " << nombreArchivo << " NO SE PUDO CREAR" << endl;
                                        return -1;
                                    }
                                }else if (blckApuntador_3.b_pointers[z] != -1){
                                    fseek(this->file, blckApuntador_3.b_pointers[z], SEEK_SET);
                                    fread(&blckCarpeta, sizeof(BloqueCarpeta), 1, this->file);
                                    for (int x = 0; x < 4; ++x) {
                                        if (blckCarpeta.b_content[x].b_inodo == -1) {
                                            strcpy(blckCarpeta.b_content[x].b_name, nombreArchivo.c_str());
                                            blckCarpeta.b_content[x].b_inodo=direccionArchivo;
                                            fseek(this->file, blckApuntador_3.b_pointers[z], SEEK_SET);
                                            fwrite(&blckCarpeta, sizeof(BloqueCarpeta), 1, this->file);
                                            return 0;

                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        // LIBRE
        else if ((i == 14) && (tablaInodo.i_block[i] == -1)) {
            if (this->sb.s_free_blocks_count>4) {
                int direccionBlckCarpetaExtra=this->insertBlckCarpetaExtra(direccionArchivo, nombreArchivo);
                int direccionBlckApuntador_1=this->insertBlckApuntador(direccionBlckCarpetaExtra);
                int direccionBlckApuntador_2=this->insertBlckApuntador(direccionBlckApuntador_1);
                int direccionBlckApuntador_3=this->insertBlckApuntador(direccionBlckApuntador_2);
                tablaInodo.i_block[i]=direccionBlckApuntador_3;
                fseek(this->file, direccionCarpeta, SEEK_SET);
                fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);
                return 0;
            }else{
                cout << "ERROR AL EJECUTAR EL COMANDO, EL ARCHIVO " << nombreArchivo << " NO SE PUDO CREAR" << endl;
                return -1;
            }
        }
    }
    cout << "EL ARCHIVO " << nombreArchivo << " NO SE PUDO CREAR"<< endl;
    return -1;
}

// podría mejorar esto (creo)
TablaInodo AdminArchivosCarpetas::addFile(int blckActual, int noBlckBitMap, std::string cadena, TablaInodo inodo) {
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
                        //actualizacion ejecutarReporte_sb
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
                                //actualizacion ejecutarReporte_sb
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
                        //actualizacion ejecutarReporte_sb
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
                        //actualizacion ejecutarReporte_sb
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

void AdminArchivosCarpetas::rename() {
    if (this->name==" "){
        cout<<"PARAMETRO NAME ES OBLIGATORIO"<<endl;
        return;
    }
    if (this->path==" "){
        cout<<"PARAMETRO PATH ES OBLIGATORIO"<<endl;
        return;
    }
    Nodo_M *nodo=this->mountList->buscar(usuario->idMount);
    if (nodo==NULL){
        cout <<"NO EXISTE MONTURA CON EL ID: "<< this->usuario->idMount<<" EN EL SISTEMA"<<endl;
        return;
    }

    if ((this->file= fopen(nodo->path.c_str(),"rb+"))){
        // validar ruta
        vector<string> rutaDividida= this->getRutaDividida(this->path);
        if (rutaDividida.empty()){
            fclose(this->file);
            cout<<"LA RUTA INGRESADA NO ES VALIDA"<<endl;
            return;
        }

        // Leer el superbloque, como siempre
        if (nodo->type=='l'){
            EBR ebr;
            fseek(file,nodo->start,SEEK_SET);
            fread(&ebr, sizeof(EBR),1,file);
            if (ebr.part_status != '2') {
                fclose(file);
                cout<<"PARTICION NO FORMATEADA, IMPOSIBLE EJECUTAR COMANDO"<<endl;
                return;
            }
            fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
        }
        else if (nodo->type=='p'){
            MBR mbr;
            fseek(this->file,0,SEEK_SET);
            fread(&mbr, sizeof(MBR),1,file);
            if (mbr.mbr_partition[nodo->pos].part_status!='2'){
                cout<<"PARTICION NO FORMATEADA, IMPOSIBLE EJECUTAR COMANDO"<<endl;
                return;
            }
            fseek(this->file,nodo->start,SEEK_SET);
        }

        fread(&this->sb, sizeof(SuperBloque),1,file);

        // Ubicar el Inodo
        TablaInodo tablaInodo;
        bool existeCarpeta=true;
        int direccionInodoArchivo=this->sb.s_inode_start;
        int direccionInodoCarpeta=direccionInodoArchivo;

        // Recordar si no existe la carpeta tira error, de lo contrario sigue
        for (int i = 0; i < rutaDividida.size(); ++i) {
            if (existeCarpeta){
                int aux=direccionInodoArchivo;
                direccionInodoArchivo=this->existeCarpeta(rutaDividida, i, direccionInodoArchivo);
                if (i!=(rutaDividida.size() - 1)){
                    direccionInodoCarpeta=direccionInodoArchivo;
                }
                if (direccionInodoArchivo == aux){
                    existeCarpeta= false;
                }
            }
            if(!existeCarpeta){
                cout<<"NO EXISTE "<< this->path<< " ESTA RUTA INDICADA" << endl;
                return;
            }
        }

        rutaDividida.push_back(this->name);
        if (direccionInodoCarpeta != this->existeCarpeta(rutaDividida, rutaDividida.size() - 1, direccionInodoCarpeta)){
            cout<<"IMPOSIBLE EJECUTAR, YA EXISTE UN ARCHIVO O CARPETA CON EL NOMBRE "<< this->name<<endl;
            return;
        }

        fseek(this->file, direccionInodoCarpeta, SEEK_SET);
        fread(&tablaInodo, sizeof(TablaInodo), 1, this->file);

        if (!this->verificarPermisoInodo_Escritura(direccionInodoArchivo)){
            cout << "IMPOSIBLE EJECUTAR, PERMISOS INSUFICIENTES EN EL ARCHIVO O CARPETA " << rutaDividida[rutaDividida.size() - 2] << endl;
            return;
        }

        this->cambiarNombre(tablaInodo, rutaDividida[rutaDividida.size() - 2]);
        tablaInodo.i_mtime= time(nullptr);
        fseek(this->file, direccionInodoCarpeta, SEEK_SET);
        fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);

        cout <<"COMANDO EJECUTADO CON EXITO, SE MODIFICO EL NOMBRE"<<endl;
        if (this->sb.s_filesystem_type==3){
            this->registrarJournal("rename",'1',this->path,this->name,nodo);
        }
        fclose(this->file);
    }else{
        cout <<"ERROR EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA"<<endl;
    }
}

void AdminArchivosCarpetas::cambiarNombre(TablaInodo tablaInodoCarpeta, std::string nombreOriginal) {
    BloqueApuntador blkApuntador_1, blkApuntador_2, blkApuntador_3;
    BloqueCarpeta blkCarpeta;

    for (int i = 0; i < 15; ++i) { // Recorrer todos los punteros del tablaInodoCarpeta carpeta
        // PUNTEROS DIRECTOS
        // OCUPADO
        if (tablaInodoCarpeta.i_block[i] != -1){
            if (i<12){
                fseek(file, tablaInodoCarpeta.i_block[i], SEEK_SET);
                fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, file);
                for (int j = 0; j < 4; ++j) { // Recorrer carpeta
                    if (blkCarpeta.b_content[j].b_name == nombreOriginal){
                        strcpy(blkCarpeta.b_content[j].b_name, name.c_str()); // cambiamos el nombre
                        // escribimos la actualización
                        fseek(this->file, tablaInodoCarpeta.i_block[i], SEEK_SET);
                        fwrite(&blkCarpeta, sizeof(BloqueCarpeta), 1, this->file);
                        return;

                    }
                }

            }
            // PUNTEROS SIMPLES
            // OCUPADO
            else if (i==12){
                fseek(file, tablaInodoCarpeta.i_block[i], SEEK_SET);
                fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, file);
                for (int j = 0; j < 16; ++j) { // Recorrer punteros directos
                    if (blkApuntador_1.b_pointers[j] != -1){
                        fseek(file, blkApuntador_1.b_pointers[j], SEEK_SET);
                        fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, file);
                        for (int k = 0; k < 4; ++k) { // Recorrer carpeta
                            if (blkCarpeta.b_content[k].b_name == nombreOriginal){
                                strcpy(blkCarpeta.b_content[k].b_name, name.c_str());
                                fseek(this->file, blkApuntador_1.b_pointers[j], SEEK_SET);
                                fwrite(&blkCarpeta, sizeof(BloqueCarpeta), 1, this->file);
                                return;
                            }
                        }
                    }
                }
            }
            // PUNTEROS DOBLES
            // OCUPADO
            else if (i==13){
                fseek(file, tablaInodoCarpeta.i_block[i], SEEK_SET);
                fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, file);
                for (int j = 0; j < 16; ++j) { // Recorrer punteros simples
                    if (blkApuntador_1.b_pointers[j] != -1){
                        fseek(file, blkApuntador_1.b_pointers[j], SEEK_SET);
                        fread(&blkApuntador_2, sizeof(BloqueApuntador), 1, file);
                        for (int k = 0; k < 16; ++k) { // Recorrer punteros directos
                            if (blkApuntador_2.b_pointers[k] != -1){
                                fseek(file, blkApuntador_2.b_pointers[k], SEEK_SET);
                                fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, file);
                                for (int z = 0; z < 4; ++z) { // Recorrer Carpeta
                                    if (blkCarpeta.b_content[z].b_name == nombreOriginal){ // realizar acción
                                        strcpy(blkCarpeta.b_content[z].b_name, name.c_str());
                                        fseek(this->file, blkApuntador_2.b_pointers[k], SEEK_SET);
                                        fwrite(&blkCarpeta, sizeof(BloqueCarpeta), 1, this->file);
                                        return;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            // PUNTEROS TRIPLES
            // OCUPADO
            else if (i==14){
                fseek(file, tablaInodoCarpeta.i_block[i], SEEK_SET);
                fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, file);
                for (int j = 0; j < 16; ++j) { // Recorrer puntero doble
                    if (blkApuntador_1.b_pointers[j] != -1){
                        fseek(file, blkApuntador_1.b_pointers[j], SEEK_SET);
                        fread(&blkApuntador_2, sizeof(BloqueApuntador), 1, file);
                        for (int k = 0; k < 16; ++k) { // Recorrer puntero simple
                            if (blkApuntador_2.b_pointers[k] != -1){
                                fseek(file, blkApuntador_2.b_pointers[k], SEEK_SET);
                                fread(&blkApuntador_3, sizeof(BloqueApuntador), 1, file);
                                for (int z = 0; z < 16; ++z) { // Recorrer punteros directos
                                    if (blkApuntador_3.b_pointers[z] != -1) {
                                        fseek(file, blkApuntador_3.b_pointers[z], SEEK_SET);
                                        fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, file);
                                        for (int x = 0; x < 4; ++x) { // Recorrer carpeta
                                            if (blkCarpeta.b_content[x].b_name == nombreOriginal){ // Realizar acción
                                                strcpy(blkCarpeta.b_content[x].b_name, name.c_str());
                                                fseek(this->file, blkApuntador_3.b_pointers[z], SEEK_SET);
                                                fwrite(&blkCarpeta, sizeof(BloqueCarpeta), 1, this->file);
                                                return;
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

void AdminArchivosCarpetas::chmod() {

    if (this->path==" "){
        cout<<"DEBE INGRESAR LA RUTA DEL ARCHIVO O CARPETA A CAMBIAR"<<endl;
        return;
    }

    // validar si es usuario admin
    if (usuario->idU==1 && usuario->idG==1){
        string u= to_string(this->ugo);
        if (u.length()>3){
            cout<<"LIMITE DE DIGITOS EXCEDIDO, COMO MAXIMO SE PUEDEN INGRESAR 3 DIGITOS"<<endl;
            return;
        }

        if (u.length()==2){
            u="0"+u;
        }
        else if (u.length()==1){
            u="00"+u;
        }

        Nodo_M *nodoMontura=this->mountList->buscar(usuario->idMount);
        if (nodoMontura == NULL){
            cout <<"NO EXISTE MONTURA CON EL ID: "<< this->usuario->idMount<<" EN EL SISTEMA"<<endl;
            return;
        }
        int direccionInodoFile;
        vector<string> rutaDividida;
        if ((this->file= fopen(nodoMontura->path.c_str(), "rb+"))){
            if (nodoMontura->type == 'l'){
                EBR ebr;

                fseek(file, nodoMontura->start, SEEK_SET);
                fread(&ebr, sizeof(EBR),1,file);
                if (ebr.part_status != '2') {
                    fclose(file);
                    cout<<"PARTICION NO FORMATEADA, IMPOSIBLE EJECUTAR COMANDO"<<endl;
                    return;
                }
                fseek(file, nodoMontura->start + sizeof(EBR), SEEK_SET);
            }
            else if (nodoMontura->type == 'p'){
                MBR mbr;

                fseek(this->file,0,SEEK_SET);
                fread(&mbr, sizeof(MBR),1,file);
                if (mbr.mbr_partition[nodoMontura->pos].part_status != '2'){
                    cout<<"PARTICION NO FORMATEADA, IMPOSIBLE EJECUTAR COMANDO"<<endl;
                    return;
                }
                fseek(this->file, nodoMontura->start, SEEK_SET);
            } 

            fread(&this->sb, sizeof(SuperBloque),1,file);


            if (this->path=="/"){
                direccionInodoFile=this->sb.s_inode_start;
            }
            else{
                vector<string> rutaDividida= this->getRutaDividida(this->path);
                if (rutaDividida.empty()){
                    fclose(this->file);
                    cout<<"LA RUTA INGRESADA NO ES VALIDA"<<endl;

                    return;
                }
                direccionInodoFile=this->getDireccionInodo(rutaDividida, 0, rutaDividida.size() - 1, this->sb.s_inode_start, file);
                if (direccionInodoFile == -1){
                    cout << "LA RUTA INDICADA NO EXISTE Y NO SE ENCONTRO NINGUN ARCHIVO" << endl;
                    return;
                }
            }

            this->cambiarPermisosRecursivo(direccionInodoFile, this->ugo, this->r);

            if (this->sb.s_filesystem_type==3){
                this->registrarJournal("chmod", '1', this->path, to_string(this->ugo), nodoMontura);
            }

            fclose(this->file);
            cout<<"COMANDO EJECUTADO CON EXITO, PERMISOS MODIFICADOS"<<endl;
        }
        else{
            cout <<"ERROR EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA"<<endl;
        }
    }
    else{
        cout<<"SOLO EL USUARIO ROOT PUEDE EJECUTAR ESTE COMANDO"<<endl;
    }
}
/**
 * Método recursivo para modificar los permisos en el correspondiente inodo.
 * Recordar que si la opción "r" esta activada y el inodo es de una carpeta
 * tiene que ir recursivamente aplicando el permiso a todos sus inodos hijos.
 * @param direccionInodo
 * @param ugo
 * @param r
 */
void AdminArchivosCarpetas::cambiarPermisosRecursivo(int direccionInodo, int ugo, bool r) {
    BloqueApuntador  blkApuntador_1,blkApuntador_2,blkApuntador_3;
    BloqueCarpeta blkCarpeta;
    TablaInodo tablaInodo;
    fseek(file, direccionInodo, SEEK_SET);
    fread(&tablaInodo, sizeof(TablaInodo), 1, file);
    if((tablaInodo.i_type != '0' && tablaInodo.i_type != '1' )){
        return;
    }
    tablaInodo.i_perm=this->ugo;
    tablaInodo.i_mtime= time(nullptr);
    fseek(file, direccionInodo, SEEK_SET);
    fwrite(&tablaInodo, sizeof(TablaInodo), 1, file);

    // si el inodo es de una carpeta y esta activada "r"
    if (tablaInodo.i_type == '0' && r){
        // RECORRER APUNTADORES
        for (int i = 0; i < 15; ++i) {
            // si esta ocupado el apuntador correspondiente
            if (tablaInodo.i_block[i] != -1){
                // APUNTADORES DIRECTOS
                if (i<12){
                    fseek(file, tablaInodo.i_block[i], SEEK_SET);
                    fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, file);
                    // BLOQUE CARPETA
                    string nombreFile;
                    for (int j = 0; j < 4; ++j) {
                        nombreFile=blkCarpeta.b_content[j].b_name;
                        if ((nombreFile != "." && nombreFile != "..") && blkCarpeta.b_content[j].b_inodo != -1) {
                            this->cambiarPermisosRecursivo(blkCarpeta.b_content[j].b_inodo, ugo, r);
                        }
                    }

                }
                // APUNTADORES SIMPLES
                else if (i==12){
                    fseek(file, tablaInodo.i_block[i], SEEK_SET);
                    fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, file);
                    // APUNTADORES DIRECTOS
                    for (int j = 0; j < 16; ++j) {
                        if (blkApuntador_1.b_pointers[j] != -1){
                            fseek(file, blkApuntador_1.b_pointers[j], SEEK_SET);
                            fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, file);
                            // BLOQUE CARPETA
                            string nombreFile;
                            for (int k = 0; k < 4; ++k) {
                                nombreFile=blkCarpeta.b_content[k].b_name;
                                // LOS NOMBRES DE "." Y ".." SE IGNORAN
                                if ((nombreFile != "." && nombreFile != "..") && blkCarpeta.b_content[k].b_inodo != -1) {
                                    this->cambiarPermisosRecursivo(blkCarpeta.b_content[k].b_inodo, ugo, r);
                                }
                            }
                        }
                    }
                }
                // APUNTADORES DOBLES
                else if (i==13){
                    fseek(file, tablaInodo.i_block[i], SEEK_SET);
                    fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, file);
                    // APUNTADORES SIMPLES
                    for (int j = 0; j < 16; ++j) {
                        if (blkApuntador_1.b_pointers[j] != -1){
                            fseek(file, blkApuntador_1.b_pointers[j], SEEK_SET);
                            fread(&blkApuntador_2, sizeof(BloqueApuntador), 1, file);
                            // APUNTADORES DIRECTOS
                            for (int k = 0; k < 16; ++k) {
                                if (blkApuntador_2.b_pointers[k] != -1){
                                    fseek(file, blkApuntador_2.b_pointers[k], SEEK_SET);
                                    fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, file);
                                    // BLOQUE CARPETA
                                    string nombreFile;
                                    for (int z = 0; z < 4; ++z) {
                                        nombreFile=blkCarpeta.b_content[z].b_name;
                                        // LOS NOMBRES DE "." Y ".." SE IGNORAN
                                        if ((nombreFile != "." && nombreFile != "..") && blkCarpeta.b_content[z].b_inodo != -1) {
                                            this->cambiarPermisosRecursivo(blkCarpeta.b_content[z].b_inodo, ugo, r);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                // APUNTADORES TRIPLES
                else if (i==14){
                    fseek(file, tablaInodo.i_block[i], SEEK_SET);
                    fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, file);
                    // APUNTADORES DOBLES
                    for (int j = 0; j < 16; ++j) {
                        if (blkApuntador_1.b_pointers[j] != -1){
                            fseek(file, blkApuntador_1.b_pointers[j], SEEK_SET);
                            fread(&blkApuntador_2, sizeof(BloqueApuntador), 1, file);
                            // APUNTADORES SIMPLES
                            for (int k = 0; k < 16; ++k) {
                                if (blkApuntador_2.b_pointers[k] != -1){
                                    fseek(file, blkApuntador_2.b_pointers[k], SEEK_SET);
                                    fread(&blkApuntador_3, sizeof(BloqueApuntador), 1, file);
                                    // APUNTADORES DIRECTOS
                                    for (int z = 0; z < 16; ++z) {
                                        if (blkApuntador_3.b_pointers[z] != -1) {
                                            fseek(file, blkApuntador_3.b_pointers[z], SEEK_SET);
                                            fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, file);
                                            // BLOQUE CARPETA
                                            string nombreFile;
                                            for (int y = 0; y < 4; ++y) {
                                                nombreFile=blkCarpeta.b_content[y].b_name;
                                                // LOS NOMBRES DE "." Y ".." SE IGNORAN
                                                if ((nombreFile != "." && nombreFile != "..") && blkCarpeta.b_content[y].b_inodo != -1) {
                                                    this->cambiarPermisosRecursivo(blkCarpeta.b_content[y].b_inodo, ugo, r);
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
}

void AdminArchivosCarpetas::chown() {
    Nodo_M *nodoMontura=this->mountList->buscar(usuario->idMount);
    if (this->path==" "){
        cout<<"EL PARAMETRO PATH ES OBLIGATORIO"<<endl;
        return;
    }
    if (nodoMontura == NULL){
        cout <<"NO EXISTE MONTURA CON EL ID: "<< this->usuario->idMount<<" EN EL SISTEMA"<<endl;
        return;
    }
    if ((this->file= fopen(nodoMontura->path.c_str(), "rb+"))){
        if (nodoMontura->type == 'l'){
            EBR ebr;
            fseek(file, nodoMontura->start, SEEK_SET);
            fread(&ebr, sizeof(EBR),1,file);
            if (ebr.part_status != '2') {
                fclose(file);
                cout<<"LA PARTICION "<<nodoMontura->name<<" NO HA SIDO FORMATEADA AL PARECER" << endl;
                return;
            }
            fseek(file, nodoMontura->start + sizeof(EBR), SEEK_SET);
        }
        else if (nodoMontura->type == 'p'){
            MBR mbr;
            fseek(this->file,0,SEEK_SET);
            fread(&mbr, sizeof(MBR),1,file);
            if (mbr.mbr_partition[nodoMontura->pos].part_status != '2'){
                cout<<"LA PARTICION "<<nodoMontura->name<<" NO HA SIDO FORMATEADA AL PARECER" << endl;
                return;
            }
            fseek(this->file, nodoMontura->start, SEEK_SET);
        }

        fread(&this->sb, sizeof(SuperBloque),1,file);

        string stringExtra;
        int direccionInodoFile;
        int idGrupo= this->getIdGrupoUsuario();
        int idUsuario= this->getIdUser();

        if (idGrupo == -1 || idUsuario == -1){
            cout<<"EL USUARIO INGRESADO NO EXISTE"<<endl;
            fclose(this->file);
            return;
        }
        if (this->path=="/"){
            direccionInodoFile=this->sb.s_inode_start;
        }
        else{
            vector<string> rutaDivivida= this->getRutaDividida(this->path);
            if (rutaDivivida.empty()){
                fclose(this->file);
                cout<<"LA RUTA INGRESADA NO ES VALIDA O NO EXISTE"<<endl;

                return;
            }
            direccionInodoFile=this->getDireccionInodo(rutaDivivida, 0, rutaDivivida.size() - 1, this->sb.s_inode_start, file);
            if (direccionInodoFile == -1){
                cout << "EL ARCHIVO INDICADO NO EXISTE" << endl;
                return;
            }
        }

        this->cambiarPropietarioRecursivo_pt1(direccionInodoFile, idUsuario, idGrupo, this->r);

        if (this->sb.s_filesystem_type==3){
            this->registrarJournal("chown", '1', this->path, this->name, nodoMontura);
        }

        cout<<"COMANDO CHOWN EJECUTADO CON EXITO, PROPIETARIO MODIFICADO"<<endl;
        fclose(this->file);
    }
    else{
        cout <<"EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA"<<endl;
    }
}

int AdminArchivosCarpetas::getIdUser() {
    string user;
    string stringAlmacenadoInodo=this->getStringAlmacenadoInodo(this->sb.s_inode_start + sizeof(TablaInodo));
    stringstream ss(stringAlmacenadoInodo);

    while (getline(ss, user, '\n')){
        if (user != ""){
            vector<string> camposUsuario=this->getCampos(user);
            if (camposUsuario[1] == "U" && camposUsuario[3] == this->name && camposUsuario[0] != "0"){
                return stoi(camposUsuario[0]);
            }
        }
    }
    return -1;
}

int AdminArchivosCarpetas::getIdGrupoUsuario() {
    string user;
    string stringAlmacenadoInodo=this->getStringAlmacenadoInodo(this->sb.s_inode_start + sizeof(TablaInodo));
    stringstream ss(stringAlmacenadoInodo);

    while (getline(ss, user, '\n')){
        if (user != ""){
            vector<string> camposUsuario=this->getCampos(user);
            if (camposUsuario[1] == "U" && camposUsuario[3] == this->name && camposUsuario[0] != "0"){
                return this->getIdGrupo(camposUsuario[2]);
            }
        }
    }
    return -1;
}

vector<string> AdminArchivosCarpetas::getCampos(std::string entrada) {
    vector<string> campos;
    string copiaEntrada=entrada;
    stringstream ss(entrada);
    string campo;

    while (getline(ss, campo, ',')){
        if (campo != "")campos.push_back(campo);
    }
    return campos;
}

int AdminArchivosCarpetas::getIdGrupo(string nameGrupo) {
    string grupo;
    string stringAlmacenadoInodo=this->getStringAlmacenadoInodo(this->sb.s_inode_start + sizeof(TablaInodo));
    stringstream ss(stringAlmacenadoInodo);

    while (getline(ss, grupo, '\n')){
        if (grupo != ""){
            vector<string> camposGrupo=this->getCampos(grupo);
            if (camposGrupo[1] == "G" && camposGrupo[2] == nameGrupo && camposGrupo[0] != "0"){
                return stoi(camposGrupo[0]);
            }
        }
    }
    return -1;
}

/**
 * Método de tipo recursivo para cambiar el propietario de los
 * inodos y de sus inodos hijos ( si la opción "r" es true)
 * @param direccionInodo
 * @param idUser
 * @param idGrupo
 * @param r
 */
void AdminArchivosCarpetas::cambiarPropietarioRecursivo_pt1(int direccionInodo, int idUser, int idGrupo, bool r) {
    BloqueApuntador  blkApuntador_1,blkApuntador_2,blkApuntador_3;
    BloqueCarpeta blkCarpeta;
    TablaInodo tablaInodo;
    fseek(file, direccionInodo, SEEK_SET);
    fread(&tablaInodo, sizeof(TablaInodo), 1, file);

    if((tablaInodo.i_type != '0' && tablaInodo.i_type != '1' )){
        return;
    }

    // Usuario root (tiene el poder de hacer lo que quiera)
    if (this->usuario->idU==1 && this->usuario->idG==1){
        tablaInodo.i_mtime= time(nullptr);
        tablaInodo.i_uid=idUser;
        tablaInodo.i_gid=idGrupo;
        fseek(file, direccionInodo, SEEK_SET);
        fwrite(&tablaInodo, sizeof(TablaInodo), 1, file);
        this->cambiarPropietarioRecursivo_pt2(tablaInodo, idUser, idGrupo, r);
    }
    // Si el usuario es propietario
    else if (tablaInodo.i_uid == this->usuario->idU && tablaInodo.i_gid == this->usuario->idG){
        tablaInodo.i_mtime= time(nullptr);
        tablaInodo.i_uid=idUser;
        tablaInodo.i_gid=idGrupo;
        fseek(file, direccionInodo, SEEK_SET);
        fwrite(&tablaInodo, sizeof(TablaInodo), 1, file);
        this->cambiarPropietarioRecursivo_pt2(tablaInodo, idUser, idGrupo, r);
    }
    // Si no lo es, solo busca recursivamente si "r" esta habilitado.
    else{
        this->cambiarPropietarioRecursivo_pt2(tablaInodo, idUser, idGrupo, r);
    }
}

void AdminArchivosCarpetas::cambiarPropietarioRecursivo_pt2(TablaInodo tablaInodo, int idUser, int idGrupo, bool r){
    BloqueApuntador  blkApuntador_1,blkApuntador_2,blkApuntador_3;
    BloqueCarpeta blkCarpeta;
    // SI ES UNA CARPETA SE RECORRER RECURSIVAMENTE
    if (r && tablaInodo.i_type == '0'){
        // RECORRER PUNTEROS
        for (int i = 0; i < 15; ++i) {
            if (tablaInodo.i_block[i] != -1){
                // PUNTEROS DIRECTOS
                if (i<12){
                    fseek(file, tablaInodo.i_block[i], SEEK_SET);
                    fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, file);
                    // BLOQUE CARPETA
                    for (int j = 0; j < 4; ++j) {
                        // RECORDAR QUE "." Y ".." NO TIENEN PROPIETARIO COMO TAL
                        string nombreFile=blkCarpeta.b_content[j].b_name;
                        if ((nombreFile != "." && nombreFile != "..")
                        && blkCarpeta.b_content[j].b_inodo != -1) {
                            this->cambiarPropietarioRecursivo_pt1(blkCarpeta.b_content[j].b_inodo,
                                                                  idUser,
                                                                  idGrupo,
                                                                  r);
                        }
                    }

                }
                // PUNTEROS SIMPLES
                else if (i==12){
                    fseek(file, tablaInodo.i_block[i], SEEK_SET);
                    fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, file);
                    // PUNTEROS DIRECTOS
                    for (int j = 0; j < 16; ++j) {
                        if (blkApuntador_1.b_pointers[j] != -1){
                            fseek(file, blkApuntador_1.b_pointers[j], SEEK_SET);
                            fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, file);
                            // BLOQUE CARPETA
                            for (int k = 0; k < 4; ++k) {
                                // RECORDAR QUE "." Y ".." NO TIENEN PROPIETARIO COMO TAL
                                string nombreFile=blkCarpeta.b_content[k].b_name;
                                if ((nombreFile != "." && nombreFile != "..")
                                && blkCarpeta.b_content[k].b_inodo != -1) {
                                    this->cambiarPropietarioRecursivo_pt1(blkCarpeta.b_content[k].b_inodo,
                                                                          idUser,
                                                                          idGrupo,
                                                                          r);
                                }
                            }
                        }
                    }
                }
                // PUNTEROS DOBLES
                else if (i==13){
                    fseek(file, tablaInodo.i_block[i], SEEK_SET);
                    fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, file);
                    // PUNTEROS SIMPLES
                    for (int j = 0; j < 16; ++j) {
                        if (blkApuntador_1.b_pointers[j] != -1){
                            fseek(file, blkApuntador_1.b_pointers[j], SEEK_SET);
                            fread(&blkApuntador_2, sizeof(BloqueApuntador), 1, file);
                            // PUNTEROS DIRECTOS
                            for (int k = 0; k < 16; ++k) {
                                if (blkApuntador_2.b_pointers[k] != -1){
                                    fseek(file, blkApuntador_2.b_pointers[k], SEEK_SET);
                                    fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, file);
                                    // BLOQUE CARPETA
                                    for (int z = 0; z < 4; ++z) {
                                        string nombreFile=blkCarpeta.b_content[z].b_name;
                                        // RECORDAR QUE "." Y ".." NO TIENEN PROPIETARIO COMO TAL
                                        if ((nombreFile != "." && nombreFile != "..") && blkCarpeta.b_content[z].b_inodo != -1) {
                                            this->cambiarPropietarioRecursivo_pt1(blkCarpeta.b_content[z].b_inodo,
                                                                                  idUser, idGrupo, r);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                // PUNTEROS TRIPLES
                else if (i==14){
                    fseek(file, tablaInodo.i_block[i], SEEK_SET);
                    fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, file);
                    // PUNTEROS DOBLES
                    for (int j = 0; j < 16; ++j) {
                        if (blkApuntador_1.b_pointers[j] != -1){
                            fseek(file, blkApuntador_1.b_pointers[j], SEEK_SET);
                            fread(&blkApuntador_2, sizeof(BloqueApuntador), 1, file);
                            // PUNTEROS SIMPLES
                            for (int k = 0; k < 16; ++k) {
                                if (blkApuntador_2.b_pointers[k] != -1){
                                    fseek(file, blkApuntador_2.b_pointers[k], SEEK_SET);
                                    fread(&blkApuntador_3, sizeof(BloqueApuntador), 1, file);
                                    // PUNTEROS DIRECTOS
                                    for (int z = 0; z < 16; ++z) {
                                        if (blkApuntador_3.b_pointers[z] != -1) {
                                            fseek(file, blkApuntador_3.b_pointers[z], SEEK_SET);
                                            fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, file);
                                            // BLOQUE CARPETA
                                            for (int x = 0; x < 4; ++x) {
                                                // RECORDAR QUE "." Y ".." NO TIENEN PROPIETARIO COMO TAL
                                                string nombreFile=blkCarpeta.b_content[x].b_name;
                                                if ((nombreFile != "." && nombreFile != "..") && blkCarpeta.b_content[x].b_inodo != -1) {
                                                    this->cambiarPropietarioRecursivo_pt1(
                                                            blkCarpeta.b_content[x].b_inodo, idUser, idGrupo, r);
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
}

void AdminArchivosCarpetas::find() {
    Nodo_M *nodoMontura=this->mountList->buscar(usuario->idMount);
    if (this->path==" "){
        cout<<"EL PARAMETRO PATH ES OBLIGATORIO"<<endl;
        return;
    }
    if (this->name==" "){
        cout<<"EL PARAMETRO NAME ES OBLIGATORIO"<<endl;
        return;
    }
    if (nodoMontura == NULL){
        cout <<"NO EXISTE MONTURA CON EL ID: "<< this->usuario->idMount<<" EN EL SISTEMA"<<endl;
        return;
    }
    if ((this->file= fopen(nodoMontura->path.c_str(), "rb+"))){
        if (nodoMontura->type == 'l'){
            EBR ebr;
            fseek(file, nodoMontura->start, SEEK_SET);
            fread(&ebr, sizeof(EBR),1,file);
            if (ebr.part_status != '2') {
                fclose(file);
                cout<<"LA PARTICION "<<nodoMontura->name<<" NO HA SIDO FORMATEADA AL PARECER" << endl;
                return;
            }
            fseek(file, nodoMontura->start + sizeof(EBR), SEEK_SET);
        }
        else if (nodoMontura->type == 'p'){
            MBR mbr;
            fseek(this->file,0,SEEK_SET);
            fread(&mbr, sizeof(MBR),1,file);
            if (mbr.mbr_partition[nodoMontura->pos].part_status != '2'){
                cout<<"LA PARTICION "<<nodoMontura->name<<" NO HA SIDO FORMATEADA AL PARECER" << endl;
                return;
            }
            fseek(this->file, nodoMontura->start, SEEK_SET);
        }
        fread(&this->sb, sizeof(SuperBloque),1,file);
        /**
         * Nota: para buscar en una carpeta en especifico utilizando el parametro PATH
         * pues simplemente utilizamos el dividirRuta y enviamos en lugar de el inodo inicial
         * el inodo de la carpeta en el metodo y ya esta.
         *
         * y para el * y ? pues agregar unos if que si el nameBuscado es eso, se imprime all.
         *
         * añadir Journal del FIND
         */
        int direccionInodoFile;
        if (this->path=="/"){
            direccionInodoFile=this->sb.s_inode_start;
        }
        else{
            vector<string> rutaDivivida= this->getRutaDividida(this->path);
            if (rutaDivivida.empty()){
                fclose(this->file);
                cout<<"LA RUTA INGRESADA NO ES VALIDA O NO EXISTE"<<endl;
                return;
            }
            direccionInodoFile=this->getDireccionInodo(rutaDivivida, 0, rutaDivivida.size() - 1, this->sb.s_inode_start, file);
            if (direccionInodoFile == -1){
                cout << "EL ARCHIVO INDICADO NO EXISTE" << endl;
                return;
            }
        }


        string ruta;
        if(this->name=="/"){ // Imprimes la root
            ruta = "/";
            cout << ruta << endl;
            fclose(this->file);
            return;
        }
        else{
                ruta = "/\n";

                string extra ="";
                // POR EL MOMENTO MOSTRAR all
                if(this->name=="*" || this->name=="*.?" || this->name=="?.*" || this->name=="?" || this->name=="*.*" || this->name=="?.?"){
                    extra = find_ImprimirBusqueda(direccionInodoFile, this->name, 1, true);
                }
                // Si es una ruta normal
                else{
                    extra = find_ImprimirBusqueda(direccionInodoFile, this->name, 1, false);
                }

                if(extra == " "){
                    fclose(this->file);
                    cout << "EL ARCHIVO O CARPETA NO EXISTE" << endl;
                    return;
                }
                ruta += extra;
        }

        cout << ruta << endl;
        fclose(this->file);
    }
    else{
        cout <<"EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA"<<endl;
    }
}

string AdminArchivosCarpetas::find_ImprimirBusqueda(int direccionInodo, string nombreBusqueda, int contadorProfundidad, bool mostrarTodo) {
    BloqueApuntador  blkApuntador_1,blkApuntador_2,blkApuntador_3;
    BloqueCarpeta blkCarpeta;
    BloqueArchivo blkArchivo;
    TablaInodo tablaInodo;
    fseek(file, direccionInodo, SEEK_SET);
    fread(&tablaInodo, sizeof(TablaInodo), 1, file);

    if((tablaInodo.i_type != '0' && tablaInodo.i_type != '1' )){
        return "";
    }

    if(tablaInodo.i_type=='1'){
        return " ";
    }
    string mostrarTodoRuta = "";
    // SI ES UNA CARPETA SE RECORRE RECURSIVAMENTE
    if (tablaInodo.i_type == '0'){
        // RECORRER PUNTEROS
        for (int i = 0; i < 15; ++i) {
            if (tablaInodo.i_block[i] != -1){
                // PUNTEROS DIRECTOS
                if (i<12){
                    fseek(file, tablaInodo.i_block[i], SEEK_SET);
                    fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, file);
                    // BLOQUE CARPETA
                    for (int j = 0; j < 4; ++j) {
                        // RECORDAR QUE "." Y ".." NO TIENEN QUE MOSTRARSE
                        string nombreFile=blkCarpeta.b_content[j].b_name;
                        if ((nombreFile != "." && nombreFile != "..") && blkCarpeta.b_content[j].b_inodo != -1) {
                            if(mostrarTodo){
                                for (int k = 0; k < contadorProfundidad; ++k) {
                                    mostrarTodoRuta += "    ";
                                }
                                mostrarTodoRuta += "| " + nombreFile + "\n";
                                string retorno = this->find_ImprimirBusqueda(blkCarpeta.b_content[j].b_inodo, nombreBusqueda, contadorProfundidad+1, mostrarTodo);
                                mostrarTodoRuta += retorno;
                            }else{
                                // Si se encuentra el nombre se retorna
                                if(nombreFile == nombreBusqueda){
                                    string retorno = "";
                                    for (int k = 0; k < contadorProfundidad; ++k) {
                                        retorno += "    ";
                                    }
                                    retorno += "| " + nombreFile + "\n";
                                    return retorno;
                                }
                                // Si no se encuentra se sigue viajando hasta encontrarlo
                                string retorno = this->find_ImprimirBusqueda(blkCarpeta.b_content[j].b_inodo, nombreBusqueda, contadorProfundidad+1, mostrarTodo);
                                // Si se encontro algo pues retornar recursivamente
                                if(retorno != " " && retorno != ""){
                                    string extra = "";
                                    for (int k = 0; k < contadorProfundidad; ++k) {
                                        extra += "    ";
                                    }
                                    extra += "| " + nombreFile + "\n";
                                    extra += retorno;
                                    return extra;
                                }
                            }
                        }
                    }

                }
                // PUNTEROS SIMPLES
                else if (i==12){
                    fseek(file, tablaInodo.i_block[i], SEEK_SET);
                    fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, file);
                    // PUNTEROS DIRECTOS
                    for (int j = 0; j < 16; ++j) {
                        if (blkApuntador_1.b_pointers[j] != -1){
                            fseek(file, blkApuntador_1.b_pointers[j], SEEK_SET);
                            fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, file);
                            // BLOQUE CARPETA
                            for (int k = 0; k < 4; ++k) {
                                // RECORDAR QUE "." Y ".." NO TIENEN QUE MOSTRARSE
                                string nombreFile=blkCarpeta.b_content[k].b_name;
                                if ((nombreFile != "." && nombreFile != "..") && blkCarpeta.b_content[k].b_inodo != -1) {
                                    if(mostrarTodo){
                                        for (int k = 0; k < contadorProfundidad; ++k) {
                                            mostrarTodoRuta += "    ";
                                        }
                                        mostrarTodoRuta += "| " + nombreFile + "\n";
                                        string retorno = this->find_ImprimirBusqueda(blkCarpeta.b_content[k].b_inodo, nombreBusqueda, contadorProfundidad+1, mostrarTodo);
                                        mostrarTodoRuta += retorno;
                                    }else {
                                        // Si se encuentra el nombre se retorna
                                        if (nombreFile == nombreBusqueda) {
                                            string retorno = "";
                                            for (int k = 0; k < contadorProfundidad; ++k) {
                                                retorno += "    ";
                                            }
                                            retorno += "| " + nombreFile + "\n";
                                            return retorno;
                                        }
                                        // Si no se encuentra se sigue viajando hasta encontrarlo
                                        string retorno = this->find_ImprimirBusqueda(blkCarpeta.b_content[k].b_inodo,
                                                                                     nombreBusqueda,
                                                                                     contadorProfundidad + 1,
                                                                                     mostrarTodo);
                                        // Si se encontro algo pues retornar recursivamente
                                        if (retorno != " " && retorno != "") {
                                            string extra = "";
                                            for (int k = 0; k < contadorProfundidad; ++k) {
                                                extra += "    ";
                                            }
                                            extra += "| " + nombreFile + "\n";
                                            extra += retorno;
                                            return extra;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                // PUNTEROS DOBLES
                else if (i==13){
                    fseek(file, tablaInodo.i_block[i], SEEK_SET);
                    fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, file);
                    // PUNTEROS SIMPLES
                    for (int j = 0; j < 16; ++j) {
                        if (blkApuntador_1.b_pointers[j] != -1){
                            fseek(file, blkApuntador_1.b_pointers[j], SEEK_SET);
                            fread(&blkApuntador_2, sizeof(BloqueApuntador), 1, file);
                            // PUNTEROS DIRECTOS
                            for (int k = 0; k < 16; ++k) {
                                if (blkApuntador_2.b_pointers[k] != -1){
                                    fseek(file, blkApuntador_2.b_pointers[k], SEEK_SET);
                                    fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, file);
                                    // BLOQUE CARPETA
                                    for (int z = 0; z < 4; ++z) {
                                        string nombreFile=blkCarpeta.b_content[z].b_name;
                                        // RECORDAR QUE "." Y ".." NO TIENEN QUE MOSTRARSE
                                        if ((nombreFile != "." && nombreFile != "..") && blkCarpeta.b_content[z].b_inodo != -1) {
                                            if(mostrarTodo){
                                                for (int k = 0; k < contadorProfundidad; ++k) {
                                                    mostrarTodoRuta += "    ";
                                                }
                                                mostrarTodoRuta += "| " + nombreFile + "\n";
                                                string retorno = this->find_ImprimirBusqueda(blkCarpeta.b_content[z].b_inodo, nombreBusqueda, contadorProfundidad+1, mostrarTodo);
                                                mostrarTodoRuta += retorno;
                                            }else {
                                                // Si se encuentra el nombre se retorna
                                                if (nombreFile == nombreBusqueda) {
                                                    string retorno = "";
                                                    for (int k = 0; k < contadorProfundidad; ++k) {
                                                        retorno += "    ";
                                                    }
                                                    retorno += "| " + nombreFile + "\n";
                                                    return retorno;
                                                }
                                                // Si no se encuentra se sigue viajando hasta encontrarlo
                                                string retorno = this->find_ImprimirBusqueda(
                                                        blkCarpeta.b_content[z].b_inodo, nombreBusqueda,
                                                        contadorProfundidad + 1, mostrarTodo);
                                                // Si se encontro algo pues retornar recursivamente
                                                if (retorno != " " && retorno != "") {
                                                    string extra = "";
                                                    for (int k = 0; k < contadorProfundidad; ++k) {
                                                        extra += "    ";
                                                    }
                                                    extra += "| " + nombreFile + "\n";
                                                    extra += retorno;
                                                    return extra;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                // PUNTEROS TRIPLES
                else if (i==14){
                    fseek(file, tablaInodo.i_block[i], SEEK_SET);
                    fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, file);
                    // PUNTEROS DOBLES
                    for (int j = 0; j < 16; ++j) {
                        if (blkApuntador_1.b_pointers[j] != -1){
                            fseek(file, blkApuntador_1.b_pointers[j], SEEK_SET);
                            fread(&blkApuntador_2, sizeof(BloqueApuntador), 1, file);
                            // PUNTEROS SIMPLES
                            for (int k = 0; k < 16; ++k) {
                                if (blkApuntador_2.b_pointers[k] != -1){
                                    fseek(file, blkApuntador_2.b_pointers[k], SEEK_SET);
                                    fread(&blkApuntador_3, sizeof(BloqueApuntador), 1, file);
                                    // PUNTEROS DIRECTOS
                                    for (int z = 0; z < 16; ++z) {
                                        if (blkApuntador_3.b_pointers[z] != -1) {
                                            fseek(file, blkApuntador_3.b_pointers[z], SEEK_SET);
                                            fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, file);
                                            // BLOQUE CARPETA
                                            for (int x = 0; x < 4; ++x) {
                                                // RECORDAR QUE "." Y ".." NO TIENEN QUE MOSTRARSE
                                                string nombreFile=blkCarpeta.b_content[x].b_name;
                                                if ((nombreFile != "." && nombreFile != "..") && blkCarpeta.b_content[x].b_inodo != -1) {
                                                    if(mostrarTodo){
                                                        for (int k = 0; k < contadorProfundidad; ++k) {
                                                            mostrarTodoRuta += "    ";
                                                        }
                                                        mostrarTodoRuta += "| " + nombreFile + "\n";
                                                        string retorno = this->find_ImprimirBusqueda(blkCarpeta.b_content[x].b_inodo, nombreBusqueda, contadorProfundidad+1, mostrarTodo);
                                                        mostrarTodoRuta += retorno;
                                                    }else {
                                                        // Si se encuentra el nombre se retorna
                                                        if (nombreFile == nombreBusqueda) {
                                                            string retorno = "";
                                                            for (int k = 0; k < contadorProfundidad; ++k) {
                                                                retorno += "    ";
                                                            }
                                                            retorno += "| " + nombreFile + "\n";
                                                            return retorno;
                                                        }
                                                        // Si no se encuentra se sigue viajando hasta encontrarlo
                                                        string retorno = this->find_ImprimirBusqueda(
                                                                blkCarpeta.b_content[x].b_inodo, nombreBusqueda,
                                                                contadorProfundidad + 1, mostrarTodo);
                                                        // Si se encontro algo pues retornar recursivamente
                                                        if (retorno != " " && retorno != "") {
                                                            string extra = "";
                                                            for (int k = 0; k < contadorProfundidad; ++k) {
                                                                extra += "    ";
                                                            }
                                                            extra += "| " + nombreFile + "\n";
                                                            extra += retorno;
                                                            return extra;
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
        }
    }
    if(mostrarTodo){
        return mostrarTodoRuta;
    }
    return " ";
}

void AdminArchivosCarpetas::edit() {
    Nodo_M *nodo=this->mountList->buscar(usuario->idMount);
    if (nodo==NULL){
        cout <<"NO EXISTE MONTURA CON EL ID: "<< this->usuario->idMount<<" EN EL SISTEMA"<<endl;
        return;
    }
    if (this->cont==" "){
        cout<<"EL PARAMETRO CONT ES OBLIGATORIO"<<endl;
        return;
    }

    if (this->path==" "){
        cout<<"EL PARAMETRO PATH ES OBLIGATORIO"<<endl;
        return;
    }

    if ((this->file= fopen(nodo->path.c_str(),"rb+"))){
        TablaInodo tablaInodo;
        vector<string> rutaDividida= this->getRutaDividida(this->path);
        if (rutaDividida.empty()){
            cout<<"LA RUTA INGRESADA NO ES VALIDA"<<endl;
            fclose(this->file);
            return;
        }
        if (nodo->type=='l'){
            EBR ebr;
            fseek(file,nodo->start,SEEK_SET);
            fread(&ebr, sizeof(EBR),1,file);
            if (ebr.part_status != '2') {
                fclose(file);
                cout<<"ERROR PARTICION SIN FORMATEAR"<<endl;
                return;
            }
            fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
        }
        else if (nodo->type=='p'){
            MBR mbr;
            fseek(this->file,0,SEEK_SET);
            fread(&mbr, sizeof(MBR),1,file);
            if (mbr.mbr_partition[nodo->pos].part_status!='2'){
                cout<<"ERROR PARTICION SIN FORMATEAR"<<endl;
                return;
            }
            fseek(this->file,nodo->start,SEEK_SET);
        }
        fread(&this->sb, sizeof(SuperBloque),1,file);

        int direccionInodoFile=this->getDireccionInodo(rutaDividida, 0, rutaDividida.size() - 1, this->sb.s_inode_start, this->file);
        if (direccionInodoFile == -1){
            cout << "LA RUTA INGRESADA NO EXISTE" << endl;
            fclose(this->file);
            return;
        }

        if (!this->verificarPermisoInodo_Escritura(direccionInodoFile)){
            cout << "IMPOSIBLE EJECUTAR EDITAR POR FALTA DE PERMISOS"<< endl;
            return;
        }

        // Leer el Inodo del File para luego leer su contenido.
        fseek(this->file, direccionInodoFile, SEEK_SET);
        fread(&tablaInodo, sizeof(TablaInodo), 1, this->file);
        string textoInodoActual=this->getStringAlmacenadoInodo(direccionInodoFile);

        string infoFileComputadora;
        FILE *fileComputadora;
        if ((fileComputadora = fopen(this->cont.c_str(), "r"))) {
            infoFileComputadora=this->leerArchivoComputadora();
            fclose(fileComputadora);
        }
        else {
            cout << "ERROR EN LA RUTA "<<this->cont << " NO EXISTE EL ARCHIVO"<< endl;
            return;
        }

        vector<string> blksActualesInodoFile=this->getArrayBlks(textoInodoActual);
        vector<string> blksNuevosInodoFile=this->getArrayBlks(infoFileComputadora);
        int blksExtraNuevos= blksNuevosInodoFile.size() - blksActualesInodoFile.size();
        if (this->sb.s_free_blocks_count < blksExtraNuevos){
            cout << "NO EXISTEN LOS BLOQUES LIBRES SUFICIENTES EN EL SISTEMA" << endl;
            return;
        }
        if (blksNuevosInodoFile.size() >= 4380){
            cout << "ERROR EL CONTENIDO NUEVO ES MUY GRANDE EXCEDE LOS BLOQUES POR INODO" << endl;
            return;
        }

        // NO, SE USAN MENOS BLOQUES DE LOS QUE YA SE USABAN
        if (blksExtraNuevos < 0){
            bool eliminarBlk=false;
            int cont=0;
            while (cont < blksActualesInodoFile.size()){
                string cadena="";
                if (cont >= blksNuevosInodoFile.size()){
                    eliminarBlk= true;
                }
                else if (cont < blksNuevosInodoFile.size()){
                    cadena=blksNuevosInodoFile[cont];
                }
                tablaInodo= this->editarInodoRecursivamente(cadena, tablaInodo, cont, eliminarBlk);
                if (tablaInodo.i_type == '2'){
                    fseek(this->file, direccionInodoFile, SEEK_SET);
                    fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);
                    cout<<"ERROR AL EJECUTAR EL COMANDO"<<endl;
                    return;
                }
                cont++;
            }
        }
        // SI, SE NECESITAN MÁS BLOQUES DE LOS QUE YA ESTABAN
        else if (blksExtraNuevos > 0){
            int inicioBlksFree = this->getInicioBloqueLibresSeguidos(blksNuevosInodoFile.size() - blksExtraNuevos);

            // NO HAY ESPACIO
            if (inicioBlksFree == -1){
                cout << "NO EXISTEN LOS BLOQUES LIBRES SUFICIENTES EN EL SISTEMA" << endl;
                return;
            }
            // SI HAY ESPACIO
            else{
                int contBlkSpace=0;
                int index=0;
                while (index < blksNuevosInodoFile.size()){
                    flagGlobal=false;
                    tablaInodo=this->addFile(index, (inicioBlksFree + contBlkSpace), blksNuevosInodoFile[index], tablaInodo);
                    if (tablaInodo.i_type == '2'){
                        fseek(this->file, direccionInodoFile, SEEK_SET);
                        fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);
                        cout<<"ERROR AL EJECUTAR EL COMANDO"<<endl;
                        return;
                    }
                    if (flagGlobal){
                        contBlkSpace++;
                    }
                    index++;
                }
            }
        }
        // IGUAL, SE UTILIZA LA MISMA CANTIDAD DE BLOQUES
        else{
            int j=0;
            while (j < blksActualesInodoFile.size()){
                string cadena="";
                if (j < blksNuevosInodoFile.size()){
                    cadena=blksNuevosInodoFile[j];
                }
                tablaInodo= this->editarInodoRecursivamente(cadena, tablaInodo, j, false);
                if (tablaInodo.i_type == '2'){
                    cout<<"ERROR AL EJECUTAR EL COMANDO"<<endl;
                    fseek(this->file, direccionInodoFile, SEEK_SET);
                    fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);
                    return;
                }
                j++;
            }
        }

        int size = 0;
        for (int i = 0; i < blksNuevosInodoFile.size(); i++) {
            size += blksNuevosInodoFile[i].length();
        }
        tablaInodo.i_s=size;
        tablaInodo.i_mtime = time(nullptr);

        if (nodo->type=='l'){
            EBR ebr;
            fseek(file,nodo->start,SEEK_SET);
            fread(&ebr, sizeof(EBR),1,file);
            fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
        }
        else if (nodo->type=='p'){
            MBR mbr;
            fseek(this->file,0,SEEK_SET);
            fread(&mbr, sizeof(MBR),1,file);
            fseek(this->file,nodo->start,SEEK_SET);
        }
        fwrite(&this->sb, sizeof(SuperBloque),1,file);

        fseek(this->file, direccionInodoFile, SEEK_SET);
        fwrite(&tablaInodo, sizeof(TablaInodo), 1, this->file);

        if (this->sb.s_filesystem_type==3){
            this->registrarJournal("edit",'1',this->path,this->cont,nodo);
        }
        cout<<"COMANDO EJECUTADO CON EXITO, EL ARCHIVO "<<this->path<< " FUE EDITADO CON EXITO"<<endl;
        fclose(this->file);
    }
    else{
        cout <<"EL DISCO SE MOVIO PORQUE NO SE ENCUENTRA"<<endl;
    }
}
/**
 * Método para buscar el primer grupo de bloques libres seguidos en el bitmap de bloques.
 * @param bits_a_Buscar
 * @return
 */
int AdminArchivosCarpetas::getInicioBloqueLibresSeguidos(int bits_a_Buscar) {
    int bitMapBlks_Start = this->sb.s_bm_block_start;
    int bitMapBlks_End = bitMapBlks_Start + this->sb.s_block_start;
    int blksLibresSeguidos = 0;
    int inicioBM = -1;
    int contAux = 0;
    int inicioB = -1;
    char bit;
    if (bits_a_Buscar > 0) {
        for (int i = bitMapBlks_Start; i < bitMapBlks_End; ++i) {
            fseek(this->file, i, SEEK_SET);
            fread(&bit, sizeof(char), 1, this->file);
            // Bloque disponible
            if (bit == '0') {
                if (blksLibresSeguidos == 0) {
                    inicioBM = i;
                    inicioB = contAux;
                }
                blksLibresSeguidos++;

            }
            // Bloque no disponible
            else {
                blksLibresSeguidos = 0;
                inicioBM = -1;
                inicioB = -1;
            }

            if (blksLibresSeguidos >= bits_a_Buscar) break;
            contAux++;
        }
        // Validar que si se encontro espacio libre
        if (inicioBM == -1 || (blksLibresSeguidos != bits_a_Buscar)) {
            cout << "NO EXISTEN LOS SUFICIENTES BLOQUES O BLOQUES SEGUIDOS EN EL SISTEMA PARA ACTUALIZAR EL ARCHIVO" << endl;
            return -1;
        }

        for (int i = inicioBM; i < (inicioBM + bits_a_Buscar); ++i) {
            char uno = '1';
            fseek(this->file, i, SEEK_SET);
            fwrite(&uno, sizeof(char), 1, this->file);

        }
        int bit2 = 0;
        this->sb.s_free_blocks_count -= bits_a_Buscar; // restar la cantidad de bloques disponibles

        for (int k = bitMapBlks_Start; k < bitMapBlks_End; k++) {
            fseek(this->file, k, SEEK_SET);
            fread(&bit, sizeof(char), 1, this->file);
            bit2++;
            if (bit == '0') break;
        }
        this->sb.s_first_blo = bit2; //ACTUALIZAR PRIMER BLOQUE DISPONIBLE
    }
    return inicioB;
}

TablaInodo AdminArchivosCarpetas::editarInodoRecursivamente(string contenidoBlk, TablaInodo tablaInodoActual, int indexBlk, bool eliminarBlk) {
    BloqueApuntador blkApuntador_1,blkApuntador_2,blkApuntador_3;
    SuperBloque sblk;
    for (int i = 0; i < 15; ++i) {
        if (tablaInodoActual.i_block[i] != -1){
            // PUNTERO DIRECTO
            if (i<12 && i == indexBlk){ // rango indexBlk[0-11] es de los punteros directos
                BloqueArchivo blkArchivo;
                // SI EL BLOQUE SE TIENE QUE ELIMINAR
                if (eliminarBlk){
                    // primero sumamos uno al número de bloques libres.
                    this->sb.s_free_blocks_count+=1;
                    // se encuentra el indice del bitmap a vaciar
                    char bit_0='0';
                    int indiceBitMap = this->getIndiceBitMapBlks(tablaInodoActual.i_block[i]);
                    fseek(this->file, this->sb.s_bm_block_start + indiceBitMap, SEEK_SET);
                    fwrite(&bit_0, sizeof(char), 1, this->file);
                    // se actualiza el bloque vacio
                    this->update_First_BlkFree();
                    tablaInodoActual.i_block[i]=-1;
                    return tablaInodoActual;
                }
                fseek(this->file, tablaInodoActual.i_block[i], SEEK_SET);
                fread(&blkArchivo, sizeof(BloqueArchivo), 1, this->file);
                strcpy(blkArchivo.b_content, contenidoBlk.c_str());
                fseek(this->file, tablaInodoActual.i_block[i], SEEK_SET);
                fwrite(&blkArchivo, sizeof(BloqueArchivo), 1, this->file);
                return tablaInodoActual;
            }
            // PUNTERO SIMPLE
            else if (i==12){
                fseek(this->file, tablaInodoActual.i_block[i], SEEK_SET);
                fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, this->file);
                for (int j = 0; j < 16; ++j) {
                    if (blkApuntador_1.b_pointers[j] != -1 && (indexBlk == (12 + j))){ // rango indexBlk[12-27] es de los punteros simples
                        BloqueArchivo blkArchivo;
                        // SI EL BLOQUE SE TIENE QUE ELIMINAR
                        if (eliminarBlk){
                            // PRIMERO ELIMINAMOS EL BLOQUE ARCHIVO
                            // primero sumamos uno al número de bloques libres.
                            this->sb.s_free_blocks_count+=1;
                            // se encuentra el indice del bitmap a vaciar
                            char bit_0='0';
                            int indiceBitMap = this->getIndiceBitMapBlks(blkApuntador_1.b_pointers[j]);
                            fseek(this->file, this->sb.s_bm_block_start + indiceBitMap, SEEK_SET);
                            fwrite(&bit_0, sizeof(char), 1, this->file);
                            // se actualiza el bloque vacio
                            this->update_First_BlkFree();
                            // se actualiza el bloque apuntador simple
                            blkApuntador_1.b_pointers[j]=-1;
                            fseek(this->file, tablaInodoActual.i_block[i], SEEK_SET);
                            fwrite(&blkApuntador_1, sizeof(BloqueApuntador), 1, this->file);

                            // VALIDAMOS SI HAY QUE ELIMINAR EL BLOQUE APUNTADOR SIMPLE
                            if (j==15){
                                int indiceBitMap_Blk = this->getIndiceBitMapBlks(tablaInodoActual.i_block[i]);
                                fseek(this->file, this->sb.s_bm_block_start + indiceBitMap_Blk, SEEK_SET);
                                fwrite(&bit_0, sizeof(char), 1, this->file);
                                this->sb.s_free_blocks_count+=1;
                                this->update_First_BlkFree();
                                tablaInodoActual.i_block[i]=-1;
                            }
                            else if (blkApuntador_1.b_pointers[j + 1] == -1){
                                int indiceBitMap_Blk = this->getIndiceBitMapBlks(tablaInodoActual.i_block[i]);

                                fseek(this->file, this->sb.s_bm_block_start + indiceBitMap_Blk, SEEK_SET);
                                fwrite(&bit_0, sizeof(char), 1, this->file);
                                this->sb.s_free_blocks_count+=1;
                                this->update_First_BlkFree();
                                tablaInodoActual.i_block[i]=-1;
                            }
                            return tablaInodoActual;
                        }
                        fseek(this->file, blkApuntador_1.b_pointers[j], SEEK_SET);
                        fread(&blkArchivo, sizeof(BloqueArchivo), 1, this->file);
                        strcpy(blkArchivo.b_content, contenidoBlk.c_str());
                        fseek(this->file, blkApuntador_1.b_pointers[j], SEEK_SET);
                        fwrite(&blkArchivo, sizeof(BloqueArchivo), 1, this->file);
                        return tablaInodoActual;
                    }
                }
            }
            // PUNTERO DOBLE
            else if (i==13){
                bool deleteBlkP1=false;
                fseek(this->file, tablaInodoActual.i_block[i], SEEK_SET);
                fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, this->file);
                for (int j = 0; j < 16; ++j) {
                    if (blkApuntador_1.b_pointers[j] != -1){
                        fseek(this->file, blkApuntador_1.b_pointers[j], SEEK_SET);
                        fread(&blkApuntador_2, sizeof(BloqueApuntador), 1, this->file);
                        for (int k = 0; k < 16; ++k) {
                            if (blkApuntador_2.b_pointers[k] != -1 && (indexBlk == (28 + k + (16 * j)))){
                                if (eliminarBlk){
                                    int indiceBitMapBlks = this->getIndiceBitMapBlks(blkApuntador_2.b_pointers[k]);
                                    char bit_0='0';
                                    fseek(this->file, this->sb.s_bm_block_start + indiceBitMapBlks, SEEK_SET);
                                    fwrite(&bit_0, sizeof(char), 1, this->file);
                                    this->sb.s_free_blocks_count+=1;
                                    this->update_First_BlkFree();
                                    blkApuntador_2.b_pointers[k]=-1;
                                    fseek(this->file, blkApuntador_1.b_pointers[j], SEEK_SET);
                                    fwrite(&blkApuntador_2, sizeof(BloqueApuntador), 1, this->file);

                                    if (k==15){
                                        deleteBlkP1=true;
                                        indiceBitMapBlks = this->getIndiceBitMapBlks(blkApuntador_1.b_pointers[j]);
                                        fseek(this->file,this->sb.s_bm_block_start+indiceBitMapBlks,SEEK_SET);
                                        fwrite(&bit_0, sizeof(char), 1, this->file);
                                        this->sb.s_free_blocks_count+=1;
                                        this->update_First_BlkFree();
                                        blkApuntador_1.b_pointers[j]=-1;
                                        fseek(this->file, tablaInodoActual.i_block[i], SEEK_SET);
                                        fwrite(&blkApuntador_1, sizeof(BloqueApuntador), 1, this->file);
                                        break;
                                    }
                                    else if (blkApuntador_2.b_pointers[k + 1] == -1){
                                        deleteBlkP1=true;
                                        indiceBitMapBlks = this->getIndiceBitMapBlks(blkApuntador_1.b_pointers[j]);
                                        fseek(this->file,this->sb.s_bm_block_start+indiceBitMapBlks,SEEK_SET);
                                        fwrite(&bit_0, sizeof(char), 1, this->file);
                                        this->sb.s_free_blocks_count+=1;
                                        this->update_First_BlkFree();
                                        blkApuntador_1.b_pointers[j]=-1;
                                        fseek(this->file, tablaInodoActual.i_block[i], SEEK_SET);
                                        fwrite(&blkApuntador_1, sizeof(BloqueApuntador), 1, this->file);
                                        break;
                                    }
                                    return tablaInodoActual;
                                }
                                BloqueArchivo blkArchivo;
                                fseek(this->file, blkApuntador_2.b_pointers[k], SEEK_SET);
                                fread(&blkArchivo, sizeof(BloqueArchivo), 1, this->file);
                                strcpy(blkArchivo.b_content, contenidoBlk.c_str());
                                fseek(this->file, blkApuntador_2.b_pointers[k], SEEK_SET);
                                fwrite(&blkArchivo, sizeof(BloqueArchivo), 1, this->file);
                                return tablaInodoActual;
                            }
                        }
                    }
                    if (deleteBlkP1){
                        char bit_0='0';
                        if (j==15){
                            int bm = this->getIndiceBitMapBlks(tablaInodoActual.i_block[i]);
                            fseek(this->file,this->sb.s_bm_block_start+bm,SEEK_SET);
                            fwrite(&bit_0, sizeof(char), 1, this->file);
                            this->sb.s_free_blocks_count+=1;
                            this->update_First_BlkFree();
                            tablaInodoActual.i_block[i]=-1;
                            return tablaInodoActual;
                        }
                        else if (blkApuntador_1.b_pointers[j + 1] == -1){
                            int bm = this->getIndiceBitMapBlks(tablaInodoActual.i_block[i]);
                            fseek(this->file,this->sb.s_bm_block_start+bm,SEEK_SET);
                            fwrite(&bit_0, sizeof(char), 1, this->file);
                            this->sb.s_free_blocks_count+=1;
                            this->update_First_BlkFree();
                            tablaInodoActual.i_block[i]=-1;
                            return tablaInodoActual;
                        }
                        else{
                            return tablaInodoActual;
                        }
                    }
                }
            }
            // PUNTERO TRIPLE
            else if (i==14){
                bool deleteBlkP1=false;
                bool deleteBlkP2= false;
                fseek(this->file, tablaInodoActual.i_block[i], SEEK_SET);
                fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, this->file);
                for (int j = 0; j < 16; ++j) {
                    if (blkApuntador_1.b_pointers[j] != -1){
                        fseek(this->file, blkApuntador_1.b_pointers[j], SEEK_SET);
                        fread(&blkApuntador_2, sizeof(BloqueApuntador), 1, this->file);
                        for (int k = 0; k < 16; ++k) {
                            if (blkApuntador_2.b_pointers[k] != -1){
                                fseek(this->file, blkApuntador_2.b_pointers[k], SEEK_SET);
                                fread(&blkApuntador_3, sizeof(BloqueApuntador), 1, this->file);
                                for (int z = 0; z < 16; ++z) {
                                    if (blkApuntador_3.b_pointers[z] != -1 && (indexBlk == (284 + z + (16 * k) + (256 * j)))) {
                                        if (eliminarBlk){
                                            char bit_0='0';
                                            this->sb.s_free_blocks_count+=1;
                                            int indiceBitMapBlks = this->getIndiceBitMapBlks(blkApuntador_3.b_pointers[z]);
                                            fseek(this->file, this->sb.s_bm_block_start + indiceBitMapBlks, SEEK_SET);
                                            fwrite(&bit_0, sizeof(char), 1, this->file);
                                            this->update_First_BlkFree();
                                            blkApuntador_3.b_pointers[z]=-1;
                                            fseek(this->file, blkApuntador_2.b_pointers[k], SEEK_SET);
                                            fread(&blkApuntador_3, sizeof(BloqueApuntador), 1, this->file);

                                            if (z == 15){
                                                indiceBitMapBlks = this->getIndiceBitMapBlks(blkApuntador_2.b_pointers[k]);
                                                fseek(this->file,this->sb.s_bm_block_start+indiceBitMapBlks,SEEK_SET);
                                                fwrite(&bit_0, sizeof(char),1,this->file);
                                                this->sb.s_free_blocks_count+=1;
                                                this->update_First_BlkFree();
                                                blkApuntador_2.b_pointers[k]=-1;
                                                fseek(this->file, blkApuntador_1.b_pointers[j], SEEK_SET);
                                                fwrite(&blkApuntador_2, sizeof(BloqueApuntador), 1, this->file);
                                                deleteBlkP2=true;
                                                break;
                                            }
                                            else if (blkApuntador_3.b_pointers[z + 1] == -1){
                                                indiceBitMapBlks = this->getIndiceBitMapBlks(blkApuntador_2.b_pointers[k]);
                                                fseek(this->file,this->sb.s_bm_block_start+indiceBitMapBlks,SEEK_SET);
                                                fwrite(&bit_0, sizeof(char),1,this->file);
                                                this->sb.s_free_blocks_count+=1;
                                                this->update_First_BlkFree();
                                                blkApuntador_2.b_pointers[k]=-1;
                                                fseek(this->file, blkApuntador_1.b_pointers[j], SEEK_SET);
                                                fwrite(&blkApuntador_2, sizeof(BloqueApuntador), 1, this->file);
                                                deleteBlkP2=true;
                                                break;
                                            }
                                            return tablaInodoActual;
                                        }
                                        BloqueArchivo blkArchivo;
                                        fseek(this->file, blkApuntador_3.b_pointers[z], SEEK_SET);
                                        fread(&blkArchivo, sizeof(BloqueArchivo), 1, this->file);
                                        strcpy(blkArchivo.b_content, contenidoBlk.c_str());
                                        fseek(this->file, blkApuntador_3.b_pointers[z], SEEK_SET);
                                        fwrite(&blkArchivo, sizeof(BloqueArchivo), 1, this->file);
                                        return tablaInodoActual;
                                    }
                                }
                            }

                            if (deleteBlkP2){
                                char bit_0='0';
                                if (k==15){
                                    int indiceBitMapBlks = this->getIndiceBitMapBlks(blkApuntador_1.b_pointers[j]);
                                    fseek(this->file, this->sb.s_bm_block_start + indiceBitMapBlks, SEEK_SET);
                                    fwrite(&bit_0, sizeof(char), 1, this->file);
                                    this->sb.s_free_blocks_count+=1;
                                    this->update_First_BlkFree();
                                    blkApuntador_1.b_pointers[j]=-1;
                                    fseek(this->file, tablaInodoActual.i_block[i], SEEK_SET);
                                    fwrite(&blkApuntador_1, sizeof(BloqueApuntador), 1, this->file);
                                    deleteBlkP1= true;
                                    break;
                                }
                                else if (blkApuntador_2.b_pointers[k + 1] == -1){
                                    int indiceBitMapBlks = this->getIndiceBitMapBlks(blkApuntador_1.b_pointers[j]);
                                    fseek(this->file, this->sb.s_bm_block_start + indiceBitMapBlks, SEEK_SET);
                                    fwrite(&bit_0, sizeof(char), 1, this->file);
                                    this->sb.s_free_blocks_count+=1;
                                    this->update_First_BlkFree();
                                    blkApuntador_1.b_pointers[j]=-1;
                                    fseek(this->file, tablaInodoActual.i_block[i], SEEK_SET);
                                    fwrite(&blkApuntador_1, sizeof(BloqueApuntador), 1, this->file);
                                    deleteBlkP1= true;
                                    break;
                                }
                                else{
                                    return tablaInodoActual;
                                }
                            }

                        }
                    }

                    if (deleteBlkP1){
                        char bit_0='0';
                        if (j==15){
                            int indiceBitMapBlks = this->getIndiceBitMapBlks(tablaInodoActual.i_block[i]);
                            fseek(this->file, this->sb.s_bm_block_start + indiceBitMapBlks, SEEK_SET);
                            fwrite(&bit_0, sizeof(char), 1, this->file);
                            this->sb.s_free_blocks_count+=1;
                            this->update_First_BlkFree();
                            tablaInodoActual.i_block[i]=-1;
                            return tablaInodoActual;
                        }
                        else if (blkApuntador_1.b_pointers[j + 1] == -1){
                            int indiceBitMapBlks = this->getIndiceBitMapBlks(tablaInodoActual.i_block[i]);
                            fseek(this->file, this->sb.s_bm_block_start + indiceBitMapBlks, SEEK_SET);
                            fwrite(&bit_0, sizeof(char), 1, this->file);
                            this->sb.s_free_blocks_count+=1;
                            this->update_First_BlkFree();
                            tablaInodoActual.i_block[i]=-1;
                            return tablaInodoActual;
                        }
                        else{
                            return tablaInodoActual;
                        }
                    }

                }
            }
        }
    }
    // SI LLEGA AQUÍ SERÍA UN ERROR PORQUE AL TENER MENOS BLOQUES DEBERÍA DE PODER VALIDAR TODOS LOS PUNTEROS
    TablaInodo tablaInodo;
    tablaInodo.i_type = '2';
    return tablaInodo;
}

int AdminArchivosCarpetas::getIndiceBitMapBlks(int direccionBlkBusqueda) {
    int contadorIndices = 0;
    int bmBlkStart = this->sb.s_bm_block_start;
    int bmBlkEnd = bmBlkStart + this->sb.s_blocks_count;
    char bit;

    for (int i = bmBlkStart; i < bmBlkEnd; i++) {
        fseek(this->file, i, SEEK_SET);
        fread(&bit, sizeof(char), 1, this->file);
        if (bit == '1' && direccionBlkBusqueda == (this->sb.s_block_start + (contadorIndices * sizeof(BloqueApuntador)))) {
            return contadorIndices;
        }
        contadorIndices++;
    }

    return -1;
}

void AdminArchivosCarpetas::update_First_BlkFree() {
    int contadorIndice = 0;
    char bit;
    int start = this->sb.s_bm_block_start;
    int end = start + this->sb.s_blocks_count;

    contadorIndice=0;
    for (int j = start; j < end; j++) {
        fseek(this->file, j, SEEK_SET);
        fread(&bit, sizeof(char), 1, this->file);
        if (bit == '0') {
            contadorIndice++;
            break;
        }
        contadorIndice++;
    }
    this->sb.s_first_blo = contadorIndice;
}


