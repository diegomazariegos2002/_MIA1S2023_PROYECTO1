#include "Particion.h"
#include "Structs.h"
#include <string>
#include <cstring>
#include <iostream>
#include <stdlib.h>

Particion::Particion() {
    this->s=0;
    this->add=0;
    this->u='k';
    this->p=" ";
    this->t='p';
    this->f='w';
    this->d=" ";
    this->name=" ";
    this->flag='n';
}

void Particion::fdisk() {
    if (this->name!=" "){
        if (this->p!=" "){
            if (this->d=="full"){
                this->deleteFullPartition();
            }else if (this->add != 0 && this->flag == 'a'){
                if (this->u =='b' || this->u =='k' || this->u =='m'){
                    if (this->add < 0){
                        this->reducePartition();
                    }else if (this->add > 0){
                        this->incrementPartition();
                    }
                }else{
                    cout<<"UNIDAD DE ALMACENAMIENTO INCORRECTA"<<endl;
                }
//validar esto tambien
            }else if (this->s>0 && this->flag == 's'){
                if (this->u =='b' || this->u =='k' || this->u =='m'){
                    if (this->t=='p'){
                        this->primaryPartition();
                    }else if (this->t=='e'){
                        this->extendPartition();
                    }else if (this->t=='l'){
                        this->LogicPartition();
                    }else{
                        cout<<"EL TIPO DE LA PARTICION ES INVALIDO"<<endl;
                    }
                }else {
                    cout << "UNIDAD DE ALMACENAMIENTO INCORRECTO" << endl;
                }
            }else{
                cout<<"PARA EL PARAMETRO -S SE ACEPTA UNICAMENTE VALORES MAYORES A CERO"<<endl;
            }
        }else{
            cout<<"ERROR FATAL EL PARAMETRO -P ES DE CARACTER OBLIGATORIO"<<endl;
        }
    }else{
        cout<<"ERROR FATAL EL PARAMETRO -NAME ES DE CARACTER OBLIGATORIO" <<endl;
    }
}

void Particion::primaryPartition() {
    Partition newPartitionPrimary;
    int pos=-1;
    FILE *file= fopen(this->p.c_str(),"rb+");
    if (file){
        fseek(file,0,SEEK_SET);
        MBR mbr;
        fread(&mbr,sizeof (MBR),1,file);
        fclose(file);
        for (int i = 0; i < 4; ++i) {
            if (mbr.mbr_partition[i].part_start == -1){
                pos=i;
                break;
            }
        }

        if (this->freeSpace(this->s, this->p, this->u, pos)){
            if (!this->existeParticion(this->p, this->name)){
                newPartitionPrimary.part_fit=this->f;
                newPartitionPrimary.part_type=this->t;
                strcpy(newPartitionPrimary.part_name, this->name.c_str());
                newPartitionPrimary.part_status='0';
                if (this->u=='b'){
                    newPartitionPrimary.part_s=this->s;
                }else if (this->u=='k'){
                    newPartitionPrimary.part_s= this->s * 1024;
                }else if (this->u=='m'){
                    newPartitionPrimary.part_s= this->s * 1024 * 1024;
                }

                //buscando donde ubicar en la dirección de memoria el comienzo de la partición
                if (pos==0){
                    newPartitionPrimary.part_start=sizeof(MBR);
                }else{
                    newPartitionPrimary.part_start= mbr.mbr_partition[pos - 1].part_start + mbr.mbr_partition[pos - 1].part_s;
                }
                mbr.mbr_partition[pos]=newPartitionPrimary;
                file= fopen(this->p.c_str(),"rb+");
                fseek(file,0,SEEK_SET);
                fwrite(&mbr,sizeof(MBR),1,file);
                fclose(file);

                MBR mbrVerificador;
                file= fopen(this->p.c_str(),"rb+");
                fseek(file,0,SEEK_SET);
                fread(&mbrVerificador, sizeof(MBR), 1, file);
                fclose(file);
                cout<<"OPERACION REALIZADA CON EXITO" <<endl;
                cout<<"PARTICION "<<pos+1<<endl;
                cout << "NOMBRE: " << mbrVerificador.mbr_partition[pos].part_name << endl;
                cout << "TIPO: PARTICION PRIMARIA"<< endl;
                cout << "INICIO: " << mbrVerificador.mbr_partition[pos].part_start << endl;
                cout << "SIZE: " << mbrVerificador.mbr_partition[pos].part_s << endl;
            }else{
                cout<<"YA HAY UNA PARTICION CON ESE NOMBRE"<<this->name<<endl;
            }
        }else{
            cout<<"NO EXISTE EL ESPACIO NECESARIO PARA REALIZAR ESTE COMANDO"<<endl;
        }
    } else{
        cout<<"NO EXISTE EL DISCO EN LA RUTA ESPECIFICADA"<<endl;
    }
}

