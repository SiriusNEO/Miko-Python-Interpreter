//
// Created by SiriusNEO on 2020/12/5.
// Basic datatype in python
//

#ifndef PYTHON_INTERPRETER_BASE_H
#define PYTHON_INTERPRETER_BASE_H

#include "Bigint.h"
#include <iomanip>
#include <memory>
#include <sstream>

const double eps = (1e-10);
enum typeT {_null, _bool, _int, _float, _str, _list, _none};

class Base {
    private:
        typeT baseType;
        bool boolData;
        Bigint intData;
        double floatData;
        std::string strData;

    public:
        bool isLeftValue;
        bool isImmutable;
        int startIndex = 0, endIndex = 0, step = 0;
        std::string nameData;
        std::vector<Base*> listData;

        explicit Base():baseType(_null),boolData(),intData(),floatData(),strData(),isLeftValue(),nameData(),listData() {}
        explicit Base(typeT typ):baseType(typ),boolData(),intData(),floatData(),strData(),isLeftValue(),nameData(),listData() {}
        explicit Base(bool obj):baseType(_bool),boolData(obj),intData(),floatData(),strData(),isLeftValue(),nameData(),listData() {}
        explicit Base(const char *obj):baseType(_str),strData(obj),boolData(),intData(),floatData(),isLeftValue(),nameData(),listData() {}
        explicit Base(const std::string& obj):baseType(_str),strData(obj),boolData(),intData(),floatData(),isLeftValue(),nameData(),listData() {
            startIndex = 0, endIndex = obj.length(), step = 0, isImmutable = true;
        }
        explicit Base(const Bigint& obj):baseType(_int),intData(obj),boolData(),floatData(),strData(),isLeftValue(),nameData(),listData() {
            intData.regularize();
        }
        explicit Base(double obj):baseType(_float),floatData(obj),boolData(),intData(),strData(),isLeftValue(),nameData(),listData() {}
        explicit Base(std::vector<Base> obj, bool _isImmutable):baseType(_list),boolData(),intData(),floatData(),strData(),isLeftValue(),nameData(),isImmutable(_isImmutable){
            startIndex = 0, endIndex = obj.size(), step = 0;
            if (!obj.empty()) {
                for (const auto& i : obj) listData.push_back(new Base(i));
            }
        }
        Base(const Base& data, const std::string& name):Base(data) {isLeftValue = true;nameData = name;}

        Base castToBool() const;
        Base castToInt() const;
        Base castToFloat() const;
        Base castToStr() const;
        Base castToList(bool _isImmutable = 0) const;
        Base castToRightValue() const;
        Base castToMaxType(typeT maxType) const;

        long long getNumber() const {
            return intData.castToLL();
        }

        void setSlice(int _startIndex, int _endIndex, int _step) {
            startIndex = _startIndex, endIndex = _endIndex, step = _step;
        }
        Base getValueList() const {
            if (baseType == _list) {
                std::vector<Base> ret;
                if (startIndex == endIndex) return *listData[startIndex];
                if (step > 0) {
                    if (startIndex < endIndex)
                        for (int i = startIndex; i < endIndex; i += step) ret.push_back(*listData[i]);
                }
                else {
                    if (endIndex < startIndex)
                        for (int i = startIndex; i > endIndex; i += step) ret.push_back(*listData[i]);
                }
                return Base(ret, isImmutable);
            }
            else if(baseType == _str){
                std::string ret;
                if (startIndex == endIndex) return Base(strData.substr(startIndex, 1));
                if (step > 0) {
                    if (startIndex < endIndex)
                        for (int i = startIndex; i < endIndex; i += step) ret += strData[i];
                }
                else {
                    if (endIndex < startIndex)
                        for (int i = startIndex; i > endIndex; i += step) ret += strData[i];
                }
                return Base(ret);
            }
            return *this;
        }

        typeT type() const {return baseType;}
        int fullSize() const { //full size
            if (baseType == _str) return strData.length();
            else return listData.size();
        }
        int size() const {
            return endIndex - startIndex;
        }

        std::vector<Base> getList() const {
            std::vector<Base> ret;
            for (auto i : listData) ret.push_back(*i);
            return ret;
        };

        bool isTrue() const;

        friend bool operator == (const Base& obj1, const Base& obj2);
        friend bool operator != (const Base& obj1, const Base& obj2);
        friend bool operator > (const Base& obj1, const Base& obj2);
        friend bool operator < (const Base& obj1, const Base& obj2);
        friend bool operator >= (const Base& obj1, const Base& obj2);
        friend bool operator <= (const Base& obj1, const Base& obj2);

