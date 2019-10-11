//
// Created by fatuity on 12/18/18.
//

#ifndef PROJ1_NUMBER_H
#define PROJ1_NUMBER_H


#include <string>
#include "Expression.h"

class Number : public Expression {
    string number;
public:
    Number(std::string num) {
        number = num;
    }

    virtual double calculate() {
        return stod(number);
    }

    virtual string getString() {
        return number;
    }
};


#endif //PROJ1_NUMBER_H
