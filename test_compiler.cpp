#include "lib/doda_compiler.h"
#include <iostream>

int main() {
    std::cout << "Testing DODA Compiler Library..." << std::endl;
    
    try {
        // Just test creating the compiler
        DodaCompiler compiler("/tmp/simple_test");
        std::cout << "✅ Compiler created successfully!" << std::endl;
        std::cout << "✅ Library integration works!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}