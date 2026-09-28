#include "parser.hpp"
#include <iostream>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[]) {
    std::string grammar_file = "grammar.txt";

    if (argc > 1) {
        grammar_file = argv[1];
    }

    try {
        Grammar grammar = Grammar::fromFile(grammar_file);
        grammar.printGrammar();

        LR1Builder builder(grammar);
        builder.first();
        builder.coleccionCanonica();
        bool ok = builder.llenarTablas();

        builder.printFirstTable();
        builder.printClosureTable();
        builder.printLRTable();

        if (!ok) {
            std::cerr << "\n[ERROR] Se detectaron conflictos en la tabla LR(1). Finalizando con codigo no cero." << std::endl;
            return 1;
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

//g++ -std=c++17 -o parser.exe main.cpp parser.cpp

//.\parser.exe tests/test_parentesis.txt
//.\parser.exe tests/test_reduce_reduce.txt
//.\parser.exe tests/test_shift_reduce.txt
//.\parser.exe tests/test_vacias.txt

//.\parser.exe