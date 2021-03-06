#ifndef CLOX_CLOXLITERAL_H
#define CLOX_CLOXLITERAL_H


#include <string>
#include <memory>
#include <unordered_map>
#include "Token.h"
#include "Chunk.h"

enum class LiteralType {
    NIL, BOOL, NUMBER, OBJ
};

class Obj;

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


enum class ObjType {
    STRING, FUNCTION, CLASS, INSTANCE, ALLOCATION
};


class Obj {
public:
    ObjType type;
    bool marked = false;

    explicit Obj(ObjType type);
    virtual ~Obj() = 0;

    bool isString() const;
    bool isClass() const;
    bool isFunction() const;
    bool isInstance() const;
    bool isAllocation() const;
};

class StringObj : public Obj {
public:

    explicit StringObj(std::string str);

    std::string str;
};

class FunctionObj : public Obj {
public:
    FunctionObj(StringObj *name, Chunk *chunk, int arity);
    ~FunctionObj() override;

    int arity;
    StringObj *name;
    Chunk *chunk;
};

class ClassObj : public Obj {
public:
    explicit ClassObj(StringObj *name);

    StringObj *name;
};

class InstanceObj : public Obj {
public:
    explicit InstanceObj(ClassObj *klass);

    ClassObj *klass;
    std::unordered_map<std::string, CLoxLiteral> fields;
};

class AllocationObj : public Obj {
public:
    explicit AllocationObj(size_t kilobytes, char* memoryBlock);
    ~AllocationObj() override;

    size_t kilobytes;
    char* memoryBlock;
};


#endif //CLOX_CLOXLITERAL_H
