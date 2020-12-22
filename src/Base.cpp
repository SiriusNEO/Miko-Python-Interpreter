//
// Created by SiriusNEO on 2020/12/5.
//

#include "Base.h"

Base Base::castToBool() const{
    switch (baseType) {
        case _bool:return *this;
        case _int: {
            if (intData != Bigint(0)) return Base(true);
            return Base(false);
        }
        case _float: return Base(bool(floatData));
        case _str: return Base(!strData.empty());
        case _none: return Base(false);
        default: throw std::invalid_argument("Error, can't cast to bool type");
    }
}

Base Base::castToInt() const{
    switch (baseType) {
        case _bool: return Base(Bigint(int(boolData)));
        case _int: return *this;
        case _float: return Base(Bigint(int(floatData)));
        case _str: {
            for (auto ch : strData) if (ch < '0' || ch > '9') throw std::invalid_argument("Error, can't cast str to int");
            return Base(Bigint(strData));
        }
        case _none: return Base(Bigint(0));
        default: throw std::invalid_argument("Error, can't cast to int type");
    }
}

Base Base::castToFloat() const{
    switch (baseType) {
        case _bool: return Base(double(boolData));
        case _int: return Base(intData.castToDouble());
        case _float: return *this;
        case _str: {
            for (auto ch : strData) if ((ch < '0' || ch > '9') && ch != '.') throw std::invalid_argument("Error, can't cast str to float");
            return Base(stringToDouble(strData));
        }
        case _none: return Base(0.0);
        default: throw std::invalid_argument("Error, can't cast to float type");
    }
}

Base Base::castToStr() const{
    switch (baseType) {
        case _bool: {
            if (boolData) return Base("True");
            return Base("False");
        }
        case _int: return Base(intData.castToString());
        case _float: return Base(doubleToString(floatData));
        case _str: return *this;
        case _none: return Base("None");
        default: throw std::invalid_argument("Error, can't cast to str type");
    }
}

Base Base::castToRightValue() const {
    Base ret = *this;
    ret.isLeftValue = false;
    return ret;
}

typeT Base::type() {
    return baseType;
}

bool Base::isTrue() const {
    switch (baseType) {
        case _bool:return boolData;
        case _int:return intData != Bigint(0);
        case _float:return floatData != 0;
        case _str:return !strData.empty();
        case _none: return false;
        default: throw std::invalid_argument("Error, something has not been defined");
    }
}

bool operator == (const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null || obj2.baseType == _null) throw std::invalid_argument("Error, something has not been defined");
    if (obj1.baseType != obj2.baseType) {
        if (obj1.baseType == _str || obj2.baseType == _str || obj1.baseType == _none || obj2.baseType == _none) return false;
        switch (obj1.baseType) {
            case _bool:return obj1 == obj2.castToBool();
            case _int:return obj1 == obj2.castToInt();
            case _float:return obj1 == obj2.castToFloat();
            default: return false;
        }
    }
    switch (obj1.baseType) {
        case _bool:return obj1.boolData == obj2.boolData;
        case _int:return obj1.intData == obj2.intData;
        case _float:return obj1.floatData == obj2.floatData;
        case _str:return obj1.strData == obj2.strData;
        default: return true;
    }
}
bool operator != (const Base& obj1, const Base& obj2) {
    return !(obj1 == obj2);
}
bool operator > (const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null || obj2.baseType == _null) throw std::invalid_argument("Error, something has not been defined");
    if (obj1.baseType != obj2.baseType) {
        if (obj1.baseType == _str || obj2.baseType == _str)
            throw std::invalid_argument("Error, comparison is undefined between non-str type and str type");
        switch (obj1.baseType) {
            case _bool:return obj1 > obj2.castToBool();
            case _int:return obj1 > obj2.castToInt();
            case _float:return obj1 > obj2.castToFloat();
            default: return false;
        }
    }
    switch (obj1.baseType) {
        case _bool:return obj1.boolData > obj2.boolData;
        case _int:return obj1.intData > obj2.intData;
        case _float:return obj1.floatData > obj2.floatData;
        case _str:return obj1.strData > obj2.strData;
        default: return false;
    }
}
bool operator < (const Base& obj1, const Base& obj2) {
    return !(obj1 >= obj2);
}
bool operator >= (const Base& obj1, const Base& obj2) {
    return obj1 > obj2 || obj1 == obj2;
}
bool operator <= (const Base& obj1, const Base& obj2) {
    return !(obj1 > obj2);
}

