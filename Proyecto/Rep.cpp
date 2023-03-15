#include "Rep.h"
#include "Structs.h"
#include "Nodo_M.h"
#include <string>
#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <sstream>
#include <filesystem>

Rep::Rep() {
    this->contadorBloques_ReporteBloques=0;
    this->mountList=new MountList();
    this->name=" ";
    this->path=" ";
    this->id=" ";
    this->directorio=" ";
    this->extension=" ";
    this->ruta=" ";
}

void Rep::generate() {
    if (this->id!=" "){
        if (this->path!=" "){
            if (this->name=="mbr"){
                this->ejecutarReporte_mbr();
            }
            else if (this->name=="disk"){
                this->ejecutarReporte_disk();
            }
            else if (this->name=="inode"){
                this->ejecutarReporte_inode();
            }
            else if (this->name=="block"){
                this->ejecutarReporte_block();
            }
            else if (this->name=="bm_inode"){
                this->ejecutarReporte_bm_inode();
            }
            else if (this->name=="bm_block"){
                this->ejecutarReporte_bm_block();
            }
            else if (this->name=="sb"){
                this->ejecutarReporte_sb();
            }
            else if (this->name=="journaling"){
                this->ejecutarReporte_Journaling();
            }
            else if (this->name=="file"){
                if (this->ruta!=" "){
                    this->ejecutarReporte_file();
                }
                else{
                    cout << "EL PARAMETRO RUTA PARA EL REPORTE FILE ES OBLIGATORIO" << endl;
                }
            }
            else if (this->name=="ls"){
                if (this->ruta!=" "){
                    this->ejecutarReporte_ls();
                }
                else{
                    cout << "EL PARAMETRO RUTA PARA EL REPORTE LS ES OBLIGATORIO" << endl;
                }
            }
            else if (this->name=="tree"){
                this->ejecutarReporte_tree();
            }
            else{
                cout << "EL NOMBRE ASIGNADO PARA EL REPORTE ES INVALIDO"<< endl;
            }
        }else{
            cout << "EL PARAMETRO DE LA UBICACION DEL REPORTE ES OBLIGATORIO"<< endl;
        }
    }else{
        cout << "EL ID DE LA PARTICION ES OBLIGATORIO"<< endl;

    }

}

