//
// Created by fatuity on 12/18/18.
//

#ifndef PROJ1_LINE_H
#define PROJ1_LINE_H


#include <string>
#include <vector>

using namespace std;

class Line {
    vector<string> line;
public:
    Line(vector<string> newLine) {
        line = newLine;
    }

    Line() {}

    string popFirst() {
        if (line.empty()) {
            throw "line is empty";
        }
        string retVal = line.at(0);
        line = vector<string>(&line[1], &line[line.size() - 1]);
        return retVal;
    }

    string operator[](size_t n) {
        return line.at(n);
    }

    int size() {
        return line.size();
    }

    bool has(int n) {
        return (!line.empty());
    }

    void addWord(string newWord) {
        line.push_back(newWord);
    }

    bool empty() {
        return line.empty();
    }

    string back() {
        if (line.empty()) {
            return nullptr;
        }
        return line.back();
    }
};


#endif //PROJ1_LINE_H