Base operator + (const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null || obj2.baseType == _null) throw std::invalid_argument("Error, something has not been defined");
    if (obj1.baseType != obj2.baseType) {
        if (obj1.baseType == _str || obj2.baseType == _str)
            throw std::invalid_argument("Error, operator + is undefined between non-str type and str type");
        if (obj1.baseType == _float) return obj1 + obj2.castToFloat();
        if (obj2.baseType == _float) return obj1.castToFloat() + obj2;
        return obj1.castToInt() + obj2.castToInt();
    }
    switch (obj1.baseType) {
        case _bool: return obj1.castToInt() + obj2.castToInt();
        case _int: return Base(obj1.intData + obj2.intData);
        case _float: return Base(obj1.floatData + obj2.floatData);
        case _str: return Base(obj1.strData + obj2.strData);
        default: throw std::invalid_argument("Error, operator + is undefined between these types");
    }
}
Base operator - (const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null || obj2.baseType == _null) throw std::invalid_argument("Error, something has not been defined");
    if (obj1.baseType != obj2.baseType) {
        if (obj1.baseType == _str || obj2.baseType == _str)
            throw std::invalid_argument("Error, operator - is undefined between non-str type and str type");
        if (obj1.baseType == _float) return obj1 - obj2.castToFloat();
        if (obj2.baseType == _float) return obj1.castToFloat() - obj2;
        return obj1.castToInt() - obj2.castToInt();
    }
    switch (obj1.baseType) {
        case _bool: return obj1.castToInt() - obj2.castToInt();
        case _int: return Base(obj1.intData - obj2.intData);
        case _float: return Base(obj1.floatData - obj2.floatData);
        default: throw std::invalid_argument("Error, operator - is undefined between these types");
    }
}
Base operator * (const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null || obj2.baseType == _null) throw std::invalid_argument("Error, something has not been defined");
    if (obj1.baseType != obj2.baseType) {
        if (obj1.baseType == _str || obj2.baseType == _str) {
            if (obj1.baseType == _float || obj2.baseType == _float)
                throw std::invalid_argument("Error, operator * is undefined between float and str");
            else if (obj1.baseType == _str) {
                std::string ret;
                int times = (int)obj2.castToInt().intData.castToLL();
                while (times--) ret += obj1.strData;
                return Base(ret);
            }
            else if (obj2.baseType == _str) return obj2 * obj1;
        }
        if (obj1.baseType == _float) return obj1 * obj2.castToFloat();
        if (obj2.baseType == _float) return obj1.castToFloat() * obj2;
        return obj1.castToInt() * obj2.castToInt();
    }
    switch (obj1.baseType) {
        case _bool: return obj1.castToInt() * obj2.castToInt();
        case _int: return Base(obj1.intData * obj2.intData);
        case _float: return Base(obj1.floatData * obj2.floatData);
        default: throw std::invalid_argument("Error, operator * is undefined between these types");
    }
}
Base intDiv(const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null || obj2.baseType == _null) throw std::invalid_argument("Error, something has not been defined");
    if (obj1.baseType == _float || obj2.baseType == _float || obj1.baseType == _str || obj2.baseType == _str)
        throw std::invalid_argument("Error, operator // is undefined between these types");
    Bigint leftInt = obj1.castToInt().intData, rightInt = obj2.castToInt().intData;
    if (rightInt == Bigint(0)) throw std::invalid_argument("Error, divided by zero");
    return Base(leftInt / rightInt);
}
Base floatDiv(const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null || obj2.baseType == _null) throw std::invalid_argument("Error, something has not been defined");
    if (obj1.baseType == _str || obj2.baseType == _str)
        throw std::invalid_argument("Error, operator / is undefined between these types");
    double leftFloat = obj1.castToFloat().floatData, rightFloat = obj2.castToFloat().floatData;
    if (rightFloat == 0) throw std::invalid_argument("Error, divided by zero");
    return Base(leftFloat / rightFloat);
}
Base operator % (const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null || obj2.baseType == _null) throw std::invalid_argument("Error, something has not been defined");
    if (obj1.baseType == _str || obj2.baseType == _str || obj1.baseType == _float || obj2.baseType == _float)
        throw std::invalid_argument("Error, operator // is undefined between these types");
    return Base(obj1.castToInt().intData % obj2.castToInt().intData);
}

Base operator && (const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null || obj2.baseType == _null) throw std::invalid_argument("Error, something has not been defined");
    return Base(obj1.castToBool().boolData && obj2.castToBool().boolData);
}

Base operator || (const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null || obj2.baseType == _null) throw std::invalid_argument("Error, something has not been defined");
    return Base(obj1.castToBool().boolData || obj2.castToBool().boolData);
}

Base Base:: operator-() const {
    switch (baseType) {
        case _bool: return -(this->castToInt());
        case _int: return Base(-intData);
        case _float: return Base(-floatData);
        default: throw std::invalid_argument("Error, operator - is undefined in this type");
    }
}

Base Base:: operator!() const {
     return Base(!this->isTrue());
}

Base& Base::operator = (const Base &obj) {
    //pass by value
    if (this == &obj) return *this;
    baseType = obj.baseType, boolData = obj.boolData, intData = obj.intData, floatData = obj.floatData, strData = obj.strData;
    return *this;
}

Base Base:: operator += (const Base& obj) {
    *this = *this + obj;
    return *this;
}
Base Base:: operator -= (const Base& obj) {
    *this = *this - obj;
    return *this;
}
Base Base:: operator *= (const Base& obj) {
    *this = *this * obj;
    return *this;
}
Base Base:: operator %= (const Base& obj) {
    *this = *this % obj;
    return *this;
}

std::ostream& operator << (std::ostream& os, Base& obj) {
    switch (obj.baseType) {
        case _bool: return os << (obj.boolData ? "True" : "False");
        case _int: return os << obj.intData;
        case _float: return os << std::fixed << std::setprecision(6) << obj.floatData;
        case _str: return os << obj.strData;
        case _none: return os << "None";
        case _tuple: {
            for (int i = 0; i < obj.tupleData.size(); ++i) {
                os << *obj.tupleData[i];
                if (i != obj.tupleData.size()-1) os << ' ';
            }
            return os;
        }
        default: throw std::invalid_argument("Error, something has not been defined");
    }
}