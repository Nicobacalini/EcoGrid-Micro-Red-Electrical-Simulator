#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <chrono>
#include <cstdint> // para uint64_t

// Estructura de la orden cargada desde el CSV o generada por la bateria
// Representa la intencion de un nodo de comprar o vender energia en el mercado.
struct Orden {
    int idOrden;
    bool esCompra;
    int idNodo;
    double precio;
    double kwh;
    uint64_t secuencia;
};

// Entidad que representa un cruce exitoso en la subasta
struct TransaccionEnergia {
    int idVendedor;
    int idComprador;
    double kwh;
    double precio;
    std::chrono::system_clock::time_point timestamp;
};

#endif // ESTRUCTURAS_H