void Rep::ejecutarReporte_mbr() {
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL){
        FILE *file;

        this->directorio= this->getCarpetas(this->path);
        this->extension= this->getExtensionFile(this->path);
        system(("sudo -S mkdir -p \'"+this->directorio+"\'").c_str());
        system(("sudo -S chmod -R 777 \'"+this->directorio+"\'").c_str());


        if ((file= fopen(nodo->path.c_str(),"rb+"))){
            MBR mbr;
            fseek(file,0,SEEK_SET);
            fread(&mbr,sizeof (MBR),1,file);
            FILE *fileDot= fopen("mbr.dot", "w");
            string cuerpoDot = "";
            char fecha[70];
            strftime(fecha,sizeof (fecha),"%Y-%m-%d %H:%M:%S",localtime(&mbr.mbr_fecha_creacion));
            string date = fecha;
            cuerpoDot += 
                "digraph G {"
                "\n"
                "node[shape=none]"
                "\n"
                "start[label=<<table CELLSPACING=\"0\">"
                "\n"
                R"(<tr><td colspan="2" bgcolor="darkorchid4"><font color="white">REPORTE DE MBR</font></td></tr>)"
                "\n"
                R"(<tr>)"
                "\n"
                R"(<td border="0">mbr_tamano</td>)"
                "\n"
                R"(<td border="0">)" + to_string(mbr.mbr_tamano) + "</td>"
                "\n"
                R"(</tr>)"
                "\n"    
                R"(<tr>)"
                                                        "\n"
                R"(<td bgcolor="plum2">mbr_fecha_creacion</td>)"
                                                        "\n"
                R"(<td bgcolor="plum2">)" + date + "</td>"
                                                     "\n"
                R"(</tr>)"
                                                     "\n"
                R"(<tr>)"
                                                     "\n"
                R"(<td border="0">mbr_disk_signature</td>)"
                                                     "\n"
                R"(<td border="0">)" + to_string(mbr.mbr_dsk_signature) + "</td>"
                                                               "\n"
                R"(</tr>)"
                                                               "\n";
                
                for (int i = 0; i < 4; ++i) {
                    if (mbr.mbr_partition[i].part_start!=-1){
                    cuerpoDot +=R"(<tr><td colspan="2" bgcolor="darkorchid4"><font color="white">Particion</font></td></tr>)";
                    cuerpoDot +=R"(<tr>)";
                    cuerpoDot +=R"(<td border="0">part_status</td>)";
                    cuerpoDot +=(R"(<td border="0">)" + string(1, mbr.mbr_partition[i].part_status) + "</td>").c_str();
                    cuerpoDot +=R"(</tr>)";
                    cuerpoDot +=R"(<tr>)";
                    cuerpoDot +=R"(<td bgcolor="plum2">part_type</td>)";
                    cuerpoDot +=(R"(<td bgcolor="plum2">)" + string (1, mbr.mbr_partition[i].part_type) + "</td>").c_str();
                    cuerpoDot +=R"(</tr>)";
                    cuerpoDot +=R"(<tr>)";
                    cuerpoDot +=R"(<td border="0">part_fit</td>)";
                    cuerpoDot +=(R"(<td border="0">)" + string(1, mbr.mbr_partition[i].part_fit) + "</td>").c_str();
                    cuerpoDot +=R"(</tr>)";
                    cuerpoDot +=R"(<tr>)";
                    cuerpoDot +=R"(<td bgcolor="plum2">part_start</td>)";
                    cuerpoDot +=(R"(<td bgcolor="plum2">)" + to_string(mbr.mbr_partition[i].part_s) + "</td>").c_str();
                    cuerpoDot +=R"(</tr>)";
                    cuerpoDot +=R"(<tr>)";
                    cuerpoDot +=R"(<td border="0">part_size</td>)";
                    cuerpoDot +=(R"(<td border="0">)" + to_string(mbr.mbr_partition[i].part_s) + "</td>").c_str();
                    cuerpoDot +=R"(</tr>)";
                    cuerpoDot +=R"(<tr>)";
                    cuerpoDot +=R"(<td bgcolor="plum2">part_name</td>)";
                    string name1=mbr.mbr_partition[i].part_name;
                    cuerpoDot +=(R"(<td bgcolor="plum2">)" + name1 + "</td>").c_str();
                    cuerpoDot +=R"(</tr>)";

                    if (mbr.mbr_partition[i].part_type=='e'){
                        EBR ebr;
                        fseek(file,mbr.mbr_partition[i].part_start,SEEK_SET);
                        fread(&ebr,sizeof(EBR),1,file);
                        if (!(ebr.part_s==-1 && ebr.part_next==-1)){
                            while (true){
                                cuerpoDot +=R"(<tr><td colspan="2" bgcolor="salmon"><font color="white">Particion Logica</font></td></tr>)";
                                cuerpoDot +=R"(<tr>)";
                                cuerpoDot +=R"(<td border="0">part_status</td>)";
                                cuerpoDot +=(R"(<td border="0">)" + string(1, ebr.part_status) + "</td>").c_str();
                                cuerpoDot +=R"(</tr>)";
                                cuerpoDot +=R"(<tr>)";
                                cuerpoDot +=R"(<td bgcolor="lightpink">part_next</td>)";
                                cuerpoDot +=(R"(<td bgcolor="lightpink">)" + to_string(ebr.part_next) + "</td>").c_str();
                                cuerpoDot +=R"(</tr>)";
                                cuerpoDot +=R"(<tr>)";
                                cuerpoDot +=R"(<td border="0">part_fit</td>)";
                                cuerpoDot +=(R"(<td border="0">)" + string(1, ebr.part_fit) + "</td>").c_str();
                                cuerpoDot +=R"(</tr>)";
                                cuerpoDot +=R"(<tr>)";
                                cuerpoDot +=R"(<td bgcolor="lightpink">part_start</td>)";
                                cuerpoDot +=(R"(<td bgcolor="lightpink">)" + to_string(ebr.part_start) + "</td>").c_str();
                                cuerpoDot +=R"(</tr>)";
                                cuerpoDot +=R"(<tr>)";
                                cuerpoDot +=R"(<td border="0">part_size</td>)";
                                cuerpoDot +=(R"(<td border="0">)" + to_string(ebr.part_s) + "</td>").c_str();
                                cuerpoDot +=R"(</tr>)";
                                cuerpoDot +=R"(<tr>)";
                                cuerpoDot +=R"(<td bgcolor="lightpink">part_name</td>)";
                                string name1=ebr.part_name;
                                cuerpoDot +=(R"(<td bgcolor="lightpink">)" + name1 + "</td>").c_str();
                                cuerpoDot +=R"(</tr>)";
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

            cuerpoDot +="</table>>];\n";
            cuerpoDot +="}";
            fprintf(fileDot, cuerpoDot.c_str());
            fclose(fileDot);
            fclose(file);
            string command="sudo -S  dot -T"+this->extension+" mbr.dot -o \""+ this->path+"\"";
            system(command.c_str());
            cout<<"REPORTE GENERADO CON EXITO:  MBR"<<endl;
        }else{
            cout << "EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA" << endl;
            return;
        }
    }else{
        cout << "LA MONTURA CON EL ID: " << this->id << " NO EXISTE O NO SE ENCUENTRA EN EL SISTEMA" << endl;
        return;
    }
}

void Rep::ejecutarReporte_disk() {
    Nodo_M *nodoMontura=this->mountList->buscar(this->id);
    if (nodoMontura != NULL){
        FILE *fileReporte;
        MBR mbr;
        this->extension= this->getExtensionFile(this->path);
        this->directorio= this->getCarpetas(this->path);
        system(("sudo -S mkdir -p \'"+this->directorio+"\'").c_str());
        system(("sudo -S chmod -R 777 \'"+this->directorio+"\'").c_str());
        if ((fileReporte= fopen(nodoMontura->path.c_str(), "rb+"))){
            fseek(fileReporte, 0, SEEK_SET);
            fread(&mbr, sizeof (MBR), 1, fileReporte);
            int size=mbr.mbr_tamano;

            FILE *fileDot= fopen("disk.dot", "w");
            fprintf(fileDot, "digraph G {\n");
            fprintf(fileDot, "node[shape=none]\n");
            fprintf(fileDot, "start[label=<<table CELLSPACING=\"0\"><tr>");
            fprintf(fileDot, "<td bgcolor=\"khaki1\" rowspan=\"2\">MBR</td>");

            int start=sizeof(MBR);
            int i=0;

            while (i<4){
                // ESTA LIBRE
                if (mbr.mbr_partition[i].part_start == -1){
                    i++;
                    while (i<4){
                        if (mbr.mbr_partition[i].part_start!=-1){
                            int porcentaje= ((mbr.mbr_partition[i].part_start - start) / (size * 1.0)) * 100.0;
                            fprintf(fileDot, ("<td bgcolor=\"lavender\" rowspan=\"2\">LIBRE <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());
                            break;
                        }
                        i++;
                    }
                    if (i==4){
                        float porcentaje= (((size - start) * 1.0) / size) * (100.0);
                        fprintf(fileDot, ("<td bgcolor=\"lavender\" rowspan=\"2\">LIBRE <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());
                        goto salida1;
                    }
                    i--;

                }
                // ESTA OCUPADA
                else{
                    if (mbr.mbr_partition[i].part_type=='e'){
                        int contadorBloquesExtendida = 0;
                        EBR ebr;
                        fseek(fileReporte, mbr.mbr_partition[i].part_start, SEEK_SET);
                        fread(&ebr, sizeof(EBR), 1, fileReporte);
                        if (!(ebr.part_s==-1 && ebr.part_next==-1)){
                            string nombre_1=ebr.part_name;
                            if (ebr.part_s>-1){
                                contadorBloquesExtendida+=2;
                            }
                            else{
                                contadorBloquesExtendida+=2;
                            }
                            fseek(fileReporte, ebr.part_next, SEEK_SET);
                            fread(&ebr, sizeof (EBR), 1, fileReporte);
                            while (true){
                                contadorBloquesExtendida+=2;
                                if (ebr.part_next==-1){
                                    if ((ebr.part_start+ebr.part_s)<mbr.mbr_partition[i].part_s){
                                        contadorBloquesExtendida+=1;
                                    }
                                    break;
                                }else{
                                    if ((ebr.part_start+ebr.part_s)<ebr.part_next){
                                        contadorBloquesExtendida+=1;
                                    }
                                }
                                fseek(fileReporte, ebr.part_next, SEEK_SET);
                                fread(&ebr, sizeof (EBR), 1, fileReporte);
                            }
                        }
                        fprintf(fileDot, ("<td bgcolor=\"darkolivegreen1\" colspan=\""+ to_string(contadorBloquesExtendida)+"\">EXTENDIDA</td>").c_str());

                    }
                    else if (mbr.mbr_partition[i].part_type=='p'){
                        float p1= (mbr.mbr_partition[i].part_s*1.0) / size;
                        float porcentaje=p1*100.0;
                        string name1=mbr.mbr_partition[i].part_name;
                        fprintf(fileDot, ("<td bgcolor=\"violet\" rowspan=\"2\">" + name1 + " <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());
                        if (i!=3){
                            if ((mbr.mbr_partition[i].part_start+mbr.mbr_partition[i].part_s)<mbr.mbr_partition[i+1].part_start){
                                porcentaje= ((mbr.mbr_partition[i+1].part_start-(mbr.mbr_partition[i].part_start+mbr.mbr_partition[i].part_s)) / size) * 100;
                                fprintf(fileDot, ("<td bgcolor=\"lavender\" rowspan=\"2\">LIBRE <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());
                            }
                        }else if ((mbr.mbr_partition[i].part_start+mbr.mbr_partition[i].part_s) < size){
                            porcentaje= ((size - (mbr.mbr_partition[i].part_start + mbr.mbr_partition[i].part_s)) / size) * 100;
                            fprintf(fileDot, ("<td bgcolor=\"lavender\" rowspan=\"2\">LIBRE <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());
                        }

                    }
                    start= mbr.mbr_partition[i].part_start + mbr.mbr_partition[i].part_s;
                }
                i++;
            }

            salida1:
            fprintf(fileDot, "</tr>\n");
            // Por si hay extendida
            i = 0;
            while (i<4){
                if (mbr.mbr_partition[i].part_start != -1){
                    if (mbr.mbr_partition[i].part_type=='e'){
                        fprintf(fileDot, "<tr>\n");
                        float porcentaje= ((mbr.mbr_partition[i].part_s) / size) * 100.0;
                        EBR ebr;
                        fseek(fileReporte, mbr.mbr_partition[i].part_start, SEEK_SET);
                        fread(&ebr, sizeof(EBR), 1, fileReporte);
                        if (!(ebr.part_s==-1 && ebr.part_next==-1)){
                            string nombre_1=ebr.part_name;
                            if (ebr.part_s>-1){
                                fprintf(fileDot, ("<td bgcolor=\"steelblue1\" rowspan=\"1\">EBR <br/>" + nombre_1 + "</td>").c_str());
                                porcentaje= ((ebr.part_s*1.0) / size) * 100.0;
                                fprintf(fileDot, ("<td bgcolor=\"tan1\"  rowspan=\"1\">Logica <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());
                            }
                            else{
                                fprintf(fileDot, "<td bgcolor=\"steelblue1\" rowspan=\"1\">EBR</td>");
                                porcentaje= (((ebr.part_next-ebr.part_start)*1.0) / size) * 100.0;
                                fprintf(fileDot, ("<td bgcolor=\"lavender\" rowspan=\"1\">Libre <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());

                            }
                            fseek(fileReporte, ebr.part_next, SEEK_SET);
                            fread(&ebr, sizeof (EBR), 1, fileReporte);
                            while (true){
                                string name1=ebr.part_name;
                                fprintf(fileDot, ("<td bgcolor=\"steelblue1\" rowspan=\"1\">EBR <br/>" + name1 + "</td>").c_str());
                                porcentaje= ((ebr.part_s*1.0) / size) * 100.0;
                                fprintf(fileDot, ("<td bgcolor=\"tan1\" rowspan=\"1\">Logica <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());
                                if (ebr.part_next==-1){
                                    if ((ebr.part_start+ebr.part_s)<mbr.mbr_partition[i].part_s){
                                        porcentaje= (((mbr.mbr_partition[i].part_s-(ebr.part_start+ebr.part_s))*1.0) / size) * 100.0;
                                        fprintf(fileDot, ("<td bgcolor=\"lavender\" rowspan=\"1\">Libre <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());
                                    }
                                    break;
                                }else{
                                    if ((ebr.part_start+ebr.part_s)<ebr.part_next){
                                        porcentaje= (((ebr.part_next-(ebr.part_start+ebr.part_s))*1.0) / size) * 100.0;
                                        fprintf(fileDot, ("<td bgcolor=\"lavender\" rowspan=\"1\">Libre <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());
                                    }
                                }
                                fseek(fileReporte, ebr.part_next, SEEK_SET);
                                fread(&ebr, sizeof (EBR), 1, fileReporte);
                            }
                        }
                        fprintf(fileDot, "</tr>\n");
                    }
                }
                i++;
            }
            fprintf(fileDot, "</table>>];\n");
            fprintf(fileDot, "}");
            fclose(fileDot);
            fclose(fileReporte);
            string command="sudo -S  dot -T"+this->extension+" disk.dot -o \""+ this->path+"\"";
            system(command.c_str());
            cout<<"REPORTE GENERADO CON EXITO:  DISK"<<endl;
        }
        else{
            cout << "EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA" << endl;
            return;
        }
    }else{
        cout << "LA MONTURA CON EL ID: " << this->id << " NO EXISTE O NO SE ENCUENTRA EN EL SISTEMA" << endl;
        return;
    }
}

string Rep::getCarpetas(string ruta) {
    std::filesystem::path path(ruta);
    std::string dir = path.parent_path().string();
    dir += "/";
    return dir;
}
string Rep::getExtensionFile(string path) {
    int i= path.find('.');
    string extension=path.substr(i+1,path.length());
    return extension;
}

void Rep::ejecutarReporte_inode() {
    Nodo_M *nodoMontura=this->mountList->buscar(this->id);
    FILE *fileDisco;
    SuperBloque superBloqueMontura;
    if (nodoMontura != NULL) {
        this->extension = this->getExtensionFile(this->path);
        this->directorio = this->getCarpetas(this->path);

        system(("sudo -S mkdir -p \'" + this->directorio + "\'").c_str());
        system(("sudo -S chmod -R 777 \'" + this->directorio + "\'").c_str());


        if ((fileDisco= fopen(nodoMontura->path.c_str(), "rb+"))){
            // leer el superbloque
            if (nodoMontura->type == 'l'){
                fseek(fileDisco, nodoMontura->start + sizeof(EBR), SEEK_SET);
            }
            else if (nodoMontura->type == 'p'){
                fseek(fileDisco, nodoMontura->start, SEEK_SET);
            }
            fread(&superBloqueMontura, sizeof(SuperBloque), 1, fileDisco);

            int contadorInodosDireccion=0;
            int contadorInodosTotal=0;
            int contadorInodosExtra=0;
            char siguienteBit;
            char bit;
            int bitMapInodos_Start=superBloqueMontura.s_bm_inode_start;
            int bitMapInodos_End= bitMapInodos_Start + superBloqueMontura.s_inodes_count;
            char date_1[70];
            string date_2="";
            TablaInodo tablaInodo;

            FILE *cuerpoDot= fopen("inodes.dot", "w");
            fprintf(cuerpoDot, "digraph G { \n rankdir=LR\n");
            fprintf(cuerpoDot, "node[shape=none]\n");

            for (int i = bitMapInodos_Start; i < bitMapInodos_End; ++i) {
                fseek(fileDisco, i, SEEK_SET);
                fread(&bit, sizeof(char), 1, fileDisco);
                if (bit=='1'){
                    fseek(fileDisco, (superBloqueMontura.s_inode_start + (contadorInodosDireccion * sizeof(TablaInodo))), SEEK_SET);
                    fread(&tablaInodo, sizeof(TablaInodo), 1, fileDisco);

                    fprintf(cuerpoDot, ("n" + to_string(contadorInodosDireccion) +
                                        R"([label=<<table CELLSPACING="0"><tr><td colspan="2">INODO )" + to_string(superBloqueMontura.s_inode_start + (contadorInodosDireccion * sizeof(TablaInodo))) +
                                        "</td></tr>\n").c_str());

                    fprintf(cuerpoDot, "<tr>\n");
                    fprintf(cuerpoDot, "<td>i_uid</td>\n");
                    fprintf(cuerpoDot, ("<td>" + to_string(tablaInodo.i_uid) + "</td>\n").c_str());
                    fprintf(cuerpoDot, "</tr>\n");

                    fprintf(cuerpoDot, "<tr>\n");
                    fprintf(cuerpoDot, "<td>i_gid</td>\n");
                    fprintf(cuerpoDot, ("<td>" + to_string(tablaInodo.i_gid) + "</td>\n").c_str());
                    fprintf(cuerpoDot, "</tr>\n");

                    fprintf(cuerpoDot, "<tr>\n");
                    fprintf(cuerpoDot, "<td>i_s</td>\n");
                    fprintf(cuerpoDot, ("<td>" + to_string(tablaInodo.i_s) + "</td>\n").c_str());
                    fprintf(cuerpoDot, "</tr>\n");

                    strftime(date_1, sizeof (date_1), "%Y-%m-%d %H:%M:%S", localtime(&tablaInodo.i_atime));
                    date_2=date_1;
                    fprintf(cuerpoDot, "<tr>\n");
                    fprintf(cuerpoDot, "<td>i_atime</td>\n");
                    fprintf(cuerpoDot, ("<td>" + date_2 + "</td>\n").c_str());
                    fprintf(cuerpoDot, "</tr>\n");

                    strftime(date_1, sizeof (date_1), "%Y-%m-%d %H:%M:%S", localtime(&tablaInodo.i_ctime));
                    date_2=date_1;
                    fprintf(cuerpoDot, "<tr>\n");
                    fprintf(cuerpoDot, "<td>i_ctime</td>\n");
                    fprintf(cuerpoDot, ("<td>" + date_2 + "</td>\n").c_str());
                    fprintf(cuerpoDot, "</tr>\n");

                    strftime(date_1, sizeof (date_1), "%Y-%m-%d %H:%M:%S", localtime(&tablaInodo.i_mtime));
                    date_2=date_1;
                    fprintf(cuerpoDot, "<tr>\n");
                    fprintf(cuerpoDot, "<td>i_mtime</td>\n");
                    fprintf(cuerpoDot, ("<td>" + date_2 + "</td>\n").c_str());
                    fprintf(cuerpoDot, "</tr>\n");

                    for (int j = 0; j < 15; ++j) {
                        fprintf(cuerpoDot, "<tr>\n");
                        fprintf(cuerpoDot, "<td>i_block</td>\n");
                        fprintf(cuerpoDot, ("<td>" + to_string(tablaInodo.i_block[j]) + "</td>\n").c_str());
                        fprintf(cuerpoDot, "</tr>\n");
                    }

                    fprintf(cuerpoDot, "<tr>\n");
                    fprintf(cuerpoDot, "<td>i_type</td>\n");
                    fprintf(cuerpoDot, ("<td>" + string(1, tablaInodo.i_type) + "</td>\n").c_str());
                    fprintf(cuerpoDot, "</tr>\n");

                    fprintf(cuerpoDot, "<tr>\n");
                    fprintf(cuerpoDot, "<td>i_perm</td>\n");
                    fprintf(cuerpoDot, ("<td>" + to_string(tablaInodo.i_perm) + "</td>\n").c_str());
                    fprintf(cuerpoDot, "</tr>\n");

                    fprintf(cuerpoDot, "</table>>]\n");
                    contadorInodosTotal++;
                }
                contadorInodosDireccion++;
            }

            contadorInodosDireccion=0;
            contadorInodosExtra=0;
            for (int i = bitMapInodos_Start; i < bitMapInodos_End; ++i) {
                fseek(fileDisco, i, SEEK_SET);
                fread(&bit, sizeof(char), 1, fileDisco);
                if (bit=='1') {
                    if((contadorInodosExtra+1)==contadorInodosTotal)break;
                    fprintf(cuerpoDot,
                            ("n" + to_string(contadorInodosDireccion) + "->n" + to_string(contadorInodosDireccion + 1) + ";\n").c_str());
                    contadorInodosExtra++;
                }
                contadorInodosDireccion++;
            }


            fprintf(cuerpoDot, "}");
            fclose(cuerpoDot);
            fclose(fileDisco);
            string command="sudo -S  dot -T"+this->extension+" inodes.dot -o \""+ this->path+"\"";
            system(command.c_str());
            cout<<"REPORTE GENERADO CON EXITO:  INODE"<<endl;

        }else{
            cout << "EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA" << endl;
            return;
        }
    }else{
        cout << "LA MONTURA CON EL ID: " << this->id << " NO EXISTE O NO SE ENCUENTRA EN EL SISTEMA" << endl;
        return;
    }
}

void Rep::ejecutarReporte_block() {
    this->contadorBloques_ReporteBloques=0;
    Nodo_M *nodoMontura=this->mountList->buscar(this->id);
    SuperBloque superBloque;
    FILE *fileDisco;
    if (nodoMontura != NULL) {
        this->directorio = this->getCarpetas(this->path);
        this->extension = this->getExtensionFile(this->path);
        system(("sudo -S mkdir -p \'" + this->directorio + "\'").c_str());
        system(("sudo -S chmod -R 777 \'" + this->directorio + "\'").c_str());

        if ((fileDisco= fopen(nodoMontura->path.c_str(), "rb+"))){
            if (nodoMontura->type == 'l'){
                fseek(fileDisco, nodoMontura->start + sizeof(EBR), SEEK_SET);
            }
            else if (nodoMontura->type == 'p'){
                fseek(fileDisco, nodoMontura->start, SEEK_SET);
            }
            fread(&superBloque, sizeof(SuperBloque), 1, fileDisco);
            TablaInodo tablaInodo;
            int contadorInodosExtra=0;
            int bmInodes_Start=superBloque.s_bm_inode_start;
            int bmInodes_End= bmInodes_Start + superBloque.s_inodes_count;
            char bit;
            BloqueApuntador blkApuntador_1,blkApuntador_2,blkApuntador_3;
            int direccionInodos=0;
            string date_2="";
            char date_1[70];

            FILE *cuerpoDot= fopen("blocks.dot", "w");
            fprintf(cuerpoDot, "digraph G {\n rankdir=LR\n");
            fprintf(cuerpoDot, "node[shape=none]\n");
            // básicamente leer cada tablaInodo para poner cada uno de sus bloques.
            for (int i = bmInodes_Start; i < bmInodes_End; ++i) {
                fseek(fileDisco, i, SEEK_SET);
                fread(&bit, sizeof(char), 1, fileDisco);
                if (bit=='1'){
                    fseek(fileDisco, (superBloque.s_inode_start + (direccionInodos * sizeof(TablaInodo))), SEEK_SET);
                    fread(&tablaInodo, sizeof(TablaInodo), 1, fileDisco);

                    for (int i = 0; i < 15; ++i) {
                        if (tablaInodo.i_block[i] != -1){
                            if (i<12){
                                if (tablaInodo.i_type == '1'){
                                    fprintf(cuerpoDot, graficarBlkArchivo(tablaInodo.i_block[i], fileDisco).c_str());
                                }
                                else if (tablaInodo.i_type == '0'){
                                    fprintf(cuerpoDot, graficarBlkCarpeta(tablaInodo.i_block[i], fileDisco).c_str());
                                }
                            }
                            else if (i==12){
                                fseek(fileDisco, tablaInodo.i_block[i], SEEK_SET);
                                fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, fileDisco);
                                fprintf(cuerpoDot, graficarBlkApuntador(tablaInodo.i_block[i], fileDisco).c_str());
                                for (int j = 0; j < 16; ++j) {
                                    if (blkApuntador_1.b_pointers[j] != -1){
                                        if (tablaInodo.i_type == '1'){
                                            fprintf(cuerpoDot,
                                                    graficarBlkArchivo(blkApuntador_1.b_pointers[j], fileDisco).c_str());
                                        }
                                        else if (tablaInodo.i_type == '0'){
                                            fprintf(cuerpoDot,
                                                    graficarBlkCarpeta(blkApuntador_1.b_pointers[j], fileDisco).c_str());
                                        }
                                    }
                                }
                            }
                            else if (i==13){
                                fseek(fileDisco, tablaInodo.i_block[i], SEEK_SET);
                                fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, fileDisco);
                                fprintf(cuerpoDot, graficarBlkApuntador(tablaInodo.i_block[i], fileDisco).c_str());
                                for (int j = 0; j < 16; ++j) {
                                    if (blkApuntador_1.b_pointers[j] != -1){
                                        fseek(fileDisco, blkApuntador_1.b_pointers[j], SEEK_SET);
                                        fread(&blkApuntador_2, sizeof(BloqueApuntador), 1, fileDisco);
                                        fprintf(cuerpoDot, graficarBlkApuntador(blkApuntador_1.b_pointers[j], fileDisco).c_str());
                                        for (int k = 0; k < 16; ++k) {
                                            if (blkApuntador_2.b_pointers[k] != -1){
                                                if (tablaInodo.i_type == '0'){
                                                    fprintf(cuerpoDot,
                                                            graficarBlkCarpeta(blkApuntador_2.b_pointers[k], fileDisco).c_str());
                                                }
                                                else if (tablaInodo.i_type == '1'){
                                                    fprintf(cuerpoDot,
                                                            graficarBlkArchivo(blkApuntador_2.b_pointers[k], fileDisco).c_str());
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else if (i==14){
                                fseek(fileDisco, tablaInodo.i_block[i], SEEK_SET);
                                fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, fileDisco);
                                fprintf(cuerpoDot, graficarBlkApuntador(tablaInodo.i_block[i], fileDisco).c_str());
                                for (int j = 0; j < 16; ++j) {
                                    if (blkApuntador_1.b_pointers[j] != -1){
                                        fseek(fileDisco, blkApuntador_1.b_pointers[j], SEEK_SET);
                                        fread(&blkApuntador_2, sizeof(BloqueApuntador), 1, fileDisco);
                                        fprintf(cuerpoDot, graficarBlkApuntador(blkApuntador_1.b_pointers[j], fileDisco).c_str());
                                        for (int k = 0; k < 16; ++k) {
                                            if (blkApuntador_2.b_pointers[k] != -1){
                                                fseek(fileDisco, blkApuntador_2.b_pointers[k], SEEK_SET);
                                                fread(&blkApuntador_3, sizeof(BloqueApuntador), 1, fileDisco);
                                                fprintf(cuerpoDot,
                                                        graficarBlkApuntador(blkApuntador_2.b_pointers[k], fileDisco).c_str());
                                                for (int z = 0; z < 16; ++z) {
                                                    if (blkApuntador_3.b_pointers[z] != -1){
                                                        if (tablaInodo.i_type == '0'){
                                                            fprintf(cuerpoDot,
                                                                    graficarBlkCarpeta(blkApuntador_3.b_pointers[z],
                                                                                       fileDisco).c_str());
                                                        }
                                                        else if (tablaInodo.i_type == '1'){
                                                            fprintf(cuerpoDot,
                                                                    graficarBlkArchivo(blkApuntador_3.b_pointers[z],
                                                                                       fileDisco).c_str());
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
                direccionInodos++;
            }

            int bmBlks_Start = superBloque.s_bm_block_start;
            int bmBlks_End = bmBlks_Start + superBloque.s_blocks_count;
            for (int i = bmBlks_Start; i < bmBlks_End; ++i) {
                fseek(fileDisco, i, SEEK_SET);
                fread(&bit, sizeof(char), 1, fileDisco);
                if (bit=='1') {
                    if((contadorInodosExtra+1)==this->contadorBloques_ReporteBloques)break;
                    fprintf(cuerpoDot,
                            ("n" + to_string(contadorInodosExtra) + "->n" + to_string(contadorInodosExtra + 1) + ";\n").c_str());
                    contadorInodosExtra++;
                }
            }


            fprintf(cuerpoDot, "}");
            fclose(cuerpoDot);
            fclose(fileDisco);
            string command="sudo -S  dot -T"+this->extension+" blocks.dot -o \""+ this->path+"\"";
            system(command.c_str());
            cout<<"REPORTE GENERADO CON EXITO:  BLOCK" <<endl;

        }else{
            cout << "EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA" << endl;
            return;
        }
    }else{
        cout << "LA MONTURA CON EL ID: " << this->id << " NO EXISTE O NO SE ENCUENTRA EN EL SISTEMA" << endl;
        return;
    }
}

string Rep::graficarBlkCarpeta(int direccionBlk, FILE *disco) {
    BloqueCarpeta blkCarpeta;
    string cuerpoDot="";
    fseek(disco, direccionBlk, SEEK_SET);
    fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, disco);

    cuerpoDot+= "n" + to_string(contadorBloques_ReporteBloques) + "[label=<<table CELLSPACING=\"0\">\n";
    cuerpoDot+="<tr>\n";
    cuerpoDot+=R"(<td colspan="2" bgcolor="lightcoral">Bloque Carpeta</td>)";
    cuerpoDot+="</tr>\n";
    cuerpoDot+="<tr>\n";
    cuerpoDot+= "<td bgcolor=\"lightpink\">b_name</td>";
    cuerpoDot+="\n";
    cuerpoDot+= "<td bgcolor=\"lightpink\">b_inodo</td>";
    cuerpoDot+="\n";
    cuerpoDot+="</tr>\n";
    for (int i = 0; i < 4; ++i) {
        string b_name="";
        for (int j = 0; j < 12; ++j) {
            if (blkCarpeta.b_content[i].b_name[j] == '\000'){
                break;
            }
            b_name+=blkCarpeta.b_content[i].b_name[j];
        }

        cuerpoDot+="<tr>\n";
        cuerpoDot+= "<td>" + b_name + "</td>\n";
        cuerpoDot+= "<td>" + to_string(blkCarpeta.b_content[i].b_inodo) + "</td>\n";
        cuerpoDot+="</tr>\n";
    }
    cuerpoDot+="</table>>]\n";
    this->contadorBloques_ReporteBloques++;
    return cuerpoDot;
}

string Rep::graficarBlkArchivo(int direccionBlk, FILE *disco) {
    BloqueArchivo blkCarpeta;
    string cuerpoDot="";
    string contenido="";
    fseek(disco, direccionBlk, SEEK_SET);
    fread(&blkCarpeta, sizeof(BloqueArchivo), 1, disco);

    for (int i = 0; i < 64; ++i) {
        if (blkCarpeta.b_content[i] == '\000'){
            break;
        }
        contenido+=blkCarpeta.b_content[i];
    }

    cuerpoDot+= "n" + to_string(contadorBloques_ReporteBloques) + "[label=<<table CELLSPACING=\"0\">\n";
    cuerpoDot+="<tr>\n";
    cuerpoDot+=R"(<td bgcolor="lemonchiffon">Bloque Archivo</td>)";
    cuerpoDot+="</tr>\n";
    cuerpoDot+="<tr>\n";
    cuerpoDot+= "<td>" + contenido + "</td>\n";
    cuerpoDot+="</tr>\n";
    cuerpoDot+="</table>>]\n";
    this->contadorBloques_ReporteBloques++;
    return  cuerpoDot;
}

string Rep::graficarBlkApuntador(int direccionBlk, FILE *disco) {
    BloqueApuntador blkApuntador;
    string cuerpoDot="";
    fseek(disco, direccionBlk, SEEK_SET);
    fread(&blkApuntador, sizeof(BloqueApuntador), 1, disco);

    cuerpoDot+= "n" + to_string(contadorBloques_ReporteBloques) + "[label=<<table CELLSPACING=\"0\">\n";
    cuerpoDot+="<tr>\n";
    cuerpoDot+=R"(<td colspan="2" bgcolor="lightcoral">Bloque de Apuntadores</td>)";
    cuerpoDot+="</tr>\n";
    for (int i = 0; i < 16; ++i) {
        cuerpoDot+="<tr>\n";
        cuerpoDot+= "<td>b_pointer " + to_string(i) + "</td>\n";
        cuerpoDot+= "<td>" + to_string(blkApuntador.b_pointers[i]) + "</td>\n";
        cuerpoDot+="</tr>\n";
    }
    cuerpoDot+="</table>>]\n";
    this->contadorBloques_ReporteBloques++;
    return  cuerpoDot;
}

void Rep::ejecutarReporte_bm_inode() {
    this->directorio = this->getCarpetas(this->path);
    this->extension = this->getExtensionFile(this->path);
    SuperBloque superBloque;
    if(this->extension=="txt"){
        Nodo_M *nodoMontura=this->mountList->buscar(this->id);
        if (nodoMontura != NULL) {
            system(("sudo -S mkdir -p \'" + this->directorio + "\'").c_str());
            system(("sudo -S chmod -R 777 \'" + this->directorio + "\'").c_str());

            FILE *fileDisco;
            if ((fileDisco= fopen(nodoMontura->path.c_str(), "rb+"))){
                if (nodoMontura->type == 'l'){
                    EBR ebr;
                    fseek(fileDisco, nodoMontura->start, SEEK_SET);
                    fread(&ebr, sizeof(EBR), 1, fileDisco);
                    if (ebr.part_status!='2'){
                        cout<<"PARTICION NO FORMATEADA, IMPOSIBLE EJECUTAR COMANDO"<<endl;
                        return;
                    }
                    fseek(fileDisco, ebr.part_start + sizeof(EBR), SEEK_SET);
                }
                else if (nodoMontura->type == 'p'){
                    MBR mbr;
                    fseek(fileDisco, 0, SEEK_SET);
                    fread(&mbr, sizeof(MBR), 1, fileDisco);

                    if (mbr.mbr_partition[nodoMontura->pos].part_status != '2'){
                        cout<<"PARTICION NO FORMATEADA, IMPOSIBLE EJECUTAR COMANDO"<<endl;
                        return;
                    }
                    fseek(fileDisco, mbr.mbr_partition[nodoMontura->pos].part_start, SEEK_SET);
                }

                fread(&superBloque, sizeof(SuperBloque), 1, fileDisco);
                char bit;
                int bitMapInode_Start=superBloque.s_bm_inode_start;
                int bitMapInode_End= bitMapInode_Start + superBloque.s_inodes_count;
                int contadorLineas=0;

                FILE *cuerpoTxt= fopen(this->path.c_str(), "w");
                for (int i = bitMapInode_Start; i < bitMapInode_End; ++i) {
                    fseek(fileDisco, i, SEEK_SET);
                    fread(&bit, sizeof(char ), 1, fileDisco);
                    fprintf(cuerpoTxt, (string(1, bit) + " ").c_str());
                    if (contadorLineas == 19){
                        fprintf(cuerpoTxt, "\n");
                        contadorLineas=0;
                    }else{
                        contadorLineas++;
                    }
                }
                fclose(cuerpoTxt);
                fclose(fileDisco);
                cout<<"REPORTE GENERADO CON EXITO:  BM INODE"<<endl;
            }else{
                cout << "EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA" << endl;
                return;
            }
        }else{
            cout << "LA MONTURA CON EL ID: " << this->id << " NO EXISTE O NO SE ENCUENTRA EN EL SISTEMA" << endl;
            return;
        }
    }
    else{
        cout << "EL REPORTE BITMAP INODO SOLO PUEDE SER EN FORMATO TXT" << endl;
    }
}

void Rep::ejecutarReporte_bm_block() {
    this->directorio = this->getCarpetas(this->path);
    this->extension = this->getExtensionFile(this->path);
    SuperBloque superBloque;
    if(this->extension=="txt"){
        Nodo_M *nodoMontura=this->mountList->buscar(this->id);
        if (nodoMontura != NULL) {
            system(("sudo -S mkdir -p \'" + this->directorio + "\'").c_str());
            system(("sudo -S chmod -R 777 \'" + this->directorio + "\'").c_str());

            FILE *fileDisco;
            if ((fileDisco= fopen(nodoMontura->path.c_str(), "rb+"))){
                SuperBloque superBloque;
                if (nodoMontura->type == 'l'){
                    EBR ebr;
                    fseek(fileDisco, nodoMontura->start, SEEK_SET);
                    fread(&ebr, sizeof(EBR), 1, fileDisco);
                    if (ebr.part_status!='2'){
                        cout<<"PARTICION NO FORMATEADA, IMPOSIBLE EJECUTAR COMANDO"<<endl;
                        return;
                    }
                    fseek(fileDisco, ebr.part_start + sizeof(EBR), SEEK_SET);
                }
                else if (nodoMontura->type == 'p'){
                    MBR mbr;
                    fseek(fileDisco, 0, SEEK_SET);
                    fread(&mbr, sizeof(MBR), 1, fileDisco);

                    if (mbr.mbr_partition[nodoMontura->pos].part_status != '2'){
                        cout<<"PARTICION NO FORMATEADA, IMPOSIBLE EJECUTAR COMANDO"<<endl;
                        return;
                    }
                    fseek(fileDisco, mbr.mbr_partition[nodoMontura->pos].part_start, SEEK_SET);
                }

                fread(&superBloque, sizeof(SuperBloque), 1, fileDisco);
                char bit;
                int contadorLinea=0;
                int bmBlk_Start=superBloque.s_bm_block_start;
                int bmBlk_End= bmBlk_Start + superBloque.s_blocks_count;


                FILE *cuerpoTxt= fopen(this->path.c_str(), "w");
                for (int i = bmBlk_Start; i < bmBlk_End; ++i) {
                    fseek(fileDisco, i, SEEK_SET);
                    fread(&bit, sizeof(char ), 1, fileDisco);
                    fprintf(cuerpoTxt, (string(1, bit) + " ").c_str());
                    if (contadorLinea == 19){
                        fprintf(cuerpoTxt, "\n");
                        contadorLinea=0;
                    }else{
                        contadorLinea++;
                    }
                }


                fclose(cuerpoTxt);
                fclose(fileDisco);
                cout<<"REPORTE GENERADO CON EXITO:  BM BLOCK"<<endl;
            }else{
                cout << "EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA" << endl;
                return;
            }
        }else{
            cout << "LA MONTURA CON EL ID: " << this->id << " NO EXISTE O NO SE ENCUENTRA EN EL SISTEMA" << endl;
            return;
        }
    }
    else{
        cout << "EL REPORTE BITMAP BLOCK SOLO PUEDE SER EN FORMATO TXT" << endl;
    }
}

void Rep::ejecutarReporte_sb() {
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL) {
        this->extension = this->getExtensionFile(this->path);
        this->directorio = this->getCarpetas(this->path);
        string nombreFile= this->getNameFileInRoute(nodo->path);
        system(("sudo -S mkdir -p \'" + this->directorio + "\'").c_str());
        system(("sudo -S chmod -R 777 \'" + this->directorio + "\'").c_str());

        FILE *fileDisco;
        if ((fileDisco= fopen(nodo->path.c_str(), "rb+"))){
            SuperBloque sb;
            if (nodo->type=='l'){
                EBR ebr;
                fseek(fileDisco, nodo->start, SEEK_SET);
                fread(&ebr, sizeof(EBR), 1, fileDisco);
                if (ebr.part_status != '2') {
                    fclose(fileDisco);
                    cout<<"PARTICION NO FORMATEADA, IMPOSIBLE EJECUTAR COMANDO"<<endl;
                    return;
                }
                fseek(fileDisco, nodo->start + sizeof(EBR), SEEK_SET);
            }
            else if (nodo->type=='p'){
                MBR mbr;
                fseek(fileDisco, 0, SEEK_SET);
                fread(&mbr, sizeof(MBR), 1, fileDisco);
                int pos=-1;
                for (int i = 0; i < 4; ++i) {
                    string name1=mbr.mbr_partition[i].part_name;
                    if (name1==nodo->name){
                        if (mbr.mbr_partition[i].part_status!='2'){
                            cout<<"PARTICION NO FORMATEADA, IMPOSIBLE EJECUTAR COMANDO"<<endl;
                            return;
                        }
                        pos=i;
                        break;
                    }
                }
                if (pos!=-1){
                    fseek(fileDisco, nodo->start, SEEK_SET);
                }else{
                    cout<<"LA PARTICION NO SE HA ENCONTRADO EN EL DISCO"<<endl;
                }
            }
            fread(&sb, sizeof(SuperBloque), 1, fileDisco);
            char fecha[100];


            FILE *cuerpoDot= fopen("superbloque.dot", "w");
            fprintf(cuerpoDot, "digraph G {\n");
            fprintf(cuerpoDot, "node[shape=none]\n");
            fprintf(cuerpoDot, "start[label=<<table CELLSPACING=\"0\">");
            fprintf(cuerpoDot, R"(<tr><td colspan="2" bgcolor="darkgreen"><font color="white">REPORTE DE SUPERBLOQUE</font></td></tr>)");

            fprintf(cuerpoDot, R"(<tr>)");
            fprintf(cuerpoDot, R"(<td bgcolor="white" border="0">sb_nombre_hd</td>)");
            fprintf(cuerpoDot, (R"(<td bgcolor="white" border="0">)" + nombreFile + "</td>)").c_str());
            fprintf(cuerpoDot, R"(</tr>)");

            fprintf(cuerpoDot, R"(<tr>)");
            fprintf(cuerpoDot, R"(<td bgcolor="green" border="0">s_filesystem_type</td>)");
            fprintf(cuerpoDot, (R"(<td bgcolor="green" border="0">)" + to_string(sb.s_filesystem_type) + "</td>)").c_str());
            fprintf(cuerpoDot, R"(</tr>)");

            fprintf(cuerpoDot, R"(<tr>)");
            fprintf(cuerpoDot, R"(<td bgcolor="white" border="0">s_inodes_count</td>)");
            fprintf(cuerpoDot, (R"(<td bgcolor="white" border="0">)" + to_string(sb.s_inodes_count) + "</td>)").c_str());
            fprintf(cuerpoDot, R"(</tr>)");

            fprintf(cuerpoDot, R"(<tr>)");
            fprintf(cuerpoDot, R"(<td bgcolor="green" border="0">s_blocks_count</td>)");
            fprintf(cuerpoDot, (R"(<td bgcolor="green" border="0">)" + to_string(sb.s_blocks_count) + "</td>)").c_str());
            fprintf(cuerpoDot, R"(</tr>)");

            fprintf(cuerpoDot, R"(<tr>)");
            fprintf(cuerpoDot, R"(<td bgcolor="white" border="0">s_free_blocks_count</td>)");
            fprintf(cuerpoDot, (R"(<td bgcolor="white" border="0">)" + to_string(sb.s_free_blocks_count) + "</td>)").c_str());
            fprintf(cuerpoDot, R"(</tr>)");

            fprintf(cuerpoDot, R"(<tr>)");
            fprintf(cuerpoDot, R"(<td bgcolor="green" border="0">s_free_inodes_count</td>)");
            fprintf(cuerpoDot, (R"(<td bgcolor="green" border="0">)" + to_string(sb.s_free_inodes_count) + "</td>)").c_str());
            fprintf(cuerpoDot, R"(</tr>)");

            strftime(fecha,sizeof (fecha),"%Y-%m-%d %H:%M:%S",localtime(&sb.s_mtime));
            string fecha1=fecha;
            fprintf(cuerpoDot, R"(<tr>)");
            fprintf(cuerpoDot, R"(<td bgcolor="white" border="0">s_mtime</td>)");
            fprintf(cuerpoDot, (R"(<td bgcolor="white" border="0">)" + fecha1 + "</td>)").c_str());
            fprintf(cuerpoDot, R"(</tr>)");

            strftime(fecha,sizeof (fecha),"%Y-%m-%d %H:%M:%S",localtime(&sb.s_umtime));
            string fecha3=fecha;
            fprintf(cuerpoDot, R"(<tr>)");
            fprintf(cuerpoDot, R"(<td bgcolor="green" border="0">s_umtime</td>)");
            fprintf(cuerpoDot, (R"(<td bgcolor="green" border="0">)" + fecha3 + "</td>)").c_str());
            fprintf(cuerpoDot, R"(</tr>)");

            fprintf(cuerpoDot, R"(<tr>)");
            fprintf(cuerpoDot, R"(<td bgcolor="white" border="0">s_mnt_count</td>)");
            fprintf(cuerpoDot, (R"(<td bgcolor="white" border="0">)" + to_string(sb.s_mnt_count) + "</td>)").c_str());
            fprintf(cuerpoDot, R"(</tr>)");

            fprintf(cuerpoDot, R"(<tr>)");
            fprintf(cuerpoDot, R"(<td bgcolor="green" border="0">s_magic</td>)");
            fprintf(cuerpoDot, (R"(<td bgcolor="green" border="0">)" + to_string(sb.s_magic) + "</td>)").c_str());
            fprintf(cuerpoDot, R"(</tr>)");

            fprintf(cuerpoDot, R"(<tr>)");
            fprintf(cuerpoDot, R"(<td bgcolor="white" border="0">s_inode_s</td>)");
            fprintf(cuerpoDot, (R"(<td bgcolor="white" border="0">)" + to_string(sb.s_inode_s) + "</td>)").c_str());
            fprintf(cuerpoDot, R"(</tr>)");

            fprintf(cuerpoDot, R"(<tr>)");
            fprintf(cuerpoDot, R"(<td bgcolor="green" border="0">s_block_s</td>)");
            fprintf(cuerpoDot, (R"(<td bgcolor="green" border="0">)" + to_string(sb.s_block_s) + "</td>)").c_str());
            fprintf(cuerpoDot, R"(</tr>)");

            fprintf(cuerpoDot, R"(<tr>)");
            fprintf(cuerpoDot, R"(<td bgcolor="white" border="0">s_firts_ino</td>)");
            fprintf(cuerpoDot, (R"(<td bgcolor="white" border="0">)" + to_string(sb.s_firts_ino) + "</td>)").c_str());
            fprintf(cuerpoDot, R"(</tr>)");

            fprintf(cuerpoDot, R"(<tr>)");
            fprintf(cuerpoDot, R"(<td bgcolor="green" border="0">s_first_blo</td>)");
            fprintf(cuerpoDot, (R"(<td bgcolor="green" border="0">)" + to_string(sb.s_first_blo) + "</td>)").c_str());
            fprintf(cuerpoDot, R"(</tr>)");

            fprintf(cuerpoDot, R"(<tr>)");
            fprintf(cuerpoDot, R"(<td bgcolor="white" border="0">s_bm_inode_start</td>)");
            fprintf(cuerpoDot, (R"(<td bgcolor="white" border="0">)" + to_string(sb.s_bm_inode_start) + "</td>)").c_str());
            fprintf(cuerpoDot, R"(</tr>)");

            fprintf(cuerpoDot, R"(<tr>)");
            fprintf(cuerpoDot, R"(<td bgcolor="green" border="0">s_bm_block_start</td>)");
            fprintf(cuerpoDot, (R"(<td bgcolor="green" border="0">)" + to_string(sb.s_bm_block_start) + "</td>)").c_str());
            fprintf(cuerpoDot, R"(</tr>)");

            fprintf(cuerpoDot, R"(<tr>)");
            fprintf(cuerpoDot, R"(<td bgcolor="white" border="0">s_inode_start</td>)");
            fprintf(cuerpoDot, (R"(<td bgcolor="white" border="0">)" + to_string(sb.s_inode_start) + "</td>)").c_str());
            fprintf(cuerpoDot, R"(</tr>)");

            fprintf(cuerpoDot, R"(<tr>)");
            fprintf(cuerpoDot, R"(<td bgcolor="green" border="0">s_block_start</td>)");
            fprintf(cuerpoDot, (R"(<td bgcolor="green" border="0">)" + to_string(sb.s_block_start) + "</td>)").c_str());
            fprintf(cuerpoDot, R"(</tr></table>>];})");

            fclose(cuerpoDot);
            fclose(fileDisco);
            string command="sudo -S  dot -T"+this->extension+" superbloque.dot -o \""+ this->path+"\"";
            system(command.c_str());
            cout<<"REPORTE GENERADO CON EXITO:  SB"<<endl;
        }
        else{
            cout << "EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA" << endl;
            return;
        }
    }
    else{
        cout << "LA MONTURA CON EL ID: " << this->id << " NO EXISTE O NO SE ENCUENTRA EN EL SISTEMA" << endl;
        return;
    }
}

string Rep::getNameFileInRoute(string path) {
    int p=0;
    string copia=path;
    while ((p=copia.find("/")) != string::npos){ // find retorna string::npos sino encuentra nada
        copia.erase(0, p + 1);
    }
    return copia;
}

void Rep::ejecutarReporte_Journaling() {
    Nodo_M *nodoMontura=this->mountList->buscar(this->id);
    if (nodoMontura != NULL) {
        this->extension = this->getExtensionFile(this->path);
        this->directorio = this->getCarpetas(this->path);
        system(("sudo -S mkdir -p \'" + this->directorio + "\'").c_str());
        system(("sudo -S chmod -R 777 \'" + this->directorio + "\'").c_str());
        SuperBloque superBloque;
        Journal journal;
        FILE *fileDisco;
        if ((fileDisco= fopen(nodoMontura->path.c_str(), "rb+"))){
            if (nodoMontura->type == 'l'){
                EBR ebr;
                fseek(fileDisco, nodoMontura->start, SEEK_SET);
                fread(&ebr, sizeof(EBR), 1, fileDisco);
                if (ebr.part_status != '2') {
                    fclose(fileDisco);
                    cout<<"PARTICION NO FORMATEADA, IMPOSIBLE EJECUTAR COMANDO"<<endl;
                    return;
                }
                fseek(fileDisco, nodoMontura->start + sizeof(EBR), SEEK_SET);
                fread(&superBloque, sizeof(SuperBloque), 1, fileDisco);
                if (superBloque.s_filesystem_type != 3){
                    cout << "LA PARTICION NO ESTA FORMATEADA CON EXT 3" << nodoMontura->name << endl;
                    return;
                }
                fseek(fileDisco, nodoMontura->start + sizeof(SuperBloque) + sizeof(EBR), SEEK_SET);
            }
            else if (nodoMontura->type == 'p'){
                MBR mbr;
                fseek(fileDisco, 0, SEEK_SET);
                fread(&mbr, sizeof(MBR), 1, fileDisco);
                string name1=mbr.mbr_partition[nodoMontura->pos].part_name;
                if (name1 == nodoMontura->name){
                    if (mbr.mbr_partition[nodoMontura->pos].part_status != '2'){
                        cout<<"PARTICION NO FORMATEADA, IMPOSIBLE EJECUTAR COMANDO"<<endl;
                        return;
                    }
                    fseek(fileDisco, nodoMontura->start, SEEK_SET);
                    fread(&superBloque, sizeof(SuperBloque), 1, fileDisco);
                    if (superBloque.s_filesystem_type != 3){
                        cout << "LA PARTICION NO ESTA FORMATEADA CON EXT 3" << nodoMontura->name << endl;
                        return;
                    }
                    fseek(fileDisco, nodoMontura->start + sizeof(SuperBloque), SEEK_SET);
                }else{
                    return;
                }
            }

            fread(&journal, sizeof(Journal), 1, fileDisco);
            FILE *cuerpoDot = fopen("journal.dot", "w");
            fprintf(cuerpoDot, "digraph G {\n");
            fprintf(cuerpoDot, "node[shape=none]\n");
            fprintf(cuerpoDot, "start[label=<<table CELLSPACING=\"0\">");
            fprintf(cuerpoDot, R"(<tr><td colspan="5" bgcolor="mediumslateblue" color="#2980B9">REPORTE JORUNALING</td></tr>)");
            fprintf(cuerpoDot, "\n");
            fprintf(cuerpoDot, R"(<tr><td bgcolor="mediumpurple1">Tipo Operacion</td>)");
            fprintf(cuerpoDot, R"(<td bgcolor="mediumpurple1">Tipo</td>)");
            fprintf(cuerpoDot, R"(<td bgcolor="mediumpurple1">Path</td>)");
            fprintf(cuerpoDot, R"(<td bgcolor="mediumpurple1">Contenido</td>)");
            fprintf(cuerpoDot, R"(<td bgcolor="mediumpurple1">Fecha</td></tr>)");
            fprintf(cuerpoDot, "\n");

            while (true){
                char fecha[70];
                fprintf(cuerpoDot, "<tr>");
                string aux=journal.journal_Tipo_Operacion;
                fprintf(cuerpoDot, ("<td>" + aux + "</td>").c_str());
                fprintf(cuerpoDot, ("<td>" + string(1, journal.journal_Tipo) + "</td>").c_str());
                aux=journal.journal_Path;
                fprintf(cuerpoDot, ("<td>" + aux + "</td>").c_str());
                aux=journal.journal_Contenido;
                fprintf(cuerpoDot, ("<td>" + aux + "</td>").c_str());
                strftime(fecha,sizeof (fecha),"%Y-%m-%d %H:%M:%S",localtime(&journal.journal_Fecha));
                aux=fecha;
                fprintf(cuerpoDot, ("<td>" + aux + "</td>").c_str());
                fprintf(cuerpoDot, "</tr>");

                if (journal.journal_Sig != -1){
                    fseek(fileDisco, journal.journal_Start + sizeof(Journal), SEEK_SET);
                    fread(&journal, sizeof(Journal), 1, fileDisco);
                }else{
                    break;
                }
            }

            fprintf(cuerpoDot, "</table>>];\n");
            fprintf(cuerpoDot, "}");
            fclose(fileDisco);
            fclose(cuerpoDot);
            system(("sudo -S  dot -T"+this->extension+" journal.dot -o \""+ this->path+"\"").c_str());
            cout<<"SE GENERO EL Journaling"<<endl;
        }
        else{
            cout << "EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA" << endl;
        }

    }else{
        cout << "LA MONTURA CON EL ID: " << this->id << " NO EXISTE O NO SE ENCUENTRA EN EL SISTEMA" << endl;
        return;
    }
}

void Rep::ejecutarReporte_file() {
    this->directorio = this->getCarpetas(this->path);
    this->extension = this->getExtensionFile(this->path);
    if(this->extension=="txt"){
        Nodo_M *nodoMontura=this->mountList->buscar(this->id);
        if (nodoMontura != NULL) {

            system(("sudo -S mkdir -p \'" + this->directorio + "\'").c_str());
            system(("sudo -S chmod -R 777 \'" + this->directorio + "\'").c_str());


            vector<string> rutaDividida= this->getRutaDividida(this->ruta);
            FILE *fileDisco;
            TablaInodo tablaInodo;
            if ((fileDisco= fopen(nodoMontura->path.c_str(), "rb+"))){
                if (nodoMontura->type == 'l'){
                    EBR ebr;
                    fseek(fileDisco, nodoMontura->start, SEEK_SET);
                    fread(&ebr, sizeof(EBR), 1, fileDisco);
                    if (ebr.part_status != '2') {
                        fclose(fileDisco);
                        cout<<"PARTICION NO FORMATEADA, IMPOSIBLE EJECUTAR COMANDO"<<endl;
                        return;
                    }
                    fseek(fileDisco, nodoMontura->start + sizeof(EBR), SEEK_SET);
                }
                else if (nodoMontura->type == 'p'){
                    MBR mbr;
                    fseek(fileDisco, 0, SEEK_SET);
                    fread(&mbr, sizeof(MBR), 1, fileDisco);
                    int pos=-1;
                    for (int i = 0; i < 4; ++i) {
                        string name1=mbr.mbr_partition[i].part_name;
                        if (name1 == nodoMontura->name){
                            if (mbr.mbr_partition[i].part_status!='2'){
                                cout<<"PARTICION NO FORMATEADA, IMPOSIBLE EJECUTAR COMANDO"<<endl;
                                return;
                            }
                            pos=i;
                            break;
                        }
                    }
                    if (pos!=-1){
                        fseek(fileDisco, nodoMontura->start, SEEK_SET);
                    }else{
                        cout<<"LA PARTICION NO SE HA ENCONTRADO EN EL DISCO"<<endl;
                    }
                }


                if (rutaDividida.empty()){
                    cout<<"LA RUTA INGRESADA NO ES VALIDA"<<endl;
                    fclose(fileDisco);
                    return;
                }
                // leer superbloque
                if (nodoMontura->type == 'l'){
                    fseek(fileDisco, nodoMontura->start + sizeof(EBR), SEEK_SET);
                    fread(&this->superBloqueGlobal, sizeof(SuperBloque), 1, fileDisco);
                }
                else if (nodoMontura->type == 'p'){
                    fseek(fileDisco, nodoMontura->start, SEEK_SET);
                    fread(&this->superBloqueGlobal, sizeof(SuperBloque), 1, fileDisco);
                }

                int direccionInodoFile= this->getDireccionInodo(rutaDividida, 0, rutaDividida.size() - 1,
                                                                this->superBloqueGlobal.s_inode_start, fileDisco);

                if (direccionInodoFile == -1){
                    cout << "EL ARCHIVO NO SE PUDO ENCONTRAR CON LA RUTA ESPECIFICADA" << endl;
                    return;
                }

                fseek(fileDisco, direccionInodoFile, SEEK_SET);
                fread(&tablaInodo, sizeof(TablaInodo), 1, fileDisco);
                tablaInodo.i_atime= time(nullptr);
                fseek(fileDisco, direccionInodoFile, SEEK_SET);
                fwrite(&tablaInodo, sizeof(TablaInodo), 1, fileDisco);
                string stringAlmacenadoInodo = this->getStringAlmacenadoInodo(direccionInodoFile, fileDisco);
                FILE *cuerpoDot= fopen(this->path.c_str(), "w");


                fprintf(cuerpoDot, (rutaDividida[rutaDividida.size() - 1] + "\n").c_str());
                fprintf(cuerpoDot, stringAlmacenadoInodo.c_str());
                fclose(cuerpoDot);
                fclose(fileDisco);
                cout << "REPORTE GENERADO CON EXITO:  FILE DE " << rutaDividida[rutaDividida.size() - 1] << endl;

            }
            else{
                cout << "EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA" << endl;
            }
        }
        else{
            cout << "LA MONTURA CON EL ID: " << this->id << " NO EXISTE O NO SE ENCUENTRA EN EL SISTEMA" << endl;
            return;
        }
    }
    else{
        cout << "EL REPORTE FILE SOLO PUEDE SER EN FORMATO TXT" << endl;
    }
}

vector<string> Rep::getRutaDividida(std::string cadena) {
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
int Rep::getDireccionInodo(vector<string> rutaDividida, int direccionActual, int numCarpetas, int rutaActual, FILE *discoActual) {
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

//Método para leer el contenido de un Inodo.
string Rep::getStringAlmacenadoInodo(int direccionInodoStart, FILE *fileDisco){
    string contenido="";
    BloqueApuntador b_Apuntador1,b_Apuntador2,b_Apuntador3;
    BloqueArchivo b_Archivo;
    TablaInodo tablaInodo;
    fseek(fileDisco, direccionInodoStart, SEEK_SET);
    fread(&tablaInodo, sizeof(TablaInodo), 1, fileDisco);
    for (int i = 0; i < 15; ++i) {
        if (tablaInodo.i_block[i] != -1){ // si, si poseen información
            if (i<12){ // apuntadores directos - leer el bloque de archivos
                fseek(fileDisco, tablaInodo.i_block[i], SEEK_SET); // nos posicionamos en la dirección de cada bloque de archivos
                fread(&b_Archivo, sizeof(BloqueArchivo), 1, fileDisco);
                for (int j = 0; j < 64; ++j) {
                    if (b_Archivo.b_content[j] == '\000'){ // si encuentra un dato nulo
                        break;
                    }
                    contenido+=b_Archivo.b_content[j]; // ir concatenando la información
                }
            }
            else if (i==12){ // apuntador simple - leer el bloque de apuntadores - leer el bloque de archivos
                // Leer el bloque de apuntadores
                fseek(fileDisco, tablaInodo.i_block[i], SEEK_SET);
                fread(&b_Apuntador1, sizeof(BloqueApuntador), 1, fileDisco);
                for (int j = 0; j < 16; ++j) { // leer apuntadores directos
                    if (b_Apuntador1.b_pointers[j] != -1){
                        fseek(fileDisco, b_Apuntador1.b_pointers[j], SEEK_SET);
                        fread(&b_Archivo, sizeof(BloqueArchivo), 1, fileDisco);
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
                fseek(fileDisco, tablaInodo.i_block[i], SEEK_SET);
                fread(&b_Apuntador1, sizeof(BloqueApuntador), 1, fileDisco);
                for (int j = 0; j < 16; ++j) { // leer apuntador simple
                    if (b_Apuntador1.b_pointers[j] != -1){
                        fseek(fileDisco, b_Apuntador1.b_pointers[j], SEEK_SET);
                        fread(&b_Apuntador2, sizeof(BloqueApuntador), 1, fileDisco);
                        for (int k = 0; k < 16; ++k) { // leer apuntador directo
                            if (b_Apuntador2.b_pointers[k] != -1){
                                fseek(fileDisco, b_Apuntador2.b_pointers[k], SEEK_SET);
                                fread(&b_Archivo, sizeof(BloqueArchivo), 1, fileDisco);
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
                fseek(fileDisco, tablaInodo.i_block[i], SEEK_SET);
                fread(&b_Apuntador1, sizeof(BloqueApuntador), 1, fileDisco);
                for (int j = 0; j < 16; ++j) { // leer apuntador doble
                    if (b_Apuntador1.b_pointers[j] != -1){
                        fseek(fileDisco, b_Apuntador1.b_pointers[j], SEEK_SET);
                        fread(&b_Apuntador2, sizeof(BloqueApuntador), 1, fileDisco);
                        for (int k = 0; k < 16; ++k) { // leer apuntador simple
                            if (b_Apuntador2.b_pointers[k] != -1){
                                fseek(fileDisco, b_Apuntador2.b_pointers[k], SEEK_SET);
                                fread(&b_Apuntador3, sizeof(BloqueApuntador), 1, fileDisco);
                                for (int z = 0; z < 16; ++z) { // leer apuntador directo
                                    fseek(fileDisco, b_Apuntador3.b_pointers[z], SEEK_SET);
                                    fread(&b_Archivo, sizeof(BloqueArchivo), 1, fileDisco);
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

void Rep::ejecutarReporte_ls() {
        Nodo_M *nodoMontura=this->mountList->buscar(this->id);
        if (nodoMontura != NULL) {

            this->extension = this->getExtensionFile(this->path);
            this->directorio = this->getCarpetas(this->path);
            system(("sudo -S mkdir -p \'" + this->directorio + "\'").c_str());
            system(("sudo -S chmod -R 777 \'" + this->directorio + "\'").c_str());

            FILE *fileDisco;
            vector<string> rutaDividida;
            TablaInodo tablaInodo;
            if ((fileDisco= fopen(nodoMontura->path.c_str(), "rb+"))){
                // Ubicar en las particiones
                if (nodoMontura->type == 'l'){
                    EBR ebr;
                    fseek(fileDisco, nodoMontura->start, SEEK_SET);
                    fread(&ebr, sizeof(EBR), 1, fileDisco);
                    if (ebr.part_status != '2') {
                        fclose(fileDisco);
                        cout<<"PARTICION NO FORMATEADA, IMPOSIBLE EJECUTAR COMANDO"<<endl;
                        return;
                    }
                    fseek(fileDisco, nodoMontura->start + sizeof(EBR), SEEK_SET);
                }
                else if (nodoMontura->type == 'p'){
                    MBR mbr;
                    fseek(fileDisco, 0, SEEK_SET);
                    fread(&mbr, sizeof(MBR), 1, fileDisco);
                    int pos=-1;
                    for (int i = 0; i < 4; ++i) {
                        string name1=mbr.mbr_partition[i].part_name;
                        if (name1 == nodoMontura->name){
                            if (mbr.mbr_partition[i].part_status!='2'){
                                cout<<"PARTICION NO FORMATEADA, IMPOSIBLE EJECUTAR COMANDO"<<endl;
                                return;
                            }
                            pos=i;
                            break;
                        }
                    }
                    if (pos!=-1){
                        fseek(fileDisco, nodoMontura->start, SEEK_SET);
                    }else{
                        cout<<"LA PARTICION NO SE HA ENCONTRADO EN EL DISCO"<<endl;
                    }
                }

                // leer el superbloque
                if (nodoMontura->type == 'l'){
                    fseek(fileDisco, nodoMontura->start + sizeof(EBR), SEEK_SET);
                    fread(&this->superBloqueGlobal, sizeof(SuperBloque), 1, fileDisco);
                }
                else if (nodoMontura->type == 'p'){
                    fseek(fileDisco, nodoMontura->start, SEEK_SET);
                    fread(&this->superBloqueGlobal, sizeof(SuperBloque), 1, fileDisco);
                }



                int direccionInodoFile=0;
                if (this->ruta!="/"){
                    rutaDividida= this->getRutaDividida(this->ruta);
                    if (rutaDividida.empty()){
                        cout<<"LA RUTA INGRESADA NO ES VALIDA"<<endl;
                        fclose(fileDisco);
                        return;
                    }
                    direccionInodoFile= this->getDireccionInodo(rutaDividida,
                                                                0,
                                                                rutaDividida.size() - 1,
                                                                this->superBloqueGlobal.s_inode_start,
                                                                fileDisco);
                }
                else{
                    direccionInodoFile=this->superBloqueGlobal.s_inode_start;
                    rutaDividida.push_back("/");
                }

                if (direccionInodoFile == -1){
                    cout << "EL ARCHIVO INDICADO NO EXISTE O NO SE ENCONTRO" << endl;
                    return;
                }

                fseek(fileDisco, direccionInodoFile, SEEK_SET);
                fread(&tablaInodo, sizeof(TablaInodo), 1, fileDisco);
                tablaInodo.i_atime= time(nullptr);
                fseek(fileDisco, direccionInodoFile, SEEK_SET);
                fwrite(&tablaInodo, sizeof(TablaInodo), 1, fileDisco);

                FILE *cuerpoDot= fopen("ls.dot", "w");

                fprintf(cuerpoDot, "digraph G {\n");
                fprintf(cuerpoDot, ("node[shape=none]\n"));
                fprintf(cuerpoDot, "start[label=<<table CELLSPACING=\"0\">\n");

                fprintf(cuerpoDot, "<tr>\n");
                fprintf(cuerpoDot, "<td><b>Permisos</b></td>\n");
                fprintf(cuerpoDot, "<td><b>Owner</b></td>\n");
                fprintf(cuerpoDot, "<td><b>Grupo</b></td>\n");
                fprintf(cuerpoDot, "<td><b>Size (en Bytes)</b></td>\n");
                fprintf(cuerpoDot, "<td><b>Fecha</b></td>\n");
                fprintf(cuerpoDot, "<td><b>Tipo</b></td>\n");
                fprintf(cuerpoDot, "<td><b>Name</b></td>\n");
                fprintf(cuerpoDot, "</tr>\n");

                fprintf(cuerpoDot,
                        this->graficarInodoRecursivoLs(direccionInodoFile, rutaDividida[rutaDividida.size() - 1],
                                                       fileDisco).c_str());

                fprintf(cuerpoDot, R"("</table>>])");
                fprintf(cuerpoDot, "}");
                fclose(cuerpoDot);
                fclose(fileDisco);
                system(("sudo -S  dot -T"+this->extension+" ls.dot -o \""+ this->path+"\"").c_str());
                cout << "REPORTE GENERADO CON EXITO:  LS DE " << rutaDividida[rutaDividida.size() - 1] << endl;

            }
            else{
                cout << "EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA" << endl;
            }
        }
        else{
            cout << "LA MONTURA CON EL ID: " << this->id << " NO EXISTE O NO SE ENCUENTRA EN EL SISTEMA" << endl;
            return;
        }
}

string Rep::graficarInodoRecursivoLs(int direccionInodo, string nombreFile, FILE *fileDisco) {
    TablaInodo tablaInodo;
    char fecha[100];
    fseek(fileDisco, direccionInodo, SEEK_SET);
    fread(&tablaInodo, sizeof(TablaInodo), 1, fileDisco);
    string cuerpoDot="";
    cuerpoDot+="<tr>\n";
    string permisosInodo = this->getPermisosInodo(tablaInodo.i_perm);
    string usuarioInodo = this->getUsuarioInodo(tablaInodo.i_uid, fileDisco);
    string grupoInodo = this->getGrupoInodo(tablaInodo.i_gid, fileDisco);
    string sizeInodo = to_string(tablaInodo.i_s);
    cuerpoDot+= "<td>" + permisosInodo + "</td>\n";
    cuerpoDot+= "<td>" + usuarioInodo + "</td>\n";
    cuerpoDot+= "<td>" + grupoInodo + "</td>\n";
    cuerpoDot+= "<td>" + sizeInodo + "</td>\n";
    strftime(fecha,sizeof (fecha),"%Y-%m-%d %H:%M:%S",localtime(&tablaInodo.i_mtime));
    string copiaFecha=fecha;
    cuerpoDot+= "<td>" + copiaFecha + "</td>\n";
    if (tablaInodo.i_type == '1'){
        cuerpoDot+="<td>Archivo</td>\n";
        cuerpoDot+= "<td>" + nombreFile + "</td>\n";
        cuerpoDot+="</tr>";
        return cuerpoDot;
    }

    cuerpoDot+="<td>Carpeta</td>\n";
    cuerpoDot+= "<td>" + nombreFile + "</td>\n";
    cuerpoDot+="</tr>\n";
    BloqueCarpeta blkCarpeta;
    BloqueApuntador blkApuntador_1,blkApuntador_2,blkApuntador_3;

    for (int i = 0; i < 15; ++i) {
        if (tablaInodo.i_block[i] != -1){
            if (i<12){
                fseek(fileDisco, tablaInodo.i_block[i], SEEK_SET);
                fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, fileDisco);
                for (int j = 0; j < 4; ++j) {
                    if (blkCarpeta.b_content[j].b_inodo != -1){
                        string nombreFile_Next="";
                        for (int l = 0; l < 12; ++l) {
                            if (blkCarpeta.b_content[j].b_name[l] == '\000'){
                                break;
                            }
                            nombreFile_Next+=blkCarpeta.b_content[j].b_name[l];
                        }
                        if (!(nombreFile_Next == "." || nombreFile_Next == "..")){ // estos no son nombres de estructuras como tal
                            cuerpoDot+= this->graficarInodoRecursivoLs(blkCarpeta.b_content[j].b_inodo,
                                                                       nombreFile_Next,
                                                                       fileDisco);
                        }
                    }
                }
            }
            else if (i==12){
                fseek(fileDisco, tablaInodo.i_block[i], SEEK_SET);
                fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, fileDisco);
                for (int j = 0; j < 16; ++j) {
                    if (blkApuntador_1.b_pointers[j] != -1){
                        fseek(fileDisco, blkApuntador_1.b_pointers[j], SEEK_SET);
                        fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, fileDisco);
                        for (int k = 0; k < 4; ++k) {
                            if (blkCarpeta.b_content[k].b_inodo != -1){
                                string nombreFile_Next="";
                                for (int z = 0; z < 12; ++z) {
                                    if (blkCarpeta.b_content[k].b_name[z] == '\000'){
                                        break;
                                    }
                                    nombreFile_Next+=blkCarpeta.b_content[k].b_name[z];
                                }
                                cuerpoDot+= this->graficarInodoRecursivoLs(blkCarpeta.b_content[k].b_inodo,
                                                                           nombreFile_Next,
                                                                           fileDisco);
                            }
                        }
                    }
                }
            }
            else if (i==13){
                        fseek(fileDisco, tablaInodo.i_block[i], SEEK_SET);
                        fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, fileDisco);
                        for (int j = 0; j < 16; ++j) {
                            if (blkApuntador_1.b_pointers[j] != -1){
                                fseek(fileDisco, blkApuntador_1.b_pointers[j], SEEK_SET);
                                fread(&blkApuntador_2, sizeof(BloqueApuntador), 1, fileDisco);
                                for (int k = 0; k < 16; ++k) {
                                    if (blkApuntador_2.b_pointers[k] != -1){
                                        fseek(fileDisco, blkApuntador_2.b_pointers[j], SEEK_SET);
                                        fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, fileDisco);
                                        for (int z = 0; z < 4; ++z) {
                                            if (blkCarpeta.b_content[z].b_inodo != -1){
                                                string nombreFile_Next="";
                                                for (int y = 0; y < 12; ++y) {
                                                    if (blkCarpeta.b_content[z].b_name[y] == '\000'){
                                                        break;
                                                    }
                                                    nombreFile_Next+=blkCarpeta.b_content[z].b_name[y];
                                                }
                                                cuerpoDot+= this->graficarInodoRecursivoLs(blkCarpeta.b_content[z].b_inodo,
                                                                                           nombreFile_Next,
                                                                                           fileDisco);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
            else if (i==14){
                        fseek(fileDisco, tablaInodo.i_block[i], SEEK_SET);
                        fread(&blkApuntador_1, sizeof(BloqueApuntador), 1, fileDisco);
                        for (int j = 0; j < 16; ++j) {
                            if (blkApuntador_1.b_pointers[j] != -1){
                                fseek(fileDisco, blkApuntador_1.b_pointers[j], SEEK_SET);
                                fread(&blkApuntador_2, sizeof(BloqueApuntador), 1, fileDisco);
                                for (int k = 0; k < 16; ++k) {
                                    if (blkApuntador_2.b_pointers[k] != -1){
                                        fseek(fileDisco, blkApuntador_2.b_pointers[k], SEEK_SET);
                                        fread(&blkApuntador_3, sizeof(BloqueApuntador), 1, fileDisco);
                                        for (int z = 0; z < 16; ++z) {
                                            if (blkApuntador_3.b_pointers[z] != -1){
                                                fseek(fileDisco, blkApuntador_3.b_pointers[j], SEEK_SET);
                                                fread(&blkCarpeta, sizeof(BloqueCarpeta), 1, fileDisco);
                                                for (int y = 0; y < 4; ++y) {
                                                    if (blkCarpeta.b_content[y].b_inodo != -1){
                                                        string name1="";
                                                        for (int x = 0; x < 12; ++x) {
                                                            if (blkCarpeta.b_content[y].b_name[x] == '\000'){
                                                                break;
                                                            }
                                                            name1+=blkCarpeta.b_content[y].b_name[x];
                                                        }
                                                        cuerpoDot+= this->graficarInodoRecursivoLs(blkCarpeta.b_content[y].b_inodo,
                                                                                                   name1,
                                                                                                   fileDisco);
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

    return cuerpoDot;
}

string Rep::getPermisosInodo(int permisoInodo) {
    string cuerpoDot="";
    string permisosString= to_string(permisoInodo);
    for (int i = 0; i < permisosString.length(); ++i) {
        cuerpoDot+="-";
        if ((permisosString[i] == '4') || (permisosString[i] == '5') || (permisosString[i] == '6') || (permisosString[i] == '7')){
            cuerpoDot+="r";
        }
        if ((permisosString[i] == '2') || (permisosString[i] == '3') || (permisosString[i] == '6') || (permisosString[i] == '7')){
            cuerpoDot+="w";
        }
        if ((permisosString[i] == '1') || (permisosString[i] == '3') || (permisosString[i] == '5') || (permisosString[i] == '7')){
            cuerpoDot+="x";
        }
    }
    return cuerpoDot;
}

string Rep::getUsuarioInodo(int idUsuario, FILE *fileDisco) {
    int contadorUsuarios=0;
    string user;
    string stringUsers= this->getStringAlmacenadoInodo(this->superBloqueGlobal.s_inode_start + sizeof(TablaInodo), fileDisco);
    stringstream ss(stringUsers);

    while (getline(ss, user, '\n')){
        if (user != ""){
            vector<string> camposUsuario= this->getCampos(user);
            if (camposUsuario[1] == "U" && camposUsuario[0] == to_string(idUsuario)){
                return camposUsuario[3];
            }
        }
    }
    return "NOT FOUND";
}

string Rep::getGrupoInodo(int idGrupo, FILE *fileDisco) {

    int contadorGrupos=0;
    string grupo;
    string stringUsers= this->getStringAlmacenadoInodo(this->superBloqueGlobal.s_inode_start + sizeof(TablaInodo), fileDisco);
    stringstream ss(stringUsers);

    while (getline(ss, grupo, '\n')){
        if (grupo != ""){
            vector<string> camposGrupo= this->getCampos(grupo);
            if (camposGrupo[1] == "G" && camposGrupo[0] == to_string(idGrupo)){
                return camposGrupo[2];
            }
        }
    }

    return "NOT FOUND";
}

vector<string> Rep::getCampos(std::string entrada) {
    vector<string> campos;
    string copiaEntrada=entrada;
    stringstream ss(entrada);
    string campo;

    while (getline(ss, campo, ',')){
        if (campo != "")campos.push_back(campo);
    }
    return campos;
}

void Rep::ejecutarReporte_tree() {
    Nodo_M *nodoMontura=this->mountList->buscar(this->id);
    if (nodoMontura != NULL) {
        this->extension = this->getExtensionFile(this->path);
        this->directorio = this->getCarpetas(this->path);

        system(("sudo -S mkdir -p \'" + this->directorio + "\'").c_str());
        system(("sudo -S chmod -R 777 \'" + this->directorio + "\'").c_str());

        SuperBloque superBloqueReporteTree;
        FILE *fileDisco;
        if ((fileDisco= fopen(nodoMontura->path.c_str(), "rb+"))){
            if (nodoMontura->type == 'l'){
                fseek(fileDisco, nodoMontura->start + sizeof(EBR), SEEK_SET);
            }
            else if (nodoMontura->type == 'p'){
                fseek(fileDisco, nodoMontura->start, SEEK_SET);
            }
            fread(&superBloqueReporteTree, sizeof(SuperBloque), 1, fileDisco);
            int bitMapInodos_Start=superBloqueReporteTree.s_bm_inode_start;
            int bitMapInodos_End= bitMapInodos_Start + superBloqueReporteTree.s_inodes_count;
            BloqueApuntador blkApuntador1,blkApuntador2,blkApuntador3;
            TablaInodo tablaInodo;
            char date[70];
            char bit;
            string date2="";
            int contador=0;

            FILE *dot= fopen("tree.dot","w");
            fprintf(dot,"digraph G {\n");
            fprintf(dot, "rankdir=LR;\n");
            fprintf(dot, "node[shape=none]\n");

            for (int i = bitMapInodos_Start; i < bitMapInodos_End; ++i) {
                fseek(fileDisco, i, SEEK_SET);
                fread(&bit, sizeof(char), 1, fileDisco);
                if (bit=='1'){
                    int direccionInodo=(superBloqueReporteTree.s_inode_start + (contador * sizeof(TablaInodo)));
                    fseek(fileDisco, direccionInodo, SEEK_SET);
                    fread(&tablaInodo, sizeof(TablaInodo), 1, fileDisco);
                    fprintf(dot, dibujarInodoReporteTree(direccionInodo, fileDisco).c_str());
                    for (int i = 0; i < 15; ++i) {
                        if (tablaInodo.i_block[i] != -1){
                            if (i<12){
                                if (tablaInodo.i_type == '0'){
                                    fprintf(dot, dibujarBlkReporteTree(tablaInodo.i_block[i], 0, fileDisco).c_str());
                                }
                                else if (tablaInodo.i_type == '1'){
                                    fprintf(dot, dibujarBlkReporteTree(tablaInodo.i_block[i], 1, fileDisco).c_str());
                                }
                                fprintf(dot, this->dibujarEnlacesTree(direccionInodo, tablaInodo.i_block[i]).c_str());
                            }
                            else if (i==12){
                                fseek(fileDisco, tablaInodo.i_block[i], SEEK_SET);
                                fread(&blkApuntador1, sizeof(BloqueApuntador), 1, fileDisco);
                                fprintf(dot, dibujarBlkReporteTree(tablaInodo.i_block[i], 2, fileDisco).c_str());
                                fprintf(dot, this->dibujarEnlacesTree(direccionInodo, tablaInodo.i_block[i]).c_str());
                                for (int j = 0; j < 16; ++j) {
                                    if (blkApuntador1.b_pointers[j] != -1){
                                        if (tablaInodo.i_type == '0'){
                                            fprintf(dot, dibujarBlkReporteTree(blkApuntador1.b_pointers[j], 0, fileDisco).c_str());
                                        }
                                        else if (tablaInodo.i_type == '1'){
                                            fprintf(dot, dibujarBlkReporteTree(blkApuntador1.b_pointers[j], 1, fileDisco).c_str());
                                        }
                                        fprintf(dot, this->dibujarEnlacesTree(tablaInodo.i_block[i],
                                                                              blkApuntador1.b_pointers[j]).c_str());
                                    }
                                }
                            }
                            else if (i==13){
                                fseek(fileDisco, tablaInodo.i_block[i], SEEK_SET);
                                fread(&blkApuntador1, sizeof(BloqueApuntador), 1, fileDisco);
                                fprintf(dot, dibujarBlkReporteTree(tablaInodo.i_block[i], 2, fileDisco).c_str());
                                fprintf(dot, this->dibujarEnlacesTree(direccionInodo, tablaInodo.i_block[i]).c_str());
                                for (int j = 0; j < 16; ++j) {
                                    if (blkApuntador1.b_pointers[j] != -1){
                                        fseek(fileDisco, blkApuntador1.b_pointers[j], SEEK_SET);
                                        fread(&blkApuntador2, sizeof(BloqueApuntador), 1, fileDisco);
                                        fprintf(dot, dibujarBlkReporteTree(blkApuntador1.b_pointers[j], 2, fileDisco).c_str());
                                        fprintf(dot, this->dibujarEnlacesTree(tablaInodo.i_block[i],
                                                                              blkApuntador1.b_pointers[j]).c_str());
                                        for (int k = 0; k < 16; ++k) {
                                            if (blkApuntador2.b_pointers[k] != -1){
                                                if (tablaInodo.i_type == '0'){
                                                    fprintf(dot,
                                                            dibujarBlkReporteTree(blkApuntador2.b_pointers[k], 0, fileDisco).c_str());
                                                }
                                                else if (tablaInodo.i_type == '1'){
                                                    fprintf(dot,
                                                            dibujarBlkReporteTree(blkApuntador2.b_pointers[k], 1, fileDisco).c_str());
                                                }
                                                fprintf(dot, this->dibujarEnlacesTree(blkApuntador1.b_pointers[j],
                                                                                      blkApuntador2.b_pointers[k]).c_str());
                                            }
                                        }
                                    }
                                }
                            }
                            else if (i==14){
                                fseek(fileDisco, tablaInodo.i_block[i], SEEK_SET);
                                fread(&blkApuntador1, sizeof(BloqueApuntador), 1, fileDisco);
                                fprintf(dot, dibujarBlkReporteTree(tablaInodo.i_block[i], 2, fileDisco).c_str());
                                fprintf(dot, this->dibujarEnlacesTree(direccionInodo, tablaInodo.i_block[i]).c_str());
                                for (int j = 0; j < 16; ++j) {
                                    if (blkApuntador1.b_pointers[j] != -1){
                                        fseek(fileDisco, blkApuntador1.b_pointers[j], SEEK_SET);
                                        fread(&blkApuntador2, sizeof(BloqueApuntador), 1, fileDisco);
                                        fprintf(dot, dibujarBlkReporteTree(blkApuntador1.b_pointers[j], 2, fileDisco).c_str());
                                        fprintf(dot, this->dibujarEnlacesTree(tablaInodo.i_block[i],
                                                                              blkApuntador1.b_pointers[j]).c_str());
                                        for (int k = 0; k < 16; ++k) {
                                            if (blkApuntador2.b_pointers[k] != -1){
                                                fseek(fileDisco, blkApuntador2.b_pointers[k], SEEK_SET);
                                                fread(&blkApuntador3, sizeof(BloqueApuntador), 1, fileDisco);
                                                fprintf(dot, dibujarBlkReporteTree(blkApuntador2.b_pointers[k], 2, fileDisco).c_str());
                                                fprintf(dot, this->dibujarEnlacesTree(blkApuntador1.b_pointers[j],
                                                                                      blkApuntador2.b_pointers[k]).c_str());
                                                for (int z = 0; z < 16; ++z) {
                                                    if (blkApuntador3.b_pointers[z] != -1){
                                                        if (tablaInodo.i_type == '0'){
                                                            fprintf(dot,
                                                                    dibujarBlkReporteTree(blkApuntador3.b_pointers[z],
                                                                                          0, fileDisco).c_str());
                                                        }
                                                        else if (tablaInodo.i_type == '1'){
                                                            fprintf(dot,
                                                                    dibujarBlkReporteTree(blkApuntador3.b_pointers[z],
                                                                                          1, fileDisco).c_str());
                                                        }
                                                        fprintf(dot,
                                                                this->dibujarEnlacesTree(blkApuntador2.b_pointers[k],
                                                                                         blkApuntador3.b_pointers[z]).c_str());
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
                contador++;
            }

            fprintf(dot,"}");
            fclose(dot);
            fclose(fileDisco);
            string command="sudo -S  dot -T"+this->extension+" tree.dot -o \""+ this->path+"\"";
            system(command.c_str());
            cout<<"COMANDO EJECUTADO CON EXITO, REPORTE TREE GENERADO"<<endl;

        }
        else{
            cout <<"EL DISCO SE MOVIO DE RUTA O NO EXISTE"<<endl;
            return;
        }
    }
    else{
        cout << "LA MONTURA CON EL ID: " << this->id << " NO EXISTE O NO SE ENCUENTRA EN EL SISTEMA" << endl;
        return;
    }
}

string Rep::dibujarInodoReporteTree(int direccionInodo, FILE *disco) {

    char date[70];
    string date2="";
    string cadenaDot="";

    TablaInodo tablaInodo;
    fseek(disco, direccionInodo, SEEK_SET);
    fread(&tablaInodo, sizeof(TablaInodo), 1, disco);

    cadenaDot+= "n" + to_string(direccionInodo) + "[label=<<table CELLSPACING=\"0\">";
    cadenaDot+= "<tr><td colspan=\"2\" border=\"0\">"+ to_string(direccionInodo) + "</td></tr>";
    cadenaDot+= R"(<tr><td colspan="2" bgcolor="lightskyblue"> TABLA INODO )" + to_string(direccionInodo) + "</td></tr>\n";

    cadenaDot+="<tr>\n";
    cadenaDot+="<td>i_uid</td>\n";
    cadenaDot+= "<td>" + to_string(tablaInodo.i_uid) + "</td>\n";
    cadenaDot+="</tr>\n";

    cadenaDot+="<tr>\n";
    cadenaDot+="<td>i_gid</td>\n";
    cadenaDot+= "<td>" + to_string(tablaInodo.i_gid) + "</td>\n";
    cadenaDot+="</tr>\n";

    cadenaDot+="<tr>\n";
    cadenaDot+="<td>i_s</td>\n";
    cadenaDot+= "<td>" + to_string(tablaInodo.i_s) + "</td>\n";
    cadenaDot+="</tr>\n";

    strftime(date, sizeof (date), "%Y-%m-%d %H:%M:%S", localtime(&tablaInodo.i_atime));
    date2=date;
    cadenaDot+="<tr>\n";
    cadenaDot+="<td>i_atime</td>\n";
    cadenaDot+= "<td>" + date2 + "</td>\n";
    cadenaDot+="</tr>\n";

    strftime(date, sizeof (date), "%Y-%m-%d %H:%M:%S", localtime(&tablaInodo.i_ctime));
    date2=date;
    cadenaDot+="<tr>\n";
    cadenaDot+="<td>i_ctime</td>\n";
    cadenaDot+= "<td>" + date2 + "</td>\n";
    cadenaDot+="</tr>\n";

    strftime(date, sizeof (date), "%Y-%m-%d %H:%M:%S", localtime(&tablaInodo.i_mtime));
    date2=date;
    cadenaDot+="<tr>\n";
    cadenaDot+="<td>i_mtime</td>\n";
    cadenaDot+= "<td>" + date2 + "</td>\n";
    cadenaDot+="</tr>\n";

    for (int j = 0; j < 15; ++j) {
        if (tablaInodo.i_block[j] != -1){
            cadenaDot+="<tr>\n";
            cadenaDot+= "<td>ap" + to_string(j) + "</td>\n";
            cadenaDot+= "<td port=\"" + to_string(tablaInodo.i_block[j]) + "\">" + to_string(tablaInodo.i_block[j]) + "</td>\n";
            cadenaDot+="</tr>\n";
        }
        else{
            cadenaDot+="<tr>\n";
            cadenaDot+="<td>i_block</td>\n";
            cadenaDot+="<td>-1</td>\n";
            cadenaDot+="</tr>\n";
        }

    }

    cadenaDot+="<tr>\n";
    cadenaDot+="<td>i_type</td>\n";
    cadenaDot+= "<td>" + string(1, tablaInodo.i_type) + "</td>\n";
    cadenaDot+="</tr>\n";

    cadenaDot+="<tr>\n";
    cadenaDot+="<td>i_perm</td>\n";
    cadenaDot+= "<td>" + to_string(tablaInodo.i_perm) + "</td>\n";
    cadenaDot+="</tr>\n";

    cadenaDot+="</table>>]\n";
    return cadenaDot;
}

string Rep::dibujarBlkReporteTree(int pos, int type, FILE *file) {
    string cuerpoDot="";
    if (type==0){
        BloqueCarpeta carpeta;
        fseek(file,pos,SEEK_SET);
        fread(&carpeta, sizeof(BloqueCarpeta),1,file);

        cuerpoDot+= "n" + to_string(pos) + "[label=<<table CELLSPACING=\"0\">\n";
        cuerpoDot+="<tr>\n";
        cuerpoDot+=R"(<td colspan="2" bgcolor="lightcoral">Bloque Carpeta</td>)";
        cuerpoDot+="</tr>\n";
        for (int i = 0; i < 4; ++i) {
            string b_name="";
            for (int j = 0; j < 12; ++j) {
                if (carpeta.b_content[i].b_name[j]=='\000'){
                    break;
                }
                b_name+=carpeta.b_content[i].b_name[j];
            }

            cuerpoDot+="<tr>\n";
            cuerpoDot+= "<td>" + b_name + "</td>\n";
            cuerpoDot+= "<td port=\"" + to_string(carpeta.b_content[i].b_inodo) + "\">" + to_string(carpeta.b_content[i].b_inodo) + "</td>\n";
            cuerpoDot+="</tr>\n";
        }
        cuerpoDot+="</table>>]\n";

        for (int i = 0; i < 4; ++i) {
            string name1=carpeta.b_content[i].b_name;
            if (carpeta.b_content[i].b_inodo!=-1 && (name1!="." && name1!="..")){
                cuerpoDot+= dibujarEnlacesTree(pos, carpeta.b_content[i].b_inodo);
            }
        }
    }
    else if (type==1){
        BloqueArchivo blkArchivo;
        string contenidoBlkArchivo="";

        fseek(file,pos,SEEK_SET);
        fread(&blkArchivo, sizeof(BloqueArchivo), 1, file);

        for (int i = 0; i < 64; ++i) {
            if (blkArchivo.b_content[i] == '\000'){
                break;
            }
            contenidoBlkArchivo+=blkArchivo.b_content[i];
        }

        cuerpoDot+= "n" + to_string(pos) + "[label=<<table CELLSPACING=\"0\">\n";
        cuerpoDot+="<tr>\n";
        cuerpoDot+=R"(<td colspan="2" bgcolor="lemonchiffon">Bloque Archivo</td>)";
        cuerpoDot+="</tr>\n";
        cuerpoDot+="<tr>\n";
        cuerpoDot+= "<td>" + contenidoBlkArchivo + "</td>\n";
        cuerpoDot+="</tr>\n";
        cuerpoDot+="</table>>]\n";

    }
    else{
        BloqueApuntador blkApuntador;
        fseek(file,pos,SEEK_SET);
        fread(&blkApuntador, sizeof(BloqueApuntador), 1, file);

        cuerpoDot+= "n" + to_string(pos) + "[label=<<table CELLSPACING=\"0\">\n";
        cuerpoDot+="<tr>\n";
        cuerpoDot+=R"(<td colspan="2" bgcolor="lightcoral">Bloque de Apuntadores</td>)";
        cuerpoDot+="</tr>\n";
        for (int i = 0; i < 16; ++i) {
            cuerpoDot+="<tr>\n";
            cuerpoDot+= "<td>b_pointer " + to_string(i) + "</td>\n";
            cuerpoDot+= "<td>" + to_string(blkApuntador.b_pointers[i]) + "</td>\n";
            cuerpoDot+="</tr>\n";
        }
        cuerpoDot+="</table>>]\n";
    }
    return cuerpoDot;
}

string Rep::dibujarEnlacesTree(int inicio, int final) {
    string cuerpoDot="";
    cuerpoDot+= "n" + to_string(inicio) + ":" + to_string(final) + "->n" + to_string(final) + ";\n";
    return cuerpoDot;
}