void Particion::extendPartition() {
    Partition newPartitionExtend;
    int indice=-1,addressEBR=-1;
    FILE *file= fopen(this->p.c_str(),"rb+");
    if (file){
        fseek(file,0,SEEK_SET);
        MBR mbr;
        fread(&mbr,sizeof (MBR),1,file);
        for (int i = 0; i < 4; ++i) {
            if (mbr.mbr_partition[i].part_start == -1){
                indice=i;
                break;
            }
        }

        if (this->freeSpace(this->s, this->p, this->u, indice)){
            if (!this->existeParticion(this->p, this->name)){
                if (!this->existeParticionExtendida(this->p)){
                    newPartitionExtend.part_fit=this->f;
                    newPartitionExtend.part_type=this->t;
                    strcpy(newPartitionExtend.part_name, this->name.c_str());
                    newPartitionExtend.part_status='0';
                    if (this->u=='b'){
                        newPartitionExtend.part_s=this->s;
                    }else if (this->u=='k'){
                        newPartitionExtend.part_s= this->s * 1024;
                    }else if (this->u=='m'){
                        newPartitionExtend.part_s= this->s * 1024 * 1024;
                    }

                    //obteniendo la dirección de memoria donde debe estar posicionada la particion
                    if (indice == 0){
                        newPartitionExtend.part_start=sizeof(MBR);
                    }else{
                        newPartitionExtend.part_start= mbr.mbr_partition[indice - 1].part_start + mbr.mbr_partition[indice - 1].part_s;
                    }

                    addressEBR=newPartitionExtend.part_start;
                    mbr.mbr_partition[indice]=newPartitionExtend;

                    fseek(file,0,SEEK_SET);
                    fwrite(&mbr,sizeof(MBR),1,file);

                    EBR ebr;
                    ebr.part_next=-1;
                    ebr.part_start=addressEBR;
                    ebr.part_s=-1;
                    ebr.part_status='0';

                    fseek(file, addressEBR, SEEK_SET);
                    fwrite(&ebr,sizeof(EBR),1,file);
                    fclose(file);

                    cout<<"OPERACION REALIZADA CON EXITO" <<endl;
                    cout << "PARTICION " << indice + 1 << endl;
                    cout << "NOMBRE: " << mbr.mbr_partition[indice].part_name << endl;
                    cout << "TIPO: PARTICION EXTENDIDA"<< endl;
                    cout << "INICIO: " << mbr.mbr_partition[indice].part_start << endl;
                    cout << "SIZE: " << mbr.mbr_partition[indice].part_s << endl;
                }else{
                    cout<<"YA EXISTE UNA PARTICION EXTENDIDA CON ESE NOMBRE"<<endl;
                }
            }else{
                cout<<"YA EXISTE UNA PARTICION CON ESE NOMBRE"<<this->name<<endl;
            }
        }else{
            cout<<"NO EXISTE EL ESPACIO NECESARIO PARA EJECUTAR ESTE COMANDO"<<endl;
        }
    }else{
        cout<<"NO EXISTE UN DISCO CON LA RUTA ESPECIFICADA"<<endl;
    }
}

