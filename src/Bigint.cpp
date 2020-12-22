//
// Created by SiriusNEO on 2020/12/1.
//

#include "Bigint.h"

bool operator == (const Bigint& obj1, const Bigint& obj2) {
    if (obj1.size() != obj2.size() || obj1.neg != obj2.neg) return false;
    for (int i = 0; i < obj1.size(); ++i) if (obj1.digit[i] != obj2.digit[i]) return false;
    return true;
}

bool operator != (const Bigint& obj1, const Bigint& obj2) {
    return !(obj1 == obj2);
}

bool operator > (const Bigint& obj1, const Bigint& obj2) {
    if (obj1.neg ^ obj2.neg) return obj2.neg;
    if (obj1.size() != obj2.size()) return (obj1.neg ^ (obj1.size() > obj2.size()));
    for (int i = obj1.size()-1; i >= 0; --i)
        if (obj1.digit[i] != obj2.digit[i]) return (obj1.neg ^ (obj1.digit[i] > obj2.digit[i]));
    return false;
}

bool operator < (const Bigint& obj1, const Bigint& obj2) {
    return !(obj1 >= obj2);
}

bool operator >= (const Bigint& obj1, const Bigint& obj2) {
    return obj1 > obj2 || obj1 == obj2;
}

bool operator <= (const Bigint& obj1, const Bigint& obj2) {
    return !(obj1 > obj2);
}

Bigint operator + (const Bigint& obj1, const Bigint& obj2) {
    int addDigit = 0;
    if (obj1.neg ^ obj2.neg) {
        if (obj1.neg) return obj2 - (-obj1);
       return obj1 - (-obj2);
    }
    else {
        Bigint ret, right;
        if (obj1.size() > obj2.size()) ret = obj1, right = obj2;
        else ret = obj2, right = obj1;
        for (int i = 0; i < right.size(); ++i) {
            ret.digit[i] += right.digit[i] + addDigit;
            (ret.digit[i] >= 10) ? ret.digit[i] -= 10, addDigit = 1 : addDigit = 0;
        }
        for (int i = right.size(); i < ret.size() && addDigit; ++i) {
            ret.digit[i] += addDigit;
            (ret.digit[i] >= 10) ? ret.digit[i] -= 10, addDigit = 1 : addDigit = 0;
        }
        if (addDigit) ret.digit.push_back(1);
        ret.regularize();
        return ret;
    }
}

Bigint operator - (const Bigint& obj1, const Bigint& obj2) {
    if (obj1.neg ^ obj2.neg) {
        return obj1 + (-obj2);
    }
    else {
        int subDigit = false;
        bool negFlag = false;
        Bigint ret, right;
        if (obj1.neg) ret = -obj1, right = -obj2, negFlag ^= true;
        else ret = obj1, right = obj2;
        if (ret < right) std::swap(ret, right), negFlag ^= true;
        for (int i = 0; i < right.size(); ++i) {
            ret.digit[i] -= right.digit[i] + subDigit;
            (ret.digit[i] < 0) ? ret.digit[i] += 10, subDigit = 1 : subDigit = 0;
        }
        for (int i = right.size(); i < ret.size(); ++i) {
            ret.digit[i] -= subDigit;
            (ret.digit[i] < 0) ? ret.digit[i] += 10, subDigit = 1 : subDigit = 0;
        }
        ret.neg ^= negFlag;
        ret.regularize();
        return ret;
    }
}

Bigint operator * (const Bigint& obj1, const Bigint& obj2) {
    Bigint ret;
    ret.digit.resize(obj1.size() + obj2.size() + 11);
    for (int i = 0; i < obj1.size(); ++i)
        for (int j = 0; j < obj2.size(); ++j)
                ret.digit[i+j] += obj1.digit[i] * obj2.digit[j];
    for (int i = 0; i < ret.size()-1; ++i) {
        ret.digit[i+1] += ret.digit[i]/10;
        ret.digit[i] %= 10;
    }
    ret.neg = obj1.neg ^ obj2.neg;
    ret.regularize();
    return ret;
}

Bigint operator / (const Bigint& obj1, const Bigint& obj2) {
    Bigint obj3 = obj1, tmp;
    obj3.neg = false;
    std::string retStr;
    int moveDigit = std::max(obj3.size() - obj2.size() + 1, 1);
    tmp.digit.pop_back();
    for (int i = 1; i < moveDigit; ++i) tmp.digit.push_back(0);
    for (int i : obj2.digit) tmp.digit.push_back(i);
    tmp.neg = false;
    while (moveDigit) {
        char ch = '0';
        while (obj3 >= tmp) obj3 -= tmp, ++ch;
        obj3 *= 10;
        retStr += ch;
        --moveDigit;
    }
    Bigint ret(retStr);
    ret.neg = obj1.neg ^ obj2.neg;
    if (ret.neg && obj3 != Bigint(0)) ret -= Bigint(1);
    ret.regularize();
    return ret;
}

Bigint operator % (const Bigint& obj1, const Bigint& obj2) {
    return obj1 - (obj2 * (obj1 / obj2));
}

Bigint Bigint::operator-() const {
    Bigint ret = *this;
    ret.neg ^= true;
    return ret;
}

Bigint Bigint::operator += (const Bigint &obj) {
    *this = *this + obj;
    return *this;
}

Bigint Bigint::operator -= (const Bigint &obj) {
    *this = *this - obj;
    return *this;
}

Bigint Bigint::operator *= (const Bigint& obj) {
    *this = *this * obj;
    return *this;
}

Bigint Bigint::operator /= (const Bigint& obj) {
    *this = *this / obj;
    return *this;
}

Bigint Bigint::operator %= (const Bigint& obj) {
    *this = *this % obj;
    return *this;
}

std::ostream& operator << (std::ostream& os, Bigint obj) {
    if (obj.neg) os << '-';
    for (auto i = obj.size()-1; i >= 0; --i) os << obj.digit[i];
    return os;
}