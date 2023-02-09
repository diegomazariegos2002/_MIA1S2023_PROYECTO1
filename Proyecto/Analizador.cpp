//
// Created by diego-mazariegos on 5/10/22.
//

#include <iostream>
#include <fstream>
#include "Analizador.h"
#include "cstring"

Analizador::Analizador(string entrada, MountList *mountList) {
    //Quitando simbolos de saltos de líneas y esas cosas en la entrada
    while ((entrada.find("\t") != string::npos) || (entrada.find("\r") != string::npos) || (entrada.find("\n") != string::npos)){
        entrada= removeSpace(entrada);
    }
    int i=0;
    while (entrada[i]==' ' && entrada.length()>0){
        i++;
    }
    entrada=entrada.erase(0,i);

    this->entrada=entrada;
    this->disco=new Disco();
    this->particion=new Particion();
    this->montar=new Montar();
    this->mountList=mountList;
}

string Analizador::removeSpace(string entrada) {
    if (entrada.find("\t") != string::npos){
        int i=entrada.find("\t");
        entrada.replace(i,1," ");
    }
    if (entrada.find("\r") != string::npos){
        int i=entrada.find("\r");
        entrada.replace(i,1," ");
    }
    if (entrada.find("\n") != string::npos){
        int i=entrada.find("\n");
        entrada.replace(i,1," ");
    }
    return entrada;
}

string Analizador::toLowerr(string cadena) {
    string cadMinus = "";
    int longitud = cadena.length();
    int i = 0;
    while (i < longitud) {
        cadMinus += tolower(cadena[i]);
        i++;
    }
    return cadMinus;
}

