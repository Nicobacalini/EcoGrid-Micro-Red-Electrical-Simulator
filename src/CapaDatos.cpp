#include "headers/CapaDatos.h"
#include "headers/NodoConsumidor.h"
#include "headers/NodoProsumidor.h"
#include "headers/NodoBateria.h"
#include <soci/postgresql/soci-postgresql.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <exception>
#include <ctime>

using namespace std; 


string CapaDatos::leerConfiguracion(const string& rutaConfig) {
    ifstream archivo(rutaConfig);
    if (!archivo.is_open()) {
        throw runtime_error("No se pudo abrir el archivo de configuracion: " + rutaConfig);
    }

    map<string, string> config;
    string linea;

    while (getline(archivo, linea)) {
        if (linea.empty() || linea[0] == '[' || linea[0] == '#' || linea[0] == ';') continue;
        size_t posIgual = linea.find('=');
        if (posIgual != string::npos) {
            string clave = linea.substr(0, posIgual);
            string valor = linea.substr(posIgual + 1);
            config[clave] = valor;
        }
    }

    ostringstream connString;
    connString << "dbname=" << config["dbname"] << " user=" << config["user"]
               << " password=" << config["password"] << " host=" << config["host"]
               << " port=" << config["port"];
    return connString.str();
}

CapaDatos::CapaDatos(const string& rutaConfig) {
    try {
        string cadenaConexion = leerConfiguracion(rutaConfig);
        sql.open(soci::postgresql, cadenaConexion);
        cout << "Conexion a la base de datos establecida con exito." << endl;
    } catch (exception const &e) {
        cerr << "Error critico al inicializar CapaDatos: " << e.what() << endl;
    }
}

CapaDatos::~CapaDatos() {
    if (sql.is_connected()) {
        sql.close();
    }
}

void CapaDatos::cargarNodos(vector<shared_ptr<NodoRed>>& listaNodos) {
    try {
        soci::rowset<soci::row> filas = (sql.prepare << "SELECT id_nodo, ubicacion, tipo, saldo_cuenta, perfil_consumo FROM public.nodos");
        
        for (soci::rowset<soci::row>::const_iterator it = filas.begin(); it != filas.end(); ++it) {
            soci::row const& fila = *it;
            
            int id = fila.get<int>(0);
            string ubi = fila.get<string>(1);
            string tipo = fila.get<string>(2);
            double saldo = fila.get<double>(3);
            
            if (tipo == "Consumidor") {
                // Instanciamos el enumerador con un valor por defecto
                PerfilConsumo perfilEnum = PerfilConsumo::Residencial; 
                
                // Verificamos nulos y convertimos texto a Enum
                if (fila.get_indicator(4) != soci::i_null) {
                    string perfilTexto = fila.get<string>(4);
                    if (perfilTexto == "Comercial") {
                        perfilEnum = PerfilConsumo::Comercial;
                    } else if (perfilTexto == "Industrial") {
                        perfilEnum = PerfilConsumo::Industrial;
                    }
                }
                
                listaNodos.push_back(make_shared<NodoConsumidor>(id, ubi, saldo, perfilEnum));
            } 
            else if (tipo == "Prosumidor") {
                listaNodos.push_back(make_shared<NodoProsumidor>(id, ubi, saldo));
            } 
            else if (tipo == "Bateria") {
                listaNodos.push_back(make_shared<NodoBateria>(id, ubi, saldo));
            }
        }
        cout << "[Exito] Se cargaron " << listaNodos.size() << " nodos en memoria." << endl;
        
    } catch (exception const &e) {
        cerr << "[DB Error] Falla al cargar los nodos: " << e.what() << endl;
    }
}

bool CapaDatos::persistirTransacciones(const vector<TransaccionEnergia>& trans) {
    if (trans.empty()) return true;

    // Inicia bloque transaccional ACID
    soci::transaction tr(sql);
    try {
        for (const auto& t : trans) {
            
            // Convertimos chrono a tm para que SOCI lo entienda como TIMESTAMP
            time_t tiempo = chrono::system_clock::to_time_t(t.timestamp);
            tm tm_transaccion = *localtime(&tiempo);

            // Insertar en tabla transacciones con fecha explicita
            // Utilizamos ROUND() en SQL para forzar a 3 decimales en kwh y 2 decimales en precio
            sql << "INSERT INTO public.transacciones (id_vendedor, id_comprador, kwh, precio_unitario, fecha_transaccion) "
                   "VALUES (:v, :c, ROUND(:k::numeric, 3), ROUND(:p::numeric, 2), :f)",
                   soci::use(t.idVendedor), soci::use(t.idComprador), soci::use(t.kwh), soci::use(t.precio), soci::use(tm_transaccion);

            // Ejecutar Procedimiento Almacenado para el Comprador
            sql << "CALL public.actualizar_saldo_y_lecturas(:id, ROUND(:kwh::numeric, 3), ROUND(:precio::numeric, 2), 'compra', :ts)",
                   soci::use(t.idComprador), soci::use(t.kwh), soci::use(t.precio), soci::use(tm_transaccion);

            // Ejecutar Procedimiento Almacenado para el Vendedor
            sql << "CALL public.actualizar_saldo_y_lecturas(:id, ROUND(:kwh::numeric, 3), ROUND(:precio::numeric, 2), 'venta', :ts)",
                   soci::use(t.idVendedor), soci::use(t.kwh), soci::use(t.precio), soci::use(tm_transaccion);
        }
        
        tr.commit(); // Si sale bien, impactamos la base
        cout << "[Mercado] Tick procesado: " << trans.size() << " transacciones guardadas." << endl;
        return true;
    } catch (exception const &e) {
        tr.rollback(); // Si falla el saldo o la base de datos, abortamos
        cerr << "[Alerta Mercado] Transacciones rechazadas. ROLLBACK ejecutado. Motivo: " << e.what() << endl;
        return false;
    }
}


double CapaDatos::obtenerPrecioBase(int hora) {
    double precio_base = 0.0;
    try {
        soci::indicator ind;
        sql << "SELECT precio_base_kwh FROM public.config_tarifas WHERE hora = :h",
               soci::into(precio_base, ind), soci::use(hora);
               
        if (sql.got_data()) {
            return precio_base;
        } else {
            cerr << "[DB Info] No se encontro precio base para la hora " << hora << ". Usando fallback de 1.00." << endl;
            return 1.00;
        }
    } catch (exception const &e) {
        cerr << "[DB Error] Error al consultar tarifa para hora " << hora << ". Usando fallback de 1.00. Motivo: " << e.what() << endl;
        return 1.00;
    }
}