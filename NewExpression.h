//
// Created by elronbandel on 12/23/18.
//

#ifndef NEWEXPRESSION_H
#define NEWEXPRESSION_H


#include "StringExpression.h"

class NewExpression: public StringExpression {
public:
    explicit NewExpression(const string& stri) :StringExpression(stri) {
    }
    NewExpression() : StringExpression(""){
    }
    const type_info& getType() const override {
        return typeid(NewExpression);
    }
};

#endif //UNTITLED4_NEWEXPRESSION_H