void Particion::LogicPartition() {
    FILE *file= fopen(this->p.c_str(),"rb+");
    if (file){
        if (this->existeParticionExtendida(this->p)){
            if (!this->existeParticion(this->p, this->name)){
                int indice=-1;
                MBR mbr;
                fseek(file,0,SEEK_SET);
                fread(&mbr,sizeof(MBR),1,file);
                for (int i = 0; i < 4; ++i) {
                    if (mbr.mbr_partition[i].part_type == 'e'){
                        indice=i;
                        break;
                    }
                }
                if (indice != -1){
                    EBR ebrAuxiliar;
                    int fullSpace=0;
                    fseek(file, mbr.mbr_partition[indice].part_start, SEEK_SET);
                    fread(&ebrAuxiliar, sizeof(EBR), 1, file);
                    //verificando si existe una particion logica
                    if (ebrAuxiliar.part_next != -1 || ebrAuxiliar.part_s != -1){
                        fullSpace+=(sizeof (EBR) + ebrAuxiliar.part_s);
                        while (ebrAuxiliar.part_next != -1 &&
                            ftell(file)<(mbr.mbr_partition[indice].part_start + mbr.mbr_partition[indice].part_s)){
                            fseek(file, ebrAuxiliar.part_next, SEEK_SET);
                            fread(&ebrAuxiliar, sizeof(EBR), 1, file);
                            fullSpace+=(sizeof (EBR) + ebrAuxiliar.part_s);
                        }
                        EBR newExtend;
                        newExtend.part_fit=this->f;
                        newExtend.part_start= ebrAuxiliar.part_start + sizeof(EBR) + ebrAuxiliar.part_s;
                        newExtend.part_status='0';
                        newExtend.part_next=-1;
                        strcpy(newExtend.part_name, this->name.c_str());
                        if (this->u=='b'){
                            newExtend.part_s=this->s;
                        }else if (this->u=='k'){
                            newExtend.part_s= this->s * 1024;
                        }else if (this->u=='m'){
                            newExtend.part_s= this->s * 1024 * 1024;
                        }
                        int freeSpace= mbr.mbr_partition[indice].part_s - fullSpace;
                        int espacioNewE= sizeof(EBR) + newExtend.part_s;
                        ebrAuxiliar.part_next=newExtend.part_start;
                        if (freeSpace >= espacioNewE){
                            fseek(file, ebrAuxiliar.part_start, SEEK_SET);
                            fwrite(&ebrAuxiliar, sizeof(EBR), 1, file);

                            fseek(file, newExtend.part_start, SEEK_SET);
                            fwrite(&newExtend, sizeof(EBR), 1, file);
                            fclose(file);
                            cout << "OPERACION REALIZADA CON EXITO" << endl;
                            cout << "Nombre particion: " << newExtend.part_name << endl;
                            cout << "Tipo: Logica"<< endl;
                            cout << "Inicio: " << newExtend.part_start << endl;
                            cout << "Size: " << newExtend.part_s << endl;
                        }else{
                            fclose(file);
                            cout<<"NO EXISTE EL ESPACIO NECESARIO PARA EJECUTAR ESTE COMANDO"<<endl;
                        }
                    // No hay particiones lógicas por lo tanto esta es la primera
                    }else{
                        ebrAuxiliar.part_fit= this->f;
                        ebrAuxiliar.part_start=mbr.mbr_partition[indice].part_start;
                        ebrAuxiliar.part_status='0';
                        if (this->u=='b'){
                            ebrAuxiliar.part_s=this->s;
                        }else if (this->u=='k'){
                            ebrAuxiliar.part_s= this->s * 1024;
                        }else if (this->u=='m'){
                            ebrAuxiliar.part_s= this->s * 1024 * 1024;
                        }
                        ebrAuxiliar.part_next=-1;
                        strcpy(ebrAuxiliar.part_name, this->name.c_str());

                        if (mbr.mbr_partition[indice].part_s >= (ebrAuxiliar.part_s + sizeof(EBR))){
                            fseek(file,0,SEEK_SET);
                            fwrite(&mbr, sizeof(MBR),1,file);

                            fseek(file, mbr.mbr_partition[indice].part_start, SEEK_SET);
                            fwrite(&ebrAuxiliar, sizeof(EBR), 1, file);
                            fclose(file);
                            cout << "OPERACION REALIZADA CON EXITO" << endl;
                            cout << "Nombre particion: " << ebrAuxiliar.part_name << endl;
                            cout << "Tipo: Logica"<< endl;
                            cout << "Inicio: " << ebrAuxiliar.part_start << endl;
                            cout << "Size: " << ebrAuxiliar.part_s << endl;
                        }else{
                            fclose(file);
                            cout<<"NO EXISTE EL ESPACIO SUFICIENTE PARA EJECUTAR ESTE COMANDO"<<endl;
                        }

                    }
                }
            }else{
                cout<<"YA HAY UNA PARTICION CON ESTE NOMBRE "<<this->name<<endl;
            }
        }else{
            cout<<"NO SE ENCONTRO UNA PARTICION EXTENDIDA PARA ALMACENAR LA PARTICION LOGICA"<<endl;
        }
    } else{
        cout<<"NO SE ENCONTRO UN DISCO EN LA RUTA ESPECIFICADA"<<endl;
    }
}

