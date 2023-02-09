#include <iostream>
#include <cstring>
#include "Analizador.h"

using namespace std;

int main() {
    MountList *mountList=new MountList();
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

        Analizador *analizador = new Analizador(lectura, mountList);
        analizador->analizarEntrada();
        //para mantener guardadas las particiones montadas en "RAM", después de ejecutar cada comando.
        mountList = analizador->mountList;
        std::printf("\n");
    }
    return 0;
}
