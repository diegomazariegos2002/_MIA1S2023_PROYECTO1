#include "Montar.h"
#include "Structs.h"
#include <string>
#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <tgmath.h>

Montar::Montar() {
    this->p=" ";
    this->name=" ";
    this->id=" ";
    this->type="full";
    this->fs="2fs";
}

void Montar::mount() {
    if (this->p!=" "){
        if (this->name!=" "){
            int indiceParticion=-1;
            FILE *file= fopen(this->p.c_str(),"rb+");
            if (file){
                MBR mbr;
                fseek(file,0,SEEK_SET);
                fread(&mbr, sizeof(MBR),1,file);

                for (int i = 0; i < 4; ++i) {
                    if (mbr.mbr_partition[i].part_name==this->name){
                        indiceParticion=i;
                        break;
                    }else if (mbr.mbr_partition[i].part_type=='e'){ // ENTRANDO A BUSCAR SI ES LÓGICA
                        EBR ebr;
                        SuperBloque superBloque;
                        fseek(file,mbr.mbr_partition[i].part_start,SEEK_SET);
                        fread(&ebr,sizeof(EBR),1,file);
                        if (!(ebr.part_s==-1 && ebr.part_next==-1)){
                            if (ebr.part_name==this->name){
                                if (ebr.part_status=='0' || ebr.part_status=='1'){
                                    ebr.part_status='1';
                                }
                                this->mountList->add(this->p,this->name,'l',ebr.part_start,-1);

                                fseek(file, ebr.part_start, SEEK_SET);
                                fwrite(&ebr, sizeof(EBR), 1, file);

                                if (ebr.part_status=='2'){
                                    fseek(file,ebr.part_start+sizeof(EBR),SEEK_SET);
                                    fread(&superBloque, sizeof(SuperBloque), 1, file);
                                    superBloque.s_mtime= time(nullptr);
                                    superBloque.s_mnt_count++;
                                    fseek(file,ebr.part_start+sizeof(EBR),SEEK_SET);
                                    fwrite(&superBloque, sizeof(SuperBloque), 1, file);
                                }

                                fclose(file);
                                return;
                            }else if(ebr.part_next!=-1){
                                fseek(file,ebr.part_next,SEEK_SET);
                                fread(&ebr,sizeof (EBR),1,file);
                                while (true){
                                    if (ebr.part_name==this->name){
                                        if (ebr.part_status=='0' || ebr.part_status=='1'){
                                            ebr.part_status='1';
                                        }
                                        this->mountList->add(this->p,this->name,'l',ebr.part_start,-1);

                                        fseek(file, ebr.part_start, SEEK_SET);
                                        fwrite(&ebr, sizeof(EBR), 1, file);

                                        if (ebr.part_status=='2'){
                                            fseek(file,ebr.part_start+sizeof(EBR),SEEK_SET);
                                            fread(&superBloque, sizeof(SuperBloque), 1, file);
                                            superBloque.s_mtime= time(nullptr);
                                            superBloque.s_mnt_count++;
                                            fseek(file,ebr.part_start+sizeof(EBR),SEEK_SET);
                                            fwrite(&superBloque, sizeof(SuperBloque), 1, file);
                                        }

                                        fclose(file);
                                        return;
                                    }
                                    if (ebr.part_next==-1){
                                        break;
                                    }
                                    fseek(file,ebr.part_next,SEEK_SET);
                                    fread(&ebr,sizeof (EBR),1,file);
                                }
                            }
                        }
                    }
                }
                if (indiceParticion != -1){
                    if (mbr.mbr_partition[indiceParticion].part_type == 'e'){
                        cout << "EN UN PARTICION EXTENDIDA NO SE PUEDE MONTAR " << endl;
                        fclose(file);
                        return;
                    }else{ // Se encontro que fue en un partición primaria
                        SuperBloque superBloque;
                        if (mbr.mbr_partition[indiceParticion].part_status == '0' || mbr.mbr_partition[indiceParticion].part_status == '1'){
                            mbr.mbr_partition[indiceParticion].part_status='1';
                        }
                        this->mountList->add(this->p, this->name, 'p', mbr.mbr_partition[indiceParticion].part_start, indiceParticion);

                        fseek(file, 0, SEEK_SET);
                        fwrite(&mbr, sizeof(MBR), 1, file);
                        if (mbr.mbr_partition[indiceParticion].part_status == '2'){
                            fseek(file, mbr.mbr_partition[indiceParticion].part_start, SEEK_SET);
                            fread(&superBloque, sizeof(SuperBloque), 1, file);
                            superBloque.s_mtime= time(nullptr);
                            superBloque.s_mnt_count++;
                            fseek(file, mbr.mbr_partition[indiceParticion].part_start, SEEK_SET);
                            fwrite(&superBloque, sizeof(SuperBloque), 1, file);
                        }
                        fclose(file);
                        return;
                    }
                }else{
                    cout<<"NO SE ENCONTRO UNA PARTICION CON ESE NOMBRE"<<endl;
                    fclose(file);
                    return;
                }
            }else{
                cout<<"NO EXISTE EL DISCO EN LA RUTA ESPECIFICADA"<<endl;
            }
        }else{
            cout<<"ERROR EL PARAMETRO NAME ES OBLIGATORIO"<<endl;
        }
    }else{
        cout<<"EL PARAMETRO RUTA ES OBLIGATORIO"<<endl;
    }
}

