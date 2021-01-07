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
        std::unordered_map <Python3Parser::ParametersContext*, std::vector<Base> > defaultArguments;
        //every floor stores all temporary variables in the corresponding function

        //for function
        std::vector<flowStmtType> flowStmtStack;
        std::vector<std::string> kwTable;
        int depth;
        std::vector<Base> returnTmp;
        bool passByReference;

        //for lc
        int LCdepth;
        Python3Parser::TestContext* LCoutExpr[maxDepth];
        std::unordered_map <std::string, Base*> LCVariable;
        std::vector<Python3Parser::Lc_exprContext*> LCStack[maxDepth];
        std::vector<Base> LCreturnTmp[maxDepth];

        //for multi loop & slice
        std::vector<int> trace;

        TrueAdministrator():depth(0),LCdepth(0),passByReference(false) {}
        ~TrueAdministrator() {
            for (auto it = globalVariable.begin(); it != globalVariable.end(); ++it)
                if (it->second) delete it->second, it->second = nullptr;
        }
        void newVariable(const std::string& name, const Base& data) {
            if (depth > 0) temporaryVariable[depth][name] = new Base(data, name);
            else globalVariable[name] = new Base(data, name);
        }
        Base* getVariable(const std::string& name) {
            if (LCdepth > 0 && LCVariable[name]) return LCVariable[name];
            if (temporaryVariable[depth][name]) return temporaryVariable[depth][name];
            return globalVariable[name];
        }
};

#endif //PYTHON_INTERPRETER_TRUEADMINISTRATOR_HPP
