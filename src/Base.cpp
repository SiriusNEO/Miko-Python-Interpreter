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
        case _list: return Base(size()==0);
        default: invalidReport(1, nameData); //must be undefined.
    }
}

Base Base::castToInt() const{
    switch (baseType) {
        case _bool: return Base(Bigint(int(boolData)));
        case _int: return *this;
        case _float: return Base(Bigint(int(floatData)));
        case _str: {
            for (auto ch : strData) if (ch < '0' || ch > '9')
                invalidReport(2, "int", typeToString(_str));
            return Base(Bigint(strData));
        }
        case _none: return Base(Bigint(0));
        case _list: invalidReport(2, "int", typeToString(_list, isImmutable));
        default: invalidReport(1, nameData);
    }
}

Base Base::castToFloat() const{
    switch (baseType) {
        case _bool: return Base(double(boolData));
        case _int: return Base(intData.castToDouble());
        case _float: return *this;
        case _str: {
            for (auto ch : strData) if ((ch < '0' || ch > '9') && ch != '.')
                invalidReport(2, "float", typeToString(_str));
            return Base(stringToDouble(strData));
        }
        case _none: return Base(0.0);
        case _list: invalidReport(2, "float", typeToString(_list, isImmutable));
        default: invalidReport(1, nameData);
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
        case _list: invalidReport(0);
        default: invalidReport(1, nameData);
    }
}

Base Base::castToList(bool _isImmutable) const {
    switch (baseType) {
        case _bool:case _int:case _float:case _none:
            invalidReport(2, "list", typeToString(baseType));
        case _list: {
            std::vector<Base> ret; //passby value
            for (auto i : listData) ret.push_back(*i);
            return Base(ret, _isImmutable);
        }
        case _str: {
            std::vector<Base> ret;
            for (auto i : strData) {
                std::string tmp;
                tmp += i;
                ret.push_back(Base(tmp));
            }
            return Base(ret, _isImmutable);
        }
        default: invalidReport(1, nameData);
    }
}

Base Base::castToRightValue() const {
    Base ret = *this;
    ret.isLeftValue = false;
    return ret;
}

Base Base::castToMaxType(typeT maxType) const {
    //not include str
    if (maxType == _float) return this->castToFloat();
    return this->castToInt();
}

bool Base::isTrue() const {
    switch (baseType) {
        case _bool:return boolData;
        case _int:return intData != Bigint(0);
        case _float:return floatData != 0;
        case _str:return !strData.empty();
        case _none: return false;
        case _list: return size()==0;
        default: invalidReport(1, nameData);
    }
}

