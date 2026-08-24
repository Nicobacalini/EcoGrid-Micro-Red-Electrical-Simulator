#ifndef GRIDMANAGER_H
#define GRIDMANAGER_H

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <queue>
#include <functional>
#include <cstdint>
#include "CapaDatos.h"
#include "NodoRed.h"
#include "Estructuras.h"


using namespace std; 

class GridManager {
private:
    CapaDatos* db; // Puntero a BD para poder inyectarlo dsp
    vector<shared_ptr<NodoRed>> nodos_activos;
    int hora_actual_simulacion;
    uint64_t contador_secuencia;

    // Estructuras del Order Book (Libro de Ofertas)
    map<double, queue<Orden>, greater<double>> bidMap; // Libro de Compras (Bids) (mayor a menor)
    
    map<double, queue<Orden>> askMap;                    // Libro de Ventas (Asks)

    // Constructor Privado para evitar que se creen instancias fuera de la clase
    GridManager();

public:
    // Metodo estatico para obtener la unica instancia en memoria
    static GridManager& getInstance();

    // Evita clonacion
    GridManager(const GridManager&) = delete;
    void operator=(const GridManager&) = delete;

    // Metodos operativos
    void inyectarBaseDatos(CapaDatos* base_datos);
    void setNodosActivos(const vector<shared_ptr<NodoRed>>& nodos);
    void setHoraActual(int hora);
    void tickMercado();
    void inicializarRed();
    void cargarOfertasDelCSV(const string& ruta_archivo);
    void ejecutarSubasta();
    void avanzarTick();
};

#endif // GRIDMANAGER_H