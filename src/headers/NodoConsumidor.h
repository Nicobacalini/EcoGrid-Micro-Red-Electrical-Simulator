#ifndef NODOCONSUMIDOR_H
#define NODOCONSUMIDOR_H

#include "NodoRed.h"

// Enumeracion estricta para los perfiles de consumo
enum class PerfilConsumo {
    Residencial,
    Comercial,
    Industrial
};

class NodoConsumidor : public NodoRed {
private:
    PerfilConsumo perfil;
    double demanda_actual; // Almacena el consumo del tick actual para el calculo

public:
    // Constructor
    NodoConsumidor(int id, const std::string& ubi, double saldo, PerfilConsumo perfil);
    
    void setDemandaActual(double kwh);

    // Metodos virtuales de la clase NodoRed
    std::string getTipo() const override;
    void imprimirDetalle() const override;
    double calcularExcedente() override;
};

#endif // NODOCONSUMIDOR_H