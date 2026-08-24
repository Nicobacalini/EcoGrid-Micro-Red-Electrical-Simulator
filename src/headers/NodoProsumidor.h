#ifndef NODOPROSUMIDOR_H
#define NODOPROSUMIDOR_H

#include "NodoRed.h"

class NodoProsumidor : public NodoRed {
private: 
    double produccion;
    double consumo;

public:
    // Constructor
    NodoProsumidor(int id, const std::string& ubi, double saldo, double produccion = 0.0, double consumo = 0.0);
    
    // Metodos virtuales de la clase NodoRed
    std::string getTipo() const override;
    void imprimirDetalle() const override;
    double calcularExcedente() override;
};

#endif // NODOPROSUMIDOR_H