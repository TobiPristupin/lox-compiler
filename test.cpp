#include <cstddef>
#include <cstdio>
#include <iostream>
#include <bitset>
#include <utility>
#include <vector>
#include <memory>


enum class LiteralType {
    NIL, BOOL, NUMBER, OBJ
};

enum class ObjType {
    STRING, FUNCTION
};


class Obj {
public:
    ObjType type;

    explicit Obj(ObjType type);
    virtual ~Obj() = 0;

    bool isString() const;
};

Obj::~Obj() = default;

class StringObj : public Obj {
public:

    explicit StringObj(std::unique_ptr<std::string> str);

    std::unique_ptr<std::string> str;
};

class FunctionObj : public Obj {
public:
    FunctionObj(std::unique_ptr<std::string> name, int arity);

    int arity;
    std::unique_ptr<std::string> name;
};


Obj::Obj(ObjType type) : type(type) {}

bool Obj::isString() const {
    return type == ObjType::STRING;
}

StringObj::StringObj(std::unique_ptr<std::string> str) : Obj(ObjType::STRING), str(std::move(str)) {}


FunctionObj::FunctionObj(std::unique_ptr<std::string> name, int arity) :
        Obj(ObjType::FUNCTION), name(std::move(name)), arity(arity) {}

int main(){
    Obj *obj = new StringObj(std::make_unique<std::string>("hola"));
    std::cout << *dynamic_cast<StringObj*>(obj)->str << "\n";
}
