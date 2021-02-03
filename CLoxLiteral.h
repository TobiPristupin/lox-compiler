#ifndef CLOX_CLOXLITERAL_H
#define CLOX_CLOXLITERAL_H


#include <string>
#include "Token.h"

enum class LiteralType {
    NIL, BOOL, NUMBER, STRING
};

std::string literalTypeToString(LiteralType type);


class CLoxLiteral {
public:
    LiteralType type = LiteralType::NIL;

    explicit CLoxLiteral(const Token &token);
    explicit CLoxLiteral(double number);
    explicit CLoxLiteral(const std::string &string);
    explicit CLoxLiteral(const char* string);
    explicit CLoxLiteral(bool boolean);
    static CLoxLiteral Nil();
    CLoxLiteral(); //Initializes the object as NIL

    bool isNumber() const;
    bool isBoolean() const;
    bool isString() const;
    bool isNil() const;

    bool truthy() const;

    double getNumber() const;
    bool getBoolean() const ;
    std::string getString() const;

    friend std::ostream& operator<<(std::ostream& os, const CLoxLiteral& object);
    friend CLoxLiteral operator+(const CLoxLiteral &lhs, const CLoxLiteral &rhs);
    friend CLoxLiteral operator-(const CLoxLiteral &lhs, const CLoxLiteral &rhs);
    friend CLoxLiteral operator*(const CLoxLiteral &lhs, const CLoxLiteral &rhs);
    friend CLoxLiteral operator/(const CLoxLiteral &lhs, const CLoxLiteral &rhs);
    friend bool operator==(const CLoxLiteral &lhs, const CLoxLiteral &rhs);
    friend bool operator!=(const CLoxLiteral &lhs, const CLoxLiteral &rhs);
    friend bool operator>(const CLoxLiteral &lhs, const CLoxLiteral &rhs);
    friend bool operator>=(const CLoxLiteral &lhs, const CLoxLiteral &rhs);
    friend bool operator<(const CLoxLiteral &lhs, const CLoxLiteral &rhs);
    friend bool operator<=(const CLoxLiteral &lhs, const CLoxLiteral &rhs);
    CLoxLiteral operator++();
    CLoxLiteral operator--();
    CLoxLiteral operator-() const;
    CLoxLiteral operator!() const;


private:

    double number = 0.0;
    bool boolean = false;
    std::string str = "";
};


#endif //CLOX_CLOXLITERAL_H
