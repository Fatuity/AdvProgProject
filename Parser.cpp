#include "Parser.h"
#include "NewExpression.h"
#include "ValueExpression.h"
#include <list>

Expression *Parser::applyOp(Expression *left, Expression *right, string op) {
    if (op == "+") {
        return new Plus(op, left, right);
    } else if (op == "-") {
        return new Minus(op, left, right);
    } else if (op == "*") {
        return new Mult(op, left, right);
    } else if (op == "/") {
        return new Div(op, left, right);
    } else if (op == neg) {
        return new Neg(op, right);
    }
}

int Parser::precendance(string opr) {
    if (opr == "+" || opr == "-") {
        return 1;
    }
    if (opr == "*" || opr == "/") {
        return 2;
    }
    if (opr == neg) {
        return 3;
    }
    return 0;
}

void Parser::shuntingYardHelper(stack<string> &oprs, stack<Expression *> &values) {
    string op;
    Expression *leftVal, *rightVal;
    if (oprs.top() == neg) {
        rightVal = values.top();
        values.pop();

        op = oprs.top();
        oprs.pop();

        values.push(applyOp(nullptr, rightVal, op));
    } else {
        rightVal = values.top();
        values.pop();

        leftVal = values.top();
        values.pop();

        op = oprs.top();
        oprs.pop();

        values.push(applyOp(leftVal, rightVal, op));
    }
}

Expression *Parser::shuntingYard(Line exp) {
    stack<Expression *> values;
    stack<string> oprs;
    string op;
    Expression *rightVal;
    Expression *leftVal;
    //run all over the expression.
    for (int i = 0; i < exp.size(); i++) {
        //if its opening braces push to oprators stack.
        if (exp[i] == "(") {
            oprs.push(exp[i]);
            //if its a number push to values stack.
        } else if (isNum(exp[i])) {
            values.push(new Number(exp[i]));
            //if its closing braces pop all operators to values stack.
        } else if (symap->exist(exp[i])) {
            values.push(new ValueExpression(symap, expressioner, exp[i]));
        } else if (exp[i] == ")") {
            while (!oprs.empty() && oprs.top() != "(") {
                shuntingYardHelper(oprs, values);
            }
            if (!oprs.empty()) {
                //pop opening braces.
                oprs.pop();
            }
        } else {
            while (!oprs.empty() && precendance(oprs.top()) >= precendance(exp[i])) {
                shuntingYardHelper(oprs, values);
            }
            oprs.push(exp[i]);
        }
    }
    while (!oprs.empty()) {
        shuntingYardHelper(oprs, values);
    }
    return values.top();
}

bool Parser::isNum(string word) {
    bool dotCount = false;
    for (int i = 0; i < word.size(); i++) {
        //the char is not a digit
        if (!isdigit(word[i])) {
            //check if its a dot (and if its the only dot)
            if (i != 0 && word[i] == '.' && !dotCount) {
                dotCount = true;
                continue;
            }
            return false;
        }
    }
    return true;
}

bool Parser::isOpr(const string &word) {
    return ((word == "+") || (word == "-") || (word == "*") ||
            (word == "/") || (word == "(") || (word == ")") || (word == ","));
}

bool Parser::isIp(const string &word) {
    vector<string> numbers = split(word, '.');
    if (numbers.size() != 4)
        return false;
    for (auto &number : numbers) {
        if (!isNum(number))
            return false;
    }
    return true;
}

bool Parser::isStringWord(const string &word) {
    if (word[0] == '\"' && word[word.size() - 1] == '\"')
        return true;
    return false;
}

bool Parser::isLegalVarName(const string &word) {
    if (isdigit(word[0]))
        return false;
    for (auto c : word) {
        if (!isdigit(c) && !isalpha(c))
            return false;
    }
    return true;
}

bool Parser::isCondition(const string &word) {
    return (word == "<" || word == ">" || word == "<=" ||
            word == ">=" || word == "==" || word == "!=");
}

bool parenthesesCheck(Line *line) {
    int counter = 0;
    for (int i = 0; i < line->size(); i++) {
        if ((*line)[i] == "(") {
            counter++;
        } else if ((*line)[i] == ")") {
            counter--;
        }
    }
    return !counter;
}

