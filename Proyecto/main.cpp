#include <iostream>
#include <cstring>
#include "Analizador.h"

using namespace std;

int main() {
    MountList *mountList=new MountList();
    Usuario *usuario=new Usuario();
    usuario->idG=0;
    usuario->idU=0;
    usuario->idMount=" ";
    usuario->nombreU=" ";

    printf("Bienvenido add la consola!!! \n");
    while (true) {
        std::cout << ">> ";
        char temporal[1000];
        fgets(temporal, sizeof(temporal), stdin);
        string lectura = temporal;
        string salida = "exit";

        if (strncmp(lectura.c_str(), salida.c_str(), salida.length()) == 0) {
            cout << "HASTA PRONTO!!!" << endl;
            break;
        }

        Analizador *analizador = new Analizador(lectura, mountList, usuario);
        analizador->analizarEntrada();
        //para mantener guardadas las particiones montadas en "RAM", después de ejecutar cada comando.
        mountList = analizador->mountList;
        usuario=analizador->usuario;
        std::printf("\n");
    }
    return 0;
}
