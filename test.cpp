#include <chrono>
#include <iostream>

int main(){
    std::cout << std::chrono::system_clock::now().time_since_epoch().count() << "\n";
}