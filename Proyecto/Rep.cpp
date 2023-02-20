#include "Rep.h"
#include "Structs.h"
#include "Nodo_M.h"
#include <string>
#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <sstream>

Rep::Rep() {
    this->path=" ";
    this->name=" ";
    this->id=" ";
    this->ruta=" ";
    this->directorio=" ";
    this->extension=" ";
    this->mountList=new MountList();
}

void Rep::generate() {
    if (this->path!=" "){
        if (this->id!=" "){
            if (this->name=="mbr"){
                this->mbr();
            }else if (this->name=="disk"){
                this->disk();
            }else if (this->name=="inode"){
                this->inode();
            }else if (this->name=="journaling"){
                this->journaling();
            }else if (this->name=="block"){
                this->block();
            }else if (this->name=="bm_inode"){
                this->bm_inode();
            }else if (this->name=="bm_block"){
                this->bm_block();
            }else if (this->name=="tree"){
                this->tree();
            }else if (this->name=="sb"){
                this->sb();
            }else if (this->name=="file"){
                this->file();
            }else if (this->name=="ls"){
                this->ls();
            }else{
                cout << "EL NOMBRE ASIGNADO PARA EL REPORTE ES INVALIDO"<< endl;
            }
        }else{
            cout << "EL ID DE LA PARTICION ES OBLIGATORIO"<< endl;
        }
    }else{
        cout << "EL PARAMETRO DE LA UBICACION DEL REPORTE ES OBLIGATORIO"<< endl;
    }

}