void Montar::unmount() {
    if (this->id!=" "){
        Nodo_M *nodo=this->mountList->buscar(this->id);
        if (nodo!=NULL){
            FILE *file;
            if (file= fopen(nodo->path.c_str(),"rb+")){
                int indiceParticion=-1;
                MBR mbr;
                fseek(file,0,SEEK_SET);
                fread(&mbr,sizeof(MBR),1,file);
                //Recorrer las particiones en busca de cual es la que se queire desmontar
                for (int i = 0; i < 4; ++i) {
                    if (mbr.mbr_partition[i].part_name==nodo->name){
                        indiceParticion=i;
                        break;
                    }else if (mbr.mbr_partition[i].part_type=='e' && nodo->type=='l'){
                        // si se logra eleminar la montura.
                        if (this->mountList->eliminar(this->id)){
                            EBR ebr;
                            SuperBloque  sb;
                            fseek(file,nodo->start,SEEK_SET);
                            fread(&ebr, sizeof(EBR),1,file);
                            if (ebr.part_status=='2'){
                                fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
                                fread(&sb, sizeof(SuperBloque),1,file);
                                sb.s_umtime= time(nullptr);
                                fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
                                fwrite(&sb, sizeof(SuperBloque),1,file);
                            }
                            cout<<"SE LOGRO DESMONTAR LA PARTICION "<<nodo->name<<endl;
                            fclose(file);
                            return;
                        }else{
                            fclose(file);
                            return;
                        }
                    }
                }

                if (indiceParticion != -1 && nodo->type == 'p'){
                    if (this->mountList->eliminar(this->id)){
                        SuperBloque  sb;
                        if (mbr.mbr_partition[indiceParticion].part_status == '2'){
                            fseek(file,nodo->start,SEEK_SET);
                            fread(&sb, sizeof(SuperBloque),1,file);
                            sb.s_umtime= time(nullptr);
                            fseek(file,nodo->start,SEEK_SET);
                            fwrite(&sb, sizeof(SuperBloque),1,file);
                        }
                        cout<<"SE LOGRO DESMONTAR LA PARTICION "<<nodo->name<<endl;
                        fclose(file);
                        return;

                    }else{
                        fclose(file);
                        return;
                    }
                }else{
                    cout<<"LA PARTICION A LA QUE HACE REFERENCIA "<<this->id<<" YA NO EXISTE EN EL DISCO"<<endl;
                }
            }else{
                cout<<"EL DISCO DURO NO SE HA PODIDO ENCONTRAR"<<endl;
            }
        }else{
            cout<<"LA PARTICION "<< this->id<<" NO EXISTE EN EL DISCO" <<endl;
        }
    }else{
        cout<<"EL PARAMETRO DE ID ES OBLIGATORIO AL USAR ESTE COMANDO"<<endl;
    }
}
// Recordar que este comando tiene que crear un archivo users.txt
// en root.
void Montar::mkfs() {
    //primero verificar los parametros, como siempre
    if(this->id != " "){
        if(this->type == "full"){
            if(this->fs =="3fs" || this->fs == "2fs"){
                Nodo_M *nodoM=this->mountList->buscar(this->id);
                if (nodoM==NULL){
                    cout<<"NO SE ENCONTRO NINGUNA PARTICION MONTADA CON EL ID "<<this->id<<endl;
                    return;
                }
                // si existe el disco
                FILE *file;
                if (file= fopen(nodoM->path.c_str(),"rb+")){
                    TablaInodo inodo;
                    Journal journal;
                    BloqueCarpeta carpeta;
                    SuperBloque sb;
                    if (nodoM->type=='p'){
                        // ahora faltaría encontrar la particion montada en el mbr.
                        MBR mbr;
                        fseek(file,0,SEEK_SET);
                        fread(&mbr, sizeof(MBR),1,file);
                        int indiceParticion=-1;
                        for (int i = 0; i < 4; ++i) {
                            string name1=mbr.mbr_partition[i].part_name;
                            if (name1==nodoM->name){
                                indiceParticion=i;
                                break;
                            }
                        }
                        int tamanio=mbr.mbr_partition[indiceParticion].part_s;
                        double n=0;
                        if(this->fs=="3fs"){
                            n=(tamanio- sizeof(SuperBloque))/(4+ sizeof(Journal) +sizeof(TablaInodo)+(3* sizeof(BloqueArchivo)));
                            sb.s_filesystem_type=3;
                        }else{ // por default 2fs
                            n=(tamanio- sizeof(SuperBloque))/(4+ sizeof(TablaInodo)+(3* sizeof(BloqueArchivo)));
                            sb.s_filesystem_type=2;
                        }

                        int numeroEstructuras=floor(n);
                        int nBloques=3*numeroEstructuras;
                        int inoding=numeroEstructuras* sizeof(TablaInodo);




















                    }else if (nodoM->type=='l'){

                    }
                }else{
                    cout << "EL DISCO YA NO EXISTE EN LA RUTA QUE TENIA LA PARTICION" << endl;
                }
            }else{
                cout << "INGRESE UN SISTEMA DE ARCHIVOS VALIDO" << endl;
            }
        }else{
            cout << "SOLO EL TIPO FULL ES VALIDO" << endl;
        }
    }else{
        cout << "EL PARAMETRO ID ES OBLIGATORIO"<<endl;
    }

}