bool Particion::freeSpace(int s, std::string p, char u, int address) {
    FILE *file= fopen(p.c_str(),"rb+");
    fseek(file,0,SEEK_SET);
    MBR mbr;
    fread(&mbr,sizeof (MBR),1,file);

    if (address > -1){
        int size=0;
        if (u=='b'){
            size=s;
        }else if (u=='k'){
            size=s*1024;
        }else if (u=='m'){
            size=s*1024*1024;
        }
        if (size>0){
            int freeSpace=0;
            if (address == 0){ // si es la primera particion
                freeSpace= mbr.mbr_tamanio - sizeof(MBR);
            }else{ // si no es la primera particion
                freeSpace= mbr.mbr_tamanio - mbr.mbr_partition[address - 1].part_start - mbr.mbr_partition[address - 1].part_s;
            }
            return freeSpace >= size;
        }
        return false;
    }else{
        return false;
    }
}

bool Particion::existeParticionExtendida(std::string p) {
    FILE *file= fopen(p.c_str(),"rb+");
    fseek(file,0,SEEK_SET);
    MBR mbr;
    fread(&mbr,sizeof (MBR),1,file);
    for (int i = 0; i < 4; ++i) {
        if (mbr.mbr_partition[i].part_type == 'e'){
            return true;
        }
    }
    return false;
}

bool Particion::existeParticion(std::string p, std::string name){
    FILE *file= fopen(p.c_str(),"rb+");
    fseek(file,0,SEEK_SET);
    MBR mbr;
    fread(&mbr,sizeof (MBR),1,file);
    for (int i = 0; i < 4; ++i) {
        string name1=mbr.mbr_partition[i].part_name;
        if (name1==name){
            fclose(file);
            return true;
        }
        if (mbr.mbr_partition[i].part_type == 'e'){
            EBR ebr;
            fseek(file, mbr.mbr_partition[i].part_start, SEEK_SET);
            fread(&ebr,sizeof (EBR),1,file);
            if (ebr.part_next != -1 || ebr.part_s != -1){
                name1=ebr.part_name;
                if (name1==name){
                    fclose(file);
                    return true;
                }
                while (ebr.part_next != -1){
                    name1=ebr.part_name;
                    if (name1==name){
                        fclose(file);
                        return true;
                    }
                    fseek(file, ebr.part_next, SEEK_SET);
                    fread(&ebr,sizeof (EBR),1,file);
                }
            }
        }

    }
    fclose(file);
    return false;
}

