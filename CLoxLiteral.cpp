#include <stdexcept>
#include <cmath>
#include <utility>
#include "CLoxLiteral.h"
#include "Utils.h"

Obj::Obj(ObjType type) : type(type) {}

bool Obj::isString() const {
    return type == ObjType::STRING;
}

StringObj::StringObj(std::string str) : Obj(ObjType::STRING), str(std::move(str)) {}


CLoxLiteral::CLoxLiteral(double number) : type(LiteralType::NUMBER), number(number) {}

CLoxLiteral::CLoxLiteral(Obj *obj) : type(LiteralType::OBJ), obj(obj) {}

CLoxLiteral::CLoxLiteral(bool boolean) : type(LiteralType::BOOL), boolean(boolean) {}

CLoxLiteral CLoxLiteral::Nil() {
    return CLoxLiteral();
}

CLoxLiteral::CLoxLiteral() : type(LiteralType::NIL) {}


bool CLoxLiteral::isNumber() const {
    return type == LiteralType::NUMBER;
}

bool CLoxLiteral::isBoolean() const {
    return type == LiteralType::BOOL;
}

bool CLoxLiteral::isObj() const {
    return type == LiteralType::OBJ;
}

bool CLoxLiteral::isNil() const {
    return type == LiteralType::NIL;
}

double CLoxLiteral::getNumber() const {
    if (!isNumber()){
        throw std::runtime_error("CLoxLiteral does not contain a number");
    }
    return number;
}

bool CLoxLiteral::getBoolean() const {
    if (!isBoolean()){
        throw std::runtime_error("CLoxLiteral does not contain a boolean");
    }
    return boolean;
}

Obj *CLoxLiteral::getObj() const {
    if (!isObj()){
        throw std::runtime_error("CLoxLiteral does not contain a obj");
    }

    return obj;
}

std::ostream &operator<<(std::ostream &os, const CLoxLiteral &object) {
    switch (object.type) {
        case LiteralType::NIL:
            os << std::string("nil");
            return os;
        case LiteralType::BOOL:
            os << (object.getBoolean() ? std::string("true") : std::string("false"));
            return os;
        case LiteralType::NUMBER:
            if (std::abs(floor(object.getNumber())) == std::abs(object.getNumber())){ //If it has no decimal part
                os << std::to_string((long long) object.getNumber());
            } else {
                os << std::to_string(object.getNumber());
            }
            return os;
        case LiteralType::OBJ:
        {
            switch (object.getObj()->type) {
                case ObjType::STRING:
                    std::string s = dynamic_cast<StringObj*>(object.getObj())->str;
                    utils::replaceAll(s, "\\n", "\n");
                    utils::replaceAll(s, "\\t", "\t");
                    os << s;
                    return os;
            }
        }
        default:
            throw std::runtime_error("Object has no string representation");
    }
}


std::string literalTypeToString(LiteralType type) {
    switch (type) {
        case LiteralType::NIL:
            return "nil";
        case LiteralType::BOOL:
            return "bool";
        case LiteralType::NUMBER:
            return "number";
        case LiteralType::OBJ:
            return "obj";
    }

    throw std::runtime_error("This should be unreachable. Missing case.");
}


