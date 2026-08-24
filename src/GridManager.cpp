#include "headers/GridManager.h"
#include "headers/NodoBateria.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath> // Necesario para round
#include <algorithm> // Necesario para min , transform y remove_if
#include <cctype>    // Para tolower y isspace

using namespace std;

// Implementacion del Patron de Diseño Singleton:
GridManager& GridManager::getInstance() {
    static GridManager instance;
    return instance;
}

// Constructor
GridManager::GridManager() : db(nullptr), hora_actual_simulacion(0), contador_secuencia(0) {}

// Inyeccion de dependencia para la base de datos
void GridManager::inyectarBaseDatos(CapaDatos* base_datos) {
    this->db = base_datos;
}

// Recibe la lista de nodos activos en la simulacion
void GridManager::setNodosActivos(const vector<shared_ptr<NodoRed>>& nodos) {
    this->nodos_activos = nodos;
}

// Actualiza la hora de la simulacion
void GridManager::setHoraActual(int hora) {
    this->hora_actual_simulacion = hora;
}

// Metodo para leer y procesar las ordenes de compra/venta desde un archivo CSV
void GridManager::cargarOfertasDelCSV(const string& ruta_archivo) {
    ifstream archivo_entrada(ruta_archivo);

    if (!archivo_entrada.is_open()) {
        cerr << "Error: No se pudo abrir el archivo CSV en la ruta: " << ruta_archivo << endl;
        return;
    }

    // La bateria ofrece su energia acumulada como venta
    if (db != nullptr) {
        double precio_base = db->obtenerPrecioBase(hora_actual_simulacion);
        for (const auto& nodo : nodos_activos) {
            if (nodo->getTipo() == "Bateria") {
                auto bateria = dynamic_pointer_cast<NodoBateria>(nodo);
                if (bateria && bateria->getCarga() > 0) {
                    Orden orden_bat;
                    orden_bat.idOrden = 0; // ID reservado para orden de bateria
                    orden_bat.esCompra = false; // Venta
                    orden_bat.idNodo = bateria->getId();
                    orden_bat.precio = precio_base;
                    orden_bat.kwh = bateria->getCarga();
                    
                    contador_secuencia++;
                    orden_bat.secuencia = contador_secuencia;
                    askMap[orden_bat.precio].push(orden_bat);
                    
                    cout << "[Bateria] Inyectando venta de " << bateria->getCarga() << " kWh a $" << precio_base << endl;
                }
                break;
            }
        }
    }

    string linea;
    getline(archivo_entrada, linea); // Descartar encabezado

    while(getline(archivo_entrada, linea)) {
        stringstream stream_linea(linea);

        string str_id, str_lado, str_nodo, str_kwh, str_precio;

        getline(stream_linea, str_id, ',');
        getline(stream_linea, str_lado, ',');
        getline(stream_linea, str_nodo, ',');
        getline(stream_linea, str_kwh, ',');
        getline(stream_linea, str_precio, ',');

        // Instanciar y convertir tipos respetando Estructuras.h
        struct Orden orden_temp;
        orden_temp.idOrden = stoi(str_id);
        orden_temp.idNodo = stoi(str_nodo);
        double raw_kwh = stod(str_kwh);
        double raw_precio = stod(str_precio);
        // Redondeamos kwh a 3 decimales y precio a 2 decimales usando round
        orden_temp.kwh = round(raw_kwh * 1000.0) / 1000.0;
        orden_temp.precio = round(raw_precio * 100.0) / 100.0;

        // Asignacion del booleano (true = compra, false = venta)
        // Eliminar todos los espacios en blanco 
        str_lado.erase(remove_if(str_lado.begin(), str_lado.end(), ::isspace), str_lado.end());
        // Convertir todo a minusculas
        transform(str_lado.begin(), str_lado.end(), str_lado.begin(), ::tolower);
        // Comparacion
        orden_temp.esCompra = (str_lado == "compra");

        // Asignar prioridad FIFO
        contador_secuencia++;
        orden_temp.secuencia = contador_secuencia;

        // Rellenamos los libros segun corresponda
        if (orden_temp.esCompra) {
            bidMap[orden_temp.precio].push(orden_temp);
        } else {
            askMap[orden_temp.precio].push(orden_temp);
        }
    }
    archivo_entrada.close();
}

