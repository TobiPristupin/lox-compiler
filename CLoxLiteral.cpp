#include <stdexcept>
#include <cmath>
#include "CLoxLiteral.h"
#include "Utils.h"


CLoxLiteral::CLoxLiteral(double number) : type(LiteralType::NUMBER), number(number) {}

CLoxLiteral::CLoxLiteral(const std::string &string) : type(LiteralType::STRING), str(string) {}

CLoxLiteral::CLoxLiteral(const char *string) : CLoxLiteral(std::string(string)) {}

CLoxLiteral::CLoxLiteral(bool boolean) : type(LiteralType::BOOL), boolean(boolean) {}

CLoxLiteral CLoxLiteral::Nil() {
    return CLoxLiteral();
}

CLoxLiteral::CLoxLiteral(const Token &token) {
    switch (token.type) {
        case TokenType::NUMBER:
            type = LiteralType::NUMBER;
            number = std::stod(token.lexeme);
            break;
        case TokenType::TRUE:
            type = LiteralType::BOOL;
            boolean = true;
            break;
        case TokenType::FALSE:
            type = LiteralType::BOOL;
            boolean = false;
            break;
        case TokenType::STRING:
            type = LiteralType::STRING;
            str = token.lexeme;
            break;
        case TokenType::NIL:
            type = LiteralType::NIL;
            break;
        default:
            throw std::runtime_error("Invalid token type when constructing CLoxLiteral");
    }
}

CLoxLiteral::CLoxLiteral() : type(LiteralType::NIL) {}


bool CLoxLiteral::isNumber() const {
    return type == LiteralType::NUMBER;
}

bool CLoxLiteral::isBoolean() const {
    return type == LiteralType::BOOL;
}

