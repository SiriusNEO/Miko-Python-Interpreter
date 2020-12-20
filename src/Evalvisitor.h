#ifndef PYTHON_INTERPRETER_EVALVISITOR_H
#define PYTHON_INTERPRETER_EVALVISITOR_H


#include "Python3BaseVisitor.h"
#include "Base.h"
#include "TrueAdministrator.hpp"

TrueAdministrator miko;

class EvalVisitor: public Python3BaseVisitor {

//todo:override all methods of Python3BaseVisitor
//Only three possible return types:int(for stmt, 1 quit), Base(for expr), vector(for testlist and para)
public:

    virtual antlrcpp::Any visitFile_input(Python3Parser::File_inputContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitFuncdef(Python3Parser::FuncdefContext *ctx) override {
        miko.functionTable[ctx->NAME()->getText()] = std::make_pair(ctx->parameters(), ctx->suite());
        return 0;
    }

    virtual antlrcpp::Any visitParameters(Python3Parser::ParametersContext *ctx) override {
        std::vector<std::string> ret;
        if (ctx->typedargslist()) return visitTypedargslist(ctx->typedargslist()).as<std::vector<std::string>>();
        return ret;
    }

    virtual antlrcpp::Any visitTypedargslist(Python3Parser::TypedargslistContext *ctx) override {
        auto tfp_list = ctx->tfpdef();
        auto testArray = ctx->test();
        std::vector<std::string> ret;
        for (auto i = 0; i < tfp_list.size(); ++i) {
            if (i < tfp_list.size() - testArray.size()) miko.newVariable(tfp_list[i]->getText(), Base(_none));
            else miko.newVariable(tfp_list[i]->getText(), visitTest(testArray[i - (tfp_list.size() - testArray.size())]));
            ret.push_back(tfp_list[i]->getText());
        }
        return ret;
    }

    virtual antlrcpp::Any visitTfpdef(Python3Parser::TfpdefContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitStmt(Python3Parser::StmtContext *ctx) override {
        if (ctx->simple_stmt()) return visitSimple_stmt(ctx->simple_stmt()).as<int>();
        return visitCompound_stmt(ctx->compound_stmt()).as<int>();
    }

    virtual antlrcpp::Any visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) override {
        return visitSmall_stmt(ctx->small_stmt()).as<int>();
    }

    virtual antlrcpp::Any visitSmall_stmt(Python3Parser::Small_stmtContext *ctx) override {
        if (ctx->expr_stmt()) return visitExpr_stmt(ctx->expr_stmt()).as<int>();
        return visitFlow_stmt(ctx->flow_stmt()).as<int>();
    }

    virtual antlrcpp::Any visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) override {
        if (ctx->augassign()) {
            std::string tmpOp = ctx->augassign()->getText();
            auto testlistArray = ctx->testlist();
            auto left_testArray = visitTestlist(testlistArray[0]).as<std::vector<Base>>(),
            right_testArray = visitTestlist(testlistArray[1]).as<std::vector<Base>>();
            if (left_testArray.size() != 1 || right_testArray.size() != 1) throw std::invalid_argument("Error, illegal expression for augmented assignment");
            if (!left_testArray[0].isLeftValue) throw std::invalid_argument("Error, not a left-value");
            Base* ptr = miko.getVariable(left_testArray[0].nameData);
            if (ptr == nullptr) throw std::invalid_argument("Error, something has not been defined");
            if (tmpOp == "+=") *ptr += right_testArray[0];
            else if (tmpOp == "-=") *ptr -= right_testArray[0];
            else if (tmpOp == "*=") *ptr *= right_testArray[0];
            else if (tmpOp == "/=") *ptr = floatDiv(*ptr, right_testArray[0]);
            else if (tmpOp == "//=") *ptr = intDiv(*ptr, right_testArray[0]);
            else if (tmpOp == "%=") *ptr %= right_testArray[0];
        }
        else {
            auto testlistArray = ctx->testlist();
            if (testlistArray.size() == 1) {
                visitTestlist(testlistArray[0]);
                return 0;
            }
            for (int i = testlistArray.size()-1; i > 0; --i) {
                auto left_testArray = visitTestlist(testlistArray[i-1]).as<std::vector<Base>>(),
                right_testArray = visitTestlist(testlistArray[i]).as<std::vector<Base>>();
                if (left_testArray.size() != right_testArray.size()) throw std::invalid_argument("Error, numbers of objects on two sides are different");
                for (int j = 0; j < left_testArray.size(); ++j) {
                    if (!left_testArray[j].isLeftValue) throw std::invalid_argument("Error, not a left-value");
                    Base* ptr = miko.getVariable(left_testArray[j].nameData);
                    if (ptr == nullptr) miko.newVariable(left_testArray[j].nameData, right_testArray[j]);
                    else *ptr = right_testArray[j];
                }
            }
        }
        return 0;
    }