        friend Base operator + (const Base& obj1, const Base& obj2);
        friend Base operator - (const Base& obj1, const Base& obj2);
        friend Base operator * (const Base& obj1, const Base& obj2);
        friend Base floatDiv(const Base& obj1, const Base& obj2);
        friend Base intDiv(const Base& obj1, const Base& obj2);
        friend Base operator % (const Base& obj1, const Base& obj2);
        friend Base operator && (const Base& obj1, const Base& obj2);
        friend Base operator || (const Base& obj1, const Base& obj2);

        Base operator - () const;
        Base operator ! () const;
        Base& operator = (const Base& obj);
        void passPtr(const Base& obj) {
            if (obj.baseType == _list) {
                listData.clear();
                for (auto i : obj.listData) listData.push_back(i);
                baseType = obj.baseType;
                isImmutable = obj.isImmutable;
                startIndex = obj.startIndex,endIndex = obj.endIndex,step = obj.step;
            }
        }
        Base operator += (const Base& obj);
        Base operator -= (const Base& obj);
        Base operator *= (const Base& obj);
        Base operator %= (const Base& obj);

        friend std::ostream& operator << (std::ostream& os, Base& obj);
};

static std::string doubleToString(double val) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(6) << val;
    return ss.str();
}
static double stringToDouble(std::string str) {
    double ret = 0, floatBase = 0.1;
    int i = 0;
    for (; i < str.size() && str[i] != '.'; ++i) ret = ret * 10 + str[i] - '0';
    for (++i; i < str.size(); ++i) ret += (str[i] - '0')*floatBase, floatBase *= 0.1;
    return ret;
}

static std::string typeToString(typeT _type, bool isImmutable = 0) {
    switch (_type) {
        case _bool: return "bool";
        case _int: return "int";
        case _float: return "float";
        case _none: return "none";
        case _str: return "str";
        case _list: {
            return (isImmutable) ? "tuple" : "list";
        }
    }
}

#define rpn "\033[34m" << reportName << "\033[0m"
#define rpn1 "\033[34m" << reportName1 << "\033[0m"
#define rpn2 "\033[34m" << reportName2 << "\033[0m"
#define paren "\033[34m" << "()" << "\033[0m"

static void invalidReport(int type, std::string reportName = "", std::string reportName1 = "", std::string reportName2 = "") {
    switch (type) {
        case 0:std::cout << "\033[31m[Sorry]: \033[0m" << "can't support this now" << '\n';break;
        case 1:std::cout << "\033[31m[UndefinedError]: \033[0m" << "undefined variable name: " << rpn <<  '\n';break;
        case 2:std::cout << "\033[31m[ArgumentError]: \033[0m" << "an invalid argument type for function: " << rpn << paren << " type：" << rpn1 << '\n';break;
        case 3:std::cout << "\033[31m[ArgumentError]: \033[0m" << "too few arguments for function: " << rpn << paren << '\n';break;
        case 4:std::cout << "\033[31m[ArgumentError]: \033[0m" << "too many arguments for function: " << rpn << paren << '\n';break;
        case 5:std::cout << "\033[31m[OperatorError]: \033[0m" << rpn << " not supported between these types：" << rpn1 << ", " << rpn2 << '\n';break;
        case 6:std::cout << "\033[31m[SyntaxError]: \033[0m" << "invalid syntax" << '\n';break;
        case 7:std::cout << "\033[31m[ZeroDivisionError] \033[0m" << "division by zero" << '\n';break;
        case 8:std::cout << "\033[31m[SyntaxError]: \033[0m" << "illegal expression for augmented assignment" << '\n';break;
        case 9:std::cout << "\033[31m[AssignmentError]: \033[0m" << "not a left-value: " << rpn << '\n';break;
        case 10:std::cout << "\033[31m[UndefinedError]: \033[0m" << "undefined function name: " << rpn << paren << '\n';break;
        case 11:std::cout << "\033[31m[AssignmentError]: \033[0m" << "numbers of objects on two sides are different" << '\n';break;
        case 12:std::cout << "\033[31m[IndexError]: \033[0m" << "index out of range：" << rpn << '\n';break;
        case 13:std::cout << "\033[31m[AssignmentError]: \033[0m" << "can't assign other types to list/tuple" << '\n';break;
        case 14:std::cout << "\033[31m[TypeError]: \033[0m" << "type: " <<  rpn << " is not subscriptable" << '\n';break;
        case 15:std::cout << "\033[31m[OperatorError]: \033[0m" << rpn << " not supported in type：" << rpn1 << '\n';break;
        case 16:std::cout << "\033[31m[AssignmentError]: \033[0m" << "type：" << rpn << " is immutable" << '\n';break;
        case 17:std::cout << "\033[31m[ArgumentError]: \033[0m" << "an unexpected keyword argument name: " << rpn << '\n';break;
    }
    throw std::invalid_argument("Compile Error");
}

#endif //PYTHON_INTERPRETER_BASE_H
