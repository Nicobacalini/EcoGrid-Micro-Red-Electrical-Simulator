#include "headers/NodoConsumidor.h"
#include <iostream>
using namespace std; 


// Constructor
NodoConsumidor::NodoConsumidor(int id, const string& ubi, double saldo, PerfilConsumo perfil)
    : NodoRed(id, ubi, saldo), perfil(perfil) {}
  
//Setea la demanda actual
void NodoConsumidor::setDemandaActual(double kwh) {
    demanda_actual = kwh;
}

//Devuelve el tipo de nodo consumidor    
string NodoConsumidor::getTipo() const {
    switch (perfil) {
        case PerfilConsumo::Residencial:
            return "Residencial";
        case PerfilConsumo::Comercial:
            return "Comercial";
        case PerfilConsumo::Industrial:
            return "Industrial";
        default:
            return "Desconocido";
    }
}

//Imprime los datos del nodo consumidor
void NodoConsumidor::imprimirDetalle() const {
    cout << "ID: " << id_nodo << endl;
    cout << "Ubicacion: " << ubicacion << endl;
    cout << "Saldo: " << saldo_cuenta << endl;
    cout << "Perfil: " << getTipo() << endl;
    cout << "Demanda actual: " << demanda_actual << endl;
}

//Calcula el excedente del nodo consumidor
double NodoConsumidor::calcularExcedente() {
    balanceEnergia = -(demanda_actual);
    return balanceEnergia;
}