bool operator == (const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null) invalidReport(1, obj1.nameData);
    if (obj2.baseType == _null) invalidReport(1, obj2.nameData);
    if (obj1.baseType != obj2.baseType) {
        if (obj1.baseType == _str || obj2.baseType == _str || obj1.baseType == _none || obj2.baseType == _none || obj1.baseType == _list || obj2.baseType == _list)
            return false;
        typeT maxType = std::max(obj1.type(), obj2.type());
        return obj1.castToMaxType(maxType) == obj2.castToMaxType(maxType);
    }
    switch (obj1.baseType) {
        case _bool:return obj1.boolData == obj2.boolData;
        case _int:return obj1.intData == obj2.intData;
        case _float:return obj1.floatData == obj2.floatData;
        case _str:return obj1.strData == obj2.strData;
        case _list:{
            if (obj1.size() != obj2.size()) return false;
            for (int i = 0; i < obj1.size(); ++i)
                if (*obj1.listData[i+obj1.startIndex] != *obj2.listData[i+obj2.startIndex]) return false;
            return true;
        }
        default: return true;
    }
}
bool operator != (const Base& obj1, const Base& obj2) {
    return !(obj1 == obj2);
}
bool operator > (const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null) invalidReport(1, obj1.nameData);
    if (obj2.baseType == _null) invalidReport(1, obj2.nameData);
    if (obj1.baseType != obj2.baseType) {
        if (obj1.baseType == _str || obj2.baseType == _str || obj1.baseType == _list || obj2.baseType == _list)
            invalidReport(5, "comparison", typeToString(obj1.type(), obj1.isImmutable), typeToString(obj2.type(), obj2.isImmutable));
        typeT maxType = std::max(obj1.type(), obj2.type());
        return obj1.castToMaxType(maxType) > obj2.castToMaxType(maxType);
    }
    switch (obj1.baseType) {
        case _bool:return obj1.boolData > obj2.boolData;
        case _int:return obj1.intData > obj2.intData;
        case _float:return obj1.floatData > obj2.floatData;
        case _str:return obj1.strData > obj2.strData;
        case _list:{
            for (int i = 0; i < std::min(obj1.size(), obj2.size()); ++i)
                if (*obj1.listData[i+obj1.startIndex] > *obj2.listData[i+obj2.startIndex]) return true;
            return false;
        }
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
    if (obj1.baseType == _null) invalidReport(1, obj1.nameData);
    if (obj2.baseType == _null) invalidReport(1, obj2.nameData);
    if (obj1.baseType == _list) {
        if (obj2.baseType == _list) {
            if (obj1.isImmutable && !obj2.isImmutable) invalidReport(5, "\'+\'", typeToString(obj1.type(), obj1.isImmutable), typeToString(obj2.type(), obj2.isImmutable));
            std::vector<Base> ret;
            for (auto i : obj1.listData) ret.push_back(*i);
            for (auto i : obj2.listData) ret.push_back(*i);
            return Base(ret, obj1.isImmutable);
        }
        else if (obj2.baseType == _str) {
            return obj1 + obj2.castToList();
        }
        else invalidReport(5, "\'+\'", typeToString(obj1.type(), obj1.isImmutable), typeToString(obj2.type(), obj2.isImmutable));
    }
    if (obj1.baseType != obj2.baseType) {
        if (obj2.type() == _list) invalidReport(5, "\'+\'", typeToString(obj1.type(), obj1.isImmutable), typeToString(obj2.type(), obj2.isImmutable));
        if (obj1.baseType == _str || obj2.baseType == _str || obj2.baseType == _list)
            invalidReport(5, "\'+\'", typeToString(obj1.type(), obj1.isImmutable), typeToString(obj2.type(), obj2.isImmutable));
        typeT maxType = std::max(obj1.type(), obj2.type());
        return obj1.castToMaxType(maxType) + obj2.castToMaxType(maxType);
    }
    switch (obj1.baseType) {
        case _bool: return obj1.castToInt() + obj2.castToInt();
        case _int: return Base(obj1.intData + obj2.intData);
        case _float: return Base(obj1.floatData + obj2.floatData);
        case _str: return Base(obj1.strData + obj2.strData);
        default: invalidReport(5, "\'+\'", typeToString(obj1.type(), obj1.isImmutable), typeToString(obj2.type(), obj2.isImmutable));
    }
}
Base operator - (const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null) invalidReport(1, obj1.nameData);
    if (obj2.baseType == _null) invalidReport(1, obj2.nameData);
    if (obj1.baseType == _list || obj2.baseType == _list)
        invalidReport(5, "\'-\'", typeToString(obj1.type(), obj1.isImmutable), typeToString(obj2.type(), obj2.isImmutable));
    if (obj1.baseType != obj2.baseType) {
        if (obj1.baseType == _str || obj2.baseType == _str)
            invalidReport(5, "\'-\'", typeToString(obj1.type(), obj1.isImmutable), typeToString(obj2.type(), obj2.isImmutable));
        typeT maxType = std::max(obj1.type(), obj2.type());
        return obj1.castToMaxType(maxType) - obj2.castToMaxType(maxType);
    }
    switch (obj1.baseType) {
        case _bool: return obj1.castToInt() - obj2.castToInt();
        case _int: return Base(obj1.intData - obj2.intData);
        case _float: return Base(obj1.floatData - obj2.floatData);
        default: invalidReport(5, "\'-\'", typeToString(obj1.type(), obj1.isImmutable), typeToString(obj2.type(), obj2.isImmutable));
    }
}
Base operator * (const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null) invalidReport(1, obj1.nameData);
    if (obj2.baseType == _null) invalidReport(1, obj2.nameData);
    if (obj1.baseType == _list || obj2.baseType == _list) {
        if (obj1.baseType == _list && obj2.baseType == _int) {
            int times = (int)obj2.getNumber();
            std::vector<Base> ret;
            if (times > 0) {
                while (times--) {
                    for (auto i : obj1.listData) ret.push_back(*i);
                }
            }
            return Base(ret, obj1.isImmutable);
        }
        invalidReport(5, "\'*\'", typeToString(obj1.type(), obj1.isImmutable), typeToString(obj2.type(), obj2.isImmutable));
    }
    if (obj1.baseType != obj2.baseType) {
        if (obj1.baseType == _str || obj2.baseType == _str) {
            if (obj1.baseType == _float || obj2.baseType == _float)
                invalidReport(5, "\'*\'", typeToString(obj1.type(), obj1.isImmutable), typeToString(obj2.type(), obj2.isImmutable));
            else if (obj1.baseType == _str) {
                std::string ret;
                int times = (int)obj2.castToInt().intData.castToLL();
                while (times--) ret += obj1.strData;
                return Base(ret);
            }
            else if (obj2.baseType == _str) return obj2 * obj1;
        }
        typeT maxType = std::max(obj1.type(), obj2.type());
        return obj1.castToMaxType(maxType) * obj2.castToMaxType(maxType);
    }
    switch (obj1.baseType) {
        case _bool: return obj1.castToInt() * obj2.castToInt();
        case _int: return Base(obj1.intData * obj2.intData);
        case _float: return Base(obj1.floatData * obj2.floatData);
        default: invalidReport(5, "\'*\'", typeToString(obj1.type(), obj1.isImmutable), typeToString(obj2.type(), obj2.isImmutable));
    }
}
Base intDiv(const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null) invalidReport(1, obj1.nameData);
    if (obj2.baseType == _null) invalidReport(1, obj2.nameData);
    if (obj1.baseType == _float || obj2.baseType == _float || obj1.baseType == _str || obj2.baseType == _str || obj1.baseType == _list || obj2.baseType == _list)
        invalidReport(5, "\'//\'", typeToString(obj1.type(), obj1.isImmutable), typeToString(obj2.type(), obj2.isImmutable));
    Bigint leftInt = obj1.castToInt().intData, rightInt = obj2.castToInt().intData;
    if (rightInt == Bigint(0)) invalidReport(7);
    return Base(leftInt / rightInt);
}
Base floatDiv(const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null) invalidReport(1, obj1.nameData);
    if (obj2.baseType == _null) invalidReport(1, obj2.nameData);
    if (obj1.baseType == _str || obj2.baseType == _str || obj1.baseType == _list || obj2.baseType == _list)
        invalidReport(5, "\'/\'");
    double leftFloat = obj1.castToFloat().floatData, rightFloat = obj2.castToFloat().floatData;
    if (rightFloat == 0) invalidReport(7);
    return Base(leftFloat / rightFloat);
}
Base operator % (const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null) invalidReport(1, obj1.nameData);
    if (obj2.baseType == _null) invalidReport(1, obj2.nameData);
    if (obj1.baseType == _str || obj2.baseType == _str || obj1.baseType == _float || obj2.baseType == _float || obj1.baseType == _list || obj2.baseType == _list)
        invalidReport(5, "\'%\'", typeToString(obj1.type(), obj1.isImmutable), typeToString(obj2.type(), obj2.isImmutable));
    return Base(obj1.castToInt().intData % obj2.castToInt().intData);
}

