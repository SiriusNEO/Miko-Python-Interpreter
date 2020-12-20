//
// Created by SiriusNEO on 2020/12/18.
//

#ifndef PYTHON_INTERPRETER_TRUEADMINISTRATOR_HPP
#define PYTHON_INTERPRETER_TRUEADMINISTRATOR_HPP

#include <Python3Parser.h>
#include <unordered_map>
#include <iostream>
#include "Base.h"

const int maxDepth = 2005;

enum flowStmtType {_break, _return, _continue};

class TrueAdministrator {
    public:
        //Visible to the Evalvisitor
        std::unordered_map <std::string, Base*> globalVariable;
        std::unordered_map <std::string, Base*> temporaryVariable[maxDepth];
        std::unordered_map <std::string, std::pair<Python3Parser::ParametersContext*, Python3Parser::SuiteContext*> > functionTable;
        //every floor stores all temporary variables in the corresponding function
        std::vector<flowStmtType> flowStmtStack;
        std::vector<std::string> kwTable;
        int depth;
        std::vector<Base> returnTmp;

        TrueAdministrator():depth(0) {}
        ~TrueAdministrator() {
            for (auto it = globalVariable.begin(); it != globalVariable.end(); ++it) delete it->second, it->second = nullptr;
        }
        void newVariable(const std::string& name, const Base& data) {
            if (depth > 0) temporaryVariable[depth][name] = new Base(data, name);
            else globalVariable[name] = new Base(data, name);
        }
        Base* getVariable(const std::string& name) {
            if (temporaryVariable[depth][name]) return temporaryVariable[depth][name];
            return globalVariable[name];
        }
};

#endif //PYTHON_INTERPRETER_TRUEADMINISTRATOR_HPP
