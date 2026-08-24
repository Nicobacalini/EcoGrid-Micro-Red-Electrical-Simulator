#ifndef NODOBATERIA_H
#define NODOBATERIA_H

#include "NodoRed.h"

class NodoBateria : public NodoRed {
private:
    double carga_actual_kwh;

public:
    // Constructor
    NodoBateria(int id, const std::string& ubi, double saldo);
    
    // Getters y Setters
    double getCarga() const;
    void actualizarCarga(double variacion_kwh);

    //Metodos virtuales de la clase NodoRed
    std::string getTipo() const override;
    void imprimirDetalle() const override;
    double calcularExcedente() override;
};

#endif // NODOBATERIA_H