void Particion::deleteFullPartition() {
    FILE *file= fopen(this->p.c_str(),"rb+");
    if (file){
        cout<<"PRESIONTE (Y) SI ESTA SEGURO DE CONTINUAR CON LA ELIMINACION TOTAL DE LA PARTICION "<< this->name <<"(Y/N): ";
        string bandera;
        getline(cin,bandera);
        if (bandera=="Y" || bandera=="y"){
            int pos=-1; // variable para ver si es una particion primara lo que hay que eliminar
            // Nos posicionamos en el principio del disco de nuestro archivo, o sea, en el MBR
            MBR mbr;
            fseek(file,0,SEEK_SET);
            fread(&mbr, sizeof(MBR),1,file);
            // A partir de aquí buscamos si la partición a eliminar es igual a una partición primario o una partición extendida. Por lo que revisamos todas.
            for (int i = 0; i < 4; ++i) {
                if (mbr.mbr_partition[i].part_name == this->name){ // si la particion actual coincide se guarda su posicion dentro de las particiones.
                    pos=i;
                    break;
                }else if (mbr.mbr_partition[i].part_type == 'e'){ // si es particion extendida se buscan en las particiones que tenga adentro
                    EBR ebr; // nos ubicamos en el EBR principal
                    fseek(file, mbr.mbr_partition[i].part_start, SEEK_SET);
                    fread(&ebr,sizeof(EBR),1,file);
                    if (!(ebr.part_s == -1 && ebr.part_next == -1)){ // si hay lógica
                        if (ebr.part_name == this->name){ // si concide
                            EBR ebrAux;
                            ebrAux.part_next=ebr.part_next;
                            ebrAux.part_start=ebr.part_start;
                            ebrAux.part_s=-1;
                            ebrAux.part_status='0';
                            ebrAux.part_fit='w';
                            strcpy(ebrAux.part_name, "");


                            // Calculando el espacio que ocupa la particion
                            int posicionInicial= ebr.part_start + sizeof(EBR);
                            int posicionFinal= ebr.part_start + ebr.part_s;

                            // Calculando la magnitud de la eliminacion
                            if ((posicionFinal - posicionInicial) / (1024 * 1024) >= 1){
                                this->deleteMegaByte(this->p, posicionInicial, posicionFinal);
                            }else if ((posicionFinal - posicionInicial) / (1024) >= 1){
                                this->deleteKiloByte(this->p, posicionInicial, posicionFinal);
                            }else{
                                this->deleteByte(this->p, posicionInicial, posicionFinal);
                            }

                            // Escribiendo el EBR correspondiente después de eliminar la particion
                            fseek(file, ebr.part_start, SEEK_SET);
                            fwrite(&ebrAux, sizeof(EBR), 1, file);

                            fclose(file);
                            cout<<"PARTICION LOGICA "<< this->name << " ELIMINADA CON EXITO."<< endl;
                            return;

                        }else if(ebr.part_next != -1){ // si no es pero todavía hay más particiones se recorreren
                            int anteriorStart=ebr.part_start;
                            fseek(file, ebr.part_next, SEEK_SET);
                            fread(&ebr,sizeof (EBR),1,file);
                            while (true){ // revisar todas las particiones

                                if (ebr.part_name == this->name){
                                    EBR auxEBR;
                                    fseek(file, anteriorStart, SEEK_SET);
                                    fread(&auxEBR,sizeof (EBR),1,file);
                                    auxEBR.part_next=ebr.part_next;

                                    fseek(file, auxEBR.part_start, SEEK_SET);
                                    fwrite(&auxEBR,sizeof(EBR),1,file);

                                    int posicionInicial=ebr.part_start;
                                    int posicionFinal= ebr.part_start + ebr.part_s;
                                    if ((posicionFinal - posicionInicial) / (1024 * 1024) >= 1){
                                        this->deleteMegaByte(this->p, posicionInicial, posicionFinal);
                                    }else if ((posicionFinal - posicionInicial) / (1024) >= 1){
                                        this->deleteKiloByte(this->p, posicionInicial, posicionFinal);
                                    }else{
                                        this->deleteByte(this->p, posicionInicial, posicionFinal);
                                    }

                                    fclose(file);
                                    cout<<"PARTICION LOGICA "<< this->name << " ELIMINADA CON EXITO."<< endl;
                                    return;
                                }
                                anteriorStart=ebr.part_start;
                                if (ebr.part_next == -1){
                                    break;
                                }
                                fseek(file, ebr.part_next, SEEK_SET);
                                fread(&ebr,sizeof (EBR),1,file);
                            }
                        }
                    }
                }
            }

            if (pos!=-1){ // si es una particion primaria lo que hay que funarse
                int posicionI=mbr.mbr_partition[pos].part_start;
                int poscionF= mbr.mbr_partition[pos].part_start + mbr.mbr_partition[pos].part_s;
                if ((poscionF-posicionI)/(1024*1024)>=1){
                    this->deleteMegaByte(this->p, posicionI, poscionF);
                }else if ((poscionF-posicionI)/(1024)>=1){
                    this->deleteKiloByte(this->p, posicionI, poscionF);
                }else{
                    this->deleteByte(this->p, posicionI, poscionF);
                }

                Partition p;
                p.part_start=-1;
                p.part_s=-1;
                p.part_fit='w';
                strcpy(p.part_name, "");
                mbr.mbr_partition[pos]=p; // actualizamos la particion en el listado de particiones del mbr
                fseek(file, 0, SEEK_SET);
                fwrite(&mbr, sizeof(MBR), 1, file);
                fclose(file);
                cout<<"PARTICION "<< this->name << " ELIMINADA CON EXITO."<< endl;
                return;
            }else{
                cout<<"EL NOMBRE DE LA PARTICION ESPECIFICADO NO EXISTE"<<endl;
            }
        }
        else{
            cout << "NOTIFICACION: SE CANCELO LA ELIMINACION DE LA PARTICION \n" << endl;
        }
        fclose(file);
    }else{
        cout<<"EL DISCO NO EXISTE EN LA RUTA ESPECIFICADA"<<endl;
    }
    cout<<endl;
}

void Particion::deleteMegaByte(string p, long posicionInicial, int posicionFinal) {
    FILE *file= fopen(p.c_str(),"rb+");
    fseek(file,posicionInicial,SEEK_SET);
    long resta= (posicionFinal - ftell(file)) / (1024 * 1024);
    if (resta >= 1){
        char buffer[1024];
        for (int i = 0; i < 1024; i++) {
            buffer[i] = '\0';
        }
        for (int j = 0; j < resta * 1024; ++j) {
            fwrite(&buffer,1024,1,file);
        }
    }
    deleteKiloByte(p, ftell(file), posicionFinal);
    fclose(file);
}

