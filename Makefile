# Compila os tres programas do projeto dentro de bin/
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iinclude

ARVORES = src/avl.cpp src/rubronegra.cpp

all: bin/arvores bin/testes bin/comparacao

bin/arvores: src/main.cpp $(ARVORES) include/*.h
	$(CXX) $(CXXFLAGS) src/main.cpp $(ARVORES) -o $@

bin/testes: src/testes.cpp $(ARVORES) include/*.h
	$(CXX) $(CXXFLAGS) src/testes.cpp $(ARVORES) -o $@

bin/comparacao: src/comparacao.cpp $(ARVORES) include/*.h
	$(CXX) $(CXXFLAGS) src/comparacao.cpp $(ARVORES) -o $@

testes: bin/testes
	./bin/testes

comparacao: bin/comparacao
	./bin/comparacao

clean:
	rm -f bin/arvores bin/testes bin/comparacao

.PHONY: all testes comparacao clean
