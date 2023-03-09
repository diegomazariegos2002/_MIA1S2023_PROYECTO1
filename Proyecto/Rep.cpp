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
            else if (this->name=="Journaling"){
                this->ejecutarReporte_Journaling();
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
            else if (this->name=="tree"){
                this->ejecutarReporte_tree();
            }
            else if (this->name=="sb"){
                this->ejecutarReporte_sb();
            }
            else if (this->name=="file"){
                this->ejecutarReporte_file();
            }
            else if (this->name=="ls"){
                this->ejecutarReporte_ls();
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
                "start[label=<<table>"
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
            fprintf(fileDot, "start[label=<<table><tr>");
            fprintf(fileDot, "<td rowspan=\"2\">MBR</td>");

            int start=sizeof(MBR);
            int i=0;

            while (i<4){
                if (mbr.mbr_partition[i].part_start == -1){ // ESTA LIBRE
                    i++;
                    while (i<4){
                        if (mbr.mbr_partition[i].part_start!=-1){
                            int porcentaje= ((mbr.mbr_partition[i].part_start - start) / (size * 1.0)) * 100.0;
                            fprintf(fileDot, ("<td rowspan=\"2\">LIBRE <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());
                            break;
                        }
                        i++;
                    }
                    if (i==4){
                        float porcentaje= (((size - start) * 1.0) / size) * (100.0);
                        fprintf(fileDot, ("<td rowspan=\"2\">LIBRE <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());
                        goto salida1;
                    }
                    i--;

                }
                else{ // ESTA OCUPADA
                    if (mbr.mbr_partition[i].part_type=='e'){
                        float porcentaje= ((mbr.mbr_partition[i].part_s) / size) * 100.0;
                        fprintf(fileDot, "<td colspan=\"100\">EXTENDIDA</td>");
                        EBR ebr;
                        fseek(fileReporte, mbr.mbr_partition[i].part_start, SEEK_SET);
                        fread(&ebr, sizeof(EBR), 1, fileReporte);
                        if (!(ebr.part_s==-1 && ebr.part_next==-1)){
                            string nombre_1=ebr.part_name;
                            if (ebr.part_s>-1){
                                fprintf(fileDot, ("<td rowspan=\"2\">EBR <br/>" + nombre_1 + "</td>").c_str());
                                porcentaje= ((ebr.part_s*1.0) / size) * 100.0;
                                fprintf(fileDot, ("<td rowspan=\"2\">Logica <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());
                            }
                            else{
                                fprintf(fileDot, "<td rowspan=\"2\">EBR</td>");
                                porcentaje= (((ebr.part_next-ebr.part_start)*1.0) / size) * 100.0;
                                fprintf(fileDot, ("<td rowspan=\"2\">Libre <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());

                            }
                            fseek(fileReporte, ebr.part_next, SEEK_SET);
                            fread(&ebr, sizeof (EBR), 1, fileReporte);
                            while (true){
                                string name1=ebr.part_name;
                                fprintf(fileDot, ("<td rowspan=\"2\">EBR <br/>" + name1 + "</td>").c_str());
                                porcentaje= ((ebr.part_s*1.0) / size) * 100.0;
                                fprintf(fileDot, ("<td rowspan=\"2\">Logica <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());
                                if (ebr.part_next==-1){
                                    if ((ebr.part_start+ebr.part_s)<mbr.mbr_partition[i].part_s){
                                        porcentaje= (((mbr.mbr_partition[i].part_s-(ebr.part_start+ebr.part_s))*1.0) / size) * 100.0;
                                        fprintf(fileDot, ("<td rowspan=\"2\">Libre <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());
                                    }
                                    break;
                                }else{
                                    if ((ebr.part_start+ebr.part_s)<ebr.part_next){
                                        porcentaje= (((ebr.part_next-(ebr.part_start+ebr.part_s))*1.0) / size) * 100.0;
                                        fprintf(fileDot, ("<td rowspan=\"2\">Libre <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());
                                    }
                                }
                                fseek(fileReporte, ebr.part_next, SEEK_SET);
                                fread(&ebr, sizeof (EBR), 1, fileReporte);
                            }
                        }
                        fprintf(fileDot, "<td rowspan=\"2\">EXTENDIDA</td>");
                    }
                    else if (mbr.mbr_partition[i].part_type=='p'){
                        float p1= (mbr.mbr_partition[i].part_s*1.0) / size;
                        float porcentaje=p1*100.0;
                        string name1=mbr.mbr_partition[i].part_name;
                        fprintf(fileDot, ("<td rowspan=\"2\">" + name1 + " <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());
                        if (i!=3){
                            if ((mbr.mbr_partition[i].part_start+mbr.mbr_partition[i].part_s)<mbr.mbr_partition[i+1].part_start){
                                porcentaje= ((mbr.mbr_partition[i+1].part_start-(mbr.mbr_partition[i].part_start+mbr.mbr_partition[i].part_s)) / size) * 100;
                                fprintf(fileDot, ("<td rowspan=\"2\">LIBRE <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());
                            }
                        }else if ((mbr.mbr_partition[i].part_start+mbr.mbr_partition[i].part_s) < size){
                            porcentaje= ((size - (mbr.mbr_partition[i].part_start + mbr.mbr_partition[i].part_s)) / size) * 100;
                            fprintf(fileDot, ("<td rowspan=\"2\">LIBRE <br/>" + to_string(lround(porcentaje)) + "</td>").c_str());
                        }

                    }
                    start= mbr.mbr_partition[i].part_start + mbr.mbr_partition[i].part_s;
                }
                i++;
            }

            salida1:
            fprintf(fileDot, "</tr></table>>];\n");
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

void Rep::ejecutarReporte_sb() {
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL) {
        this->directorio = this->getCarpetas(this->path);
        this->extension = this->getExtensionFile(this->path);
        string nameD=this->getName(nodo->path);
        system(("sudo -S mkdir -p \'" + this->directorio + "\'").c_str());
        system(("sudo -S chmod -R 777 \'" + this->directorio + "\'").c_str());

        FILE *file;
        if ((file= fopen(nodo->path.c_str(),"rb+"))){
            SuperBloque sb;
            if (nodo->type=='p'){
                MBR mbr;
                fseek(file,0,SEEK_SET);
                fread(&mbr, sizeof(MBR),1,file);
                int pos=-1;
                for (int i = 0; i < 4; ++i) {
                    string name1=mbr.mbr_partition[i].part_name;
                    if (name1==nodo->name){
                        if (mbr.mbr_partition[i].part_status!='2'){
                            cout<<"NO SE HA FORMATEADO LA MONTURA DE LA PARTICION"<<nodo->name<<endl;
                            return;
                        }
                        pos=i;
                        break;
                    }
                }
                if (pos!=-1){
                    fseek(file,nodo->start,SEEK_SET);
                }else{
                    cout<<"PARTICION NO EXISTE EN EL DISCO"<<endl;
                }
            }else if (nodo->type=='l'){
                EBR ebr;
                fseek(file,nodo->start,SEEK_SET);
                fread(&ebr, sizeof(EBR),1,file);
                if (ebr.part_status != '2') {
                    fclose(file);
                    cout<<"NO SE HA FORMATEADO LA MONTURA DE LA PARTICION"<<nodo->name<<endl;
                    return;
                }
                fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
            }
            fread(&sb, sizeof(SuperBloque),1,file);
            fclose(file);

            FILE *dot= fopen("rep.dot","w");
            fprintf(dot,"digraph G {\n");
            fprintf(dot, "node[shape=none]\n");
            fprintf(dot, "start[label=<<table>");
            fprintf(dot,R"(<tr><td colspan="2" bgcolor="#147e0d"><font color="white">REPORTE DE SUPERBLOQUE</font></td></tr>)");

            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#b4f0b1">sb_nombre_hd</td>)");
            fprintf(dot,(R"(<td bgcolor="#b4f0b1">)"+nameD+"</td>)").c_str());
            fprintf(dot,R"(</tr>)");

            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#27ba40">s_filesystem_type</td>)");
            fprintf(dot,(R"(<td bgcolor="#27ba40">)"+ to_string(sb.s_filesystem_type)+"</td>)").c_str());
            fprintf(dot,R"(</tr>)");

            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#b4f0b1">s_inodes_count</td>)");
            fprintf(dot,(R"(<td bgcolor="#b4f0b1">)"+ to_string(sb.s_inodes_count)+"</td>)").c_str());
            fprintf(dot,R"(</tr>)");

            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#27ba40">s_blocks_count</td>)");
            fprintf(dot,(R"(<td bgcolor="#27ba40">)"+ to_string(sb.s_blocks_count)+"</td>)").c_str());
            fprintf(dot,R"(</tr>)");

            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#b4f0b1">s_free_blocks_count</td>)");
            fprintf(dot,(R"(<td bgcolor="#b4f0b1">)"+ to_string(sb.s_free_blocks_count)+"</td>)").c_str());
            fprintf(dot,R"(</tr>)");

            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#27ba40">s_free_inodes_count</td>)");
            fprintf(dot,(R"(<td bgcolor="#27ba40">)"+ to_string(sb.s_free_inodes_count)+"</td>)").c_str());
            fprintf(dot,R"(</tr>)");

            char fecha[100];
            strftime(fecha,sizeof (fecha),"%Y-%m-%d %H:%M:%S",localtime(&sb.s_mtime));
            string fecha1=fecha;
            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#b4f0b1">s_mtime</td>)");
            fprintf(dot,(R"(<td bgcolor="#b4f0b1">)"+ fecha1+"</td>)").c_str());
            fprintf(dot,R"(</tr>)");

            strftime(fecha,sizeof (fecha),"%Y-%m-%d %H:%M:%S",localtime(&sb.s_umtime));
            string fecha3=fecha;
            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#27ba40">s_umtime</td>)");
            fprintf(dot,(R"(<td bgcolor="#27ba40">)"+ fecha3+"</td>)").c_str());
            fprintf(dot,R"(</tr>)");

            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#b4f0b1">s_mnt_count</td>)");
            fprintf(dot,(R"(<td bgcolor="#b4f0b1">)"+ to_string(sb.s_mnt_count)+"</td>)").c_str());
            fprintf(dot,R"(</tr>)");

            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#27ba40">s_magic</td>)");
            fprintf(dot,(R"(<td bgcolor="#27ba40">)"+ to_string(sb.s_magic)+"</td>)").c_str());
            fprintf(dot,R"(</tr>)");

            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#b4f0b1">s_inode_s</td>)");
            fprintf(dot,(R"(<td bgcolor="#b4f0b1">)"+ to_string(sb.s_inode_s)+"</td>)").c_str());
            fprintf(dot,R"(</tr>)");

            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#27ba40">s_block_s</td>)");
            fprintf(dot,(R"(<td bgcolor="#27ba40">)"+ to_string(sb.s_block_s)+"</td>)").c_str());
            fprintf(dot,R"(</tr>)");

            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#b4f0b1">s_firts_ino</td>)");
            fprintf(dot,(R"(<td bgcolor="#b4f0b1">)"+ to_string(sb.s_firts_ino)+"</td>)").c_str());
            fprintf(dot,R"(</tr>)");

            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#27ba40">s_first_blo</td>)");
            fprintf(dot,(R"(<td bgcolor="#27ba40">)"+ to_string(sb.s_first_blo)+"</td>)").c_str());
            fprintf(dot,R"(</tr>)");

            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#b4f0b1">s_bm_inode_start</td>)");
            fprintf(dot,(R"(<td bgcolor="#b4f0b1">)"+ to_string(sb.s_bm_inode_start)+"</td>)").c_str());
            fprintf(dot,R"(</tr>)");

            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#27ba40">s_bm_block_start</td>)");
            fprintf(dot,(R"(<td bgcolor="#27ba40">)"+ to_string(sb.s_bm_block_start)+"</td>)").c_str());
            fprintf(dot,R"(</tr>)");

            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#b4f0b1">s_inode_start</td>)");
            fprintf(dot,(R"(<td bgcolor="#b4f0b1">)"+ to_string(sb.s_inode_start)+"</td>)").c_str());
            fprintf(dot,R"(</tr>)");

            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#27ba40">s_block_start</td>)");
            fprintf(dot,(R"(<td bgcolor="#27ba40">)"+ to_string(sb.s_block_start)+"</td>)").c_str());
            fprintf(dot,R"(</tr>)");

            fprintf(dot,"</table>>];\n");
            fprintf(dot,"}");
            fclose(dot);
            string command="sudo -S  dot -T"+this->extension+" rep.dot -o \""+ this->path+"\"";
            system(command.c_str());
            cout<<"REPORTE GENERADO CON EXITO:  SB"<<endl;
        }else{
            cout << "EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA" << endl;
            return;
        }
    }else{
        cout << "LA MONTURA CON EL ID: " << this->id << " NO EXISTE O NO SE ENCUENTRA EN EL SISTEMA" << endl;
        return;
    }
}

void Rep::ejecutarReporte_inode() {
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL) {
        this->directorio = this->getCarpetas(this->path);
        this->extension = this->getExtensionFile(this->path);
        system(("sudo -S mkdir -p \'" + this->directorio + "\'").c_str());
        system(("sudo -S chmod -R 777 \'" + this->directorio + "\'").c_str());

        FILE *file;
        if ((file= fopen(nodo->path.c_str(),"rb+"))){
            SuperBloque sb;

            if (nodo->type=='p'){
                fseek(file,nodo->start,SEEK_SET);
            }else if (nodo->type=='l'){
                fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
            }
            fread(&sb, sizeof(SuperBloque),1,file);
            int start=sb.s_bm_inode_start,
                end=start+sb.s_inodes_count;
            TablaInodo inodo;
            char bit;
            int cont=0;
            char fecha[70];
            string fecha2="";

            FILE *dot= fopen("rep.dot","w");
            fprintf(dot,"digraph G {\n");
            fprintf(dot, "node[shape=none]\n");

            for (int i = start; i < end; ++i) {
                fseek(file,i,SEEK_SET);
                fread(&bit, sizeof(char),1,file);
                if (bit=='1'){
                    fseek(file,(sb.s_inode_start+(cont* sizeof(TablaInodo))),SEEK_SET);
                    fread(&inodo, sizeof(TablaInodo),1,file);

                    fprintf(dot,("n"+ to_string(cont)+
                    R"([label=<<table><tr><td colspan="2">INODO )"+ to_string(sb.s_inode_start+(cont* sizeof(TablaInodo)))+
                    "</td></tr>\n").c_str());

                    fprintf(dot,"<tr>\n");
                    fprintf(dot,"<td>i_uid</td>\n");
                    fprintf(dot,("<td>"+ to_string(inodo.i_uid)+"</td>\n").c_str());
                    fprintf(dot,"</tr>\n");

                    fprintf(dot,"<tr>\n");
                    fprintf(dot,"<td>i_gid</td>\n");
                    fprintf(dot,("<td>"+ to_string(inodo.i_gid)+"</td>\n").c_str());
                    fprintf(dot,"</tr>\n");

                    fprintf(dot,"<tr>\n");
                    fprintf(dot,"<td>i_s</td>\n");
                    fprintf(dot,("<td>"+ to_string(inodo.i_s)+"</td>\n").c_str());
                    fprintf(dot,"</tr>\n");

                    strftime(fecha,sizeof (fecha),"%Y-%m-%d %H:%M:%S",localtime(&inodo.i_atime));
                    fecha2=fecha;
                    fprintf(dot,"<tr>\n");
                    fprintf(dot,"<td>i_atime</td>\n");
                    fprintf(dot,("<td>"+fecha2+"</td>\n").c_str());
                    fprintf(dot,"</tr>\n");

                    strftime(fecha,sizeof (fecha),"%Y-%m-%d %H:%M:%S",localtime(&inodo.i_ctime));
                    fecha2=fecha;
                    fprintf(dot,"<tr>\n");
                    fprintf(dot,"<td>i_ctime</td>\n");
                    fprintf(dot,("<td>"+fecha2+"</td>\n").c_str());
                    fprintf(dot,"</tr>\n");

                    strftime(fecha,sizeof (fecha),"%Y-%m-%d %H:%M:%S",localtime(&inodo.i_mtime));
                    fecha2=fecha;
                    fprintf(dot,"<tr>\n");
                    fprintf(dot,"<td>i_mtime</td>\n");
                    fprintf(dot,("<td>"+fecha2+"</td>\n").c_str());
                    fprintf(dot,"</tr>\n");

                    for (int j = 0; j < 15; ++j) {
                        fprintf(dot,"<tr>\n");
                        fprintf(dot,"<td>i_block</td>\n");
                        fprintf(dot,("<td>"+ to_string(inodo.i_block[j])+"</td>\n").c_str());
                        fprintf(dot,"</tr>\n");
                    }

                    fprintf(dot,"<tr>\n");
                    fprintf(dot,"<td>i_type</td>\n");
                    fprintf(dot,("<td>"+ string(1,inodo.i_type)+"</td>\n").c_str());
                    fprintf(dot,"</tr>\n");

                    fprintf(dot,"<tr>\n");
                    fprintf(dot,"<td>i_perm</td>\n");
                    fprintf(dot,("<td>"+ to_string(inodo.i_perm)+"</td>\n").c_str());
                    fprintf(dot,"</tr>\n");

                    fprintf(dot,"</table>>]\n");
                }
                cont++;
            }

            fprintf(dot,"}");
            fclose(dot);
            fclose(file);
            string command="sudo -S  dot -T"+this->extension+" rep.dot -o \""+ this->path+"\"";
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

void Rep::ejecutarReporte_bm_inode() {
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL) {
        this->directorio = this->getCarpetas(this->path);
        this->extension = this->getExtensionFile(this->path);
        string nameD=this->getName(nodo->path);
        system(("sudo -S mkdir -p \'" + this->directorio + "\'").c_str());
        system(("sudo -S chmod -R 777 \'" + this->directorio + "\'").c_str());

        FILE *file;
        if ((file= fopen(nodo->path.c_str(),"rb+"))){
            SuperBloque sb;
            if (nodo->type=='p'){
                MBR mbr;
                fseek(file,0,SEEK_SET);
                fread(&mbr, sizeof(MBR),1,file);

                if (mbr.mbr_partition[nodo->pos].part_status!='2'){
                    cout<<"NO SE HA FORMATEADO LA PARTICION EN UN SISTEMA DE ARCHIVOS"<<endl;
                    return;
                }
                fseek(file,mbr.mbr_partition[nodo->pos].part_start,SEEK_SET);
            }else if (nodo->type=='l'){
                EBR ebr;
                fseek(file,nodo->start,SEEK_SET);
                fread(&ebr, sizeof(EBR),1,file);
                if (ebr.part_status!='2'){
                    cout<<"NO SE HA FORMATEADO LA PARTICION EN UN SISTEMA DE ARCHIVOS"<<endl;
                    return;
                }
                fseek(file,ebr.part_start+ sizeof(EBR),SEEK_SET);
            }
            fread(&sb, sizeof(SuperBloque),1,file);
            int start=sb.s_bm_inode_start;
            int end=start+sb.s_inodes_count;
            int controlador=0;
            char bit;
            FILE *dot= fopen("rep.dot","w");

            fprintf(dot,"digraph G {\n");
            fprintf(dot, ("node[shape=none, lblstyle=\"align=left\"]\n"));
            fprintf(dot,R"(start[label=")");

            for (int i = start; i < end; ++i) {
                fseek(file,i,SEEK_SET);
                fread(&bit, sizeof(char ),1,file);
                fprintf(dot,(string(1,bit)+" ").c_str());
                if (controlador==19){
                    fprintf(dot,("\\n "));
                    controlador=0;
                }else{
                    controlador++;
                }
            }

            fprintf(dot,R"("])");
            fprintf(dot,"}");
            fclose(dot);
            fclose(file);
            string command="sudo -S  dot -T"+this->extension+" rep.dot -o \""+ this->path+"\"";
            system(command.c_str());
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

void Rep::ejecutarReporte_bm_block() {
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL) {
        this->directorio = this->getCarpetas(this->path);
        this->extension = this->getExtensionFile(this->path);
        string nameD=this->getName(nodo->path);
        system(("sudo -S mkdir -p \'" + this->directorio + "\'").c_str());
        system(("sudo -S chmod -R 777 \'" + this->directorio + "\'").c_str());

        FILE *file;
        if ((file= fopen(nodo->path.c_str(),"rb+"))){
            SuperBloque sb;
            if (nodo->type=='p'){
                MBR mbr;
                fseek(file,0,SEEK_SET);
                fread(&mbr, sizeof(MBR),1,file);

                if (mbr.mbr_partition[nodo->pos].part_status!='2'){
                    cout<<"NO SE HA FORMATEADO LA PARTICION EN UN SISTEMA DE ARCHIVOS"<<endl;
                    return;
                }
                fseek(file,mbr.mbr_partition[nodo->pos].part_start,SEEK_SET);
            }else if (nodo->type=='l'){
                EBR ebr;
                fseek(file,nodo->start,SEEK_SET);
                fread(&ebr, sizeof(EBR),1,file);
                if (ebr.part_status!='2'){
                    cout<<"NO SE HA FORMATEADO LA PARTICION EN UN SISTEMA DE ARCHIVOS"<<endl;
                    return;
                }
                fseek(file,ebr.part_start+ sizeof(EBR),SEEK_SET);
            }
            fread(&sb, sizeof(SuperBloque),1,file);
            int start=sb.s_bm_block_start;
            int end=start+sb.s_blocks_count;
            int controlador=0;
            char bit;
            FILE *dot= fopen("rep.dot","w");

            fprintf(dot,"digraph G {\n");
            fprintf(dot, ("node[shape=none, lblstyle=\"align=left\"]\n"));
            fprintf(dot,R"(start[label=")");

            for (int i = start; i < end; ++i) {
                fseek(file,i,SEEK_SET);
                fread(&bit, sizeof(char ),1,file);
                fprintf(dot,(string(1,bit)+" ").c_str());
                if (controlador==19){
                    fprintf(dot,("\\n "));
                    controlador=0;
                }else{
                    controlador++;
                }
            }

            fprintf(dot,R"("])");
            fprintf(dot,"}");
            fclose(dot);
            fclose(file);
            string command="sudo -S  dot -T"+this->extension+" rep.dot -o \""+ this->path+"\"";
            system(command.c_str());
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

void Rep::ejecutarReporte_file() {
    if (this->ruta!=" "){
        Nodo_M *nodo=this->mountList->buscar(this->id);
        if (nodo!=NULL) {

            this->directorio = this->getCarpetas(this->path);
            this->extension = this->getExtensionFile(this->path);
            string nameD=this->getName(nodo->path);
            system(("sudo -S mkdir -p \'" + this->directorio + "\'").c_str());
            system(("sudo -S chmod -R 777 \'" + this->directorio + "\'").c_str());

            FILE *file;
            if ((file= fopen(nodo->path.c_str(),"rb+"))){
                if (nodo->type=='p'){
                    MBR mbr;
                    fseek(file,0,SEEK_SET);
                    fread(&mbr, sizeof(MBR),1,file);
                    int pos=-1;
                    for (int i = 0; i < 4; ++i) {
                        string name1=mbr.mbr_partition[i].part_name;
                        if (name1==nodo->name){
                            if (mbr.mbr_partition[i].part_status!='2'){
                                cout<<"NO SE HA FORMATEADO LA MONTURA DE LA PARTICION"<<nodo->name<<endl;
                                return;
                            }
                            pos=i;
                            break;
                        }
                    }
                    if (pos!=-1){
                        fseek(file,nodo->start,SEEK_SET);
                    }else{
                        cout<<"PARTICION NO EXISTE EN EL DISCO"<<endl;
                    }
                }else if (nodo->type=='l'){
                    EBR ebr;
                    fseek(file,nodo->start,SEEK_SET);
                    fread(&ebr, sizeof(EBR),1,file);
                    if (ebr.part_status != '2') {
                        fclose(file);
                        cout<<"NO SE HA FORMATEADO LA MONTURA DE LA PARTICION"<<nodo->name<<endl;
                        return;
                    }
                    fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
                }

                vector<string> rutaS= this->splitRuta(this->ruta);
                if (rutaS.empty()){
                    cout<<"RUTA INVALIDA"<<endl;
                    fclose(file);
                    return;
                }

                if (nodo->type=='p'){
                    fseek(file,nodo->start,SEEK_SET);
                    fread(&this->superBloqueGlobal, sizeof(SuperBloque), 1, file);
                }else if (nodo->type=='l'){
                    fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
                    fread(&this->superBloqueGlobal, sizeof(SuperBloque), 1, file);
                }

                TablaInodo inodo;

                int posInodoF=this->getInodoF(rutaS, 0,rutaS.size()-1, this->superBloqueGlobal.s_inode_start, file);

                if (posInodoF==-1){
                    cout << "ARCHIVO NO ENCONTRADO " << endl;
                    return;
                }

                fseek(file,posInodoF,SEEK_SET);
                fread(&inodo, sizeof(TablaInodo),1,file);
                inodo.i_atime= time(nullptr);
                fseek(file,posInodoF,SEEK_SET);
                fwrite(&inodo, sizeof(TablaInodo),1,file);

                FILE *dot= fopen("rep.dot","w");

                fprintf(dot,"digraph G {\n");
                fprintf(dot, ("node[shape=none, lblstyle=\"align=left\"]\n"));
                fprintf(dot,R"(start[label=")");

                fprintf(dot,(rutaS[rutaS.size()-1]+"\n").c_str());
                string content=this->getContent(posInodoF,file);
                fprintf(dot,content.c_str());
                fprintf(dot,R"("])");
                fprintf(dot,"}");
                fclose(dot);
                fclose(file);
                string command="sudo -S  dot -T"+this->extension+" rep.dot -o \""+ this->path+"\"";
                system(command.c_str());
                cout<<"REPORTE GENERADO CON EXITO:  FILE DE "<<rutaS[rutaS.size()-1]<<endl;

            }else{
                cout << "EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA" << endl;
            }
        }else{
            cout << "LA MONTURA CON EL ID: " << this->id << " NO EXISTE O NO SE ENCUENTRA EN EL SISTEMA" << endl;
            return;
        }
    }
}

void Rep::ejecutarReporte_ls() {
    if (this->ruta!=" "){
        Nodo_M *nodo=this->mountList->buscar(this->id);
        if (nodo!=NULL) {

            this->directorio = this->getCarpetas(this->path);
            this->extension = this->getExtensionFile(this->path);
            string nameD=this->getName(nodo->path);
            system(("sudo -S mkdir -p \'" + this->directorio + "\'").c_str());
            system(("sudo -S chmod -R 777 \'" + this->directorio + "\'").c_str());

            FILE *file;
            if ((file= fopen(nodo->path.c_str(),"rb+"))){
                if (nodo->type=='p'){
                    MBR mbr;
                    fseek(file,0,SEEK_SET);
                    fread(&mbr, sizeof(MBR),1,file);
                    int pos=-1;
                    for (int i = 0; i < 4; ++i) {
                        string name1=mbr.mbr_partition[i].part_name;
                        if (name1==nodo->name){
                            if (mbr.mbr_partition[i].part_status!='2'){
                                cout<<"NO SE HA FORMATEADO LA MONTURA DE LA PARTICION"<<nodo->name<<endl;
                                return;
                            }
                            pos=i;
                            break;
                        }
                    }
                    if (pos!=-1){
                        fseek(file,nodo->start,SEEK_SET);
                    }else{
                        cout<<"PARTICION NO EXISTE EN EL DISCO"<<endl;
                    }
                }else if (nodo->type=='l'){
                    EBR ebr;
                    fseek(file,nodo->start,SEEK_SET);
                    fread(&ebr, sizeof(EBR),1,file);
                    if (ebr.part_status != '2') {
                        fclose(file);
                        cout<<"NO SE HA FORMATEADO LA MONTURA DE LA PARTICION"<<nodo->name<<endl;
                        return;
                    }
                    fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
                }



                if (nodo->type=='p'){
                    fseek(file,nodo->start,SEEK_SET);
                    fread(&this->superBloqueGlobal, sizeof(SuperBloque), 1, file);
                }else if (nodo->type=='l'){
                    fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
                    fread(&this->superBloqueGlobal, sizeof(SuperBloque), 1, file);
                }

                TablaInodo inodo;
                int posInodoF=0;
                vector<string> rutaS;

                if (this->ruta=="/"){
                    posInodoF=this->superBloqueGlobal.s_inode_start;
                    rutaS.push_back("/");
                }else{
                    rutaS= this->splitRuta(this->ruta);
                    if (rutaS.empty()){
                        cout<<"RUTA INVALIDA"<<endl;
                        fclose(file);
                        return;
                    }
                    posInodoF=this->getInodoF(rutaS, 0,rutaS.size()-1, this->superBloqueGlobal.s_inode_start, file);
                }

                if (posInodoF==-1){
                    cout << "ARCHIVO NO ENCONTRADO " << endl;
                    return;
                }

                fseek(file,posInodoF,SEEK_SET);
                fread(&inodo, sizeof(TablaInodo),1,file);
                inodo.i_atime= time(nullptr);
                fseek(file,posInodoF,SEEK_SET);
                fwrite(&inodo, sizeof(TablaInodo),1,file);

                FILE *dot= fopen("rep.dot","w");

                fprintf(dot,"digraph G {\n");
                fprintf(dot, ("node[shape=none]\n"));
                fprintf(dot,"start[label=<<table>\n");

                fprintf(dot,"<tr>\n");
                fprintf(dot,"<td>Permisos</td>\n");
                fprintf(dot,"<td>Usuario</td>\n");
                fprintf(dot,"<td>Grupo</td>\n");
                fprintf(dot,"<td>Size</td>\n");
                fprintf(dot,"<td>Fecha</td>\n");
                fprintf(dot,"<td>Tipo</td>\n");
                fprintf(dot,"<td>Nombre</td>\n");
                fprintf(dot,"</tr>\n");

                fprintf(dot,this->lsInodo(posInodoF,rutaS[rutaS.size()-1],file).c_str());

                fprintf(dot,R"("</table>>])");
                fprintf(dot,"}");
                fclose(dot);
                fclose(file);
                string command="sudo -S  dot -T"+this->extension+" rep.dot -o \""+ this->path+"\"";
                system(command.c_str());
                cout<<"REPORTE GENERADO CON EXITO:  LS DE "<<rutaS[rutaS.size()-1]<<endl;

            }else{
                cout << "EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA" << endl;
            }
        }else{
            cout << "LA MONTURA CON EL ID: " << this->id << " NO EXISTE O NO SE ENCUENTRA EN EL SISTEMA" << endl;
            return;
        }
    }

}

string Rep::lsInodo(int pos,string name1, FILE *file) {
    string dot="";
    TablaInodo inodo;
    fseek(file,pos,SEEK_SET);
    fread(&inodo, sizeof(TablaInodo),1,file);

    dot+="<tr>\n";
    dot+="<td>"+this->getPermiso(inodo.i_perm)+"</td>\n";
    dot+="<td>"+ this->getUsuario(inodo.i_uid,file)+"</td>\n";
    dot+="<td>"+ this->getGrupo(inodo.i_gid,file)+"</td>\n";
    dot+="<td>"+to_string(inodo.i_s)+"</td>\n";
    char fecha[100];
    strftime(fecha,sizeof (fecha),"%Y-%m-%d %H:%M:%S",localtime(&inodo.i_mtime));
    string fecha2=fecha;
    dot+="<td>"+fecha2+"</td>\n";
    if (inodo.i_type=='1'){
        dot+="<td>Archivo</td>\n";
        dot+="<td>"+name1+"</td>\n";
        dot+="</tr>";
        return dot;
    }

    dot+="<td>Carpeta</td>\n";
    dot+="<td>"+name1+"</td>\n";
    dot+="</tr>\n";
    BloqueApuntador apuntador1,apuntador2,apuntador3;
    BloqueCarpeta carpeta;

    for (int i = 0; i < 15; ++i) {
                if (inodo.i_block[i]!=-1){
                    if (i<12){
                        fseek(file,inodo.i_block[i],SEEK_SET);
                        fread(&carpeta, sizeof(BloqueCarpeta),1,file);
                        for (int c = 0; c < 4; ++c) {
                            if (carpeta.b_content[c].b_inodo!=-1){
                                string name1="";
                                for (int l = 0; l < 12; ++l) {
                                    if (carpeta.b_content[c].b_name[l]=='\000'){
                                        break;
                                    }
                                    name1+=carpeta.b_content[c].b_name[l];
                                }
                                if (!(name1=="." || name1=="..")){
                                    dot+=this->lsInodo(carpeta.b_content[c].b_inodo,name1,file);
                                }
                            }
                        }
                    } else if (i==12){
                        fseek(file,inodo.i_block[i],SEEK_SET);
                        fread(&apuntador1, sizeof(BloqueApuntador),1,file);
                        for (int j = 0; j < 16; ++j) {
                            if (apuntador1.b_pointers[j]!=-1){
                                fseek(file,apuntador1.b_pointers[j],SEEK_SET);
                                fread(&carpeta, sizeof(BloqueCarpeta),1,file);
                                for (int c = 0; c < 4; ++c) {
                                    if (carpeta.b_content[c].b_inodo!=-1){
                                        string name1="";
                                        for (int l = 0; l < 12; ++l) {
                                            if (carpeta.b_content[c].b_name[l]=='\000'){
                                                break;
                                            }
                                            name1+=carpeta.b_content[c].b_name[l];
                                        }
                                        dot+=this->lsInodo(carpeta.b_content[c].b_inodo,name1,file);
                                    }
                                }
                            }
                        }
                    }else if (i==13){
                        fseek(file,inodo.i_block[i],SEEK_SET);
                        fread(&apuntador1, sizeof(BloqueApuntador),1,file);
                        for (int j = 0; j < 16; ++j) {
                            if (apuntador1.b_pointers[j]!=-1){
                                fseek(file,apuntador1.b_pointers[j],SEEK_SET);
                                fread(&apuntador2, sizeof(BloqueApuntador),1,file);
                                for (int k = 0; k < 16; ++k) {
                                    if (apuntador2.b_pointers[k]!=-1){
                                        fseek(file,apuntador2.b_pointers[j],SEEK_SET);
                                        fread(&carpeta, sizeof(BloqueCarpeta),1,file);
                                        for (int c = 0; c < 4; ++c) {
                                            if (carpeta.b_content[c].b_inodo!=-1){
                                                string name1="";
                                                for (int l = 0; l < 12; ++l) {
                                                    if (carpeta.b_content[c].b_name[l]=='\000'){
                                                        break;
                                                    }
                                                    name1+=carpeta.b_content[c].b_name[l];
                                                }
                                                dot+=this->lsInodo(carpeta.b_content[c].b_inodo,name1,file);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }else if (i==14){
                        fseek(file,inodo.i_block[i],SEEK_SET);
                        fread(&apuntador1, sizeof(BloqueApuntador),1,file);
                        for (int j = 0; j < 16; ++j) {
                            if (apuntador1.b_pointers[j]!=-1){
                                fseek(file,apuntador1.b_pointers[j],SEEK_SET);
                                fread(&apuntador2, sizeof(BloqueApuntador),1,file);
                                for (int k = 0; k < 16; ++k) {
                                    if (apuntador2.b_pointers[k]!=-1){
                                        fseek(file,apuntador2.b_pointers[k],SEEK_SET);
                                        fread(&apuntador3, sizeof(BloqueApuntador),1,file);
                                        for (int l = 0; l < 16; ++l) {
                                            if (apuntador3.b_pointers[l]!=-1){
                                                fseek(file,apuntador3.b_pointers[j],SEEK_SET);
                                                fread(&carpeta, sizeof(BloqueCarpeta),1,file);
                                                for (int c = 0; c < 4; ++c) {
                                                    if (carpeta.b_content[c].b_inodo!=-1){
                                                        string name1="";
                                                        for (int t = 0; t < 12; ++t) {
                                                            if (carpeta.b_content[c].b_name[t]=='\000'){
                                                                break;
                                                            }
                                                            name1+=carpeta.b_content[c].b_name[t];
                                                        }
                                                        dot+=this->lsInodo(carpeta.b_content[c].b_inodo,name1,file);
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

    return dot;
}

string Rep::getUsuario(int id, FILE *file) {
    string content=this->getContent(this->superBloqueGlobal.s_inode_start + sizeof(TablaInodo), file);
    vector<string> split;
    string usuario;
    stringstream ss(content);
    string copia=content;

    while (getline(ss,usuario,'\n')){
        if (usuario != ""){
            vector<string> datos=this->splitDatos(usuario);
            if (datos[1]=="U" &&  datos[0]== to_string(id)){
                return datos[3];
            }
        }
    }
    return "NONE";
}

string Rep::getGrupo(int id, FILE *file) {
    string content=this->getContent(this->superBloqueGlobal.s_inode_start + sizeof(TablaInodo), file);
    vector<string> split;
    string usuario;
    stringstream ss(content);
    string copia=content;

    while (getline(ss,usuario,'\n')){
        if (usuario != ""){
            vector<string> datos=this->splitDatos(usuario);
            if (datos[1]=="G" &&  datos[0]== to_string(id)){
                return datos[2];
            }
        }
    }

    return "NONE";
}

vector<string> Rep::splitDatos(std::string cadena) {
    vector<string> split;
    string dato;
    stringstream ss(cadena);
    string copia=cadena;

    while (getline(ss,dato,',')){
        if (dato != "")split.push_back(dato);
    }
    return split;
}

string Rep::getPermiso(int permiso) {
    string perm= to_string(permiso);
    string dot="";
    for (int i = 0; i < perm.length(); ++i) {
        dot+="-";
        if ((perm[i]=='4') || (perm[i]=='5') || (perm[i]=='6') || (perm[i]=='7')){
            dot+="r";
        }
        if ((perm[i]=='2') || (perm[i]=='3') || (perm[i]=='6') || (perm[i]=='7')){
            dot+="w";
        }if ((perm[i]=='1') || (perm[i]=='3') || (perm[i]=='5') || (perm[i]=='7')){
            dot+="x";
        }
        dot+=" ";
    }
    return dot;
}

void Rep::ejecutarReporte_Journaling() {
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL) {
        this->directorio = this->getCarpetas(this->path);
        this->extension = this->getExtensionFile(this->path);
        string nameD=this->getName(nodo->path);
        system(("sudo -S mkdir -p \'" + this->directorio + "\'").c_str());
        system(("sudo -S chmod -R 777 \'" + this->directorio + "\'").c_str());

        FILE *file;
        if ((file= fopen(nodo->path.c_str(),"rb+"))){
            SuperBloque sb;
            if (nodo->type=='p'){
                MBR mbr;
                fseek(file,0,SEEK_SET);
                fread(&mbr, sizeof(MBR),1,file);
                string name1=mbr.mbr_partition[nodo->pos].part_name;
                if (name1==nodo->name){
                    if (mbr.mbr_partition[nodo->pos].part_status!='2'){
                        cout<<"NO SE HA FORMATEADO LA MONTURA DE LA PARTICION"<<nodo->name<<endl;
                        return;
                    }
                    fseek(file,nodo->start,SEEK_SET);
                    fread(&sb, sizeof(SuperBloque),1,file);
                    if (sb.s_filesystem_type!=3){
                        cout<<"LA PARTICION NO ESTA FORMATEADA CON EXT 3"<<nodo->name<<endl;
                        return;
                    }
                    fseek(file,nodo->start+ sizeof(SuperBloque),SEEK_SET);
                }else{
                    return;
                }
            }else if (nodo->type=='l'){
                EBR ebr;
                fseek(file,nodo->start,SEEK_SET);
                fread(&ebr, sizeof(EBR),1,file);
                if (ebr.part_status != '2') {
                    fclose(file);
                    cout<<"NO SE HA FORMATEADO LA MONTURA DE LA PARTICION"<<nodo->name<<endl;
                    return;
                }
                fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
                fread(&sb, sizeof(SuperBloque),1,file);
                if (sb.s_filesystem_type!=3){
                    cout<<"LA PARTICION NO ESTA FORMATEADA CON EXT 3"<<nodo->name<<endl;
                    return;
                }
                fseek(file,nodo->start+ sizeof(SuperBloque)+ sizeof(EBR),SEEK_SET);
            }

            Journal actJounal;
            fread(&actJounal, sizeof(Journal),1,file);
            FILE *dot = fopen("rep.dot", "w");
            fprintf(dot, "digraph G {\n");
            fprintf(dot, "node[shape=none]\n");
            fprintf(dot, "start[label=<<table>");
            fprintf(dot, R"(<tr><td colspan="5" bgcolor="#6fee84" color="#2980B9">REPORTE JORUNALING</td></tr>)");
            fprintf(dot, "\n");
            fprintf(dot, R"(<tr><td bgcolor="#b7f4c1" color="#2980B9">Tipo Operacion</td>)");
            fprintf(dot, R"(<td bgcolor="#b7f4c1">Tipo</td>)");
            fprintf(dot, R"(<td bgcolor="#b7f4c1">Path</td>)");
            fprintf(dot, R"(<td bgcolor="#b7f4c1">Contenido</td>)");
            fprintf(dot, R"(<td bgcolor="#b7f4c1">Fecha</td></tr>)");
            fprintf(dot, "\n");

            while (true){
                fprintf(dot,"<tr>");
                string aux=actJounal.journal_Tipo_Operacion;
                fprintf(dot,("<td>"+aux+"</td>").c_str());
                fprintf(dot,("<td>"+ string(1,actJounal.journal_Tipo)+"</td>").c_str());
                aux=actJounal.journal_Path;
                fprintf(dot,("<td>"+aux+"</td>").c_str());
                aux=actJounal.journal_Contenido;
                fprintf(dot,("<td>"+aux+"</td>").c_str());
                char fecha[70];
                strftime(fecha,sizeof (fecha),"%Y-%m-%d %H:%M:%S",localtime(&actJounal.journal_Fecha));
                aux=fecha;
                fprintf(dot,("<td>"+aux+"</td>").c_str());
                fprintf(dot,"</tr>");

                if (actJounal.journal_Sig!=-1){
                    fseek(file,actJounal.journal_Start+ sizeof(Journal),SEEK_SET);
                    fread(&actJounal, sizeof(Journal),1, file);
                }else{
                    break;
                }
            }

            fprintf(dot, "</table>>];\n");
            fprintf(dot, "}");
            fclose(file);
            fclose(dot);
            string command="sudo -S  dot -T"+this->extension+" rep.dot -o \""+ this->path+"\"";
            system(command.c_str());
            cout<<"SE GENERO EL Journaling"<<endl;
        }else{
            cout << "EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA" << endl;
        }

    }else{
        cout << "LA MONTURA CON EL ID: " << this->id << " NO EXISTE O NO SE ENCUENTRA EN EL SISTEMA" << endl;
        return;
    }
}

string Rep::getName(std::string path) {
    string aux=path;
    size_t p=0;
    string directorio="";
    while ((p=aux.find("/"))!= string::npos){
        directorio += aux.substr(0,p)+"/";
        aux.erase(0,p+1);
    }
    return aux;
}


string Rep::getContent(int inodoStart, FILE *file) {
    TablaInodo inodo;
    BloqueArchivo archivo;
    BloqueApuntador apuntador1,apuntador2,apuntador3;
    string content="";
    fseek(file,inodoStart,SEEK_SET);
    fread(&inodo, sizeof(TablaInodo),1,file);
    for (int i = 0; i < 15; ++i) {
        if (inodo.i_block[i]!=-1){
            if (i<12){
                fseek(file,inodo.i_block[i],SEEK_SET);
                fread(&archivo, sizeof(BloqueArchivo),1,file);
                for (int j = 0; j < 64; ++j) {
                    if (archivo.b_content[j]=='\000'){
                        break;
                    }
                    content+=archivo.b_content[j];
                }
            } else if (i==12){
                fseek(file,inodo.i_block[i],SEEK_SET);
                fread(&apuntador1, sizeof(BloqueApuntador),1,file);
                for (int j = 0; j < 16; ++j) {
                    if (apuntador1.b_pointers[j]!=-1){
                        fseek(file,apuntador1.b_pointers[j],SEEK_SET);
                        fread(&archivo, sizeof(BloqueArchivo),1,file);
                        for (int j = 0; j < 64; ++j) {
                            if (archivo.b_content[j]=='\000'){
                                break;
                            }
                            content+=archivo.b_content[j];
                        }
                    }
                }
            }else if (i==13){
                fseek(file,inodo.i_block[i],SEEK_SET);
                fread(&apuntador1, sizeof(BloqueApuntador),1,file);
                for (int j = 0; j < 16; ++j) {
                    if (apuntador1.b_pointers[j]!=-1){
                        fseek(file,apuntador1.b_pointers[j],SEEK_SET);
                        fread(&apuntador2, sizeof(BloqueApuntador),1,file);
                        for (int k = 0; k < 16; ++k) {
                            if (apuntador2.b_pointers[k]!=-1){
                                fseek(file,apuntador2.b_pointers[k],SEEK_SET);
                                fread(&archivo, sizeof(BloqueArchivo),1,file);
                                for (int j = 0; j < 64; ++j) {
                                    if (archivo.b_content[j]=='\000'){
                                        break;
                                    }
                                    content+=archivo.b_content[j];
                                }
                            }
                        }
                    }
                }
            }else if (i==14){
                fseek(file,inodo.i_block[i],SEEK_SET);
                fread(&apuntador1, sizeof(BloqueApuntador),1,file);
                for (int j = 0; j < 16; ++j) {
                    if (apuntador1.b_pointers[j]!=-1){
                        fseek(file,apuntador1.b_pointers[j],SEEK_SET);
                        fread(&apuntador2, sizeof(BloqueApuntador),1,file);
                        for (int k = 0; k < 16; ++k) {
                            if (apuntador2.b_pointers[k]!=-1){
                                fseek(file,apuntador2.b_pointers[k],SEEK_SET);
                                fread(&apuntador3, sizeof(BloqueApuntador),1,file);
                                for (int l = 0; l < 16; ++l) {
                                    fseek(file,apuntador3.b_pointers[l],SEEK_SET);
                                    fread(&archivo, sizeof(BloqueArchivo),1,file);
                                    for (int j = 0; j < 64; ++j) {
                                        if (archivo.b_content[j]=='\000'){
                                            break;
                                        }
                                        content+=archivo.b_content[j];
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return content;
}

vector<string> Rep::splitRuta(std::string cadena) {
    vector<string> split;
    string aux;
    stringstream ss(cadena);
    string copia=cadena;
    while (getline(ss,aux,'/')){
        if (aux != ""){
            split.push_back(aux);
        }
    }
    return split;
}

int Rep::getInodoF(vector<std::string> rutaS, int posAct, int rutaSize, int start, FILE *file) {
    TablaInodo inodo;
    BloqueCarpeta carpeta;
    BloqueApuntador apuntador1,apuntador2,apuntador3;

    fseek(file,start,SEEK_SET);
    fread(&inodo, sizeof(TablaInodo),1,file);

    for (int i = 0; i < 15; ++i) {
        if (inodo.i_block[i]!=-1){
            if (i<12){
                fseek(file,inodo.i_block[i],SEEK_SET);
                fread(&carpeta, sizeof(BloqueCarpeta),1,file);
                for (int c = 0; c < 4; ++c) {
                    if (carpeta.b_content[c].b_name==rutaS[posAct]){
                        if (posAct < rutaSize) {
                            return this->getInodoF(rutaS, posAct + 1, rutaSize,
                                                   carpeta.b_content[c].b_inodo,file);
                        }
                        if (posAct == rutaSize) {
                            return carpeta.b_content[c].b_inodo;
                        }
                    }
                }

            } else if (i==12){
                fseek(file,inodo.i_block[i],SEEK_SET);
                fread(&apuntador1, sizeof(BloqueApuntador),1,file);
                for (int j = 0; j < 16; ++j) {
                    if (apuntador1.b_pointers[j]!=-1){
                        fseek(file,apuntador1.b_pointers[j],SEEK_SET);
                        fread(&carpeta, sizeof(BloqueCarpeta),1,file);
                        for (int c = 0; c < 4; ++c) {
                            if (carpeta.b_content[c].b_name==rutaS[posAct]){
                                if (posAct < rutaSize) {
                                    return this->getInodoF(rutaS, posAct + 1, rutaSize,
                                                           carpeta.b_content[c].b_inodo,file);
                                }
                                if (posAct == rutaSize) {
                                    return carpeta.b_content[c].b_inodo;
                                }
                            }
                        }
                    }
                }
            }else if (i==13){
                fseek(file,inodo.i_block[i],SEEK_SET);
                fread(&apuntador1, sizeof(BloqueApuntador),1,file);
                for (int j = 0; j < 16; ++j) {
                    if (apuntador1.b_pointers[j]!=-1){
                        fseek(file,apuntador1.b_pointers[j],SEEK_SET);
                        fread(&apuntador2, sizeof(BloqueApuntador),1,file);
                        for (int k = 0; k < 16; ++k) {
                            if (apuntador2.b_pointers[k]!=-1){
                                fseek(file,apuntador2.b_pointers[k],SEEK_SET);
                                fread(&carpeta, sizeof(BloqueCarpeta),1,file);
                                for (int c = 0; c < 4; ++c) {
                                    if (carpeta.b_content[c].b_name==rutaS[posAct]){
                                        if (posAct < rutaSize) {
                                            return this->getInodoF(rutaS, posAct + 1, rutaSize,
                                                                   carpeta.b_content[c].b_inodo,file);
                                        }
                                        if (posAct == rutaSize) {
                                            return carpeta.b_content[c].b_inodo;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }else if (i==14){
                fseek(file,inodo.i_block[i],SEEK_SET);
                fread(&apuntador1, sizeof(BloqueApuntador),1,file);
                for (int j = 0; j < 16; ++j) {
                    if (apuntador1.b_pointers[j]!=-1){
                        fseek(file,apuntador1.b_pointers[j],SEEK_SET);
                        fread(&apuntador2, sizeof(BloqueApuntador),1,file);
                        for (int k = 0; k < 16; ++k) {
                            if (apuntador2.b_pointers[k]!=-1){
                                fseek(file,apuntador2.b_pointers[k],SEEK_SET);
                                fread(&apuntador3, sizeof(BloqueApuntador),1,file);
                                for (int l = 0; l < 16; ++l) {
                                    if (apuntador3.b_pointers[l]!=-1) {
                                        fseek(file, apuntador3.b_pointers[l], SEEK_SET);
                                        fread(&carpeta, sizeof(BloqueCarpeta), 1, file);
                                        for (int c = 0; c < 4; ++c) {
                                            if (carpeta.b_content[c].b_name == rutaS[posAct]) {
                                                if (posAct < rutaSize) {
                                                    return this->getInodoF(rutaS, posAct + 1, rutaSize,
                                                                           carpeta.b_content[c].b_inodo, file);
                                                }
                                                if (posAct == rutaSize) {
                                                    return carpeta.b_content[c].b_inodo;
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

void Rep::ejecutarReporte_block() {
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL) {
        this->directorio = this->getCarpetas(this->path);
        this->extension = this->getExtensionFile(this->path);
        system(("sudo -S mkdir -p \'" + this->directorio + "\'").c_str());
        system(("sudo -S chmod -R 777 \'" + this->directorio + "\'").c_str());

        FILE *file;
        if ((file= fopen(nodo->path.c_str(),"rb+"))){
            SuperBloque sb;

            if (nodo->type=='p'){
                fseek(file,nodo->start,SEEK_SET);
            }else if (nodo->type=='l'){
                fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
            }
            fread(&sb, sizeof(SuperBloque),1,file);
            int start=sb.s_bm_inode_start,
                end=start+sb.s_inodes_count;
            TablaInodo inodo;
            BloqueApuntador apuntador1,apuntador2,apuntador3;
            char bit;
            int cont=0;
            char fecha[70];
            string fecha2="";

            FILE *dot= fopen("rep.dot","w");
            fprintf(dot,"digraph G {\n");
            fprintf(dot, "node[shape=none]\n");

            for (int i = start; i < end; ++i) {
                fseek(file,i,SEEK_SET);
                fread(&bit, sizeof(char),1,file);
                if (bit=='1'){
                    fseek(file,(sb.s_inode_start+(cont* sizeof(TablaInodo))),SEEK_SET);
                    fread(&inodo, sizeof(TablaInodo),1,file);

                    for (int i = 0; i < 15; ++i) {
                        if (inodo.i_block[i]!=-1){
                            if (i<12){
                                if (inodo.i_type=='0'){
                                    fprintf(dot, graphBlockCarpeta(inodo.i_block[i],file).c_str());
                                }else if (inodo.i_type=='1'){
                                    fprintf(dot, graphBlockArchivo(inodo.i_block[i],file).c_str());
                                }
                            } else if (i==12){
                                fseek(file,inodo.i_block[i],SEEK_SET);
                                fread(&apuntador1, sizeof(BloqueApuntador),1,file);
                                fprintf(dot, graphBlockApuntador(inodo.i_block[i],file).c_str());
                                for (int j = 0; j < 16; ++j) {
                                    if (apuntador1.b_pointers[j]!=-1){
                                        if (inodo.i_type=='0'){
                                            fprintf(dot, graphBlockCarpeta(apuntador1.b_pointers[j],file).c_str());
                                        }else if (inodo.i_type=='1'){
                                            fprintf(dot, graphBlockArchivo(apuntador1.b_pointers[j],file).c_str());
                                        }
                                    }
                                }
                            }else if (i==13){
                                fseek(file,inodo.i_block[i],SEEK_SET);
                                fread(&apuntador1, sizeof(BloqueApuntador),1,file);
                                fprintf(dot, graphBlockApuntador(inodo.i_block[i],file).c_str());
                                for (int j = 0; j < 16; ++j) {
                                    if (apuntador1.b_pointers[j]!=-1){
                                        fseek(file,apuntador1.b_pointers[j],SEEK_SET);
                                        fread(&apuntador2, sizeof(BloqueApuntador),1,file);
                                        fprintf(dot, graphBlockApuntador(apuntador1.b_pointers[j],file).c_str());
                                        for (int k = 0; k < 16; ++k) {
                                            if (apuntador2.b_pointers[k]!=-1){
                                                if (inodo.i_type=='0'){
                                                    fprintf(dot, graphBlockCarpeta(apuntador2.b_pointers[k],file).c_str());
                                                }else if (inodo.i_type=='1'){
                                                    fprintf(dot, graphBlockArchivo(apuntador2.b_pointers[k],file).c_str());
                                                }
                                            }
                                        }
                                    }
                                }
                            }else if (i==14){
                                fseek(file,inodo.i_block[i],SEEK_SET);
                                fread(&apuntador1, sizeof(BloqueApuntador),1,file);
                                fprintf(dot, graphBlockApuntador(inodo.i_block[i],file).c_str());
                                for (int j = 0; j < 16; ++j) {
                                    if (apuntador1.b_pointers[j]!=-1){
                                        fseek(file,apuntador1.b_pointers[j],SEEK_SET);
                                        fread(&apuntador2, sizeof(BloqueApuntador),1,file);
                                        fprintf(dot, graphBlockApuntador(apuntador1.b_pointers[j],file).c_str());
                                        for (int k = 0; k < 16; ++k) {
                                            if (apuntador2.b_pointers[k]!=-1){
                                                fseek(file,apuntador2.b_pointers[k],SEEK_SET);
                                                fread(&apuntador3, sizeof(BloqueApuntador),1,file);
                                                fprintf(dot, graphBlockApuntador(apuntador2.b_pointers[k],file).c_str());
                                                for (int l = 0; l < 16; ++l) {
                                                    if (apuntador3.b_pointers[l]!=-1){
                                                        if (inodo.i_type=='0'){
                                                            fprintf(dot, graphBlockCarpeta(apuntador3.b_pointers[l],file).c_str());
                                                        }else if (inodo.i_type=='1'){
                                                            fprintf(dot, graphBlockArchivo(apuntador3.b_pointers[l],file).c_str());
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
                cont++;
            }

            fprintf(dot,"}");
            fclose(dot);
            fclose(file);
            string command="sudo -S  dot -T"+this->extension+" rep.dot -o \""+ this->path+"\"";
            system(command.c_str());
            cout<<"REPORTE GENERADO CON EXITO:  BLOCK"<<endl;

        }else{
            cout << "EL DISCO SE MOVIO DE LUGAR PORQUE NO SE ENCUENTRA" << endl;
            return;
        }
    }else{
        cout << "LA MONTURA CON EL ID: " << this->id << " NO EXISTE O NO SE ENCUENTRA EN EL SISTEMA" << endl;
        return;
    }
}

string Rep::graphBlockCarpeta(int pos, FILE *file) {
    string dot="";
    BloqueCarpeta carpeta;
    fseek(file,pos,SEEK_SET);
    fread(&carpeta, sizeof(BloqueCarpeta),1,file);

    dot+="n"+ to_string(pos)+"[label=<<table>\n";
    dot+="<tr>\n";
    dot+=R"(<td colspan="2" bgcolor="#c3f8b6">Bloque Carpeta</td>)";
    dot+="</tr>\n";
    for (int i = 0; i < 4; ++i) {
        string b_name="";
        for (int j = 0; j < 12; ++j) {
            if (carpeta.b_content[i].b_name[j]=='\000'){
                break;
            }
            b_name+=carpeta.b_content[i].b_name[j];
        }

        dot+="<tr>\n";
        dot+="<td colspan=\"2\" bgcolor=\"#b6f8d3\">b_content "+ to_string(i)+"</td>";
        dot+="\n";
        dot+="</tr>\n";
        dot+="<tr>\n";
        dot+="<td>b_name</td>\n";
        dot+="<td>"+b_name+"</td>\n";
        dot+="</tr>\n";
        dot+="<tr>\n";
        dot+="<td>b_inodo</td>\n";
        dot+="<td>"+ to_string(carpeta.b_content[i].b_inodo)+"</td>\n";
        dot+="</tr>\n";
    }
    dot+="</table>>]\n";

    return dot;
}

string Rep::graphBlockArchivo(int pos, FILE *file) {
    string dot="",content="";
    BloqueArchivo archivo;
    fseek(file,pos,SEEK_SET);
    fread(&archivo, sizeof(BloqueArchivo),1,file);

    for (int i = 0; i < 64; ++i) {
        if (archivo.b_content[i]=='\000'){
            break;
        }
        content+=archivo.b_content[i];
    }

    dot+="n"+ to_string(pos)+"[label=<<table>\n";
    dot+="<tr>\n";
    dot+=R"(<td bgcolor="#c3f8b6">Bloque Archivo</td>)";
    dot+="</tr>\n";
    dot+="<tr>\n";
    dot+="<td>"+content+"</td>\n";
    dot+="</tr>\n";
    dot+="</table>>]\n";

    return  dot;
}

string Rep::graphBlockApuntador(int pos, FILE *file) {
    string dot="";
    BloqueApuntador apuntador;
    fseek(file,pos,SEEK_SET);
    fread(&apuntador, sizeof(BloqueApuntador),1,file);

    dot+="n"+ to_string(pos)+"[label=<<table>\n";
    dot+="<tr>\n";
    dot+=R"(<td colspan="2" bgcolor="#c3f8b6">Bloque Apuntadores</td>)";
    dot+="</tr>\n";
    for (int i = 0; i < 16; ++i) {
        dot+="<tr>\n";
        dot+="<td>b_pointer "+ to_string(i)+"</td>\n";
        dot+="<td>"+ to_string(apuntador.b_pointers[i])+"</td>\n";
        dot+="</tr>\n";
    }
    dot+="</table>>]\n";

    return  dot;
}

void Rep::ejecutarReporte_tree() {
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL) {
        this->directorio = this->getCarpetas(this->path);
        this->extension = this->getExtensionFile(this->path);
        system(("sudo -S mkdir -p \'" + this->directorio + "\'").c_str());
        system(("sudo -S chmod -R 777 \'" + this->directorio + "\'").c_str());

        SuperBloque superBloqueReporteTree;
        FILE *file;
        if ((file= fopen(nodo->path.c_str(),"rb+"))){
            if (nodo->type=='l'){
                fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
            }
            else if (nodo->type=='p'){
                fseek(file,nodo->start,SEEK_SET);
            }
            fread(&superBloqueReporteTree, sizeof(SuperBloque), 1, file);
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
                fseek(file,i,SEEK_SET);
                fread(&bit, sizeof(char),1,file);
                if (bit=='1'){
                    int direccionInodo=(superBloqueReporteTree.s_inode_start + (contador * sizeof(TablaInodo)));
                    fseek(file, direccionInodo, SEEK_SET);
                    fread(&tablaInodo, sizeof(TablaInodo), 1, file);
                    fprintf(dot, dibujarInodoReporteTree(direccionInodo, file).c_str());
                    for (int i = 0; i < 15; ++i) {
                        if (tablaInodo.i_block[i] != -1){
                            if (i<12){
                                if (tablaInodo.i_type == '0'){
                                    fprintf(dot, dibujarBlkReporteTree(tablaInodo.i_block[i], 0, file).c_str());
                                }else if (tablaInodo.i_type == '1'){
                                    fprintf(dot, dibujarBlkReporteTree(tablaInodo.i_block[i], 1, file).c_str());
                                }
                                fprintf(dot, this->enlaces(direccionInodo, tablaInodo.i_block[i]).c_str());
                            }
                            else if (i==12){
                                fseek(file, tablaInodo.i_block[i], SEEK_SET);
                                fread(&blkApuntador1, sizeof(BloqueApuntador), 1, file);
                                fprintf(dot, dibujarBlkReporteTree(tablaInodo.i_block[i], 2, file).c_str());
                                fprintf(dot, this->enlaces(direccionInodo, tablaInodo.i_block[i]).c_str());
                                for (int j = 0; j < 16; ++j) {
                                    if (blkApuntador1.b_pointers[j] != -1){
                                        if (tablaInodo.i_type == '0'){
                                            fprintf(dot, dibujarBlkReporteTree(blkApuntador1.b_pointers[j], 0, file).c_str());
                                        }else if (tablaInodo.i_type == '1'){
                                            fprintf(dot, dibujarBlkReporteTree(blkApuntador1.b_pointers[j], 1, file).c_str());
                                        }
                                        fprintf(dot, this->enlaces(tablaInodo.i_block[i], blkApuntador1.b_pointers[j]).c_str());
                                    }
                                }
                            }
                            else if (i==13){
                                fseek(file, tablaInodo.i_block[i], SEEK_SET);
                                fread(&blkApuntador1, sizeof(BloqueApuntador), 1, file);
                                fprintf(dot, dibujarBlkReporteTree(tablaInodo.i_block[i], 2, file).c_str());
                                fprintf(dot, this->enlaces(direccionInodo, tablaInodo.i_block[i]).c_str());
                                for (int j = 0; j < 16; ++j) {
                                    if (blkApuntador1.b_pointers[j] != -1){
                                        fseek(file, blkApuntador1.b_pointers[j], SEEK_SET);
                                        fread(&blkApuntador2, sizeof(BloqueApuntador), 1, file);
                                        fprintf(dot, dibujarBlkReporteTree(blkApuntador1.b_pointers[j], 2, file).c_str());
                                        fprintf(dot, this->enlaces(tablaInodo.i_block[i], blkApuntador1.b_pointers[j]).c_str());
                                        for (int k = 0; k < 16; ++k) {
                                            if (blkApuntador2.b_pointers[k] != -1){
                                                if (tablaInodo.i_type == '0'){
                                                    fprintf(dot,
                                                            dibujarBlkReporteTree(blkApuntador2.b_pointers[k], 0, file).c_str());
                                                }else if (tablaInodo.i_type == '1'){
                                                    fprintf(dot,
                                                            dibujarBlkReporteTree(blkApuntador2.b_pointers[k], 1, file).c_str());
                                                }
                                                fprintf(dot, this->enlaces(blkApuntador1.b_pointers[j],
                                                                           blkApuntador2.b_pointers[k]).c_str());
                                            }
                                        }
                                    }
                                }
                            }
                            else if (i==14){
                                fseek(file, tablaInodo.i_block[i], SEEK_SET);
                                fread(&blkApuntador1, sizeof(BloqueApuntador), 1, file);
                                fprintf(dot, dibujarBlkReporteTree(tablaInodo.i_block[i], 2, file).c_str());
                                fprintf(dot, this->enlaces(direccionInodo, tablaInodo.i_block[i]).c_str());
                                for (int j = 0; j < 16; ++j) {
                                    if (blkApuntador1.b_pointers[j] != -1){
                                        fseek(file, blkApuntador1.b_pointers[j], SEEK_SET);
                                        fread(&blkApuntador2, sizeof(BloqueApuntador), 1, file);
                                        fprintf(dot, dibujarBlkReporteTree(blkApuntador1.b_pointers[j], 2, file).c_str());
                                        fprintf(dot, this->enlaces(tablaInodo.i_block[i], blkApuntador1.b_pointers[j]).c_str());
                                        for (int k = 0; k < 16; ++k) {
                                            if (blkApuntador2.b_pointers[k] != -1){
                                                fseek(file, blkApuntador2.b_pointers[k], SEEK_SET);
                                                fread(&blkApuntador3, sizeof(BloqueApuntador), 1, file);
                                                fprintf(dot, dibujarBlkReporteTree(blkApuntador2.b_pointers[k], 2, file).c_str());
                                                fprintf(dot, this->enlaces(blkApuntador1.b_pointers[j],
                                                                           blkApuntador2.b_pointers[k]).c_str());
                                                for (int l = 0; l < 16; ++l) {
                                                    if (blkApuntador3.b_pointers[l] != -1){
                                                        if (tablaInodo.i_type == '0'){
                                                            fprintf(dot,
                                                                    dibujarBlkReporteTree(blkApuntador3.b_pointers[l],
                                                                                          0, file).c_str());
                                                        }else if (tablaInodo.i_type == '1'){
                                                            fprintf(dot,
                                                                    dibujarBlkReporteTree(blkApuntador3.b_pointers[l],
                                                                                          1, file).c_str());
                                                        }
                                                        fprintf(dot, this->enlaces(blkApuntador2.b_pointers[k],
                                                                                   blkApuntador3.b_pointers[l]).c_str());
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
            fclose(file);
            string command="sudo -S  dot -T"+this->extension+" tree.dot -o \""+ this->path+"\"";
            system(command.c_str());
            cout<<"COMANDO EJECUTADO CON EXITO, REPORTE TREE GENERADO"<<endl;

        }
        else{
            cout <<"EL DISCO SE MOVIO DE RUTA O NO EXISTE"<<endl;
            return;
        }
    }else{
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

    cadenaDot+= "n" + to_string(direccionInodo) + "[label=<<table>";
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
    string dot="";
    if (type==0){
        BloqueCarpeta carpeta;
        fseek(file,pos,SEEK_SET);
        fread(&carpeta, sizeof(BloqueCarpeta),1,file);

        dot+="n"+ to_string(pos)+"[label=<<table>\n";
        dot+="<tr>\n";
        dot+=R"(<td colspan="2" bgcolor="lightcoral">Bloque Carpeta</td>)";
        dot+="</tr>\n";
        for (int i = 0; i < 4; ++i) {
            string b_name="";
            for (int j = 0; j < 12; ++j) {
                if (carpeta.b_content[i].b_name[j]=='\000'){
                    break;
                }
                b_name+=carpeta.b_content[i].b_name[j];
            }

            dot+="<tr>\n";
            dot+="<td>"+b_name+"</td>\n";
            dot+="<td port=\""+ to_string(carpeta.b_content[i].b_inodo)+"\">"+ to_string(carpeta.b_content[i].b_inodo)+"</td>\n";
            dot+="</tr>\n";
        }
        dot+="</table>>]\n";

        for (int i = 0; i < 4; ++i) {
            string name1=carpeta.b_content[i].b_name;
            if (carpeta.b_content[i].b_inodo!=-1 && (name1!="." && name1!="..")){
                dot+= enlaces(pos, carpeta.b_content[i].b_inodo);
            }
        }
    }else if (type==1){
        string content="";
        BloqueArchivo archivo;
        fseek(file,pos,SEEK_SET);
        fread(&archivo, sizeof(BloqueArchivo),1,file);

        for (int i = 0; i < 64; ++i) {
            if (archivo.b_content[i]=='\000'){
                break;
            }
            content+=archivo.b_content[i];
        }

        dot+="n"+ to_string(pos)+"[label=<<table>\n";
        dot+="<tr>\n";
        dot+=R"(<td colspan="2" bgcolor="lemonchiffon">Bloque Archivo</td>)";
        dot+="</tr>\n";
        dot+="<tr>\n";
        dot+="<td>"+content+"</td>\n";
        dot+="</tr>\n";
        dot+="</table>>]\n";

    } else{
        BloqueApuntador apuntador;
        fseek(file,pos,SEEK_SET);
        fread(&apuntador, sizeof(BloqueApuntador),1,file);

        dot+="n"+ to_string(pos)+"[label=<<table>\n";
        dot+="<tr>\n";
        dot+=R"(<td colspan="2" bgcolor="#c3f8b6">Bloque Apuntadores</td>)";
        dot+="</tr>\n";
        for (int i = 0; i < 16; ++i) {
            dot+="<tr>\n";
            dot+="<td>b_pointer "+ to_string(i)+"</td>\n";
            dot+="<td>"+ to_string(apuntador.b_pointers[i])+"</td>\n";
            dot+="</tr>\n";
        }
        dot+="</table>>]\n";
    }
    return dot;
}

string Rep::enlaces(int inicio, int final) {
    string dot="";
    dot+="n"+ to_string(inicio)+":"+ to_string(final)+"->n"+ to_string(final)+";\n";
    return dot;
}

string Rep::getDireccionCarpetas(int direccionInodoCarpeta) {

    return std::string();
}

