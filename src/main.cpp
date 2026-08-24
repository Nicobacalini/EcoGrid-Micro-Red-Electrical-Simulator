#include "headers/GridManager.h"
#include "headers/NodoRed.h"
#include "headers/CapaDatos.h"
#include <iostream>
#include <vector>
#include <string>
#include <memory>



using namespace std; 

int main() {
    cout << "======================================================" << endl;
    cout << "   Iniciando Simulador EcoGrid - Micro-Red Electrica  " << endl;
    cout << "======================================================" << endl;

    // Inicializar la Capa de Datos
    CapaDatos base_datos("util/config.ini");

    // Cargar los Nodos a la Memoria (Polimorfismo con Smart Pointers)
    vector<shared_ptr<NodoRed>> nodos_activos;
    base_datos.cargarNodos(nodos_activos);

    // Validacion de seguridad
    if (nodos_activos.empty()) {
        cerr << "[Error] No se pudieron cargar los nodos. Abortando simulacion." << endl;
        return 1;
    }

    // Inicializar el Orquestador (Singleton) e Inyectar Dependencias
    GridManager& mercado = GridManager::getInstance();
    mercado.inyectarBaseDatos(&base_datos);
    mercado.setNodosActivos(nodos_activos);

    // Definir los Ticks de Simulacion
    vector<string> archivos_csv = {
        "util/datos_ofertas_10.csv",
        "util/datos_ofertas_12.csv",
        "util/datos_ofertas_14.csv",
        "util/datos_ofertas_15.csv",
        "util/datos_ofertas_18.csv",
        "util/datos_ofertas_20.csv"
    };

    cout << "\n--- Iniciando Ciclo de Subastas (" << archivos_csv.size() << " ticks) ---" << endl;

    // Bucle Principal del Simulador
    for (const string& ruta_archivo : archivos_csv) {
        cout << "\n[Tick] Procesando archivo: " << ruta_archivo << endl;

        // Extraer la hora del nombre del archivo
        size_t pos_guion = ruta_archivo.find_last_of('_');
        size_t pos_punto = ruta_archivo.find_last_of('.');
        if (pos_guion != string::npos && pos_punto != string::npos) {
            string hora_str = ruta_archivo.substr(pos_guion + 1, pos_punto - pos_guion - 1);
            int hora = stoi(hora_str);
            mercado.setHoraActual(hora);
        }

        // Cargar el trafico de red simulado a las colas de prioridad
        mercado.cargarOfertasDelCSV(ruta_archivo);

        // Ejecutar el algoritmo de matching (Subasta Doble Continua)
        // Esto tambien dispara automaticamente la persistencia en base de datos
        mercado.ejecutarSubasta();
    }

    cout << "\n======================================================" << endl;
    cout << "      Simulacion Finalizada. Cerrando conexiones.     " << endl;
    cout << "======================================================" << endl;

    return 0;
}