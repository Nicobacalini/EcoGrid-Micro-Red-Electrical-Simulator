# Compiler and Flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./src -I/usr/include/postgresql
LIBS = -lsoci_core -lsoci_postgresql -lpq

# Target Executable
TARGET = ecogrid

# Source and Header Files
SRCS = src/main.cpp \
       src/CapaDatos.cpp \
       src/GridManager.cpp \
       src/NodoBateria.cpp \
       src/NodoConsumidor.cpp \
       src/NodoProsumidor.cpp \
       src/NodoRed.cpp

OBJS = $(SRCS:.cpp=.o)

# Default Rule
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean Build Files
clean:
	rm -f $(OBJS) $(TARGET)

# Run Executable
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
