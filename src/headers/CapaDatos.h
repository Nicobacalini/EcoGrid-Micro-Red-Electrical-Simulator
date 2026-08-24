#ifndef CAPADATOS_H
#define CAPADATOS_H

#include <string>
#include <vector>
#include <memory>
#include <soci/soci.h>
#include "Estructuras.h"
#include "NodoRed.h"

using namespace std; 


// La clase CapaDatos es responsable de toda la comunicacion con la Base de Datos.
// Abstrae el uso de SQL y de la libreria SOCI del resto del programa.
class CapaDatos {
private:
    soci::session sql;
    // Metodo auxiliar para leer el archivo de configuracion de la BD
    string leerConfiguracion(const string& rutaConfig);

public:
    CapaDatos(const string& rutaConfig);
    ~CapaDatos();
    bool persistirTransacciones(const vector<TransaccionEnergia>& trans);
    void cargarNodos(vector<shared_ptr<NodoRed>>& listaNodos);
    double obtenerPrecioBase(int hora);
};

#endif // CAPADATOS_H