//
// Created by elronbandel on 12/23/18.
//

#ifndef VALUEEXPRESSION_H
#define VALUEEXPRESSION_H

#include "StringExpression.h"
#include "BindedSymbolMap.h"
#include "Expressioner.h"
#include "AssignmentCommand.h"
#include "BindCommand.h"
#include "Expression.h"
#include "BindedValue.h"

class ValueExpression : public Expression {

    Expressioner *expressioner;
    BindedSymbolMap *symap;
    string name;

public:
    //constructor:
    ValueExpression(BindedSymbolMap *bindedSymbolMap, Expressioner *expr, const string &nam) {
        symap = bindedSymbolMap;
        expressioner = expr;
        name = nam;
    }

    double calculate() const override {
        if (expressioner->next() != nullptr) {
            if (expressioner->next()->getType() == typeid(AssignmentCommand)) {
                Expointer next = expressioner->popNext();
                if (expressioner->argumentsInLine() == 0)
                    throw string("Error: operator = with no argument to assign");
                next = expressioner->popNext();
                if (symap->exist(name) && next->getType() != typeid(StringExpression)
                    && next->getType() != typeid(BindCommand)) {
                    symap->set(name, next->calculate());
                } else if (next->getType() == typeid(BindCommand)) {
                    symap->set(name, new BindedValue(next->getString(), symap));
                } else if (next->getType() == typeid(StringExpression)) {
                    symap->set(name, new StringValue(next->getString()));
                } else {
                    symap->set(name, new LocalValue(next->calculate()));
                }
            }
        }

        return *(*symap)[name];
    }

    string getString() const override {
        return (string) *(*symap)[name];
    }

    string getName() {
        return name;
    }

    const type_info &getType() const override {
        return typeid(ValueExpression);
    }
};

#endif //UNTITLED4_VALUEEXPRESSION_H
