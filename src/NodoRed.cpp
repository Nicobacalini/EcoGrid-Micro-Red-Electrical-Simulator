#include "headers/NodoRed.h"

//Constructor
NodoRed::NodoRed(int id, const std::string& ubi, double saldo)
    : id_nodo(id), ubicacion(ubi), saldo_cuenta(saldo), balanceEnergia(0.0) {}

int NodoRed::getId() const {
    return id_nodo;
}

std::string NodoRed::getUbicacion() const {
    return ubicacion;
}

double NodoRed::getSaldo() const {
    return saldo_cuenta;
}