Base operator && (const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null) invalidReport(1, obj1.nameData);
    if (obj2.baseType == _null) invalidReport(1, obj2.nameData);
    return Base(obj1.castToBool().boolData && obj2.castToBool().boolData);
}

Base operator || (const Base& obj1, const Base& obj2) {
    if (obj1.baseType == _null) invalidReport(1, obj1.nameData);
    if (obj2.baseType == _null) invalidReport(1, obj2.nameData);
    return Base(obj1.castToBool().boolData || obj2.castToBool().boolData);
}

Base Base:: operator-() const {
    switch (baseType) {
        case _bool: return -(this->castToInt());
        case _int: return Base(-intData);
        case _float: return Base(-floatData);
        default: invalidReport(15, "\'-\'", typeToString(baseType, isImmutable));
    }
}

Base Base:: operator!() const {
     return Base(!this->isTrue());
}

Base& Base::operator = (const Base &obj) {
    //pass by value, not modify nameData
    if (this == &obj) return *this;
    baseType = obj.baseType;
    boolData = obj.boolData, intData = obj.intData, floatData = obj.floatData, strData = obj.strData;
    startIndex = obj.startIndex, endIndex = obj.endIndex, step = obj.step;
    if (baseType == _list) {
        listData.clear();
        for (auto i : obj.listData) listData.push_back(new Base(*i));
        isImmutable = obj.isImmutable;
    }
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
        case _str: {
            if (!obj.isImmutable) return os << obj.strData;
            if (obj.startIndex < obj.endIndex) {
                for (int i = obj.startIndex; i < obj.endIndex; ++i) os << obj.strData[i];
            }
            return os;
        }
        case _none: return os << "None";
        case _list: {
            if (obj.isImmutable) {
                os << '(';
                for (int i = 0; i < obj.fullSize(); ++i) {
                    os << *obj.listData[i];
                    if (i != obj.endIndex - 1) os << ", ";
                    else if (obj.size() == 1) os << ',';
                }
                os << ')';
                return os;
            }
            os << '[';
            for (int i = 0; i < obj.fullSize(); ++i) {
                os << *obj.listData[i];
                if (i != obj.endIndex - 1) os << ", ";
            }
            os << ']';
            return os;
        }
        default: invalidReport(1, obj.nameData);
    }
}