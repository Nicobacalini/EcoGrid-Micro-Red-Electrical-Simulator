#ifndef NODORED_H
#define NODORED_H

#include <string>

// Clase Base Abstracta
// Define los atributos comunes que cualquier nodo del mercado debe tener
class NodoRed {
protected:
    int id_nodo;
    std::string ubicacion;
    double saldo_cuenta;
    double balanceEnergia;

public:
    NodoRed(int id, const std::string& ubi, double saldo);
    virtual ~NodoRed() = default;

    int getId() const;
    std::string getUbicacion() const;
    double getSaldo() const;

    // Metodos Virtuales Puros (= 0) 
    // Obligan a cualquier clase que herede de NodoRed a implementar estos metodos
    virtual std::string getTipo() const = 0;
    virtual void imprimirDetalle() const = 0;
    virtual double calcularExcedente() = 0;
}; 
#endif // NODORED_H



