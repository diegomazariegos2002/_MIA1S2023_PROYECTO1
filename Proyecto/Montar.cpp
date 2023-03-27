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
                        // Corrección al parecer si se montan extendidas.
                        this->mountList->add(this->p, this->name, 'e', mbr.mbr_partition[indiceParticion].part_start, indiceParticion);
                        cout << "PARTICION EXTENDIDA MONTADA" << endl;
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
                        }
                        else{
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
                        cout<<"SE LOGRO DESMONTAR LA PARTICION PRIMARIA "<<nodo->name<<endl;
                        fclose(file);
                        return;

                    }else{
                        fclose(file);
                        return;
                    }
                }
                // correcicón para desmontar extendida
                else if(indiceParticion != -1 && nodo->type == 'e'){
                    this->mountList->eliminar(this->id);
                    cout<<"SE LOGRO DESMONTAR LA PARTICION EXTENDIDA "<<nodo->name<<endl;
                }
                else{
                    cout<<"LA MONTURA CON ID "<<this->id<<" HACE REFERENCIA A UNA PARTICION QUE YA NO EXISTE"<<endl;
                }
            }
            else{
                cout<<"NO SE PUDO ENCONTRAR EL DISCO"<<endl;
            }
        }else{
            cout<<"LA MONTURA "<< this->id<<" NO EXISTE EN EL DISCO" <<endl;
        }
    }else{
        cout<<"IMPOSIBLE EJECUTAR EL PARAMETRO DE ID ES OBLIGATORIO AL USAR ESTE COMANDO"<<endl;
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
                    //Todas las estructuras que vamos a formatear en el disco
                    TablaInodo tablaInodo;
                    Journal journal;
                    BloqueCarpeta bloqueCarpeta;
                    SuperBloque superBloque;
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
                        int size=mbr.mbr_partition[indiceParticion].part_s;
                        double n=0;
                        // formulas despejadas para encontrar n
                        if(this->fs=="3fs"){
                            n= (size - sizeof(SuperBloque)) / (4 + sizeof(Journal) + sizeof(TablaInodo) + (3 * sizeof(BloqueArchivo)));
                            superBloque.s_filesystem_type=3;
                        }else{ // por default 2fs
                            n= (size - sizeof(SuperBloque)) / (4 + sizeof(TablaInodo) + (3 * sizeof(BloqueArchivo)));
                            superBloque.s_filesystem_type=2;
                        }

                        int numInodos=floor(n);
                        int numBlocks= 3 * numInodos; // recordar que el número de bloques es el triple de inodos.
                        int espacioOcupado_Inodos = numInodos * sizeof(TablaInodo);

                        superBloque.s_inodes_count=numInodos;
                        superBloque.s_blocks_count=numBlocks;
                        superBloque.s_free_blocks_count= numBlocks - 2; //-2 debido a root y users
                        superBloque.s_free_inodes_count= numInodos - 2; //-2 debido a root y users
                        superBloque.s_mtime= time(nullptr);
                        superBloque.s_umtime=0;
                        superBloque.s_mnt_count=1; // recordar cuantos mounts han habido
                        superBloque.s_magic=0xEF53;
                        superBloque.s_inode_s= sizeof(TablaInodo);
                        superBloque.s_block_s= sizeof(BloqueArchivo);
                        superBloque.s_firts_ino=1; // Inodo root
                        superBloque.s_first_blo=2; // solo usamos el bloque carpeta y el de users.txt
                        if (this->fs=="2fs"){
                            superBloque.s_bm_inode_start = nodoM->start + sizeof(SuperBloque);
                        }else{
                            int journaling = numInodos * sizeof(Journal);
                            superBloque.s_bm_inode_start = nodoM->start + sizeof(SuperBloque) + journaling;
                        }
                        superBloque.s_bm_block_start= superBloque.s_bm_inode_start + numInodos;
                        superBloque.s_inode_start= superBloque.s_bm_block_start + numBlocks;
                        superBloque.s_block_start= superBloque.s_inode_start + espacioOcupado_Inodos;

                        //Declarando primer Inodo para la raíz.
                        tablaInodo.i_uid=1;
                        tablaInodo.i_gid=1;
                        tablaInodo.i_s=0;
                        tablaInodo.i_atime = time(nullptr);
                        tablaInodo.i_ctime = time(nullptr);
                        tablaInodo.i_mtime = time(nullptr);
                        tablaInodo.i_block[0]=superBloque.s_block_start;
                        for (int i = 1; i < 15; i++) {
                            tablaInodo.i_block[i] = -1;
                        }
                        tablaInodo.i_type = '0'; // es de tipo carpeta por ser la carpeta madre
                        tablaInodo.i_perm=664;

                        //Escribir el primer inodo en la lista de Tablas de Inodos
                        fseek(file, superBloque.s_inode_start, SEEK_SET);
                        fwrite(&tablaInodo, sizeof(TablaInodo), 1, file);

                        //Creando primer Bloque carpeta
                        strcpy(bloqueCarpeta.b_content[0].b_name, ".");
                        bloqueCarpeta.b_content[0].b_inodo=superBloque.s_inode_start;
                        strcpy(bloqueCarpeta.b_content[1].b_name, "..");
                        bloqueCarpeta.b_content[1].b_inodo = superBloque.s_inode_start;
                        strcpy(bloqueCarpeta.b_content[2].b_name, "users.txt");
                        bloqueCarpeta.b_content[2].b_inodo = superBloque.s_inode_start + sizeof(TablaInodo); // Segundo Inodo utilizado para el archivo Users.txt
                        strcpy(bloqueCarpeta.b_content[3].b_name, "");
                        bloqueCarpeta.b_content[3].b_inodo = -1;
                        //Escribiendo el bloque
                        fseek(file, superBloque.s_block_start, SEEK_SET); // Primer bloque utilizado, carpeta root
                        fwrite(&bloqueCarpeta, sizeof(BloqueCarpeta), 1, file);

                        //Creando el archivo para administracion de usuarios
                        TablaInodo inodoUsuarios; // su Inodo respectivo
                        BloqueArchivo archivoUsuarios; // su bloque respectivo

                        inodoUsuarios.i_uid=1;
                        inodoUsuarios.i_gid=1;
                        inodoUsuarios.i_atime = time(nullptr);
                        inodoUsuarios.i_ctime = time(nullptr);
                        inodoUsuarios.i_mtime = time(nullptr);
                        inodoUsuarios.i_perm=664; // Lectura y escritura (Owner y grupo), lectura (otros)
                        inodoUsuarios.i_block[0]= superBloque.s_block_start + sizeof(BloqueCarpeta); // apunta al segundo Bloque (Bloque archivo Users.txt)
                        for (int i = 1; i < 15; i++) {
                            inodoUsuarios.i_block[i] = -1;
                        }
                        string s="1,G,root\n1,U,root,root,123\n";
                        inodoUsuarios.i_s= sizeof(s);
                        inodoUsuarios.i_type = '1'; // tipo archivo

                        // Segundo Inodo del archivo users.txt, segundo inodo
                        fseek(file, superBloque.s_inode_start + sizeof(TablaInodo), SEEK_SET);
                        fwrite(&inodoUsuarios, sizeof(TablaInodo), 1, file);

                        //utilizando memset para inicializar el bloque que almacenara el .txt
                        memset(archivoUsuarios.b_content, '\0', sizeof(archivoUsuarios.b_content));
                        strcpy(archivoUsuarios.b_content, "1,G,root\n1,U,root,root,123\n");
                        //escribiendo el bloque de archivos (segundo bloque)
                        fseek(file, superBloque.s_block_start + sizeof(BloqueCarpeta), SEEK_SET);
                        fwrite(&archivoUsuarios, sizeof(BloqueArchivo), 1, file);

                        // Cada acción en el 3fs se registra
                        if (this->fs=="3fs"){
                            journal.journal_Sig=-1;
                            journal.journal_Tipo='0';
                            journal.journal_Size=0;
                            journal.journal_Fecha= time(nullptr);
                            strcpy(journal.journal_Tipo_Operacion,"mkfs");
                            journal.journal_Start=mbr.mbr_partition[indiceParticion].part_start + sizeof(SuperBloque);
                        }

                        // Actualización de datos
                        mbr.mbr_partition[indiceParticion].part_status='2';
                        fseek(file, 0, SEEK_SET);
                        fwrite(&mbr, sizeof(MBR), 1, file);
                        fseek(file, mbr.mbr_partition[indiceParticion].part_start, SEEK_SET);
                        fwrite(&superBloque, sizeof(SuperBloque), 1, file);

                        // Escribiendo en el bitmap de Inodos.
                        char ch0 = '0';
                        char ch1 = '1';
                        for (int i = 0; i < numInodos; i++) {
                            fseek(file, superBloque.s_bm_inode_start + i, SEEK_SET);
                            fwrite(&ch0, sizeof(char), 1, file);
                        }
                        fseek(file, superBloque.s_bm_inode_start, SEEK_SET);
                        fwrite(&ch1, sizeof(char), 1, file);
                        fwrite(&ch1, sizeof(char), 1, file);

                        // Escribiendo en el BitMap de Bloques
                        for (int i = 0; i < numBlocks; i++) {
                            fseek(file, superBloque.s_bm_block_start + i, SEEK_SET);
                            fwrite(&ch0, sizeof(char), 1, file);
                        }
                        fseek(file, superBloque.s_bm_block_start, SEEK_SET);
                        fwrite(&ch1, sizeof(char), 1, file);
                        fwrite(&ch1, sizeof(char), 1, file);

                        //Actualización de Journal en el Disco
                        if (this->fs=="3fs"){
                            fseek(file, mbr.mbr_partition[indiceParticion].part_start + sizeof(SuperBloque), SEEK_SET);
                            fwrite(&journal, sizeof(Journal), 1, file);
                        }
                        fclose(file);
                        cout<<"PARTICION FORMATEADA CON EXITO"<<endl;

                    }
                    else if (nodoM->type=='l'){
                        // Lo mismo que con la partición primara solo que aquí no es con el MBR sino con el EBR
                        EBR ebr;
                        fseek(file,nodoM->start,SEEK_SET);
                        fread(&ebr, sizeof(EBR),1,file);

                        int tamanio=ebr.part_s;
                        double n=0;
                        if (this->fs=="2fs"){
                            n=(tamanio- sizeof(SuperBloque))/(4+ sizeof(TablaInodo)+(3* sizeof(BloqueArchivo)));
                            superBloque.s_filesystem_type=2;
                        }else{
                            n=(tamanio- sizeof(SuperBloque))/(4+ sizeof(Journal) +sizeof(TablaInodo)+(3* sizeof(BloqueArchivo)));
                            superBloque.s_filesystem_type=3;
                        }
                        int numeroEstructuras=floor(n);
                        int nBloques=3*numeroEstructuras;
                        int inoding=numeroEstructuras* sizeof(TablaInodo);

                        superBloque.s_inodes_count=numeroEstructuras;
                        superBloque.s_blocks_count=nBloques;
                        superBloque.s_free_blocks_count= nBloques - 2;
                        superBloque.s_free_inodes_count= numeroEstructuras - 2;
                        superBloque.s_mtime= time(nullptr);
                        superBloque.s_umtime=0;
                        superBloque.s_mnt_count=1;
                        superBloque.s_magic=0xEF53;
                        superBloque.s_inode_s= sizeof(TablaInodo);
                        superBloque.s_block_s= sizeof(BloqueArchivo);
                        superBloque.s_firts_ino=1;
                        superBloque.s_first_blo=1;
                        if (this->fs=="2fs"){
                            superBloque.s_bm_inode_start = nodoM->start + sizeof(EBR) + sizeof(SuperBloque);
                        }else{
                            int journaling = numeroEstructuras * sizeof(Journal);
                            superBloque.s_bm_inode_start = nodoM->start + sizeof(EBR) + sizeof(SuperBloque) + journaling;
                        }
                        superBloque.s_bm_block_start= superBloque.s_bm_inode_start + numeroEstructuras;
                        superBloque.s_inode_start= superBloque.s_bm_block_start + nBloques;
                        superBloque.s_block_start= superBloque.s_inode_start + inoding;

                        tablaInodo.i_uid=1;
                        tablaInodo.i_gid=1;
                        tablaInodo.i_atime = time(nullptr);
                        tablaInodo.i_ctime = time(nullptr);
                        tablaInodo.i_mtime = time(nullptr);
                        tablaInodo.i_perm=664;
                        tablaInodo.i_block[0]=superBloque.s_block_start;
                        for (int i = 1; i < 15; i++) {
                            tablaInodo.i_block[i] = -1;
                        }
                        tablaInodo.i_type = '0';
                        fseek(file, superBloque.s_inode_start, SEEK_SET);
                        fwrite(&tablaInodo, sizeof(TablaInodo), 1, file);

                        strcpy(bloqueCarpeta.b_content[0].b_name, ".");
                        bloqueCarpeta.b_content[0].b_inodo=superBloque.s_inode_start;
                        strcpy(bloqueCarpeta.b_content[1].b_name, "..");
                        bloqueCarpeta.b_content[1].b_inodo = superBloque.s_inode_start;
                        strcpy(bloqueCarpeta.b_content[2].b_name, "users.txt");
                        bloqueCarpeta.b_content[2].b_inodo = superBloque.s_inode_start + sizeof(TablaInodo);
                        strcpy(bloqueCarpeta.b_content[3].b_name, "");
                        bloqueCarpeta.b_content[3].b_inodo = -1;
                        fseek(file, superBloque.s_block_start, SEEK_SET);
                        fwrite(&bloqueCarpeta, sizeof(BloqueCarpeta), 1, file);

                        //Se crea users.txt
                        TablaInodo inodoU;
                        BloqueArchivo archivoU;

                        inodoU.i_uid=1;
                        inodoU.i_gid=1;
                        inodoU.i_atime = time(nullptr);
                        inodoU.i_ctime = time(nullptr);
                        inodoU.i_mtime = time(nullptr);
                        inodoU.i_perm=700;
                        inodoU.i_block[0]= superBloque.s_block_start + sizeof(BloqueCarpeta);
                        for (int i = 1; i < 15; i++) {
                            inodoU.i_block[i] = -1;
                        }
                        inodoU.i_type = '1';
                        fseek(file, superBloque.s_inode_start + sizeof(TablaInodo), SEEK_SET);
                        fwrite(&inodoU, sizeof(TablaInodo), 1, file);

                        memset(archivoU.b_content, '\0', sizeof(archivoU.b_content));
                        strcpy(archivoU.b_content, "1,G,root,\n1,U,root,root,123,\n");
                        fseek(file, superBloque.s_block_start + sizeof(BloqueCarpeta), SEEK_SET);
                        fwrite(&archivoU, sizeof(BloqueArchivo), 1, file);

                        //Se termina el proceso
                        if (this->fs=="3fs"){
                            journal.journal_Sig=-1;
                            journal.journal_Tipo='0';
                            journal.journal_Size=0;
                            journal.journal_Fecha= time(nullptr);
                            strcpy(journal.journal_Tipo_Operacion,"mkfs");
                            journal.journal_Start=ebr.part_start + sizeof(SuperBloque) + sizeof(EBR);
                        }

                        ebr.part_status='2';
                        fseek(file, nodoM->start, SEEK_SET);
                        fwrite(&ebr, sizeof(EBR), 1, file);
                        fseek(file, nodoM->start+ sizeof(EBR), SEEK_SET);
                        fwrite(&superBloque, sizeof(SuperBloque), 1, file);

                        char ch0 = '0';
                        char ch1 = '1';
                        for (int i = 0; i < numeroEstructuras; i++) {
                            fseek(file, superBloque.s_bm_inode_start + i, SEEK_SET);
                            fwrite(&ch0, sizeof(char), 1, file);
                        }
                        fseek(file, superBloque.s_bm_inode_start, SEEK_SET);
                        fwrite(&ch1, sizeof(char), 1, file);
                        fwrite(&ch1, sizeof(char), 1, file);

                        for (int i = 0; i < nBloques; i++) {
                            fseek(file, superBloque.s_bm_block_start + i, SEEK_SET);
                            fwrite(&ch0, sizeof(char), 1, file);
                        }
                        fseek(file, superBloque.s_bm_block_start, SEEK_SET);
                        fwrite(&ch1, sizeof(char), 1, file);
                        fwrite(&ch1, sizeof(char), 1, file);

                        if (this->fs=="3fs"){
                            fseek(file, ebr.part_start+ sizeof(EBR) + sizeof(SuperBloque), SEEK_SET);
                            fwrite(&journal, sizeof(Journal), 1, file);
                        }
                        fclose(file);
                        cout<<"PARTICION FORMATEADA CON EXITO"<<endl;

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