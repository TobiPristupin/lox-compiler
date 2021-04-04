#include <chrono>
#include <iostream>
#include <vector>

class Outer {
public:

    class Inner {
    public:
         static void innerFunc(){
            std::cout << stack.back() << "\n";
        }
    };


private:
    static std::vector<std::string> stack;
};

int main(){
    Outer::Inner::innerFunc();
}