void Rep::mbr() {
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL){
        this->directorio=this->getDirectorio(this->path);
        this->extension=this->getExtension(this->path);
        system(("sudo -S mkdir -p \'"+this->directorio+"\'").c_str());
        system(("sudo -S chmod -R 777 \'"+this->directorio+"\'").c_str());

        FILE *file;
        if ((file= fopen(nodo->path.c_str(),"rb+"))){
            MBR mbr;
            fseek(file,0,SEEK_SET);
            fread(&mbr,sizeof (MBR),1,file);

            FILE *dot= fopen("rep.dot","w");
            fprintf(dot,"digraph G {\n");
            fprintf(dot, "node[shape=none]\n");
            fprintf(dot, "start[label=<<table>");
            fprintf(dot,R"(<tr><td colspan="2" bgcolor="#6308d8"><font color="white">REPORTE DE MBR</font></td></tr>)");
            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td>mbr_tamano</td>)");
            fprintf(dot,(R"(<td>)"+ to_string(mbr.mbr_tamano)+"</td>").c_str());
            fprintf(dot,R"(</tr>)");
            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td bgcolor="#b48be8">mbr_fecha_creacion</td>)");
            char fecha[70];
            strftime(fecha,sizeof (fecha),"%Y-%m-%d %H:%M:%S",localtime(&mbr.mbr_fecha_creacion));
            string f=fecha;
            fprintf(dot,(R"(<td bgcolor="#b48be8">)"+f+"</td>").c_str());
            fprintf(dot,R"(</tr>)");
            fprintf(dot,R"(<tr>)");
            fprintf(dot,R"(<td>mbr_disk_signature</td>)");
            fprintf(dot,(R"(<td>)"+ to_string(mbr.mbr_dsk_signature)+"</td>").c_str());
            fprintf(dot,R"(</tr>)");

            for (int i = 0; i < 4; ++i) {
                if (mbr.mbr_partition[i].part_start!=-1){
                    fprintf(dot,R"(<tr><td colspan="2" bgcolor="#6308d8"><font color="white">Particion</font></td></tr>)");
                    fprintf(dot,R"(<tr>)");
                    fprintf(dot,R"(<td>part_status</td>)");
                    fprintf(dot,(R"(<td>)"+string(1,mbr.mbr_partition[i].part_status)+"</td>").c_str());
                    fprintf(dot,R"(</tr>)");
                    fprintf(dot,R"(<tr>)");
                    fprintf(dot,R"(<td bgcolor="#b48be8">part_type</td>)");
                    fprintf(dot,(R"(<td bgcolor="#b48be8">)"+string (1,mbr.mbr_partition[i].part_type)+"</td>").c_str());
                    fprintf(dot,R"(</tr>)");
                    fprintf(dot,R"(<tr>)");
                    fprintf(dot,R"(<td>part_fit</td>)");
                    fprintf(dot,(R"(<td>)"+ string(1,mbr.mbr_partition[i].part_fit)+"</td>").c_str());
                    fprintf(dot,R"(</tr>)");
                    fprintf(dot,R"(<tr>)");
                    fprintf(dot,R"(<td bgcolor="#b48be8">part_start</td>)");
                    fprintf(dot,(R"(<td bgcolor="#b48be8">)"+to_string(mbr.mbr_partition[i].part_s)+"</td>").c_str());
                    fprintf(dot,R"(</tr>)");
                    fprintf(dot,R"(<tr>)");
                    fprintf(dot,R"(<td>part_size</td>)");
                    fprintf(dot,(R"(<td>)"+ to_string(mbr.mbr_partition[i].part_s)+"</td>").c_str());
                    fprintf(dot,R"(</tr>)");
                    fprintf(dot,R"(<tr>)");
                    fprintf(dot,R"(<td bgcolor="#b48be8">part_name</td>)");
                    string name1=mbr.mbr_partition[i].part_name;
                    fprintf(dot,(R"(<td bgcolor="#b48be8">)"+name1+"</td>").c_str());
                    fprintf(dot,R"(</tr>)");

                    if (mbr.mbr_partition[i].part_type=='e'){
                        EBR ebr;
                        fseek(file,mbr.mbr_partition[i].part_start,SEEK_SET);
                        fread(&ebr,sizeof(EBR),1,file);
                        if (!(ebr.part_s==-1 && ebr.part_next==-1)){
                            while (true){
                                fprintf(dot,R"(<tr><td colspan="2" bgcolor="#ff738c"><font color="white">Particion Logica</font></td></tr>)");
                                fprintf(dot,R"(<tr>)");
                                fprintf(dot,R"(<td>part_status</td>)");
                                fprintf(dot,(R"(<td>)"+string(1,ebr.part_status)+"</td>").c_str());
                                fprintf(dot,R"(</tr>)");
                                fprintf(dot,R"(<tr>)");
                                fprintf(dot,R"(<td bgcolor="#ffb2c0">part_next</td>)");
                                fprintf(dot,(R"(<td bgcolor="#ffb2c0">)"+ to_string(ebr.part_next)+"</td>").c_str());
                                fprintf(dot,R"(</tr>)");
                                fprintf(dot,R"(<tr>)");
                                fprintf(dot,R"(<td>part_fit</td>)");
                                fprintf(dot,(R"(<td>)"+string(1,ebr.part_fit)+"</td>").c_str());
                                fprintf(dot,R"(</tr>)");
                                fprintf(dot,R"(<tr>)");
                                fprintf(dot,R"(<td bgcolor="#ffb2c0">part_start</td>)");
                                fprintf(dot,(R"(<td bgcolor="#ffb2c0">)"+ to_string(ebr.part_start)+"</td>").c_str());
                                fprintf(dot,R"(</tr>)");
                                fprintf(dot,R"(<tr>)");
                                fprintf(dot,R"(<td>part_size</td>)");
                                fprintf(dot,(R"(<td>)"+ to_string(ebr.part_s)+"</td>").c_str());
                                fprintf(dot,R"(</tr>)");
                                fprintf(dot,R"(<tr>)");
                                fprintf(dot,R"(<td bgcolor="#ffb2c0">part_name</td>)");
                                string name1=ebr.part_name;
                                fprintf(dot,(R"(<td bgcolor="#ffb2c0">)"+name1+"</td>").c_str());
                                fprintf(dot,R"(</tr>)");
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

            fprintf(dot,"</table>>];\n");
            fprintf(dot,"}");
            fclose(dot);
            fclose(file);
            string command="sudo -S  dot -T"+this->extension+" rep.dot -o \""+ this->path+"\"";
            system(command.c_str());
            cout<<"SE GENERO EL REPORTE MBR"<<endl;
        }else{
            cout <<"DISCO INEXISTENTE"<<endl;
            return;
        }
    }else{
        cout <<"NO SE HA ENCONTRADO ALGUNA MONTURA CON EL ID: "<< this->id<<endl;
        return;
    }
}

void Rep::disk() {
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL){
        this->directorio=this->getDirectorio(this->path);
        this->extension=this->getExtension(this->path);
        system(("sudo -S mkdir -p \'"+this->directorio+"\'").c_str());
        system(("sudo -S chmod -R 777 \'"+this->directorio+"\'").c_str());

        FILE *file;
        if ((file= fopen(nodo->path.c_str(),"rb+"))){
            MBR mbr;
            fseek(file,0,SEEK_SET);
            fread(&mbr,sizeof (MBR),1,file);
            int tamanioT=mbr.mbr_tamano;

            FILE *dot= fopen("rep.dot","w");
            fprintf(dot,"digraph G {\n");
            fprintf(dot, "node[shape=none]\n");
            fprintf(dot, "start[label=<<table><tr>");
            fprintf(dot, "<td rowspan=\"2\">MBR</td>");

            int i=0;
            int inicio=sizeof(MBR);
            while (i<4){
                if (mbr.mbr_partition[i].part_start!=-1){
                    if (mbr.mbr_partition[i].part_type=='p'){
                        float p1=(mbr.mbr_partition[i].part_s*1.0)/tamanioT;
                        float porcentaje=p1*100.0;
                        string name1=mbr.mbr_partition[i].part_name;
                        fprintf(dot, ("<td rowspan=\"2\">"+name1+" <br/>"+ to_string(lround(porcentaje))+"</td>").c_str());
                        if (i!=3){
                            if ((mbr.mbr_partition[i].part_start+mbr.mbr_partition[i].part_s)<mbr.mbr_partition[i+1].part_start){
                                porcentaje=((mbr.mbr_partition[i+1].part_start-(mbr.mbr_partition[i].part_start+mbr.mbr_partition[i].part_s))/tamanioT)*100;
                                fprintf(dot, ("<td rowspan=\"2\">LIBRE <br/>"+ to_string(lround(porcentaje))+"</td>").c_str());
                            }
                        }else if ((mbr.mbr_partition[i].part_start+mbr.mbr_partition[i].part_s)<tamanioT){
                            porcentaje=((tamanioT-(mbr.mbr_partition[i].part_start+mbr.mbr_partition[i].part_s))/tamanioT)*100;
                            fprintf(dot, ("<td rowspan=\"2\">LIBRE <br/>"+ to_string(lround(porcentaje))+"</td>").c_str());
                        }

                    }else if (mbr.mbr_partition[i].part_type=='e'){
                        float porcentaje=((mbr.mbr_partition[i].part_s)/tamanioT)*100.0;
                        fprintf(dot, "<td rowspan=\"2\">EXTENDIDA</td>");
                        EBR ebr;
                        fseek(file,mbr.mbr_partition[i].part_start,SEEK_SET);
                        fread(&ebr,sizeof(EBR),1,file);
                        if (!(ebr.part_s==-1 && ebr.part_next==-1)){
                            if (ebr.part_s>-1){
                                string name1=ebr.part_name;
                                fprintf(dot, ("<td rowspan=\"2\">EBR <br/>"+name1+"</td>").c_str());
                                porcentaje=((ebr.part_s*1.0)/tamanioT)*100.0;
                                fprintf(dot, ("<td rowspan=\"2\">Logica <br/>"+ to_string(lround(porcentaje))+"</td>").c_str());
                            }else{
                                string name1=ebr.part_name;
                                fprintf(dot, "<td rowspan=\"2\">EBR</td>");
                                porcentaje=(((ebr.part_next-ebr.part_start)*1.0)/tamanioT)*100.0;
                                fprintf(dot, ("<td rowspan=\"2\">Libre <br/>"+ to_string(lround(porcentaje))+"</td>").c_str());

                            }
                            fseek(file,ebr.part_next,SEEK_SET);
                            fread(&ebr,sizeof (EBR),1,file);
                            while (true){
                                string name1=ebr.part_name;
                                fprintf(dot, ("<td rowspan=\"2\">EBR <br/>"+name1+"</td>").c_str());
                                porcentaje=((ebr.part_s*1.0)/tamanioT)*100.0;
                                fprintf(dot, ("<td rowspan=\"2\">Logica <br/>"+ to_string(lround(porcentaje))+"</td>").c_str());
                                if (ebr.part_next==-1){
                                    if ((ebr.part_start+ebr.part_s)<mbr.mbr_partition[i].part_s){
                                        porcentaje=(((mbr.mbr_partition[i].part_s-(ebr.part_start+ebr.part_s))*1.0)/tamanioT)*100.0;
                                        fprintf(dot, ("<td rowspan=\"2\">Libre <br/>"+ to_string(lround(porcentaje))+"</td>").c_str());
                                    }
                                    break;
                                }else{
                                    if ((ebr.part_start+ebr.part_s)<ebr.part_next){
                                        porcentaje=(((ebr.part_next-(ebr.part_start+ebr.part_s))*1.0)/tamanioT)*100.0;
                                        fprintf(dot, ("<td rowspan=\"2\">Libre <br/>"+ to_string(lround(porcentaje))+"</td>").c_str());
                                    }
                                }
                                fseek(file,ebr.part_next,SEEK_SET);
                                fread(&ebr,sizeof (EBR),1,file);
                            }
                        }
                        fprintf(dot, "<td rowspan=\"2\">EXTENDIDA</td>");
                    }
                    inicio=mbr.mbr_partition[i].part_start+mbr.mbr_partition[i].part_s;
                }else{
                    i++;
                    while (i<4){
                        if (mbr.mbr_partition[i].part_start!=-1){
                            int porcentaje=((mbr.mbr_partition[i].part_start-inicio)/(tamanioT*1.0))*100.0;
                            fprintf(dot, ("<td rowspan=\"2\">LIBRE <br/>"+ to_string(lround(porcentaje))+"</td>").c_str());
                            break;
                        }
                        i++;
                    }
                    if (i==4){
                        float porcentaje=(((tamanioT-inicio)*1.0)/tamanioT)*(100.0);
                        fprintf(dot, ("<td rowspan=\"2\">LIBRE <br/>"+ to_string(lround(porcentaje))+"</td>").c_str());
                        goto t0;
                    }
                    i--;
                }
                i++;
            }
            t0:
            fprintf(dot,"</tr></table>>];\n");
            fprintf(dot,"}");
            fclose(dot);
            fclose(file);
            string command="sudo -S  dot -T"+this->extension+" rep.dot -o \""+ this->path+"\"";
            system(command.c_str());
            cout<<"SE GENERO EL REPORTE DISK"<<endl;
        }else{
            cout <<"DISCO INEXISTENTE"<<endl;
            return;
        }
    }else{
        cout <<"NO SE HA ENCONTRADO ALGUNA MONTURA CON EL ID: "<< this->id<<endl;
        return;
    }
}

void Rep::sb() {
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL) {
        this->directorio = this->getDirectorio(this->path);
        this->extension = this->getExtension(this->path);
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
            cout<<"SE GENERO EL REPORTE SB"<<endl;
        }else{
            cout <<"DISCO INEXISTENTE"<<endl;
            return;
        }
    }else{
        cout <<"NO SE HA ENCONTRADO ALGUNA MONTURA CON EL ID: "<< this->id<<endl;
        return;
    }
}

void Rep::inode() {
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL) {
        this->directorio = this->getDirectorio(this->path);
        this->extension = this->getExtension(this->path);
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
            cout<<"SE GENERO EL REPORTE INODE"<<endl;

        }else{
            cout <<"DISCO INEXISTENTE"<<endl;
            return;
        }
    }else{
        cout <<"NO SE HA ENCONTRADO ALGUNA MONTURA CON EL ID: "<< this->id<<endl;
        return;
    }
}

void Rep::bm_inode() {
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL) {
        this->directorio = this->getDirectorio(this->path);
        this->extension = this->getExtension(this->path);
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
            cout<<"SE GENERO EL REPORTE BM INODE"<<endl;
        }else{
            cout <<"DISCO INEXISTENTE"<<endl;
            return;
        }
    }else{
        cout <<"NO SE HA ENCONTRADO ALGUNA MONTURA CON EL ID: "<< this->id<<endl;
        return;
    }
}

void Rep::bm_block() {
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL) {
        this->directorio = this->getDirectorio(this->path);
        this->extension = this->getExtension(this->path);
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
            cout<<"SE GENERO EL REPORTE BM BLOCK"<<endl;
        }else{
            cout <<"DISCO INEXISTENTE"<<endl;
            return;
        }
    }else{
        cout <<"NO SE HA ENCONTRADO ALGUNA MONTURA CON EL ID: "<< this->id<<endl;
        return;
    }
}

void Rep::file() {
    if (this->ruta!=" "){
        Nodo_M *nodo=this->mountList->buscar(this->id);
        if (nodo!=NULL) {

            this->directorio = this->getDirectorio(this->path);
            this->extension = this->getExtension(this->path);
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
                    fread(&this->sbb, sizeof(SuperBloque),1,file);
                }else if (nodo->type=='l'){
                    fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
                    fread(&this->sbb, sizeof(SuperBloque),1,file);
                }

                TablaInodo inodo;

                int posInodoF=this->getInodoF(rutaS,0,rutaS.size()-1,this->sbb.s_inode_start,file);

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
                cout<<"SE GENERO EL REPORTE FILE DE "<<rutaS[rutaS.size()-1]<<endl;

            }else{
                cout <<"DISCO INEXISTENTE"<<endl;
            }
        }else{
            cout <<"NO SE HA ENCONTRADO ALGUNA MONTURA CON EL ID: "<< this->id<<endl;
            return;
        }
    }
}

void Rep::ls() {
    if (this->ruta!=" "){
        Nodo_M *nodo=this->mountList->buscar(this->id);
        if (nodo!=NULL) {

            this->directorio = this->getDirectorio(this->path);
            this->extension = this->getExtension(this->path);
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
                    fread(&this->sbb, sizeof(SuperBloque),1,file);
                }else if (nodo->type=='l'){
                    fseek(file,nodo->start+ sizeof(EBR),SEEK_SET);
                    fread(&this->sbb, sizeof(SuperBloque),1,file);
                }

                TablaInodo inodo;
                int posInodoF=0;
                vector<string> rutaS;

                if (this->ruta=="/"){
                    posInodoF=this->sbb.s_inode_start;
                    rutaS.push_back("/");
                }else{
                    rutaS= this->splitRuta(this->ruta);
                    if (rutaS.empty()){
                        cout<<"RUTA INVALIDA"<<endl;
                        fclose(file);
                        return;
                    }
                    posInodoF=this->getInodoF(rutaS,0,rutaS.size()-1,this->sbb.s_inode_start,file);
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
                cout<<"SE GENERO EL REPORTE LS DE "<<rutaS[rutaS.size()-1]<<endl;

            }else{
                cout <<"DISCO INEXISTENTE"<<endl;
            }
        }else{
            cout <<"NO SE HA ENCONTRADO ALGUNA MONTURA CON EL ID: "<< this->id<<endl;
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
    string content=this->getContent(this->sbb.s_inode_start+ sizeof(TablaInodo),file);
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
    string content=this->getContent(this->sbb.s_inode_start+ sizeof(TablaInodo),file);
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

void Rep::journaling() {
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL) {
        this->directorio = this->getDirectorio(this->path);
        this->extension = this->getExtension(this->path);
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
            cout <<"DISCO INEXISTENTE"<<endl;
        }

    }else{
        cout <<"NO SE HA ENCONTRADO ALGUNA MONTURA CON EL ID: "<< this->id<<endl;
        return;
    }
}

string Rep::getDirectorio(std::string path) {
    string aux=path;
    size_t p=0;
    string directorio="";
    while ((p=aux.find("/"))!= string::npos){
        directorio += aux.substr(0,p)+"/";
        aux.erase(0,p+1);
    }
    return directorio;
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
string Rep::getExtension(std::string path) {
    int i= path.find('.');
    string extension=path.substr(i+1,path.length());
    return extension;
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

void Rep::block() {
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL) {
        this->directorio = this->getDirectorio(this->path);
        this->extension = this->getExtension(this->path);
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
            cout<<"SE GENERO EL REPORTE BLOCK"<<endl;

        }else{
            cout <<"DISCO INEXISTENTE"<<endl;
            return;
        }
    }else{
        cout <<"NO SE HA ENCONTRADO ALGUNA MONTURA CON EL ID: "<< this->id<<endl;
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

void Rep::tree() {
    Nodo_M *nodo=this->mountList->buscar(this->id);
    if (nodo!=NULL) {
        this->directorio = this->getDirectorio(this->path);
        this->extension = this->getExtension(this->path);
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
            fprintf(dot, "rankdir=LR;\n");
            fprintf(dot, "node[shape=none]\n");

            for (int i = start; i < end; ++i) {
                fseek(file,i,SEEK_SET);
                fread(&bit, sizeof(char),1,file);
                if (bit=='1'){
                    int posInodo=(sb.s_inode_start+(cont* sizeof(TablaInodo)));
                    fseek(file,posInodo,SEEK_SET);
                    fread(&inodo, sizeof(TablaInodo),1,file);
                    fprintf(dot, treeInodo(posInodo,file).c_str());
                    for (int i = 0; i < 15; ++i) {
                        if (inodo.i_block[i]!=-1){
                            if (i<12){
                                if (inodo.i_type=='0'){
                                    fprintf(dot, treeBlock(inodo.i_block[i],0,file).c_str());
                                }else if (inodo.i_type=='1'){
                                    fprintf(dot, treeBlock(inodo.i_block[i],1,file).c_str());
                                }
                                fprintf(dot,this->conexiones(posInodo,inodo.i_block[i]).c_str());
                            } else if (i==12){
                                fseek(file,inodo.i_block[i],SEEK_SET);
                                fread(&apuntador1, sizeof(BloqueApuntador),1,file);
                                fprintf(dot, treeBlock(inodo.i_block[i],2,file).c_str());
                                fprintf(dot,this->conexiones(posInodo,inodo.i_block[i]).c_str());
                                for (int j = 0; j < 16; ++j) {
                                    if (apuntador1.b_pointers[j]!=-1){
                                        if (inodo.i_type=='0'){
                                            fprintf(dot, treeBlock(apuntador1.b_pointers[j],0,file).c_str());
                                        }else if (inodo.i_type=='1'){
                                            fprintf(dot, treeBlock(apuntador1.b_pointers[j],1,file).c_str());
                                        }
                                        fprintf(dot,this->conexiones(inodo.i_block[i],apuntador1.b_pointers[j]).c_str());
                                    }
                                }
                            }else if (i==13){
                                fseek(file,inodo.i_block[i],SEEK_SET);
                                fread(&apuntador1, sizeof(BloqueApuntador),1,file);
                                fprintf(dot, treeBlock(inodo.i_block[i],2,file).c_str());
                                fprintf(dot,this->conexiones(posInodo,inodo.i_block[i]).c_str());
                                for (int j = 0; j < 16; ++j) {
                                    if (apuntador1.b_pointers[j]!=-1){
                                        fseek(file,apuntador1.b_pointers[j],SEEK_SET);
                                        fread(&apuntador2, sizeof(BloqueApuntador),1,file);
                                        fprintf(dot, treeBlock(apuntador1.b_pointers[j],2,file).c_str());
                                        fprintf(dot,this->conexiones(inodo.i_block[i],apuntador1.b_pointers[j]).c_str());
                                        for (int k = 0; k < 16; ++k) {
                                            if (apuntador2.b_pointers[k]!=-1){
                                                if (inodo.i_type=='0'){
                                                    fprintf(dot, treeBlock(apuntador2.b_pointers[k],0,file).c_str());
                                                }else if (inodo.i_type=='1'){
                                                    fprintf(dot, treeBlock(apuntador2.b_pointers[k],1,file).c_str());
                                                }
                                                fprintf(dot,this->conexiones(apuntador1.b_pointers[j],apuntador2.b_pointers[k]).c_str());
                                            }
                                        }
                                    }
                                }
                            }else if (i==14){
                                fseek(file,inodo.i_block[i],SEEK_SET);
                                fread(&apuntador1, sizeof(BloqueApuntador),1,file);
                                fprintf(dot, treeBlock(inodo.i_block[i],2,file).c_str());
                                fprintf(dot,this->conexiones(posInodo,inodo.i_block[i]).c_str());
                                for (int j = 0; j < 16; ++j) {
                                    if (apuntador1.b_pointers[j]!=-1){
                                        fseek(file,apuntador1.b_pointers[j],SEEK_SET);
                                        fread(&apuntador2, sizeof(BloqueApuntador),1,file);
                                        fprintf(dot, treeBlock(apuntador1.b_pointers[j],2,file).c_str());
                                        fprintf(dot,this->conexiones(inodo.i_block[i],apuntador1.b_pointers[j]).c_str());
                                        for (int k = 0; k < 16; ++k) {
                                            if (apuntador2.b_pointers[k]!=-1){
                                                fseek(file,apuntador2.b_pointers[k],SEEK_SET);
                                                fread(&apuntador3, sizeof(BloqueApuntador),1,file);
                                                fprintf(dot, treeBlock(apuntador2.b_pointers[k],2,file).c_str());
                                                fprintf(dot,this->conexiones(apuntador1.b_pointers[j],apuntador2.b_pointers[k]).c_str());
                                                for (int l = 0; l < 16; ++l) {
                                                    if (apuntador3.b_pointers[l]!=-1){
                                                        if (inodo.i_type=='0'){
                                                            fprintf(dot, treeBlock(apuntador3.b_pointers[l],0,file).c_str());
                                                        }else if (inodo.i_type=='1'){
                                                            fprintf(dot, treeBlock(apuntador3.b_pointers[l],1,file).c_str());
                                                        }
                                                        fprintf(dot,this->conexiones(apuntador2.b_pointers[k],apuntador3.b_pointers[l]).c_str());
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
            cout<<"SE GENERO EL REPORTE TREE"<<endl;

        }else{
            cout <<"DISCO INEXISTENTE"<<endl;
            return;
        }
    }else{
        cout <<"NO SE HA ENCONTRADO ALGUNA MONTURA CON EL ID: "<< this->id<<endl;
        return;
    }
}

string Rep::treeBlock(int pos, int type, FILE *file) {
    string dot="";
    if (type==0){
        BloqueCarpeta carpeta;
        fseek(file,pos,SEEK_SET);
        fread(&carpeta, sizeof(BloqueCarpeta),1,file);

        dot+="n"+ to_string(pos)+"[label=<<table>\n";
        dot+="<tr>\n";
        dot+=R"(<td colspan="2" bgcolor="#f34037">Bloque Carpeta</td>)";
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
                dot+= conexiones(pos,carpeta.b_content[i].b_inodo);
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
        dot+=R"(<td colspan="2" bgcolor="#c3f8b6">Bloque Archivo</td>)";
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

string Rep::treeInodo(int pos, FILE *file) {
    string dot="";
    char fecha[70];
    string fecha2="";

    TablaInodo inodo;
    fseek(file,pos,SEEK_SET);
    fread(&inodo, sizeof(TablaInodo),1,file);

    dot+="n"+ to_string(pos)+R"([label=<<table><tr><td colspan="2" bgcolor="#376ef3">INODO )"+ to_string(pos)+"</td></tr>\n";

    dot+="<tr>\n";
    dot+="<td>i_uid</td>\n";
    dot+="<td>"+ to_string(inodo.i_uid)+"</td>\n";
    dot+="</tr>\n";

    dot+="<tr>\n";
    dot+="<td>i_gid</td>\n";
    dot+="<td>"+ to_string(inodo.i_gid)+"</td>\n";
    dot+="</tr>\n";

    dot+="<tr>\n";
    dot+="<td>i_s</td>\n";
    dot+="<td>"+ to_string(inodo.i_s)+"</td>\n";
    dot+="</tr>\n";

    strftime(fecha,sizeof (fecha),"%Y-%m-%d %H:%M:%S",localtime(&inodo.i_atime));
    fecha2=fecha;
    dot+="<tr>\n";
    dot+="<td>i_atime</td>\n";
    dot+="<td>"+fecha2+"</td>\n";
    dot+="</tr>\n";

    strftime(fecha,sizeof (fecha),"%Y-%m-%d %H:%M:%S",localtime(&inodo.i_ctime));
    fecha2=fecha;
    dot+="<tr>\n";
    dot+="<td>i_ctime</td>\n";
    dot+="<td>"+fecha2+"</td>\n";
    dot+="</tr>\n";

    strftime(fecha,sizeof (fecha),"%Y-%m-%d %H:%M:%S",localtime(&inodo.i_mtime));
    fecha2=fecha;
    dot+="<tr>\n";
    dot+="<td>i_mtime</td>\n";
    dot+="<td>"+fecha2+"</td>\n";
    dot+="</tr>\n";

    for (int j = 0; j < 15; ++j) {
        if (inodo.i_block[j]!=-1){
            dot+="<tr>\n";
            dot+="<td>ap"+ to_string(j)+"</td>\n";
            dot+="<td port=\""+to_string(inodo.i_block[j])+"\">"+ to_string(inodo.i_block[j])+"</td>\n";
            dot+="</tr>\n";
        }else{
            dot+="<tr>\n";
            dot+="<td>i_block</td>\n";
            dot+="<td>-1</td>\n";
            dot+="</tr>\n";
        }

    }

    dot+="<tr>\n";
    dot+="<td>i_type</td>\n";
    dot+="<td>"+ string(1,inodo.i_type)+"</td>\n";
    dot+="</tr>\n";

    dot+="<tr>\n";
    dot+="<td>i_perm</td>\n";
    dot+="<td>"+ to_string(inodo.i_perm)+"</td>\n";
    dot+="</tr>\n";

    dot+="</table>>]\n";
    return dot;
}

string Rep::conexiones(int inicio, int final) {
    string dot="";
    dot+="n"+ to_string(inicio)+":"+ to_string(final)+"->n"+ to_string(final)+";\n";
    return dot;
}