    virtual antlrcpp::Any visitAugassign(Python3Parser::AugassignContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) override {
        if (ctx->break_stmt()) miko.flowStmtStack.push_back(_break);
        else if (ctx->continue_stmt()) miko.flowStmtStack.push_back(_continue);
        else {
            miko.flowStmtStack.push_back(_return);
            if (ctx->return_stmt()->testlist()) miko.returnTmp = visitTestlist(ctx->return_stmt()->testlist()).as<std::vector<Base>>();
            else miko.returnTmp.push_back(Base(_none));
        }
        return 1;  //quit now
    }
    //not used
    virtual antlrcpp::Any visitBreak_stmt(Python3Parser::Break_stmtContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitContinue_stmt(Python3Parser::Continue_stmtContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitReturn_stmt(Python3Parser::Return_stmtContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitCompound_stmt(Python3Parser::Compound_stmtContext *ctx) override {
        if (ctx->if_stmt()) return visitIf_stmt(ctx->if_stmt()).as<int>();
        else if (ctx->while_stmt()) return visitWhile_stmt(ctx->while_stmt()).as<int>();
        return visitFuncdef(ctx->funcdef()).as<int>();
    }

    virtual antlrcpp::Any visitIf_stmt(Python3Parser::If_stmtContext *ctx) override {
        int elifCnt = ctx->ELIF().size();
        auto testArray = ctx->test();
        auto suiteArray = ctx->suite();
        if (visitTest(testArray[0]).as<Base>().isTrue()) return visitSuite(suiteArray[0]).as<int>();
        for (int i = 1; i <= elifCnt; ++i) if (visitTest(testArray[i]).as<Base>().isTrue()) return visitSuite(suiteArray[i]).as<int>();
        if (ctx->ELSE()) return visitSuite(suiteArray[elifCnt+1]).as<int>();
        return 0;
    }

    virtual antlrcpp::Any visitWhile_stmt(Python3Parser::While_stmtContext *ctx) override {
        while (visitTest(ctx->test()).as<Base>().isTrue()) {
            visitSuite(ctx->suite());
            if (!miko.flowStmtStack.empty()) {
                if (miko.flowStmtStack.back() == _break) {
                    miko.flowStmtStack.pop_back();
                    return 0; //stop the loop but don't convey the quit sign, pop the break
                }
                if (miko.flowStmtStack.back() == _return) return 1;
                //stop the loop and quit the function, don't pop the return (since this will be done later)
            }
        }
        return 0;
    }

    virtual antlrcpp::Any visitSuite(Python3Parser::SuiteContext *ctx) override {
        if (ctx->simple_stmt()) return visitSimple_stmt(ctx->simple_stmt()).as<int>();
        auto stmtArray = ctx->stmt();
        for (auto i : stmtArray) if(visitStmt(i).as<int>()) return 1;
        return 0;
    }

    virtual antlrcpp::Any visitTest(Python3Parser::TestContext *ctx) override {
        return visitChildren(ctx); //directly go to or_test
    }

    virtual antlrcpp::Any visitOr_test(Python3Parser::Or_testContext *ctx) override {
        auto andtestArray = ctx->and_test();
        if (andtestArray.size() == 1) return visitAnd_test(andtestArray[0]).as<Base>();
        for (auto i : andtestArray) if (visitAnd_test(i).as<Base>().isTrue()) return Base(true);
        return Base(false);
    }

    virtual antlrcpp::Any visitAnd_test(Python3Parser::And_testContext *ctx) override {
        auto nottestArray = ctx->not_test();
        if (nottestArray.size() == 1) return visitNot_test(nottestArray[0]).as<Base>();
        for (auto i : nottestArray) if (!visitNot_test(i).as<Base>().isTrue()) return Base(false);
        return Base(true);
    }

    virtual antlrcpp::Any visitNot_test(Python3Parser::Not_testContext *ctx) override {
        if (ctx->comparison()) return visitComparison(ctx->comparison()).as<Base>();
        else return !visitNot_test(ctx->not_test()).as<Base>();
    }

    virtual antlrcpp::Any visitComparison(Python3Parser::ComparisonContext *ctx) override {
        auto arithexprArray = ctx->arith_expr();
        if (arithexprArray.size() == 1) return visitArith_expr(ctx->arith_expr()[0]).as<Base>();
        auto comp_op_list = ctx->comp_op();
        bool ret = true;
        Base obj1, obj2 = visitArith_expr(arithexprArray[0]);
        for (int i = 1; i < arithexprArray.size(); ++i) {
            bool tmp = true;
            std::string tmpOp = comp_op_list[i-1]->getText();
            obj1 = obj2, obj2 = visitArith_expr(arithexprArray[i]);
            if (tmpOp == "<") tmp = (obj1 < obj2);
            else if (tmpOp == "<=") tmp = (obj1 <= obj2);
            else if (tmpOp == ">") tmp = (obj1 > obj2);
            else if (tmpOp == ">=") tmp = (obj1 >= obj2);
            else if (tmpOp == "==") tmp = (obj1 == obj2);
            else if (tmpOp == "!=") tmp = (obj1 != obj2);
            if (!tmp) return Base(false);
        }
        return Base(true);
    }

    virtual antlrcpp::Any visitComp_op(Python3Parser::Comp_opContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitArith_expr(Python3Parser::Arith_exprContext *ctx) override {
        auto termArray = ctx->term();
        if (termArray.size() == 1) return visitTerm(termArray[0]).as<Base>();
        auto opArray = ctx->addorsub_op();
        Base ret = visitTerm(termArray[0]);
        for (int i = 1; i < termArray.size(); ++i) {
            std::string tmpOp = opArray[i-1]->getText();
            if (tmpOp == "+") ret += visitTerm(termArray[i]);
            else if (tmpOp == "-") ret -= visitTerm(termArray[i]);
        }
        return ret;
    }

    virtual antlrcpp::Any visitAddorsub_op(Python3Parser::Addorsub_opContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitTerm(Python3Parser::TermContext *ctx) override {
        auto factorArray = ctx->factor();
        if (factorArray.size() == 1) return visitFactor(factorArray[0]).as<Base>();
        auto opArray = ctx->muldivmod_op();
        Base ret = visitFactor(factorArray[0]);
        for (int i = 1; i < factorArray.size(); ++i) {
            std::string tmpOp = opArray[i-1]->getText();
            if (tmpOp == "*") ret *= visitFactor(factorArray[i]);
            else if (tmpOp == "/") ret = floatDiv(ret, visitFactor(factorArray[i]));
            else if (tmpOp == "//") ret = intDiv(ret, visitFactor(factorArray[i]));
            else if (tmpOp == "%") ret %= visitFactor(factorArray[i]);
        }
        return ret;
    }

    virtual antlrcpp::Any visitMuldivmod_op(Python3Parser::Muldivmod_opContext *ctx) override {
        return visitChildren(ctx);
    }

    virtual antlrcpp::Any visitFactor(Python3Parser::FactorContext *ctx) override {
        if (ctx->factor()) {
            std::string ctxText = ctx->getText();
            return (ctxText[0] == '+') ? visitFactor(ctx->factor()).as<Base>() : -visitFactor(ctx->factor()).as<Base>();
        }
        return visitAtom_expr(ctx->atom_expr()).as<Base>();
    }

    virtual antlrcpp::Any visitAtom_expr(Python3Parser::Atom_exprContext *ctx) override {
        if (!ctx->trailer()) return visitAtom(ctx->atom()).as<Base>();
        auto functionName = visitAtom(ctx->atom()).as<Base>();
        auto func = miko.functionTable[functionName.nameData];
        miko.kwTable.clear();
        auto argumentData = visitTrailer(ctx->trailer()).as<std::vector<Base>>(); //should be a vector<Base>
        if (func.first) {
            ++miko.depth;
            auto argumentName = visitParameters(func.first).as<std::vector<std::string>>();
            if (!argumentName.empty()) {
                if (argumentName.size() != argumentData.size())
                    throw std::invalid_argument("Error, invalid numbers of arguments");
                for (int i = 0; i < argumentData.size(); ++i) {
                    if (i < argumentData.size() - miko.kwTable.size()) *miko.getVariable(argumentName[i]) = argumentData[i];
                    else *miko.getVariable(miko.kwTable[i-(argumentData.size()-miko.kwTable.size())]) = argumentData[i];
                }
            }
            visitSuite(func.second);
            for (auto it = miko.temporaryVariable[miko.depth].begin(); it != miko.temporaryVariable[miko.depth].end(); ++it) delete it->second, it->second = nullptr;
            --miko.depth;
            if (miko.flowStmtStack.empty()) return Base(_none);
            miko.flowStmtStack.pop_back();
            auto ret = miko.returnTmp;
            miko.returnTmp.clear();
            if (ret.size() == 1) return ret[0];
            return Base(ret);
        }
        else if (functionName.nameData == "print") { 
            for (int i = 0; i < argumentData.size(); ++i) {
                std::cout << argumentData[i];
                if (i != argumentData.size() - 1) std::cout << ' ';
            }
            std::cout << std::endl;
            return Base(_none);
        }
        else if (functionName.nameData == "int") {
            if (argumentData.size() > 1) throw std::invalid_argument("Error, function expected at most 1 argument");
            return argumentData[0].castToInt();
        }
        else if (functionName.nameData == "float") {
            if (argumentData.size() > 1) throw std::invalid_argument("Error, function expected at most 1 argument");
            return argumentData[0].castToFloat();
        }
        else if (functionName.nameData == "str") {
            if (argumentData.size() > 1) throw std::invalid_argument("Error, function expected at most 1 argument");
            return argumentData[0].castToStr();
        }
        else if (functionName.nameData == "bool") {
            if (argumentData.size() > 1) throw std::invalid_argument("Error, function expected at most 1 argument");
            return argumentData[0].castToBool();
        }
    }

    virtual antlrcpp::Any visitTrailer(Python3Parser::TrailerContext *ctx) override {
        if (ctx->arglist()) return visitArglist(ctx->arglist()).as<std::vector<Base>>();
        return std::vector<Base>();
    }

    virtual antlrcpp::Any visitAtom(Python3Parser::AtomContext *ctx) override {
        std::string ctxText = ctx->getText();
        if (ctx->NUMBER()) {
            if (ctxText.find('.') == std::string::npos) return Base(Bigint(ctxText));
            return Base(stringToDouble(ctxText));
        }
        else if (ctx->NAME()) {
            Base* tmp = miko.getVariable(ctx->NAME()->getText());
            if (tmp) return *tmp;
            else return Base(Base(_null), ctx->NAME()->getText());
        }
        else if (ctx->FALSE()) return Base(false);
        else if (ctx->TRUE()) return Base(true);
        else if (ctx->test()) return visitTest(ctx->test()).as<Base>();
        auto stringArray = ctx->STRING();
        if (!stringArray.empty()) {
            std::string ret;
            for (auto i : stringArray) ret += i->getText().substr(1, i->getText().size()-2);
            return Base(ret);
        }
        return Base(_none);
    }

    virtual antlrcpp::Any visitTestlist(Python3Parser::TestlistContext *ctx) override {
        auto testArray = ctx->test();
        std::vector<Base> ret;
        for (auto i : testArray) {
            auto nowTest = visitTest(i).as<Base>();
            if (nowTest.type() == _tuple) {
                for (auto j : nowTest.tupleData) ret.push_back(*j);
            }
            else ret.push_back(nowTest);
        }
        return ret;
    }

    virtual antlrcpp::Any visitArglist(Python3Parser::ArglistContext *ctx) override {
        auto argumentArray = ctx->argument();
        std::vector<Base> ret;
        for (auto i : argumentArray) ret.push_back(visitArgument(i));
        return ret;
    }

    virtual antlrcpp::Any visitArgument(Python3Parser::ArgumentContext *ctx) override {
        if (ctx->test().size() == 1) return visitTest(ctx->test()[0]).as<Base>();
        miko.kwTable.push_back(ctx->test()[0]->getText());
        return visitTest(ctx->test()[1]).as<Base>();
    }
};


#endif //PYTHON_INTERPRETER_EVALVISITOR_H
