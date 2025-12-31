#include "sandbox_project/top.hpp"

#include <iostream>

int main() {
    std::cout << "Expected 3, Got: " << a() << std::endl; 
    return a();  // expect 3
}
