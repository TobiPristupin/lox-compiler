#include <cstddef>
#include <cstdio>
#include <iostream>
#include <bitset>
#include <utility>
#include <vector>

enum class LiteralType {
    NIL, BOOL, NUMBER, OBJ
};

enum class ObjType {
    STRING
};


class Obj {
public:

    explicit Obj(ObjType type) : type(type) {}
    virtual ~Obj() = default;

    ObjType type;
};

class StringObj : public Obj {
public:

    explicit StringObj(std::string ptr) : str(std::move(ptr)), Obj(ObjType::STRING) {}
    ~StringObj() override {
        std::cout << "calling string destructor\n";
    }

    std::string str;
};

int main(){
    auto *str = new StringObj("cacon");
    std::vector<Obj*> objects;
    objects.push_back(str);

    for (Obj *obj : objects){
        delete obj;
    }
}
