//
// Created by SiriusNEO on 2020/12/5.
// Basic datatype in python
//

#ifndef PYTHON_INTERPRETER_BASE_H
#define PYTHON_INTERPRETER_BASE_H

#include "Bigint.h"
#include <iomanip>
#include <memory>

const double eps = (1e-10);
enum typeT {_null, _bool, _int, _float, _str, _tuple, _none};

class Base {
    private:
        typeT baseType;
        bool boolData;
        Bigint intData;
        double floatData;
        std::string strData;

    public:
        bool isLeftValue;
        std::string nameData;
        std::vector<Base*> tupleData;  //copy prohibited

        explicit Base():baseType(_null),boolData(),intData(),floatData(),strData(),isLeftValue(),nameData(),tupleData() {}
        explicit Base(typeT typ):baseType(typ),boolData(),intData(),floatData(),strData(),isLeftValue(),nameData(),tupleData() {}
        explicit Base(bool obj):baseType(_bool),boolData(obj),intData(),floatData(),strData(),isLeftValue(),nameData(),tupleData() {}
        explicit Base(const char *obj):baseType(_str),strData(obj),boolData(),intData(),floatData(),isLeftValue(),nameData(),tupleData() {}
        explicit Base(const std::string& obj):baseType(_str),strData(obj),boolData(),intData(),floatData(),isLeftValue(),nameData(),tupleData() {}
        explicit Base(const Bigint& obj):baseType(_int),intData(obj),boolData(),floatData(),strData(),isLeftValue(),nameData(),tupleData() {}
        explicit Base(double obj):baseType(_float),floatData(obj),boolData(),intData(),strData(),isLeftValue(),nameData(),tupleData() {}
        explicit Base(std::vector<Base> obj):baseType(_tuple),boolData(),intData(),floatData(),strData(),isLeftValue(),nameData(){
            for (const auto& i : obj) tupleData.push_back(new Base(i));
        }
        Base(const Base& data, const std::string& name):Base(data) {isLeftValue = true;nameData = name;}

        Base castToBool() const;
        Base castToInt() const;
        Base castToFloat() const;
        Base castToStr() const;
        Base castToRightValue() const;

        typeT type();

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
        Base operator += (const Base& obj);
        Base operator -= (const Base& obj);
        Base operator *= (const Base& obj);
        Base operator %= (const Base& obj);

        friend std::ostream& operator << (std::ostream& os, Base& obj);
};

inline static std::string doubleToString(double val) {
    int moveDigit = 0;
    double tmp = val;
    std::string str;
    if (val == 0) return "0.0";
    if (tmp < 0) tmp = -tmp;
    while (tmp - int(tmp) >= eps) tmp *= 10, ++moveDigit;
    while (moveDigit--) str += char(int(tmp)%10+'0'), tmp /= 10;
    str += '.';
    while (int(tmp) > 0) str += char(int(tmp)%10+'0'), tmp /= 10;
    std::reverse(str.begin(), str.end());
    if (val - int(val) < eps && val - (int(val)) > -eps) str += '0';
    return (val > 0) ? str : "-" + str;
}
inline static double stringToDouble(std::string str) {
    double ret = 0, floatBase = 0.1;
    int i = 0;
    for (; i < str.size() && str[i] != '.'; ++i) ret = ret * 10 + str[i] - '0';
    for (++i; i < str.size(); ++i) ret += (str[i] - '0')*floatBase, floatBase *= 0.1;
    return ret;
}

#endif //PYTHON_INTERPRETER_BASE_H
