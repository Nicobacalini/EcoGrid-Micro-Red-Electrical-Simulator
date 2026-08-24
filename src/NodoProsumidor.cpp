#include "headers/NodoProsumidor.h"
#include <iostream>

using namespace std; 

// Constructor
NodoProsumidor::NodoProsumidor(int id, const string& ubi, double saldo, double produccion, double consumo)
    : NodoRed(id, ubi, saldo), produccion(produccion), consumo(consumo) {}

// Devuelve el tipo de nodo
string NodoProsumidor::getTipo() const {
    return "Prosumidor";
}

void NodoProsumidor::imprimirDetalle() const {
    cout << "ID: " << id_nodo << endl;
    cout << "Ubicacion: " << ubicacion << endl;
    cout << "Saldo: " << saldo_cuenta << endl;
    cout << "Tipo: " << getTipo() << endl;
    cout << "Produccion: " << produccion << " kWh" << endl;
    cout << "Consumo: " << consumo << " kWh" << endl;
}

// Calcula el excedente del nodo prosumidor
double NodoProsumidor::calcularExcedente() {
    balanceEnergia = produccion - consumo;
    return balanceEnergia;
}