#ifndef CLOX_CLOXLITERAL_H
#define CLOX_CLOXLITERAL_H


#include <string>
#include <memory>
#include "Token.h"

enum class LiteralType {
    NIL, BOOL, NUMBER, OBJ
};

enum class ObjType {
    STRING
};


class Obj {
public:
    ObjType type;

    explicit Obj(ObjType type);
    virtual ~Obj() = default;

    bool isString() const;
};

class StringObj : public Obj {
public:

    StringObj(std::string str);

    std::string str;
};

std::string literalTypeToString(LiteralType type);


class CLoxLiteral { //This could be made into a more efficient std::variant some day (i.e. probably never....)
public:
    LiteralType type = LiteralType::NIL;

    explicit CLoxLiteral(double number);
    explicit CLoxLiteral(Obj *obj);
    explicit CLoxLiteral(bool boolean);
    static CLoxLiteral Nil();
    CLoxLiteral(); //Initializes the object as NIL

    bool isNumber() const;
    bool isBoolean() const;
    bool isObj() const;
    bool isNil() const;

    double getNumber() const;
    bool getBoolean() const ;
    Obj* getObj() const;

    friend std::ostream& operator<<(std::ostream& os, const CLoxLiteral& object);

private:

    double number = 0.0;
    bool boolean = false;
    Obj *obj = nullptr;
};


#endif //CLOX_CLOXLITERAL_H
