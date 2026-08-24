# ⚡ EcoGrid - Simulador de Micro-Red Eléctrica

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg?style=flat-square&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/17)
[![PostgreSQL](https://img.shields.io/badge/PostgreSQL-14%2B-336791.svg?style=flat-square&logo=postgresql&logoColor=white)](https://www.postgresql.org/)
[![SOCI](https://img.shields.io/badge/SOCI-C%2B%2B%20Database%20Access-orange.svg?style=flat-square)](https://soci.sourceforge.net/)
[![License](https://img.shields.io/badge/License-MIT-green.svg?style=flat-square)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Linux-lightgrey.svg?style=flat-square&logo=linux)](https://www.linux.org/)

**EcoGrid** es un simulador orientado a objetos desarrollado en C++17 que modela un mercado Peer-to-Peer (P2P) de energía en micro-redes eléctricas distribuidas. El sistema procesa ofertas de compra y venta de energía en tiempo real mediante un algoritmo de **Subasta Doble Continua**, integrando una batería comunitaria para absorber o inyectar excedentes y garantizando persistencia transaccional ACID sobre **PostgreSQL** a través de **SOCI**.

---

## 📋 Índice

- [Características Principales](#-características-principales)
- [Arquitectura y Patrones de Diseño](#-arquitectura-y-patrones-de-diseño)
- [Requisitos del Sistema](#-requisitos-del-sistema)
- [Instalación y Configuración](#-instalación-y-configuración)
- [Compilación y Ejecución](#-compilación-y-ejecución)
- [Estructura del Proyecto](#-estructura-del-proyecto)
- [Formato de Datos de Entrada](#-formato-de-datos-de-entrada)
- [Modelo de Base de Datos](#-modelo-de-base-de-datos)
- [Licencia](#-licencia)

---

## ✨ Características Principales

- **Modelado Polimórfico de Nodos:**
  - **Consumidores:** Residenciales, comerciales e industriales.
  - **Prosumidores:** Nodos capaces de consumir y generar energía excedente.
  - **Batería Comunitaria:** Nodo regulador con tarifa base dinámica según franja horaria.
- **Motor de Subasta Doble Continua:**
  - Libro de órdenes (*Order Book*) con prioridad por **precio** y criterio **FIFO** (tiempo).
  - Emparejamiento (*matching*) automático cuando el precio comprador es mayor o igual al precio vendedor ($P_{comprador} \ge P_{vendedor}$).
  - Cálculo de precio de liquidación transparente (promedio entre oferta y demanda).
- **Persistencia Transaccional Robusta (ACID):**
  - Validación de saldo comprador mediante **Triggers en PostgreSQL**.
  - Procedimientos almacenados PL/pgSQL para la actualización atómica de saldos y registros históricos.
  - Rollback automático e inmediatez ante fallos de saldo o inconsistencias.
- **Simulación por Ticks:**
  - Procesamiento modular de ofertas mediante archivos CSV por franjas horarias.

---

## 🏗️ Arquitectura y Patrones de Diseño

El sistema fue diseñado aplicando principios SOLID y patrones de diseño clásicos de la ingeniería de software:

1. **Pattern Singleton (`GridManager`):**
   - Garantiza una única instancia centralizada del mercado que administra el libro de órdenes, los nodos activos y el ciclo de subasta.
2. **Polimorfismo y Smart Pointers (`NodoRed`):**
   - Jerarquía de clases con base abstracta `NodoRed` heredada por `NodoConsumidor`, `NodoProsumidor` y `NodoBateria`. Gestiones de memoria seguras mediante `std::shared_ptr`.
3. **Capa de Acceso a Datos / DAO (`CapaDatos`):**
   - Encapsula la lógica de comunicación SQL utilizando la biblioteca **SOCI**, aislando el dominio de la simulación de la persistencia de datos.
4. **Libro de Órdenes (Priority Order Maps):**
   - Utiliza contenedores asociativos `std::map<double, std::queue<Orden>>` para mantener las puntas compradora (Bids) y vendedora (Asks) ordenadas eficientemente.

---

## 🛠️ Requisitos del Sistema

- **Sistema Operativo:** Linux (Ubuntu 20.04+, Debian 11+ o derivados).
- **Compilador:** Compatible con **C++17** (`g++` 9+ o `clang`).
- **Base de Datos:** **PostgreSQL** 12+.
- **Librerías C++ / PostgreSQL:**
  - `libsoci-dev` (SOCI Core)
  - `libsoci-postgresql-dev` (SOCI Backend para PostgreSQL)
  - `libpq-dev` (Librería cliente C/C++ de PostgreSQL)

### Instalación de Dependencias (Ubuntu / Debian)

```bash
sudo apt update
sudo apt install build-essential postgresql postgresql-contrib libsoci-dev libsoci-postgresql-dev libpq-dev
```

---

## ⚙️ Instalación y Configuración

### 1. Clonar el Repositorio

```bash
git clone https://github.com/tu-usuario/EcoGrid.git
cd EcoGrid
```

### 2. Configurar la Base de Datos en PostgreSQL

Asegúrate de que el servicio de PostgreSQL esté iniciado:

```bash
sudo systemctl start postgresql
```

Crea la base de datos `ecogrid` y ejecuta el script de inicialización de esquema y datos semilla:

```bash
createdb ecogrid
psql -d ecogrid -f db/schema.sql
```

### 3. Configurar Credenciales de Conexión

Copia el archivo de configuración de plantilla `util/config.ini.example` a `util/config.ini`:

```bash
cp util/config.ini.example util/config.ini
```

Edita `util/config.ini` introduciendo las credenciales de tu servidor PostgreSQL local:

```ini
[database]
host=localhost
port=5432
dbname=ecogrid
user=postgres
password=TU_PASSWORD_AQUI
```

> ⚠️ **Nota:** El archivo `util/config.ini` está excluido del control de versiones mediante `.gitignore` para no exponer credenciales.

---

## 🚀 Compilación y Ejecución

### Opción 1: Mediante `Makefile` (Recomendado)

Compilar el proyecto:
```bash
make
```

Ejecutar la simulación:
```bash
make run
```

Limpiar archivos compilados:
```bash
make clean
```

### Opción 2: Compilación Manual con `g++`

```bash
g++ -std=c++17 -Wall -Wextra -I./src -I/usr/include/postgresql \
  src/main.cpp \
  src/CapaDatos.cpp \
  src/GridManager.cpp \
  src/NodoBateria.cpp \
  src/NodoConsumidor.cpp \
  src/NodoProsumidor.cpp \
  src/NodoRed.cpp \
  -lsoci_core -lsoci_postgresql -lpq \
  -o ecogrid

./ecogrid
```

---

## 📁 Estructura del Proyecto

```text
EcoGrid/
├── Makefile                       # Script de automatización de compilación
├── README.md                      # Documentación del proyecto
├── .gitignore                     # Filtro de archivos para Git
├── db/
│   └── schema.sql                 # Tablas, triggers, funciones PL/pgSQL y datos semilla
├── src/
│   ├── main.cpp                   # Punto de entrada y orquestador del ciclo de simulación
│   ├── CapaDatos.cpp              # Persistencia e integración con SOCI / PostgreSQL
│   ├── GridManager.cpp            # Motor de subasta y gestión del Order Book
│   ├── NodoRed.cpp                # Implementación de la clase base de nodos
│   ├── NodoConsumidor.cpp         # Implementación de nodos consumidores
│   ├── NodoProsumidor.cpp         # Implementación de nodos prosumidores
│   ├── NodoBateria.cpp            # Implementación de la batería comunitaria
│   └── headers/                   # Archivos de cabecera (.h)
│       ├── CapaDatos.h
│       ├── Estructuras.h
│       ├── GridManager.h
│       ├── NodoBateria.h
│       ├── NodoConsumidor.h
│       ├── NodoProsumidor.h
│       └── NodoRed.h
└── util/
    ├── config.ini.example         # Plantilla de configuración de la base de datos
    └── datos_ofertas_*.csv        # Datos de ofertas de prueba por cada tick (10, 12, 14, 15, 18, 20)
```

---

## 📊 Formato de Datos de Entrada

La simulación procesa eventos en tiempo discreto (ticks) leídos desde archivos CSV almacenados en la carpeta `util/`.

Ejemplo de estructura de archivo de ofertas (`datos_ofertas_10.csv`):

```csv
id_orden,lado,id_nodo,kwh,precio
1001,venta,2,5,2.0
2001,compra,1,5,2.5
```

- `id_orden`: Identificador único de la oferta.
- `lado`: Operación (`compra` o `venta`).
- `id_nodo`: Identificador del nodo emisor (debe existir en la base de datos).
- `kwh`: Cantidad de energía ofertada en kilovatios-hora.
- `precio`: Precio ofertado por kWh en la moneda del sistema.

---

## 🗄️ Modelo de Base de Datos

El esquema SQL (`db/schema.sql`) define las siguientes tablas y reglas de negocio:

- **`nodos`**: Registro de participantes (id, ubicación, tipo, saldo de cuenta, perfil de consumo).
- **`transacciones`**: Histórico de emparejamientos ejecutados (vendedor, comprador, kWh, precio unitario, timestamp).
- **`lecturas_historicas`**: Métricas de consumo, producción y balance por tick de cada nodo.
- **`config_tarifas`**: Precios de referencia por franja horaria.
- **Trigger `trg_validar_saldo`**: Intercepta inserciones en `transacciones` para impedir operaciones de compradores sin fondos suficientes.
- **Procedimiento `actualizar_saldo_y_lecturas`**: Modifica saldos y genera registros en `lecturas_historicas` de forma atómica.

---