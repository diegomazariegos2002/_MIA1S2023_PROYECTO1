#ifndef CODIGO_FUENTE_ADMINUSUARIOS_H
#define CODIGO_FUENTE_ADMINUSUARIOS_H

#include <string>
#include <vector>
#include "MountList.h"
#include "Structs.h"
#include "Usuario.h"

using namespace std;

class AdminUsuarios {
    public:
        AdminUsuarios();
        string id;
        string name;
        string pass;
        string group;
        string content;
        MountList *mountList;
        Usuario *usuario;
        FILE *file;
        SuperBloque sb;
        bool cambioCont;
        void login();
        void logout();
        string getContenido(int startInodo);
        vector<string>getUsers(string entrada);
        vector<string>getGrupos(string entrada);
        vector<string> getCampos(string entrada);
        vector<string> getArrayBlks(string cadena);
        void mkgrp();
        void rmgrp();
        void mkusr();
        void rmusr();
        void chgrp();
        bool verificarGrupoExistencia(vector<string> listaGrupos, string name);
        bool usrExist(vector<string> usuarios, string name);
        string generarNuevoIdGrupos(vector<string> grupos);
        string getUID(vector<string> usuarios);

        TablaInodo agregarArchivo(string cadena, TablaInodo inodo, int j,int aux);
        void escribirJorunal(string tipo_Op,char tipo,string nombre,string contenido,Nodo_M *nodo);
};


#endif //CODIGO_FUENTE_ADMINUSUARIOS_H