void Analizador::analizarEntrada() {
    if (this->entrada.length()>0) {
        string entradaMinus = this->toLowerr(this->entrada);
        if (strncmp(entradaMinus.c_str(), "#", 1) == 0) {
            //Es un comentario entonces solo se ignora todo.
            return;
        }
        //MKDISK
        else if (strncmp(entradaMinus.c_str(), "mkdisk", 6) == 0) {
            int i = 6;
            // Consumimos de espacios
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }
            entradaMinus.erase(0, i);
            this->entrada = this->entrada.erase(0, i);

            //parte de verificar comandos
            while (this->entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), "-s", 2) == 0) {
                    i = entradaMinus.find("->") + 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    int s = stoi(entradaMinus.substr(0, i));
                    this->disco->s = s;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                } else if (strncmp(entradaMinus.c_str(), "-f", 2) == 0) {
                    i = entradaMinus.find("->") + 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    string f = entradaMinus.substr(0, i);
                    this->disco->f = f;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                } else if (strncmp(entradaMinus.c_str(), "-u", 2) == 0) {
                    i = entradaMinus.find("->") + 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    string u = entradaMinus.substr(0, i);
                    this->disco->u = u;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                } else if (strncmp(entradaMinus.c_str(), "-path", 5) == 0) {
                    i = entradaMinus.find("->") + 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        string p = this->entrada.substr(0, i);
                        i += 2;
                        this->disco->p = p;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        string p = this->entrada.substr(0, i);
                        this->disco->p = p;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                } else if (strncmp(entradaMinus.c_str(), "#", 1) == 0) {
                    //No se opera, ya que entro un comentario
                    break;
                } else {
                    cout << "ERROR EN EL COMANDO: " << entradaMinus << endl;
                    return;
                }
            }
            this->disco->mkdisk();
        }
        //RMDISK
        else if (strncmp(entradaMinus.c_str(), "rmdisk", 6) == 0) {
            int i = 6;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }
            entradaMinus.erase(0, i);
            this->entrada.erase(0, i);

            //parte de verificar comandos
            while (this->entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), "-path", 5) == 0) {
                    i = entradaMinus.find("->") + 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus.erase(0, 1);
                        this->entrada.erase(0, 1);
                        i = entradaMinus.find(".");
                        i+=4;
                        string p = this->entrada.substr(0, i);
                        i += 1;
                        this->disco->p = p;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        int i = entradaMinus.find("\"");
                        entradaMinus.erase(0, i);
                        this->entrada.erase(0, i);
                    }else{
                        i = entradaMinus.find(" ");
                        string p = this->entrada.substr(0, i);
                        this->disco->p = p;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus.erase(0, i);
                        this->entrada.erase(0, i);
                    }
                } else if (strncmp(entradaMinus.c_str(), "#", 1) == 0) {
                    //es un comentario entonces se ignora todo
                    break;
                }else{
                    cout << "ERROR EN EL COMANDO: " << entradaMinus << endl;
                    return;
                }
            }
            this->disco->rmdisk();
        }
        //FDISK
        else if (strncmp(entradaMinus.c_str(), "fdisk", 5) == 0) {
            int i = 5;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }
            entradaMinus = entradaMinus.erase(0, i);
            this->entrada = this->entrada.erase(0, i);
            //se ven que paremetros trae el comando
            while (this->entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), "-s", 2) == 0) {
                    i = entradaMinus.find("->") + 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    int s = stoi(entradaMinus.substr(0, i));
                    this->particion->s = s;
                    if (this->particion->flag == 'n') {
                        this->particion->flag = 's';
                    }
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                } else if (strncmp(entradaMinus.c_str(), "-u", 2) == 0) {
                    i = entradaMinus.find("->") + 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    string u = entradaMinus.substr(0, i);
                    this->particion->u = u[0];
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                } else if (strncmp(entradaMinus.c_str(), "-path", 5) == 0) {
                    i = entradaMinus.find("->") + 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        string p = this->entrada.substr(0, i);
                        i += 2;
                        this->particion->p = p;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        string p = this->entrada.substr(0, i);
                        this->particion->p = p;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                } else if (strncmp(entradaMinus.c_str(), "-t", 2) == 0) {
                    i = entradaMinus.find("->") + 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    string t = entradaMinus.substr(0, i);
                    this->particion->t = t[0];
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                } else if (strncmp(entradaMinus.c_str(), "-f", 2) == 0) {
                    i = entradaMinus.find("->") + 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    string f = entradaMinus.substr(0, i);
                    if (!(f == "bestfit" || f == "firstfit" || f == "worstfit")) {
                        cout << "OPCION INVALIDA PARA -f" << endl;
                        return;
                    }
                    this->particion->f = f[0];
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                } else if (strncmp(entradaMinus.c_str(), "-delete", 7) == 0) {
                    i = entradaMinus.find("->") + 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    string d = entradaMinus.substr(0, i);
                    if (d != "full") {
                        cout << "OPCION " << d << " INVALIDA PARA -delete" << endl;
                        return;
                    }
                    this->particion->d = d;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                } else if (strncmp(entradaMinus.c_str(), "-name", 5) == 0) {
                    i = entradaMinus.find("->") + 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        string n = this->entrada.substr(0, i);
                        i += 2;
                        this->particion->name = n;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        string n = this->entrada.substr(0, i);
                        this->particion->name = n;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                } else if (strncmp(entradaMinus.c_str(), "-add", 4) == 0) {
                    i = entradaMinus.find("->") + 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    int add = stoi(entradaMinus.substr(0, i));
                    this->particion->add = add;
                    if (this->particion->flag == 'n') {
                        this->particion->flag = 'a';
                    }
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                } else if (strncmp(entradaMinus.c_str(), "#", 1) == 0) {
                    //es un comentario entonces no se hace nada
                    break;
                } else {
                    cout << "ERROR EN EL COMANDO DE ENTRADA: " << entradaMinus << endl;
                    return;
                }
            }
            this->particion->fdisk();
        }
        //MOUNT
        else if (strncmp(entradaMinus.c_str(), "mount", 5) == 0){
            int i = 5;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }
            entradaMinus = entradaMinus.erase(0, i);
            this->entrada = this->entrada.erase(0, i);
            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), "-path", 5) == 0) {
                    i = entradaMinus.find("->") + 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        string p = this->entrada.substr(0, i);
                        i += 2;
                        this->montar->p = p;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        string p = this->entrada.substr(0, i);
                        this->montar->p = p;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                }else if (strncmp(entradaMinus.c_str(), "-name", 5) == 0) {
                    i = entradaMinus.find("->") + 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        string n = this->entrada.substr(0, i);
                        i += 2;
                        this->montar->name = n;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        string n = this->entrada.substr(0, i);
                        this->montar->name = n;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                }else if (strncmp(entradaMinus.c_str(), "#", 1) == 0) {
                    // Es un comentario entonces no se opera
                    break;
                }else {
                    cout << "ERROR EN: " << entradaMinus << endl;
                    return;
                }                
            }
            montar->mountList=this->mountList;
            montar->mount();
            this->mountList=this->montar->mountList;
        }
        //UNMOUNT
        else if (strncmp(entradaMinus.c_str(), "unmount", 7) == 0){
            int i = 7;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }
            entradaMinus = entradaMinus.erase(0, i);
            this->entrada = this->entrada.erase(0, i);
            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), "-id", 3) == 0) {
                    i = entradaMinus.find("->") + 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        string id = this->entrada.substr(0, i);
                        i += 2;
                        this->montar->id = id;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        string id = this->entrada.substr(0, i);
                        this->montar->id = id;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }
                }else if (strncmp(entradaMinus.c_str(), "#", 1) == 0) {
                    //No se opera, ya que entro un comentario
                    break;
                }else {
                    cout << "ERROR EN: " << entradaMinus << endl;
                    return;
                }
            }
            montar->mountList= this->mountList;
            montar->unmount();
            this->mountList=montar->mountList;
        }
        //EXEC
        else if(strncmp(entradaMinus.c_str(), "exec", 4) == 0){
            string path = "";
            int i = 4;
            while(entradaMinus[i] == ' ' && entradaMinus.length() > 0){
                i++;
            }
            entradaMinus.erase(0, i);
            entrada.erase(0, i);

            //Se comprueban que los parametros vengan correctos
            while(entrada.length() > 0){
                if(strncmp(entradaMinus.c_str(), "-path", 5) == 0){
                    i = entradaMinus.find("->") + 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus.erase(0, i);
                    entrada.erase(0, i);

                    if(entradaMinus[0] == '\"'){
                        entradaMinus.erase(0, 1);
                        entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        string p = entrada.substr(0, i);
                        path = p;
                        i += 2;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus.erase(0, i);
                        entrada = entrada.erase(0, i);
                    } else{
                        i = entradaMinus.find(" ");
                        string p = entrada.substr(0, i);
                        path = p;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus=entradaMinus.erase(0, i);
                        entrada = entrada.erase(0, i);
                    }
                } else if (strncmp(entradaMinus.c_str(), "#", 1) == 0) {
                    //Es un comentario entonces no se hace nada
                    break;
                } else {
                    cout << "ERROR EN EL COMANDO: " << entradaMinus << endl;
                    return;
                }
            }
            exec(path);
            return;
        }
    }
}

void Analizador::exec(string path) {
    ifstream script(path.c_str());
    string linea;
    if (!script.is_open()){
        cout << "NO EXISTE O NO SE PUEDE ABRIR EL ARCHIVOS. " << endl;
    }
    while(getline(script, linea)){
        if (linea != "\n" && linea.length() > 0){
            cout << "** "<<linea<<endl;
            Analizador *analizador = new Analizador(linea, this->mountList);
            analizador->analizarEntrada();
        }
    }
}