Line Parser::getMathLine(Line *line) {
    Line mathExp;
    if (!parenthesesCheck(line)) {
        throw "Error: parenthesis count is'nt equal : " + (*line)[0];
    }
    while (!line->empty()) {
        if (!(isNum((*line)[0]) || isOpr((*line)[0]) || symap->exist((*line)[0]))) {
            return mathExp;
        }
        //if there's only one number
        if (line->size() == 1 && isNum((*line)[0])) {
            mathExp.addWord(line->popFirst());
            return mathExp;
        }
        //if theres only one variable
        if (line->size() == 1 && symap->exist((*line)[0])) {
            mathExp.addWord(line->popFirst());
            return mathExp;
        }
        if (line->size() > 1) {
            //if theres a number and then no operators its the end of an expression.
            if (isNum(((*line)[0])) && (!isOpr((*line)[1]) ||
                                        (*line)[1] == ",")) {
                mathExp.addWord(line->popFirst());
                return mathExp;
            }
            //if theres a symbol and no operators its the end of an expression.
            if (symap->exist((*line)[0]) && (!isOpr((*line)[1]) ||
                                             (*line)[1] == ",")) {
                //add the variable to the mathExp
                string val = line->popFirst();
                mathExp.addWord(val);
                return mathExp;
            }

        }
        if ((*line)[0] == ",") {
            line->popFirst();
            continue;
        }
        //if the word is an operator.
        if (isOpr((*line)[0])) {
            //if the expression ended with an operator throw exception.
            if (1 == line->size() && (*line)[0] != ")") {
                throw "Error: illegal expression : " + (*line)[0];
            }
            if (line->size() > 1) {
                //if the next is not a number and not a "-" the expression is illegal.
                if ((!isNum((*line)[1]))
                    && (!symap->exist((*line)[1])) && ((*line)[0] != ")")) {
                    if ((*line)[1] != "-" && (*line)[1] != "(") {
                        throw "Error: illegal line : " + (*line)[0] + " " + (*line)[1];
                    }
                    if ((*line)[1] == "-" && (mathExp.empty()) && (*line)[0] != "(") {
                        throw "Error: illegal line : " + (*line)[0] + " " + (*line)[1];
                    } else if ((*line)[0] != "(" && (*line)[1] == "-" &&
                               !isNum(mathExp.back()) &&
                               mathExp.back() != ")" &&
                               !symap->exist(mathExp.back())) {
                        throw "Error: illegal line : " + (*line)[0] + " " + (*line)[1];
                    }
                    //if the next char is an unary minus and the size > 2.
                    if (line->size() > 2) {
                        //if the next char does not fit to an unary minus throw exception.
                        if ((*line)[1] != "(" && (*line)[2] != "(" &&
                            !isNum((*line)[2]) &&
                            (!symap->exist((*line)[2]))) {
                            throw "Error: illegal line: " + (*line)[0];
                        }
                    }
                }

            }
            //if its "-" check if its unary.
            if ((*line)[0] == "-") {
                if ((mathExp.empty()) || ((mathExp.back() != ")") && isOpr(mathExp.back()))) {
                    line->popFirst();
                    mathExp.addWord(neg);
                    continue;
                }
            }
        }
        //if its not one of the cases above, add to the current exp and continue the loop.
        mathExp.addWord(line->popFirst());
    } //end of while loop
    return mathExp;
}

Expression *Parser::getConditionExpression(list<Expression *> &expList, Line *line) {
    Expression *left, *right;
    string strVal = line->popFirst();
    if (line->empty()) {
        throw "Error : condition Expression is not legitimate " + strVal;
    }
    string word = line->first();
    left = expList.back();
    expList.pop_back();
    if (isNum(word) || isOpr(word) || symap->exist(word) || word == "(") {
        right = shuntingYard(getMathLine(line));
    } else {
        throw "Error : condition Expression is not legitimate " + (*line)[0];
    }
    return new ConditionExpression(strVal, left, right);
}

list<Expression *> Parser::next() {
    Line *line = new Line(lexer.lexer(inputer.next()));
    list<Expression *> expList;
    while (!line->empty()) {
        Line temp;
        string word = line->first();

        if (dictionary.find(word) != dictionary.end()) {
            //if the word is recognized by the map add it to list
            expList.emplace_back(dictionary[word]);
            line->popFirst();
        } else if (isNum(word) || isOpr(word) || symap->exist(word) || word == "(") {
            //if its the begginig of maths exp
            expList.emplace_back(shuntingYard(getMathLine(line)));
        } else if (isIp(word)) {
            expList.emplace_back(new StringExpression(word));
            line->popFirst();
        } else if (isStringWord(word)) {
            //if its a word in commas "____"
            expList.emplace_back(new StringExpression(word.substr(1, word.size() - 2)));
            line->popFirst();
        } else if (isLegalVarName(word)) {
            //if its letters can emphsaize new name for var
            expList.emplace_back(new NewExpression(word));
            line->popFirst();
        } else if (line->size() == 1 && symap->exist(word)) {
            expList.emplace_back(new ValueExpression(symap, expressioner, word));
            line->popFirst();
        } else if (isCondition(word)) {
            expList.emplace_back(getConditionExpression(expList, line));
        } else {
            delete line;
            throw "Error: illegal expression: " + word;
        };

    }
    delete line;
    return expList;
}

