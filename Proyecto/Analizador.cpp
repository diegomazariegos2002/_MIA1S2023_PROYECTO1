//
// Created by diego-mazariegos on 5/10/22.
//

#include <iostream>
#include <fstream>
#include "Analizador.h"
#include "cstring"

Analizador::Analizador(string entrada, MountList *mountList, Usuario *usuario) {
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
    this->rep=new Rep();
    this->usuario=usuario;
    this->adminU=new AdminUsuarios();
    this->adminArcCarpt= new AdminArchivosCarpetas();
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
                if (strncmp(entradaMinus.c_str(), ">size", 5) == 0) {
                    i = entradaMinus.find("=") + 1;
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

                } else if (strncmp(entradaMinus.c_str(), ">fit", 4) == 0) {
                    i = entradaMinus.find("=") + 1;
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

                } else if (strncmp(entradaMinus.c_str(), ">unit", 5) == 0) {
                    i = entradaMinus.find("=") + 1;
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

                } else if (strncmp(entradaMinus.c_str(), ">path", 5) == 0) {
                    i = entradaMinus.find("=") + 1;
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
                if (strncmp(entradaMinus.c_str(), ">path", 5) == 0) {
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus.erase(0, 1);
                        this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        string p = this->entrada.substr(0, i);
                        i += 2;
                        this->disco->p = p;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus.erase(0, i);
                        this->entrada.erase(0, i);
                    }
                    else{
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
                }
                else{
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
                if (strncmp(entradaMinus.c_str(), ">size", 5) == 0) {
                    i = entradaMinus.find("=") + 1;
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

                } else if (strncmp(entradaMinus.c_str(), ">unit", 5) == 0) {
                    i = entradaMinus.find("=") + 1;
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

                } else if (strncmp(entradaMinus.c_str(), ">path", 5) == 0) {
                    i = entradaMinus.find("=") + 1;
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

                } else if (strncmp(entradaMinus.c_str(), ">type", 5) == 0) {
                    i = entradaMinus.find("=") + 1;
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

                } else if (strncmp(entradaMinus.c_str(), ">fit", 4) == 0) {
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    string f = entradaMinus.substr(0, i);
                    if (!(f == "bf" || f == "ff" || f == "wf")) {
                        cout << "OPCION INVALIDA PARA -f" << endl;
                        return;
                    }
                    this->particion->f = f[0];
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                } else if (strncmp(entradaMinus.c_str(), ">delete", 7) == 0) {
                    i = entradaMinus.find("=") + 1;
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

                } else if (strncmp(entradaMinus.c_str(), ">name", 5) == 0) {
                    i = entradaMinus.find("=") + 1;
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

                } else if (strncmp(entradaMinus.c_str(), ">add", 4) == 0) {
                    i = entradaMinus.find("=") + 1;
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
                if (strncmp(entradaMinus.c_str(), ">path", 5) == 0) {
                    i = entradaMinus.find("=") + 1;
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
                    }
                    else {
                        i = entradaMinus.find(" ");
                        string p = this->entrada.substr(0, i);
                        this->montar->p = p;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                }
                else if (strncmp(entradaMinus.c_str(), ">name", 5) == 0) {
                    i = entradaMinus.find("=") + 1;
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
                if (strncmp(entradaMinus.c_str(), ">id", 3) == 0) {
                    i = entradaMinus.find("=") + 1;
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
        //MKFS
        else if (strncmp(entradaMinus.c_str(), "mkfs", 4) == 0){
            int i = 4;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }
            entradaMinus = entradaMinus.erase(0, i);
            this->entrada = this->entrada.erase(0, i);
            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), ">id", 3) == 0) {
                    i = entradaMinus.find("=") + 1;
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
                }
                else if (strncmp(entradaMinus.c_str(), ">type", 5) == 0) {
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    string type = entradaMinus.substr(0, i);
                    this->montar->type = type;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                }
                else if (strncmp(entradaMinus.c_str(), ">fs", 3) == 0) {
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    string fs = entradaMinus.substr(0, i);
                    this->montar->fs = fs;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                }
                else if (strncmp(entradaMinus.c_str(), "#", 1) == 0) {
                    //No se opera, ya que entro un comentario
                    break;
                }
                else {
                    cout << "ERROR EN EL COMANDO: " << entradaMinus << endl;
                    return;
                }
            }

            montar->mountList= this->mountList;
            montar->mkfs();
            this->mountList=montar->mountList;
        }
        //REP
        else if (strncmp(entradaMinus.c_str(), "rep", 3) == 0) {
            int i = 3;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }
            entradaMinus = entradaMinus.erase(0, i);
            entrada = entrada.erase(0, i);

            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), ">path", 5) == 0) {
                    i = entradaMinus.find("=") + 1;
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
                        this->rep->path = p;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        string p = this->entrada.substr(0, i);
                        this->rep->path = p;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                }else if (strncmp(entradaMinus.c_str(), ">ruta", 5) == 0) {
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        this->rep->ruta = this->entrada.substr(0, i);
                        i += 2;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        this->rep->ruta = this->entrada.substr(0, i);
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                }else if (strncmp(entradaMinus.c_str(), ">name", 5) == 0) {
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    string n = entradaMinus.substr(0, i);
                    this->rep->name = n;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);


                }else if (strncmp(entradaMinus.c_str(), ">id", 3) == 0) {
                    i = entradaMinus.find("=") + 1;
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
                        this->rep->id = id;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        string id = this->entrada.substr(0, i);
                        this->rep->id = id;
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
                    cout << "ERROR EN EL COMANDO: " << entradaMinus << endl;
                    return;
                }
            }
            this->rep->mountList=this->mountList;
            this->rep->generate();
            this->mountList=this->rep->mountList;
        }
        //PAUSE
        else if (strncmp(entradaMinus.c_str(), "pause", 4) == 0){
            cout<<"PRESIONE CUALQUIER TECLA PARA CONTINUAR"<<endl;
            getchar();
            return;
        }
        //LOGIN
        else if (strncmp(entradaMinus.c_str(), "login", 5) == 0) {
            int i = 5;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }
            entradaMinus = entradaMinus.erase(0, i);
            entrada = entrada.erase(0, i);

            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), ">user", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    string name = this->entrada.substr(0, i);
                    this->adminU->name = name;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                }
                else if (strncmp(entradaMinus.c_str(), ">pass", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    string pass = this->entrada.substr(0, i);
                    this->adminU->pass = pass;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                }
                else if (strncmp(entradaMinus.c_str(), ">id", 3) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    string id = this->entrada.substr(0, i);
                    this->adminU->id = id;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                }
                else if (strncmp(entradaMinus.c_str(), "#", 1) == 0) {
                    //Comentario
                    break;
                }else {
                    cout << "ERROR EN EL COMANDO: " << entradaMinus << endl;
                    return;
                }
            }

            adminU->usuario=this->usuario;
            adminU->mountList= this->mountList;
            adminU->login();
            this->mountList=adminU->mountList;
            this->usuario=adminU->usuario;
        }
        //LOGOUT
        else if (strncmp(entradaMinus.c_str(), "logout", 6) == 0) {
            int i = 6;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }
            entradaMinus = entradaMinus.erase(0, i);
            entrada = entrada.erase(0, i);

            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), "#", 1) == 0) {
                    //No se opera, ya que entro un comentario
                    break;
                }else {
                    cout << "ERROR EN EL COMANDO: " << entradaMinus << endl;
                    return;
                }
            }

            adminU->usuario=this->usuario;
            adminU->mountList= this->mountList;
            adminU->logout();
            this->usuario=adminU->usuario;
            this->mountList=adminU->mountList;
        }
        //MKGRP
        else if (strncmp(entradaMinus.c_str(), "mkgrp", 5) == 0) {
            int i = 5;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }
            entradaMinus = entradaMinus.erase(0, i);
            entrada = entrada.erase(0, i);

            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), ">name", 4) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        string name = this->entrada.substr(0, i);
                        i += 2;
                        this->adminU->name = name;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        string name = this->entrada.substr(0, i);
                        this->adminU->name = name;
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
                    cout << "ERROR EN EL COMANDO: " << entradaMinus << endl;
                    return;
                }
            }

            adminU->mountList= this->mountList;
            adminU->usuario=this->usuario;
            adminU->mkgrp();
            this->usuario=adminU->usuario;
            this->mountList=adminU->mountList;

        }
        //RMGRP
        else if (strncmp(entradaMinus.c_str(), "rmgrp", 5) == 0) {
            int i = 5;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }

            entrada = entrada.erase(0, i);
            entradaMinus = entradaMinus.erase(0, i);

            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), ">name", 4) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        string name = this->entrada.substr(0, i);
                        i += 2;
                        this->adminU->name = name;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        string name = this->entrada.substr(0, i);
                        this->adminU->name = name;
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
                    cout << "ERROR EN EL COMANDO: " << entradaMinus << endl;
                    return;
                }
            }
            adminU->mountList= this->mountList;
            adminU->usuario=this->usuario;
            adminU->rmgrp();
            this->usuario=adminU->usuario;
            this->mountList=adminU->mountList;

        }
        //MKUSR
        else if (strncmp(entradaMinus.c_str(), "mkusr", 5) == 0) {
            int i = 5;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }
            entradaMinus = entradaMinus.erase(0, i);
            entrada = entrada.erase(0, i);

            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), ">user", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        string name = this->entrada.substr(0, i);
                        i += 2;
                        this->adminU->name = name;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        string name = this->entrada.substr(0, i);
                        this->adminU->name = name;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                }else if (strncmp(entradaMinus.c_str(), ">pass", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    string pass = this->entrada.substr(0, i);
                    this->adminU->pass = pass;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                }else if (strncmp(entradaMinus.c_str(), ">grp", 4) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        string grp = this->entrada.substr(0, i);
                        i += 2;
                        this->adminU->group = grp;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        string grp = this->entrada.substr(0, i);
                        this->adminU->group = grp;
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
                    cout << "ERROR EN EL COMANDO INGRESADO: " << entradaMinus << endl;
                    return;
                }
            }
            adminU->usuario=this->usuario;
            adminU->mountList= this->mountList;
            adminU->mkusr();
            this->mountList=adminU->mountList;
            this->usuario=adminU->usuario;
        }
        //RMUSR
        else if (strncmp(entradaMinus.c_str(), "rmusr", 5) == 0) {
            int i = 5;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }

            entradaMinus = entradaMinus.erase(0, i);
            entrada = entrada.erase(0, i);

            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), ">user", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        string name = this->entrada.substr(0, i);
                        i += 2;
                        this->adminU->name = name;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        string name = this->entrada.substr(0, i);
                        this->adminU->name = name;
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
                    cout << "ERROR EN EL COMANDO INGRESADO: " << entradaMinus << endl;
                    return;
                }
            }
            adminU->mountList= this->mountList;
            adminU->usuario=this->usuario;
            adminU->rmusr();
            this->usuario=adminU->usuario;
            this->mountList=adminU->mountList;

        }
        //CHGRP
        else if (strncmp(entradaMinus.c_str(), "chgrp", 5) == 0) {
            int i = 5;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }

            entradaMinus = entradaMinus.erase(0, i);
            entrada = entrada.erase(0, i);

            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), ">user", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    string name = this->entrada.substr(0, i);
                    this->adminU->name = name;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                }else if (strncmp(entradaMinus.c_str(), ">grp", 4) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        string grp = this->entrada.substr(0, i);
                        i += 2;
                        this->adminU->group = grp;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        string grp = this->entrada.substr(0, i);
                        this->adminU->group = grp;
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
                    cout << "ERROR EN EL COMANDO DE ENTRADA: " << entradaMinus << endl;
                    return;
                }
            }
            adminU->mountList= this->mountList;
            adminU->usuario=this->usuario;
            adminU->chgrp();
            this->usuario=adminU->usuario;
            this->mountList=adminU->mountList;

        }
        //CAT
        else if (strncmp(entradaMinus.c_str(), "cat", 3) == 0) {
            int i = 3;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }

            entradaMinus = entradaMinus.erase(0, i);
            entrada = entrada.erase(0, i);
            this->adminArcCarpt->rutasCat = *new vector<string>;

            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), ">file", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    this->entrada = this->entrada.erase(0, i);
                    entradaMinus = entradaMinus.erase(0, i);


                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        this->adminArcCarpt->rutasCat.push_back(this->entrada.substr(0, i));
                        i += 2;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        this->adminArcCarpt->rutasCat.push_back(this->entrada.substr(0, i));
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }



                }
                else if (strncmp(entradaMinus.c_str(), "#", 1) == 0) {
                    //No se opera, ya que entro un comentario
                    break;
                }
                else {
                    cout << "ERROR EL COMANDO TIENE ALGO MALO: " << entradaMinus << endl;
                    return;
                }
            }
            adminArcCarpt->mountList= this->mountList;
            adminArcCarpt->usuario=this->usuario;
            adminArcCarpt->cat();
            this->mountList=adminArcCarpt->mountList;
            this->usuario=adminArcCarpt->usuario;
        }
        //MKDIR
        else if (strncmp(entradaMinus.c_str(), "mkdir", 5) == 0) {
            int i = 5;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }

            entradaMinus = entradaMinus.erase(0, i);
            entrada = entrada.erase(0, i);

            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), ">path", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");

                        this->adminArcCarpt->path = this->entrada.substr(0, i);
                        i += 2;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        this->adminArcCarpt->path = this->entrada.substr(0, i);
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                }
                else if (strncmp(entradaMinus.c_str(), ">r", 2) == 0){
                    i = 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    this->adminArcCarpt->r = true;

                }
                else if (strncmp(entradaMinus.c_str(), "#", 1) == 0) {
                    //No se opera, ya que entro un comentario
                    break;
                }else {
                    cout << "ERROR REVISAR ENTRADA " << entradaMinus << endl;
                    return;
                }
            }
            adminArcCarpt->usuario=this->usuario;
            adminArcCarpt->mountList= this->mountList;
            adminArcCarpt->mkdir();
            this->mountList=adminArcCarpt->mountList;
            this->usuario=adminArcCarpt->usuario;


        }
        //MKFILE
        else if (strncmp(entradaMinus.c_str(), "mkfile", 6) == 0) {
            int i = 6;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }

            entradaMinus = entradaMinus.erase(0, i);
            entrada = entrada.erase(0, i);

            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), ">path", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");

                        this->adminArcCarpt->path = this->entrada.substr(0, i);
                        i += 2;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        this->adminArcCarpt->path = this->entrada.substr(0, i);
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                }else if (strncmp(entradaMinus.c_str(), ">cont", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        this->adminArcCarpt->cont = this->entrada.substr(0, i);
                        i += 2;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        this->adminArcCarpt->cont = this->entrada.substr(0, i);
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                }else if (strncmp(entradaMinus.c_str(), ">size", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    string s = this->entrada.substr(0, i);
                    this->adminArcCarpt->size = stoi(s);
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                }else if (strncmp(entradaMinus.c_str(), ">r", 2) == 0){
                    i = 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    this->adminArcCarpt->r = true;

                }else if (strncmp(entradaMinus.c_str(), "#", 1) == 0) {
                    //No se opera, ya que entro un comentario
                    break;
                }else {
                    cout << "ERROR EL COMANDO INGRESADO: " << entradaMinus << endl;
                    return;
                }
            }

            adminArcCarpt->usuario=this->usuario;
            adminArcCarpt->mountList= this->mountList;
            adminArcCarpt->mkfile();
            this->mountList=adminArcCarpt->mountList;
            this->usuario=adminArcCarpt->usuario;
        }
        //RENAME
        else if (strncmp(entradaMinus.c_str(), "rename", 6) == 0) {
            int i = 6;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }
            entradaMinus = entradaMinus.erase(0, i);
            entrada = entrada.erase(0, i);

            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), ">path", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        this->adminArcCarpt->path = this->entrada.substr(0, i);
                        i += 2;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        this->adminArcCarpt->path = this->entrada.substr(0, i);
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                }
                else if (strncmp(entradaMinus.c_str(), ">name", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");

                        this->adminArcCarpt->name = this->entrada.substr(0, i);
                        i += 2;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        this->adminArcCarpt->name = this->entrada.substr(0, i);
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                }
                else if (strncmp(entradaMinus.c_str(), "#", 1) == 0) {
                    //No se opera, ya que entro un comentario
                    break;
                }
                else {
                    cout << "ERROR EN LA ENTRADA: " << entradaMinus << endl;
                    return;
                }
            }
            adminArcCarpt->usuario=this->usuario;
            adminArcCarpt->mountList= this->mountList;
            adminArcCarpt->rename();
            this->mountList=adminArcCarpt->mountList;
            this->usuario=adminArcCarpt->usuario;
        }
        //CHMOD
        else if (strncmp(entradaMinus.c_str(), "chmod", 5) == 0) {
            int i = 5;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }

            entradaMinus = entradaMinus.erase(0, i);
            entrada = entrada.erase(0, i);

            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), ">path", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");

                        this->adminArcCarpt->path = this->entrada.substr(0, i);
                        i += 2;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        this->adminArcCarpt->path = this->entrada.substr(0, i);
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                }else if (strncmp(entradaMinus.c_str(), ">ugo", 4) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    this->adminArcCarpt->ugo = stoi(this->entrada.substr(0, i));
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                }else if (strncmp(entradaMinus.c_str(), ">r", 2) == 0){
                    i = 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    this->adminArcCarpt->r = true;

                }else if (strncmp(entradaMinus.c_str(), "#", 1) == 0) {

                    //comentario
                    break;
                }else {
                    cout << "ERROR EN LA ENTRADA: " << entradaMinus << endl;
                    return;
                }
            }
            this->adminArcCarpt->usuario=this->usuario;
            this->adminArcCarpt->mountList= this->mountList;
            this->adminArcCarpt->chmod();
            this->mountList=this->adminArcCarpt->mountList;
            this->usuario=this->adminArcCarpt->usuario;

        }
        //CHOWN
        else if (strncmp(entradaMinus.c_str(), "chown", 5) == 0) {
            int i = 5;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }

            entradaMinus = entradaMinus.erase(0, i);
            entrada = entrada.erase(0, i);

            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), ">path", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        this->adminArcCarpt->path = this->entrada.substr(0, i);
                        i += 2;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        this->adminArcCarpt->path = this->entrada.substr(0, i);
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                }
                else if (strncmp(entradaMinus.c_str(), ">user", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    i = entradaMinus.find(" ");
                    this->adminArcCarpt->name =this->entrada.substr(0, i);
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                }
                else if (strncmp(entradaMinus.c_str(), ">r", 2) == 0){
                    i = 2;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);
                    this->adminArcCarpt->r = true;

                }
                else if (strncmp(entradaMinus.c_str(), "#", 1) == 0) {
                    // Comentario
                    break;
                }else {
                    cout << "ERROR EN LA ENTRADA: " << entradaMinus << endl;
                    return;
                }
            }

            adminArcCarpt->usuario=this->usuario;
            adminArcCarpt->mountList= this->mountList;
            adminArcCarpt->chown();
            this->mountList=adminArcCarpt->mountList;
            this->usuario=adminArcCarpt->usuario;

        }
        //FIND
        else if (strncmp(entradaMinus.c_str(), "find", 4) == 0) {
            int i = 4;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }

            entradaMinus = entradaMinus.erase(0, i);
            entrada = entrada.erase(0, i);

            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), ">path", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        this->adminArcCarpt->path = this->entrada.substr(0, i);
                        i += 2;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        this->adminArcCarpt->path = this->entrada.substr(0, i);
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                }
                if (strncmp(entradaMinus.c_str(), ">name", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        this->adminArcCarpt->name = this->entrada.substr(0, i);
                        i += 2;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }
                    else {
                        i = entradaMinus.find(" ");
                        this->adminArcCarpt->name = this->entrada.substr(0, i);
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                }
                else if (strncmp(entradaMinus.c_str(), "#", 1) == 0) {
                    // Comentario
                    break;
                }else {
                    cout << "ERROR EN LA ENTRADA: " << entradaMinus << endl;
                    return;
                }
            }

            adminArcCarpt->usuario=this->usuario;
            adminArcCarpt->mountList= this->mountList;
            adminArcCarpt->find();
            this->mountList=adminArcCarpt->mountList;
            this->usuario=adminArcCarpt->usuario;

        }
        //EDIT
        else if (strncmp(entradaMinus.c_str(), "edit", 4) == 0) {
            int i = 4;
            while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                i++;
            }

            entradaMinus = entradaMinus.erase(0, i);
            entrada = entrada.erase(0, i);

            while (entrada.length() > 0) {
                if (strncmp(entradaMinus.c_str(), ">path", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        this->adminArcCarpt->path = this->entrada.substr(0, i);
                        i += 2;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        this->adminArcCarpt->path = this->entrada.substr(0, i);
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                }else if (strncmp(entradaMinus.c_str(), ">cont", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus = entradaMinus.erase(0, i);
                    this->entrada = this->entrada.erase(0, i);

                    if (entradaMinus[0] == '\"') {
                        entradaMinus = entradaMinus.erase(0, 1);
                        this->entrada = this->entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        this->adminArcCarpt->cont = this->entrada.substr(0, i);
                        i += 2;
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    } else {
                        i = entradaMinus.find(" ");
                        this->adminArcCarpt->cont = this->entrada.substr(0, i);
                        while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                            i++;
                        }
                        entradaMinus = entradaMinus.erase(0, i);
                        this->entrada = this->entrada.erase(0, i);
                    }

                }else if (strncmp(entradaMinus.c_str(), "#", 1) == 0) {
                    //COMENTARIO
                    break;
                }else {
                    cout << "ERROR EN EL COMANDO DE ENTRADA: " << entradaMinus << endl;
                    return;
                }
            }
            adminArcCarpt->usuario=this->usuario;
            adminArcCarpt->mountList= this->mountList;
            adminArcCarpt->edit();
            this->mountList=adminArcCarpt->mountList;
            this->usuario=adminArcCarpt->usuario;
        }

        //EXECUTE
        else if(strncmp(entradaMinus.c_str(), "execute", 7) == 0){
            string path = "";
            int i = 7;
            while(entradaMinus[i] == ' ' && entradaMinus.length() > 0){
                i++;
            }
            entradaMinus.erase(0, i);
            entrada.erase(0, i);

            //Se comprueban que los parametros vengan correctos
            while(entrada.length() > 0){
                if(strncmp(entradaMinus.c_str(), ">path", 5) == 0){
                    i = entradaMinus.find("=") + 1;
                    while (entradaMinus[i] == ' ' && entradaMinus.length() > 0) {
                        i++;
                    }
                    entradaMinus.erase(0, i);
                    entrada.erase(0, i);

                    if(entradaMinus[0] == '\"'){
                        entradaMinus.erase(0, 1);
                        entrada.erase(0, 1);
                        i = entradaMinus.find("\"");
                        path = entrada.substr(0, i);
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
        else{
            cout <<"COMANDO INVALIDO ASEGURESE DE ESCRIBIR BIEN TODO"<< endl;
        }
    }
}

void Analizador::exec(string path) {
    ifstream file(path.c_str());
    string linea;
    if (!file.is_open()){
        cout << "NO EXISTE O NO SE PUEDE ABRIR EL ARCHIVO. " << endl;
    }

    int i = path.find(".")+1;
    path.erase(0,i);
    if(strncmp(path.c_str(), "eea", 3) != 0){
        cout << "EL ARCHIVO INGRESADO NO POSEE LA EXTENSION VALIDA .EEA" << endl;
        return;
    }
    while(getline(file, linea)){
        if (linea != "\n" && linea.length() > 0){
            cout << "// "<<linea<<endl;
            Analizador *analizador = new Analizador(linea, this->mountList, this->usuario);
            analizador->analizarEntrada();
            this->mountList=analizador->mountList;
            this->usuario=analizador->usuario;
        }
    }
}





