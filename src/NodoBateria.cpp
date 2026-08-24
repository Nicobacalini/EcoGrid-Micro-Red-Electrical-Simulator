#include "headers/NodoBateria.h"
#include <iostream>

using namespace std; 

//Constructor
NodoBateria::NodoBateria(int id, const string& ubi, double saldo)
: NodoRed(id, ubi, saldo), carga_actual_kwh(0.0) {}

//Devuelve el tipo de nodo
string NodoBateria::getTipo() const {
    return "Bateria";
}

void NodoBateria::imprimirDetalle() const {
    cout << "ID: " << id_nodo << endl;
    cout << "Ubicacion: " << ubicacion << endl;
    cout << "Saldo: " << saldo_cuenta << endl;
    cout << "Tipo: " << getTipo() << endl;
    cout << "Carga actual: " << carga_actual_kwh << endl;
}


double NodoBateria::getCarga() const{
    return carga_actual_kwh;
}

void NodoBateria::actualizarCarga(double variacion_kwh){
    carga_actual_kwh += variacion_kwh;
}

double NodoBateria::calcularExcedente() {
    balanceEnergia = carga_actual_kwh > 0 ? carga_actual_kwh : 0.0;
    return balanceEnergia;
}