// Este metodo se encarga de cruzar las ofertas de compra (bids) y venta (asks)
// Se basa en encontrar compatibilidad de precios (Comprador dispuesto a pagar >= Vendedor dispuesto a cobrar)
void GridManager::ejecutarSubasta() {
    // Lista afuera del bucle para acumular las transacciones exitosas
    vector<TransaccionEnergia> transacciones_del_tick;

    // Clonamos los mapas para evitar desincronizacion de memoria si hay un ROLLBACK
    auto bidMap_clon = bidMap;
    auto askMap_clon = askMap;

    while (!bidMap_clon.empty() && !askMap_clon.empty()) {
        auto mejorBid = bidMap_clon.begin();
        auto mejorAsk = askMap_clon.begin();
        
        if (mejorBid->first >= mejorAsk->first) { 
            Orden& ordenCompra = mejorBid->second.front(); 
            Orden& ordenVenta = mejorAsk->second.front();
            
            // La cantidad de energia a intercambiar sera el minimo entre lo que se pide y lo que se ofrece
            double energia_transferida = min(ordenCompra.kwh, ordenVenta.kwh);
            
            // Precio es el promedio entre lo que el comprador propuso y el vendedor pidio
            double precio_clearing = (ordenCompra.precio + ordenVenta.precio) / 2.0; 
            
            // Estructura TransaccionEnergia
            struct TransaccionEnergia transaccion;
            transaccion.idComprador = ordenCompra.idNodo;
            transaccion.idVendedor = ordenVenta.idNodo;
            transaccion.kwh = energia_transferida;
            transaccion.precio = precio_clearing;
            transaccion.timestamp = chrono::system_clock::now();

            cout << "  [Cruce] Nodo " << ordenVenta.idNodo << " (Vendedor) le vendio a Nodo " << ordenCompra.idNodo 
                 << " (Comprador): " << energia_transferida << " kWh a $" << precio_clearing << endl;

            // Guardamos la transaccion en la lista externa
            transacciones_del_tick.push_back(transaccion);
            
            // Actualizar remanentes
            ordenCompra.kwh -= energia_transferida;
            ordenVenta.kwh -= energia_transferida;

            cout << "    -> Comprador " << ordenCompra.idNodo << " remanente: " << (ordenCompra.kwh <= 0.001 ? 0.0 : ordenCompra.kwh) << " kWh" << endl;
            cout << "    -> Vendedor " << ordenVenta.idNodo << " remanente: " << (ordenVenta.kwh <= 0.001 ? 0.0 : ordenVenta.kwh) << " kWh" << endl;

            // limpiar ordenes vacias
            if (ordenCompra.kwh <= 0.001) {
                mejorBid->second.pop();
            }
            if (ordenVenta.kwh <= 0.001) {
                mejorAsk->second.pop();
            }
        } else {
            break;
        }
        
        if (mejorBid->second.empty()) {
            bidMap_clon.erase(mejorBid); 
        }
        if (mejorAsk->second.empty()) {
            askMap_clon.erase(mejorAsk); 
        }
    }

    // Intervencion de la bateria comunitaria
    if (!askMap_clon.empty()){
        // Buscar el ID de la bateria dinamicamente en los nodos activos
        int id_bateria = -1;
        for (const auto& nodo : nodos_activos) {
            if (nodo->getTipo() == "Bateria") {
                id_bateria = nodo->getId();
                break;
            }
        }

        if (id_bateria != -1 && db != nullptr) {
            // Buscamos precio base en la tabla config_tarifas
            double precio_base = db->obtenerPrecioBase(hora_actual_simulacion); 

            for (auto& par_precio_cola : askMap_clon) {
                queue<Orden>& cola_ventas = par_precio_cola.second;
                
                while (!cola_ventas.empty()) {
                    // Obtenemos la orden del frente
                    Orden& orden_sobrante = cola_ventas.front();
                    
                    struct TransaccionEnergia venta_sobrante;
                    venta_sobrante.idComprador = id_bateria;
                    venta_sobrante.idVendedor = orden_sobrante.idNodo;
                    venta_sobrante.kwh = orden_sobrante.kwh;
                    venta_sobrante.precio = precio_base;
                    venta_sobrante.timestamp = chrono::system_clock::now();

                    cout << "  [Absorcion Bateria] Bateria (Nodo " << id_bateria << ") compra excedente a Nodo " 
                         << orden_sobrante.idNodo << ": " << orden_sobrante.kwh << " kWh a precio base $" << precio_base << endl;

                    // Hacemos push_back al vector transacciones_del_tick
                    transacciones_del_tick.push_back(venta_sobrante);
                    
                    // Sacamos la orden procesada de la cola
                    cola_ventas.pop();
                }
            }
            // La bateria absorbe toda la energia que pidan a precio base
            // Se vacia el mapa clonado
            askMap_clon.clear();
        } 
    }
    
    // Ahora, con todas las transacciones acumuladas, se las pasamos a la base de datos
    // Se ejecuta siempre, haya actuado la bateria o no.
    if (db != nullptr && !transacciones_del_tick.empty()) {
        bool exito = db->persistirTransacciones(transacciones_del_tick); 
        if (exito) {
            // Si la base de datos hizo COMMIT exitosamente, guardamos los mapas clonados (RAM = DB)
            bidMap = bidMap_clon;
            askMap = askMap_clon;
            
            // Si la bateria compro o vendio, actualizamos su carga en memoria
            for (const auto& trans : transacciones_del_tick) {
                for (auto& nodo : nodos_activos) {
                    if (nodo->getTipo() == "Bateria") {
                        auto bateria = dynamic_pointer_cast<NodoBateria>(nodo);
                        if (trans.idComprador == bateria->getId()) {
                            bateria->actualizarCarga(trans.kwh); // Compro excedente, aumenta carga
                        } else if (trans.idVendedor == bateria->getId()) {
                            bateria->actualizarCarga(-trans.kwh); // Vendio su carga, disminuye
                        }
                    }
                }
            }
            
            // La demanda insatisfecha se descarta al final del tick
            if (!bidMap.empty()) {
                cout << "[Mercado] Descartando demanda insatisfecha." << endl;
                bidMap.clear();
            }
        } else {
            // Si fallo, descartamos el mercado cargado para no arrastrar errores al siguiente tick.
            cerr << "[Info] El estado de la memoria RAM fue revertido para coincidir con la Base de Datos. Limpiando ordenes fallidas." << endl;
            bidMap.clear();
            askMap.clear();
        }
    } else if (transacciones_del_tick.empty()) {
        // Si no hubo cruces
        bidMap = bidMap_clon;
        askMap = askMap_clon;
        
        if (!bidMap.empty()) {
            cout << "[Mercado] Descartando demanda insatisfecha." << endl;
            bidMap.clear();
        }
    }
}