bool CLoxLiteral::isString() const {
    return type == LiteralType::STRING;
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

std::string CLoxLiteral::getString() const {
    if (!isString()){
        throw std::runtime_error("CLoxLiteral does not contain a string");
    }
    return str;
}

bool CLoxLiteral::truthy() const {//In clox every literal is considered true except for nil and false
    if (isBoolean()){
        return getBoolean();
    } else if (isNil()){
        return false;
    }

    return true;
}

CLoxLiteral operator+(const CLoxLiteral &lhs, const CLoxLiteral &rhs) {
    if (lhs.isNumber() && rhs.isNumber()){
        return CLoxLiteral(lhs.getNumber() + rhs.getNumber());
    } else if (lhs.isString() && rhs.isString()){
        return CLoxLiteral(lhs.getString() + rhs.getString());
    }

//    else if (lhs.isString() && rhs.isNumber()){
//        return CLoxLiteral(lhs.getString() + std::to_string(rhs.getNumber()));
//    } else if (lhs.isNumber() && rhs.isString()){
//        return CLoxLiteral(std::to_string(lhs.getNumber()) + rhs.getString());
//    }

    else {
        throw std::runtime_error("Cannot apply operator '+' to operands of type " + literalTypeToString(lhs.type) + " and " + literalTypeToString(rhs.type));
    }
}

CLoxLiteral operator-(const CLoxLiteral &lhs, const CLoxLiteral &rhs) {
    if (lhs.isNumber() && rhs.isNumber()){
        return CLoxLiteral(lhs.getNumber() - rhs.getNumber());
    } else {
        throw std::runtime_error("Cannot apply operator '-' to operands of type " + literalTypeToString(lhs.type) + " and " + literalTypeToString(rhs.type));
    }
}

CLoxLiteral operator*(const CLoxLiteral &lhs, const CLoxLiteral &rhs) {
    if (lhs.isNumber() && rhs.isNumber()){
        return CLoxLiteral(lhs.getNumber() * rhs.getNumber());
    } else {
        throw std::runtime_error("Cannot apply operator '*' to operands of type " + literalTypeToString(lhs.type) + " and " + literalTypeToString(rhs.type));
    }
}

CLoxLiteral operator/(const CLoxLiteral &lhs, const CLoxLiteral &rhs) {
    if (lhs.isNumber() && rhs.isNumber()){
        if (rhs.getNumber() == 0.0){
            throw std::runtime_error("Cannot divide by zero");
        }
        return CLoxLiteral(lhs.getNumber() / rhs.getNumber());
    } else {
        throw std::runtime_error("Cannot apply operator '/' to operands of type " + literalTypeToString(lhs.type) + " and " + literalTypeToString(rhs.type));
    }
}

bool operator==(const CLoxLiteral &lhs, const CLoxLiteral &rhs) {
    if (lhs.type != rhs.type) return false;

    if (lhs.isNumber() && rhs.isNumber()){
        return lhs.getNumber() == rhs.getNumber();
    } else if (lhs.isString() && rhs.isString()){
        return lhs.getString() == rhs.getString();
    } else if (lhs.isBoolean() && rhs.isBoolean()){
        return lhs.getBoolean() == rhs.getBoolean();
    } else if (lhs.isNil() && rhs.isNil()){
        return true;
    }

    throw std::runtime_error("This should be unreachable. Missing case");
}

bool operator!=(const CLoxLiteral &lhs, const CLoxLiteral &rhs) {
    return !(lhs == rhs);
}

bool operator>(const CLoxLiteral &lhs, const CLoxLiteral &rhs) {
    if (lhs.isNumber() && rhs.isNumber()){
        return lhs.getNumber() > rhs.getNumber();
    } else if (lhs.isString() && rhs.isString()){
        return lhs.getString() > rhs.getString();
    } else {
        throw std::runtime_error("Cannot apply operator '>' to operands of type " + literalTypeToString(lhs.type) + " and " + literalTypeToString(rhs.type));
    }
}

bool operator>=(const CLoxLiteral &lhs, const CLoxLiteral &rhs) {
    return lhs > rhs || lhs == rhs;
}

bool operator<(const CLoxLiteral &lhs, const CLoxLiteral &rhs) {
    if (lhs.isNumber() && rhs.isNumber()){
        return lhs.getNumber() < rhs.getNumber();
    } else if (lhs.isString() && rhs.isString()){
        return lhs.getString() < rhs.getString();
    } else {
        throw std::runtime_error("Cannot apply operator '<' to operands of type " + literalTypeToString(lhs.type) + " and " + literalTypeToString(rhs.type));
    }
}

bool operator<=(const CLoxLiteral &lhs, const CLoxLiteral &rhs) {
    return lhs < rhs || lhs == rhs;
}

CLoxLiteral CLoxLiteral::operator-() const {
    if (this->isNumber()){
        return CLoxLiteral(-getNumber());
    }

    throw std::runtime_error("Cannot apply unary operator '-' to operand of type " + literalTypeToString(this->type));
}

CLoxLiteral CLoxLiteral::operator!() const {
    if (this->isBoolean()){
        return CLoxLiteral(!truthy());
    }

    throw std::runtime_error("Cannot apply unary operator '!' to operand of type " + literalTypeToString(this->type));
}

CLoxLiteral CLoxLiteral::operator++() {
    if (isNumber()){
        return CLoxLiteral(++number);
    }

    throw std::runtime_error("Cannot apply prefix operator '++' to operand of type " + literalTypeToString(this->type));
}

CLoxLiteral CLoxLiteral::operator--() {
    if (isNumber()){
        return CLoxLiteral(--number);
    }

    throw std::runtime_error("Cannot apply prefix operator '--' to operand of type " + literalTypeToString(this->type));
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
        case LiteralType::STRING:
        {
            std::string s = object.getString();
            utils::replaceAll(s, "\\n", "\n");
            utils::replaceAll(s, "\\t", "\t");
            os << s;
            return os;
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
        case LiteralType::STRING:
            return "string";
    }

    throw std::runtime_error("This should be unreachable. Missing case.");
}