void Particion::deleteKiloByte(string p, long posicionInicial, int posicionFinal) {
    FILE *file= fopen(p.c_str(),"rb+");
    fseek(file, posicionInicial, SEEK_SET);
    long resta= (posicionFinal - ftell(file)) / (1024);
    if (resta >= 1){
        char buffer[1024];
        for (int i = 0; i < 1024; i++) {
            buffer[i] = '\0';
        }
        for (int j = 0; j < resta; ++j) {
            fwrite(&buffer,1024,1,file);
        }
    }
    deleteByte(p, ftell(file), posicionFinal);
    fclose(file);
}

void Particion::deleteByte(string p, long posicionInicial, int posicionFinal) {
    FILE *file= fopen(p.c_str(),"rb+");
    fseek(file, posicionInicial, SEEK_SET);
    long resta= posicionFinal - ftell(file);
    if (resta >= 1){
        char buffer='\0';
        for (int j = 0; j < resta; ++j) {
            fwrite(&buffer,1,1,file);
        }
    }
    fclose(file);
}

void Particion::reducePartition() {
    FILE *file= fopen(this->p.c_str(),"rb+");
    if (file) {
        // Calculo de la reducción
        int cantReduccion=0, pos = -1;

        if (this->u == 'b') {
            cantReduccion = this->add * -1;
        } else if (this->u == 'k') {
            cantReduccion = this->add * 1024 * -1;
        } else if (this->u == 'm') {
            cantReduccion = this->add * 1024 * 1024 * -1;
        }
        MBR mbr;
        fseek(file,0,SEEK_SET);
        fread(&mbr, sizeof(MBR),1,file);

        // Busqueda de la particion
        for (int i = 0; i < 4; ++i) {
            if (mbr.mbr_partition[i].part_name == this->name){
                pos=i;
                break;
            }else if (mbr.mbr_partition[i].part_type == 'e'){ // si es una extendida
                EBR ebr;
                fseek(file, mbr.mbr_partition[i].part_start, SEEK_SET);
                fread(&ebr,sizeof(EBR),1,file);
                if (!(ebr.part_s == -1 && ebr.part_next == -1)){
                    if (ebr.part_name == this->name){
                        int tamanioExtendida = ebr.part_s - sizeof(EBR);
                        if (tamanioExtendida > cantReduccion){ // Verifacacion al reducir
                            ebr.part_s= ebr.part_s - cantReduccion;
                            fseek(file, ebr.part_start, SEEK_SET);
                            fwrite(&ebr, sizeof(EBR),1,file);
                            fclose(file);
                            cout << "SE REDUJERON" << cantReduccion << " DE LA PARTICION " << this->name << endl;
                            cout << "EL TAMANO DE LA PARTICION AHORA ES DE " << ebr.part_s << endl;
                            return;
                        }else{
                            fclose(file);
                            cout<<"LA PARTICION INDICADA NO SE PUEDE REDUCIR LA CANTIDAD INDICADA"<<endl;
                            return;
                        }
                    }else if(ebr.part_next != -1){
                        fseek(file, ebr.part_next, SEEK_SET);
                        fread(&ebr,sizeof (EBR),1,file);
                        while (true){
                            if (ebr.part_name == this->name){
                                int tamanio= ebr.part_s - sizeof(EBR);
                                if (tamanio > cantReduccion){
                                    ebr.part_s= ebr.part_s - cantReduccion;
                                    fseek(file, ebr.part_start, SEEK_SET);
                                    fwrite(&ebr, sizeof(EBR),1,file);
                                    fclose(file);
                                    cout << "SE REDUJERON" << cantReduccion << " DE LA PARTICION " << this->name << endl;
                                    cout << "EL TAMANO DE LA PARTICION AHORA ES DE " << ebr.part_s << endl;
                                    return;
                                }else{
                                    fclose(file);
                                    cout<<"NO SE PUEDE REDUCIR LA PARTICION"<<endl;
                                    return;
                                }
                            }
                            if (ebr.part_next == -1){
                                break;
                            }
                            fseek(file, ebr.part_next, SEEK_SET);
                            fread(&ebr,sizeof (EBR),1,file);
                        }
                    }
                }
            }
        }

        if (pos!=-1){
            int size=mbr.mbr_partition[pos].part_s;
            if (size > cantReduccion){
                mbr.mbr_partition[pos].part_s= size - cantReduccion;
                fseek(file,0,SEEK_SET);
                fwrite(&mbr, sizeof(MBR),1,file);
                fclose(file);
                cout << "SE REDUJERON" << cantReduccion << " DE LA PARTICION " << this->name << endl;
                cout << "EL TAMANO DE LA PARTICION AHORA ES DE " << mbr.mbr_partition[pos].part_s << endl;
            }else{
                fclose(file);
                cout<<"LA PARTICION INDICADA NO SE PUEDE REDUCIR"<<endl;
                return;
            }
        }else{
            cout<<"LA PARTICION INDICADA NO EXISTE, VERIFICAR QUE EL NOMBRE ESTE BIEN ESCRITO"<<this->name<<endl;
        }
    }else{
        cout<<"NO EXISTE UN DISCO EN LA RUTA INDICADA"<<endl;
    }
}

