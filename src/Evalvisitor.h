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
        if (ctx->parameters()->typedargslist()) {
            auto testArray = ctx->parameters()->typedargslist()->test();
            for (auto i : testArray)
                miko.defaultArguments[ctx->parameters()].push_back(visitTest(i));
            }
        return 0;
    }

    virtual antlrcpp::Any visitParameters(Python3Parser::ParametersContext *ctx) override {
        std::vector<std::string> ret;
        if (ctx->typedargslist()) {
            auto tfpArray = ctx->typedargslist()->tfpdef();
            auto defaultArray = miko.defaultArguments[ctx];
            for (auto i = 0; i < tfpArray.size(); ++i) {
                if (i < tfpArray.size() - defaultArray.size()) miko.newVariable(tfpArray[i]->getText(), Base(_none));
                else miko.newVariable(tfpArray[i]->getText(), defaultArray[i - (tfpArray.size() - defaultArray.size())]);
                ret.push_back(tfpArray[i]->getText());
            }
        }
        return ret;
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
            miko.passByReference = true;
            miko.trace.clear();
            auto left_testArray = visitTestlist(testlistArray[0]).as<Base>().getList();
            miko.passByReference = false;
            auto right_testArray = visitTestlist(testlistArray[1]).as<Base>().getList();
            if (left_testArray.size() != 1 || right_testArray.size() != 1) invalidReport(8);
            if (!left_testArray[0].isLeftValue) invalidReport(9, testlistArray[0]->getText());
            Base* ptr = miko.getVariable(left_testArray[0].nameData);
            if (ptr == nullptr) invalidReport(1, left_testArray[0].nameData);
            if (right_testArray[0].type() == _null) invalidReport(1, right_testArray[0].nameData);
            for (auto i : miko.trace) ptr = ptr->listData[i];
            if (ptr->type() == _list) {
                if (left_testArray[0].step && ptr->isImmutable) invalidReport(16, typeToString(_list, 1));
                if (left_testArray[0].startIndex == left_testArray[0].endIndex) {
                    if (tmpOp == "+=") *(left_testArray[0].listData[left_testArray[0].startIndex]) += right_testArray[0];
                    else if (tmpOp == "-=") *(left_testArray[0].listData[left_testArray[0].startIndex]) -= right_testArray[0];
                    else if (tmpOp == "*=") *(left_testArray[0].listData[left_testArray[0].startIndex]) *= right_testArray[0];
                    else if (tmpOp == "/=") *(left_testArray[0].listData[left_testArray[0].startIndex]) = floatDiv(*ptr, right_testArray[0]);
                    else if (tmpOp == "//=") *(left_testArray[0].listData[left_testArray[0].startIndex]) = intDiv(*ptr, right_testArray[0]);
                    else if (tmpOp == "%=") *(left_testArray[0].listData[left_testArray[0].startIndex]) %= right_testArray[0];
                }
                else if (!left_testArray[0].step) {
                    if (tmpOp == "+=") *ptr += right_testArray[0];
                    else if (tmpOp == "-=") *ptr -= right_testArray[0];
                    else if (tmpOp == "*=") *ptr *= right_testArray[0];
                    else if (tmpOp == "/=") *ptr = floatDiv(*ptr, right_testArray[0]);
                    else if (tmpOp == "//=") *ptr = intDiv(*ptr, right_testArray[0]);
                    else if (tmpOp == "%=") *ptr %= right_testArray[0];
                }
                else {
                    if (tmpOp == "+=") {
                        if (left_testArray[0].startIndex > left_testArray[0].endIndex)
                            left_testArray[0].endIndex = left_testArray[0].startIndex;
                        if (right_testArray[0].type() != _list) invalidReport(13);
                        std::vector<Base*> tmp;
                        for (int k = left_testArray[0].fullSize()-1; k >= left_testArray[0].endIndex; --k)
                            tmp.push_back(left_testArray[0].listData[k]), ptr->listData.pop_back();
                        for (int k = right_testArray[0].startIndex; k < right_testArray[0].endIndex; ++k)
                            ptr->listData.push_back(right_testArray[0].listData[k]);
                        for (int k = tmp.size()-1; k >= 0; --k) ptr->listData.push_back(tmp[k]);
                        ptr->endIndex = ptr->fullSize();
                        ptr->isImmutable = right_testArray[0].isImmutable;
                    }
                    else if (tmpOp == "-=")
                        invalidReport(5, "\'-\'", typeToString(_list, 0), typeToString(_list, right_testArray[0].isImmutable));
                    else if (tmpOp == "*=") {
                        if (left_testArray[0].startIndex > left_testArray[0].endIndex)
                            left_testArray[0].endIndex = left_testArray[0].startIndex;
                        if (right_testArray[0].type() != _int)
                            invalidReport(5, "\'*\'", typeToString(_list, 0), typeToString(right_testArray[0].type()));
                        std::vector<Base*> tmp;
                        std::vector<Base> mulTmp;
                        for (int k = left_testArray[0].fullSize()-1; k >= left_testArray[0].endIndex; --k)
                            tmp.push_back(left_testArray[0].listData[k]), ptr->listData.pop_back();
                        for (int k = left_testArray[0].endIndex - 1; k >= left_testArray[0].startIndex; --k)
                            mulTmp.push_back(*ptr->listData[k]), ptr->listData.pop_back();
                        int times = (int)right_testArray[0].getNumber();
                        if (times > 0) {
                            while (times--) {
                                for (int k = mulTmp.size()-1; k >= 0; --k) ptr->listData.push_back(new Base(mulTmp[k]));
                            }
                        }
                        for (int k = tmp.size()-1; k >= 0; --k) ptr->listData.push_back(tmp[k]);
                        ptr->endIndex = ptr->fullSize();
                    }
                    else if (tmpOp == "/=")
                        invalidReport(5, "\'/\'", typeToString(_list, 0), typeToString(_list, right_testArray[0].isImmutable));
                    else if (tmpOp == "//=")
                        invalidReport(5, "\'//\'", typeToString(_list, 0), typeToString(_list, right_testArray[0].isImmutable));
                    else if (tmpOp == "%=")
                        invalidReport(5, "\'%\'", typeToString(_list, 0), typeToString(_list, right_testArray[0].isImmutable));
                }
            }
            else {
                if (ptr->type() == _str && ptr->isImmutable) invalidReport(16, typeToString(_str));
                if (tmpOp == "+=") *ptr += right_testArray[0];
                else if (tmpOp == "-=") *ptr -= right_testArray[0];
                else if (tmpOp == "*=") *ptr *= right_testArray[0];
                else if (tmpOp == "/=") *ptr = floatDiv(*ptr, right_testArray[0]);
                else if (tmpOp == "//=") *ptr = intDiv(*ptr, right_testArray[0]);
                else if (tmpOp == "%=") *ptr %= right_testArray[0];
            }
        }
        else {
            auto testlistArray = ctx->testlist();
            if (testlistArray.size() == 1) {
                visitTestlist(testlistArray[0]);
                return 0;
            }
            for (int i = testlistArray.size()-1; i > 0; --i) {
                miko.passByReference = true;
                miko.trace.clear();
                auto left_testArray = visitTestlist(testlistArray[i-1]).as<Base>().getList();
                miko.passByReference = false;
                auto right_testArray = visitTestlist(testlistArray[i]).as<Base>().getList();
                if (left_testArray.size() != right_testArray.size()) invalidReport(11);
                for (int j = 0; j < left_testArray.size(); ++j) {
                    if (!left_testArray[j].isLeftValue) invalidReport(9, testlistArray[i-1]->getText());
                    Base* ptr = miko.getVariable(left_testArray[j].nameData);
                    if (right_testArray[j].type() == _null) invalidReport(1, right_testArray[j].nameData);
                    if (ptr == nullptr) miko.newVariable(left_testArray[j].nameData, right_testArray[j]);
                    else {
                        for (auto i : miko.trace) ptr = ptr->listData[i];
                        if (ptr->type() == _list) {
                            if (!left_testArray[j].step) ptr->passPtr(right_testArray[j]);
                            else if (ptr->isImmutable) invalidReport(16, typeToString(_list, 1));
                            else if (left_testArray[j].startIndex == left_testArray[j].endIndex)
                                *(left_testArray[j].listData[left_testArray[j].startIndex]) = right_testArray[j]; //single
                            else {
                                if (left_testArray[j].startIndex > left_testArray[j].endIndex)
                                    left_testArray[j].endIndex = left_testArray[j].startIndex;
                                if (right_testArray[j].type() != _list) invalidReport(13);
                                std::vector<Base*> tmp;
                                for (int k = left_testArray[j].fullSize()-1; k >= left_testArray[j].endIndex; --k)
                                    tmp.push_back(left_testArray[j].listData[k]), ptr->listData.pop_back();
                                for (int k = left_testArray[j].startIndex; k < left_testArray[j].endIndex; ++k)
                                    ptr->listData.pop_back();
                                for (int k = right_testArray[j].startIndex; k < right_testArray[j].endIndex; ++k)
                                    ptr->listData.push_back(right_testArray[j].listData[k]);
                                for (int k = tmp.size()-1; k >= 0; --k) ptr->listData.push_back(tmp[k]);
                                ptr->endIndex = ptr->fullSize();
                                ptr->isImmutable = right_testArray[j].isImmutable;
                            }
                        }
                        else {
                            if (ptr->type() == _str && left_testArray[j].step) invalidReport(16, typeToString(_str));
                            *ptr = right_testArray[j];
                        }
                    }
                }
            }
        }
        return 0;
    }

    virtual antlrcpp::Any visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) override {
        if (ctx->break_stmt()) miko.flowStmtStack.push_back(_break);
        else if (ctx->continue_stmt()) miko.flowStmtStack.push_back(_continue);
        else {
            miko.flowStmtStack.push_back(_return);
            if (ctx->return_stmt()->testlist()) miko.returnTmp = visitTestlist(ctx->return_stmt()->testlist()).as<Base>().getList();
            else miko.returnTmp.push_back(Base(_none));
        }
        return 1;  //quit now
    }

    virtual antlrcpp::Any visitCompound_stmt(Python3Parser::Compound_stmtContext *ctx) override {
        if (ctx->if_stmt()) return visitIf_stmt(ctx->if_stmt()).as<int>();
        else if (ctx->while_stmt()) return visitWhile_stmt(ctx->while_stmt()).as<int>();
        else if (ctx->for_stmt()) return visitFor_stmt(ctx->for_stmt()).as<int>();
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

    virtual antlrcpp::Any visitFor_stmt(Python3Parser::For_stmtContext *ctx) override {
        Base seq = visitTest(ctx->test()).as<Base>();
        if (seq.type() != _list && seq.type() != _str) invalidReport(14, typeToString(seq.type()));
        if (seq.type() == _str) seq = seq.castToList(true);
        if (seq.startIndex > seq.endIndex) return 0;
        miko.newVariable(ctx->NAME()->getText(), Base()); //global variable
        for (int i = seq.startIndex; i < seq.endIndex; ++i) {
            Base* ptr = miko.getVariable(ctx->NAME()->getText());
            *ptr = *seq.listData[i];
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

    virtual antlrcpp::Any visitArith_expr(Python3Parser::Arith_exprContext *ctx) override {
        auto termArray = ctx->term();
        if (termArray.size() == 1) return visitTerm(termArray[0]).as<Base>();
        auto opArray = ctx->addorsub_op();
        Base ret = visitTerm(termArray[0]).as<Base>();
        ret.isLeftValue = false;
        for (int i = 1; i < termArray.size(); ++i) {
            std::string tmpOp = opArray[i-1]->getText();
            if (tmpOp == "+") ret += visitTerm(termArray[i]);
            else if (tmpOp == "-") ret -= visitTerm(termArray[i]);
        }
        return ret;
    }

    virtual antlrcpp::Any visitTerm(Python3Parser::TermContext *ctx) override {
        auto factorArray = ctx->factor();
        if (factorArray.size() == 1) return visitFactor(factorArray[0]).as<Base>();
        auto opArray = ctx->muldivmod_op();
        Base ret = visitFactor(factorArray[0]).as<Base>();
        ret.isLeftValue = false;
        for (int i = 1; i < factorArray.size(); ++i) {
            std::string tmpOp = opArray[i-1]->getText();
            if (tmpOp == "*") ret *= visitFactor(factorArray[i]);
            else if (tmpOp == "/") ret = floatDiv(ret, visitFactor(factorArray[i]));
            else if (tmpOp == "//") ret = intDiv(ret, visitFactor(factorArray[i]));
            else if (tmpOp == "%") ret %= visitFactor(factorArray[i]);
        }
        return ret;
    }

    virtual antlrcpp::Any visitFactor(Python3Parser::FactorContext *ctx) override {
        if (ctx->factor()) {
            std::string ctxText = ctx->getText();
            return (ctxText[0] == '+') ? visitFactor(ctx->factor()).as<Base>() : -visitFactor(ctx->factor()).as<Base>();
        }
        return visitAtom_expr(ctx->atom_expr()).as<Base>();
    }

    virtual antlrcpp::Any visitAtom_expr(Python3Parser::Atom_exprContext *ctx) override {
        auto indexArray = ctx->index();
        if (!indexArray.empty()) {
            Base ret = visitAtom(ctx->atom()).as<Base>();
            if (ret.type() == _list) {
                if (miko.passByReference) {
                    for (int i = 0; i < indexArray.size(); ++i) {
                        int ll = ret.startIndex, rr = ret.endIndex;
                        if (ll == rr)  {
                            ret.passPtr(*ret.listData[ll]); //single
                            if (miko.passByReference) miko.trace.push_back(ll);
                        }
                        if (indexArray[i]->slice()) {
                            if (indexArray[i]->slice()->step())
                                if (visitTest(indexArray[i]->slice()->step()->test()).as<Base>().getNumber() != 1)
                                    invalidReport(0);
                            if (indexArray[i]->slice()->startindex())
                                ll = visitTest(indexArray[i]->slice()->startindex()->test()).as<Base>().getNumber() + ret.startIndex;
                            if ( indexArray[i]->slice()->endindex())
                                rr = visitTest(indexArray[i]->slice()->endindex()->test()).as<Base>().getNumber() + ret.startIndex;
                            if (ll < ret.startIndex) ll = ret.endIndex + ll - ret.startIndex;
                            if (ll < ret.startIndex) ll = ret.startIndex;
                            if (ll > ret.endIndex) ll = ret.endIndex;
                            if (rr < ret.startIndex) rr = ret.endIndex + rr - ret.startIndex;
                            if (rr > ret.endIndex) rr = ret.endIndex;
                            if (ll == rr) rr = ll - 1; //empty slice
                        }
                        else {
                            int index = visitTest(indexArray[i]->test()).as<Base>().getNumber() + ret.startIndex;
                            if (index < ret.startIndex) index = ret.endIndex + index - ret.startIndex;
                            if (index < ret.startIndex || index >= ret.endIndex) invalidReport(12, ctx->getText());
                            ll = rr = index;
                            //ll == rr means this is a element
                            //ll == rr - 1 means this a list that contains one element.
                        }
                        ret.setSlice(ll, rr, 1);
                    }
                    return ret;
                }
                else {
                    for (int i = 0; i < indexArray.size(); ++i) {
                        int ll = 0, rr = ret.size(), ss = 1;
                        if (indexArray[i]->slice()) {
                            if (indexArray[i]->slice()->step())
                                ss = visitTest(indexArray[i]->slice()->step()->test()).as<Base>().getNumber();
                            if (ss < 0) ll = ret.size()-1, rr = -999999999;
                            if (indexArray[i]->slice()->startindex())
                                ll = visitTest(indexArray[i]->slice()->startindex()->test()).as<Base>().getNumber();
                            if ( indexArray[i]->slice()->endindex())
                                rr = visitTest(indexArray[i]->slice()->endindex()->test()).as<Base>().getNumber();
                            if (ss > 0) {
                                if (ll < 0) ll = ret.size() + ll;
                                if (ll < 0) ll = 0;
                                if (ll > ret.size()) ll = ret.size();
                                if (rr < 0) rr = ret.size() + rr;
                                if (rr > ret.size()) rr = ret.size();
                                if (ll == rr) rr = ll - 1; //empty slice
                            }
                            else {
                                if (rr < 0) rr = ret.size() + rr;
                                if (rr < 0) rr = -1;
                                if (rr > ret.size()) rr = ret.size();
                                if (ll < 0) ll = ret.size() + ll;
                                if (ll >= ret.size()) ll = ret.size()-1;
                                if (ll == rr) ll = rr - 1; //empty slice
                            }
                        }
                        else {
                            int index = visitTest(indexArray[i]->test()).as<Base>().getNumber();
                            if (index < 0) index = ret.size() + index;
                            if (index < 0 || index >= ret.size()) invalidReport(12, ctx->getText());
                            ll = rr = index;
                            //ll == rr means this is a element
                            //ll == rr - 1 means this a list that contains one element.
                        }
                        ret.setSlice(ll, rr, ss);
                        ret = ret.getValueList();
                    }
                    return ret;
                }
            }
            else if (ret.type() == _str) {
                if (miko.passByReference) {
                    for (int i = 0; i < indexArray.size(); ++i) {
                        int ll = ret.startIndex, rr = ret.endIndex;
                        if (indexArray[i]->slice()) {
                            if (indexArray[i]->slice()->step())
                                if (visitTest(indexArray[i]->slice()->step()->test()).as<Base>().getNumber() != 1)
                                    invalidReport(0);
                            if (indexArray[i]->slice()->startindex())
                                ll = visitTest(indexArray[i]->slice()->startindex()->test()).as<Base>().getNumber() + ret.startIndex;
                            if ( indexArray[i]->slice()->endindex())
                                rr = visitTest(indexArray[i]->slice()->endindex()->test()).as<Base>().getNumber() + ret.startIndex;
                                rr = ret.endIndex;
                            if (ll < ret.startIndex) ll = ret.endIndex + ll - ret.startIndex;
                            if (ll < ret.startIndex) ll = ret.startIndex;
                            if (ll > ret.endIndex) ll = ret.endIndex;
                            if (rr < ret.startIndex) rr = ret.endIndex + rr - ret.startIndex;
                            if (rr > ret.endIndex) rr = ret.endIndex;
                            if (ll == rr) rr = ll - 1; //empty slice
                        }
                        else {
                            int index = visitTest(indexArray[i]->test()).as<Base>().getNumber() + ret.startIndex;
                            if (index < ret.startIndex) index = ret.endIndex + index - ret.startIndex;
                            if (index < ret.startIndex || index >= ret.endIndex) invalidReport(12, ctx->getText());
                            ll = index, rr = index + 1; //also a slice
                        }
                        ret.setSlice(ll, rr, 1);
                    }
                    return ret;
                }
                else {
                    for (int i = 0; i < indexArray.size(); ++i) {
                        int ll = 0, rr = ret.size(), ss = 1;
                        if (indexArray[i]->slice()) {
                            if (indexArray[i]->slice()->step())
                                ss = visitTest(indexArray[i]->slice()->step()->test()).as<Base>().getNumber();
                            if (ss < 0) if (ss < 0) ll = ret.size()-1, rr = -999999999;
                            if (indexArray[i]->slice()->startindex())
                                ll = visitTest(indexArray[i]->slice()->startindex()->test()).as<Base>().getNumber();
                            if ( indexArray[i]->slice()->endindex())
                                rr = visitTest(indexArray[i]->slice()->endindex()->test()).as<Base>().getNumber();
                            if (ss > 0) {
                                if (ll < 0) ll = ret.size() + ll;
                                if (ll < 0) ll = 0;
                                if (ll > ret.size()) ll = ret.size();
                                if (rr < 0) rr = ret.size() + rr;
                                if (rr > ret.size()) rr = ret.size();
                                if (ll == rr) rr = ll - 1; //empty slice
                            }
                            else {
                                if (rr < 0) rr = ret.size() + rr;
                                if (rr < 0) rr = -1;
                                if (rr > ret.size()) rr = ret.size();
                                if (ll < 0) ll = ret.size() + ll;
                                if (ll >= ret.size()) ll = ret.size()-1;
                                if (ll == rr) ll = rr - 1; //empty slice
                            }
                        }
                        else {
                            int index = visitTest(indexArray[i]->test()).as<Base>().getNumber();
                            if (index < 0) index = ret.size() + index;
                            if (index < 0 || index >= ret.size()) invalidReport(12, ctx->getText());
                            ll = rr = index;
                            //ll == rr means this is a element
                            //ll == rr - 1 means this a list that contains one element.
                        }
                        ret.setSlice(ll, rr, ss);
                        ret = ret.getValueList();
                    }
                    return ret;
                }
            }
        }
        if (!ctx->trailer()) return visitAtom(ctx->atom()).as<Base>();
        auto functionName = visitAtom(ctx->atom()).as<Base>();
        auto func = miko.functionTable[functionName.nameData];
        miko.kwTable.clear();
        auto argumentData = visitTrailer(ctx->trailer()).as<std::vector<Base>>(); //should be a vector<Base>
        if (func.first) {
            ++miko.depth;
            auto argumentName = visitParameters(func.first).as<std::vector<std::string>>();
            if (!argumentName.empty()) {
                if (argumentData.size() < argumentName.size() - miko.defaultArguments[func.first].size())
                    invalidReport(3, functionName.nameData);
                if (argumentData.size() > argumentName.size())
                    invalidReport(4, functionName.nameData);
                for (int i = 0; i < argumentData.size(); ++i) {
                    if (i < argumentData.size() - miko.kwTable.size()) *miko.getVariable(argumentName[i]) = argumentData[i];
                    else {
                        if (!miko.getVariable(miko.kwTable[i-(argumentData.size()-miko.kwTable.size())]))
                            invalidReport(17, miko.kwTable[i-(argumentData.size()-miko.kwTable.size())]);
                        *miko.getVariable(miko.kwTable[i-(argumentData.size()-miko.kwTable.size())]) = argumentData[i];
                    }
                }
            }
            visitSuite(func.second);
            for (auto it = miko.temporaryVariable[miko.depth].begin(); it != miko.temporaryVariable[miko.depth].end(); ++it)
                if (it->second) delete it->second, it->second = nullptr;
            --miko.depth;
            if (miko.flowStmtStack.empty()) return Base(_none);
            miko.flowStmtStack.pop_back();
            auto ret = miko.returnTmp;
            miko.returnTmp.clear();
            if (ret.size() == 1) return ret[0];
            return Base(ret, true);
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
            if (argumentData.size() > 1) invalidReport(4, functionName.nameData);
            return argumentData[0].castToInt();
        }
        else if (functionName.nameData == "float") {
            if (argumentData.size() > 1) invalidReport(4, functionName.nameData);
            return argumentData[0].castToFloat();
        }
        else if (functionName.nameData == "str") {
            if (argumentData.size() > 1) invalidReport(4, functionName.nameData);
            return argumentData[0].castToStr();
        }
        else if (functionName.nameData == "bool") {
            if (argumentData.size() > 1) invalidReport(4, functionName.nameData);
            return argumentData[0].castToBool();
        }
        else if (functionName.nameData == "tuple") {
            if (argumentData.size() > 1) invalidReport(4, functionName.nameData);
            return argumentData[0].castToList(1);
        }
        else if (functionName.nameData == "list") {
            if (argumentData.size() > 1) invalidReport(4, functionName.nameData);
            return argumentData[0].castToList(0);
        }
        else if (functionName.nameData == "type") {
            if (argumentData.size() > 1) invalidReport(4, functionName.nameData);
            return Base("<class '"+typeToString(argumentData[0].type(), argumentData[0].isImmutable)+"'>");
        }
        else if (functionName.nameData == "len") {
            if (argumentData.size() > 1) invalidReport(4, functionName.nameData);
            return Base(Bigint(argumentData[0].size()));
        }
        else if (functionName.nameData == "range") {
            int _start = 0, _stop, _step = 1;
            switch (argumentData.size()) {
                case 0:invalidReport(3, functionName.nameData);
                case 1:{
                    if (argumentData[0].type() != _int)
                        invalidReport(2, functionName.nameData, typeToString(argumentData[0].type(), argumentData[0].isImmutable));
                    _stop = argumentData[0].getNumber();
                    break;
                }
                case 2:{
                    if (argumentData[0].type() != _int)
                        invalidReport(2, functionName.nameData, typeToString(argumentData[0].type(), argumentData[0].isImmutable));
                    if (argumentData[1].type() != _int)
                        invalidReport(2, functionName.nameData, typeToString(argumentData[1].type(), argumentData[1].isImmutable));
                    _start = argumentData[0].getNumber();
                    _stop = argumentData[1].getNumber();
                    break;
                }
                case 3:{
                    if (argumentData[0].type() != _int)
                        invalidReport(2, functionName.nameData, typeToString(argumentData[0].type(), argumentData[0].isImmutable));
                    if (argumentData[1].type() != _int)
                        invalidReport(2, functionName.nameData, typeToString(argumentData[1].type(), argumentData[1].isImmutable));
                    if (argumentData[2].type() != _int)
                        invalidReport(2, functionName.nameData, typeToString(argumentData[2].type(), argumentData[2].isImmutable));
                    _start = argumentData[0].getNumber();
                    _stop = argumentData[1].getNumber();
                    _step = argumentData[2].getNumber();
                    break;
                }
                default:invalidReport(4, functionName.nameData);
            }
            std::vector<Base> ret;
            if (_start < _stop) {
                for (int i = _start; i < _stop; i += _step) ret.push_back(Base(Bigint(i)));
            }
            return Base(ret, true);
        }
        else invalidReport(10, functionName.nameData);
    }

    virtual antlrcpp::Any visitTrailer(Python3Parser::TrailerContext *ctx) override {
        if (ctx->arglist()) return visitArglist(ctx->arglist()).as<std::vector<Base>>();
        return std::vector<Base>();
    }

    void enterLCexpr(int depth) {
        if (depth == miko.LCStack[miko.LCdepth].size()) {
            miko.LCreturnTmp[miko.LCdepth].push_back(visitTest(miko.LCoutExpr[miko.LCdepth]).as<Base>());
            return;
        }
        auto seq = visitTest(miko.LCStack[miko.LCdepth][depth]->for_expr()->test()).as<Base>();
        if (seq.type() != _list && seq.type() != _str) invalidReport(14, typeToString(seq.type()));
        if (seq.type() == _str) seq = seq.castToList(1);
        if (seq.startIndex > seq.endIndex) return ;
        //miko.newVariable(ctx->NAME()->getText(), Base()); //global variable
        miko.LCVariable[miko.LCStack[miko.LCdepth][depth]->for_expr()->NAME()->getText()] = new Base(); //lc variable
        for (int i = seq.startIndex; i < seq.endIndex; ++i) {
                miko.LCVariable[miko.LCStack[miko.LCdepth][depth]->for_expr()->NAME()->getText()] = seq.listData[i];
            if (miko.LCStack[miko.LCdepth][depth]->if_expr() && !visitTest(miko.LCStack[miko.LCdepth][depth]->if_expr()->test()).as<Base>().isTrue())
                continue;
            enterLCexpr(depth+1);
        }
    }

    virtual antlrcpp::Any visitAtom(Python3Parser::AtomContext *ctx) override {
        std::string ctxText = ctx->getText();
        if (ctx->tuple()) {
            std::vector<Base> ret;
            if (ctx->tuple()->test()) ret.push_back(visitTest(ctx->tuple()->test()));
            if (ctx->tuple()->testlist()) {
                Base tmp = visitTestlist(ctx->tuple()->testlist()).as<Base>();
                for (auto i : tmp.listData) ret.push_back(*i);
            }
            return Base(ret, true);
        }
        else if (ctx->list()) {
            std::vector<Base> ret;
            if (ctx->list()->testlist()) {
                Base tmp = visitTestlist(ctx->list()->testlist()).as<Base>();
                for (auto i : tmp.listData) ret.push_back(*i);
            }
            return Base(ret, false);
        }
        else if (ctx->comprehension()) {
            miko.LCdepth++;
            miko.LCreturnTmp[miko.LCdepth].clear();
            miko.LCStack[miko.LCdepth] = ctx->comprehension()->lc_expr();
            miko.LCoutExpr[miko.LCdepth] = ctx->comprehension()->test();
            enterLCexpr(0);
            for (auto i : miko.LCStack[miko.LCdepth]) {
                delete miko.LCVariable[i->for_expr()->NAME()->getText()], miko.LCVariable[i->for_expr()->NAME()->getText()] = nullptr;
            }
            return Base(miko.LCreturnTmp[miko.LCdepth--], false);
        }
        else if (ctx->NUMBER()) {
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
        for (auto i : testArray)
            ret.push_back(visitTest(i).as<Base>());
        return Base(ret, 1);
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