void Particion::incrementPartition() {
    FILE *file= fopen(this->p.c_str(),"rb+");
    if (file) {
        int incremento=0, address = -1;

        if (this->u == 'b') {
            incremento = this->add;
        } else if (this->u == 'k') {
            incremento = this->add * 1024;
        } else if (this->u == 'm') {
            incremento = this->add * 1024 * 1024;
        }

        MBR mbr;
        fseek(file,0,SEEK_SET);
        fread(&mbr, sizeof(MBR),1,file);

        for (int i = 0; i < 4; ++i) { // Recorrer las particiones principales
            if (mbr.mbr_partition[i].part_name == this->name) { // si una de las principales es
                address = i;
                break;
            }else if(mbr.mbr_partition[i].part_type == 'e'){ // si una principal es extendida revisar sus lógicas
                EBR ebr;
                fseek(file, mbr.mbr_partition[i].part_start, SEEK_SET);
                fread(&ebr,sizeof(EBR),1,file);
                if (!(ebr.part_s == -1 && ebr.part_next == -1)){
                    if (ebr.part_name == this->name){ // si una logica coincide
                        if (ebr.part_next != -1){ // Si hay una particion lógica más adelante hay que verificar otras cosas
                            if ((ebr.part_start + ebr.part_s) >= ebr.part_next){
                                fclose(file);
                                cout<<"El AUMENTO INDICADO SOBREPASA EL LIMITE DE LA PARTICION"<<endl;
                                return;
                            }else if ((ebr.part_start + ebr.part_s + incremento) < ebr.part_next){
                                ebr.part_s+=incremento; // Se aumenta el SIZE de la particion
                                fseek(file, ebr.part_start, SEEK_SET);
                                fwrite(&ebr, sizeof(EBR),1,file);
                                fclose(file);
                                cout << "SE INCREMENTO EN " << incremento << " BYTES EL SIZE DE LA PARTICION " << this->name << endl;
                                return;
                            }else{
                                fclose(file);
                                cout<<"NO EXISTE EL ESPACIO NECESARIO EN LA PARTICION EXTENDIDA PARA INCREMENTAR LA PARTICION LOGICA"<<endl;
                                return;
                            }
                        }else{ // si ya era la última lógica hay que verificar que no se pase de la particion extendida en donde esta
                            if ((ebr.part_start + ebr.part_s + incremento) <= (mbr.mbr_partition[i].part_start + mbr.mbr_partition[i].part_s)){
                                ebr.part_s+=incremento;
                                fseek(file, ebr.part_start, SEEK_SET);
                                fwrite(&ebr, sizeof(EBR),1,file);
                                fclose(file);
                                cout << "SE INCREMENTO EN " << incremento << " BYTES EL SIZE DE LA PARTICION " << this->name << endl;
                                return;
                            }else{
                                fclose(file);
                                cout<<"NO EXISTE EL ESPACIO NECESARIO EN LA PARTICION EXTENDIDA PARA INCREMENTAR LA PARTICION LOGICA"<<endl;
                                return;
                            }
                        }
                    }else if(ebr.part_next != -1){
                        fseek(file, ebr.part_next, SEEK_SET);
                        fread(&ebr,sizeof (EBR),1,file);
                        while (true){
                            if (ebr.part_name == this->name){
                                if (ebr.part_next != -1){
                                    if ((ebr.part_start + ebr.part_s) >= ebr.part_next){
                                        fclose(file);
                                        cout<<"NO HAY ESPACIO SUFICIENTE PARA AUMENTAR LA PARTICION"<<endl;
                                        return;
                                    }else if ((ebr.part_start + ebr.part_s + incremento) < ebr.part_next){
                                        ebr.part_s+=incremento;
                                        fseek(file, ebr.part_start, SEEK_SET);
                                        fwrite(&ebr, sizeof(EBR),1,file);
                                        fclose(file);
                                        cout << "SE AUMENTO EN " << incremento << " BYTES EL SIZE DE LA PARTCION " << this->name << endl;
                                        return;
                                    }else{
                                        fclose(file);
                                        cout<<"NO EXISTE EL ESPACIO NECESARIO EN LA PARTICION EXTENDIDA PARA INCREMENTAR LA PARTICION LOGICA"<<endl;
                                        return;
                                    }
                                }else{
                                    if ((ebr.part_start + ebr.part_s + incremento) <= (mbr.mbr_partition[i].part_start + mbr.mbr_partition[i].part_s)){
                                        ebr.part_s+=incremento;
                                        fseek(file, ebr.part_start, SEEK_SET);
                                        fwrite(&ebr, sizeof(EBR),1,file);
                                        fclose(file);
                                        cout << "SE AUMENTO EN " << incremento << " BYTES EL SIZE DE LA PARTCION " << this->name << endl;
                                        return;
                                    }else{
                                        fclose(file);
                                        cout<<"NO EXISTE EL ESPACIO NECESARIO EN LA PARTICION EXTENDIDA PARA INCREMENTAR LA PARTICION LOGICA"<<endl;
                                        return;
                                    }
                                }
                            }
                            if (ebr.part_next == -1){
                                break;
                            }
                            fseek(file, ebr.part_next, SEEK_SET);
                            fread(&ebr,sizeof (EBR),1,file);
                        }
                    }
                }
            }
        }

        if (address != -1){
            if (address == 3){
                if ((mbr.mbr_partition[address].part_start + mbr.mbr_partition[address].part_s + incremento) <= mbr.mbr_tamanio){
                    mbr.mbr_partition[address].part_s+=incremento;
                    fseek(file,0,SEEK_SET);
                    fwrite(&mbr, sizeof(MBR),1,file);
                    fclose(file);
                    cout << "SE AUMENTO EN " << incremento << " BYTES EL SIZE DE LA PARTCION " << this->name << endl;
                    return;
                }else{
                    fclose(file);
                    cout<<"NO EXISTE EL ESPACIO NECESARIO EN LA PARTICION PARA AUMENTARLA"<<endl;
                    return;
                }
            }else{
               //Si no existe la siguiente particion
               if (mbr.mbr_partition[address + 1].part_start == -1){
                    if ((mbr.mbr_partition[address].part_start + mbr.mbr_partition[address].part_s + incremento) <= mbr.mbr_tamanio){
                        mbr.mbr_partition[address].part_s+=incremento;
                        fseek(file,0,SEEK_SET);
                        fwrite(&mbr,sizeof(MBR),1,file);
                        fclose(file);
                        cout << "SE AUMENTO EN " << incremento << " BYTES EL SIZE DE LA PARTCION " << this->name << endl;
                        return;
                    }else{
                        fclose(file);
                        cout<<"NO HAY ESPACIO SUFICIENTE PARA AUMENTAR LA PARTICION"<<endl;
                        return;
                    }
                }else{
                    if ((mbr.mbr_partition[address].part_start + mbr.mbr_partition[address].part_s) >= mbr.mbr_partition[address + 1].part_start){
                        fclose(file);
                        cout<<"NO HAY ESPACIO SUFICIENTE PARA AUMENTAR LA PARTICION"<<endl;
                        return;
                    }else if ((mbr.mbr_partition[address].part_start + mbr.mbr_partition[address].part_s + incremento) < mbr.mbr_partition[address + 1].part_start){
                        mbr.mbr_partition[address].part_s+=incremento;
                        fseek(file,0,SEEK_SET);
                        fwrite(&mbr,sizeof(MBR),1,file);
                        fclose(file);
                        cout << "SE AUMENTO EN " << incremento << " BYTES EL SIZE DE LA PARTCION " << this->name << endl;
                        return;
                    }else{
                        fclose(file);
                        cout<<"NO HAY ESPACIO SUFICIENTE PARA AUMENTAR LA PARTICION"<<endl;
                        return;
                    }
                }
            }
        }else{
            cout<<"NO EXISTE LA PARTICION "<<this->name<<endl;
        }
    }else{
        cout<<"NO EXISTE UN DISCO EN LA RUTA INDICADA"<<endl;
    }
}