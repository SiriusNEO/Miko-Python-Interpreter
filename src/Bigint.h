//
// Created by SiriusNEO on 2020/12/1.
//

#ifndef PYTHON_INTERPRETER_BIGINT_H
#define PYTHON_INTERPRETER_BIGINT_H

#include <vector>
#include <cstring>
#include <iostream>
#include <algorithm>

class Bigint {
    private:
        std::vector<int> digit;
        bool neg;

    public:
        Bigint():neg(false) {digit.push_back(0);}
        Bigint(int obj) {
            if (obj < 0) neg = true, obj = -obj;
            else neg = false;
            if (obj == 0) digit.push_back(0);
            else while (obj > 0) digit.push_back(obj % 10), obj /= 10;
        }
        Bigint(const std::string& numStr) {
            neg = false;
            std::string tmpStr = numStr;
            std::reverse(tmpStr.begin(), tmpStr.end());
            for (auto i : tmpStr) {
                if (i == '-') neg = true;
                else digit.push_back(int(i - '0'));
            }
        }

        inline void regularize() {
            while (!digit.back() && digit.size() > 1) digit.pop_back();
            if (digit.size() == 1 && digit[0] == 0 && neg) neg = false;
        }
        inline long long castToLL() const {
            long long ret = 0;
            for (int i = digit.size()-1; i >=0; --i) ret = ret * 10 + digit[i];
            return neg ? -ret : ret;
        }
        inline double castToDouble() const {
            double ret = 0;
            for (int i = digit.size()-1; i >=0; --i) ret = ret * 10.0 + digit[i];
            return neg ? -ret : ret;
        }
        inline std::string castToString() const {
            std::string ret;
            if (neg) ret += '-';
            for (int i = digit.size()-1; i >= 0; --i) ret += char(digit[i]+'0');
            return ret;
        }
        inline int size() const {return digit.size();}

        friend bool operator == (const Bigint& obj1, const Bigint& obj2);
        friend bool operator != (const Bigint& obj1, const Bigint& obj2);
        friend bool operator > (const Bigint& obj1, const Bigint& obj2);
        friend bool operator < (const Bigint& obj1, const Bigint& obj2);
        friend bool operator >= (const Bigint& obj1, const Bigint& obj2);
        friend bool operator <= (const Bigint& obj1, const Bigint& obj2);

        friend Bigint operator + (const Bigint& obj1, const Bigint& obj2);
        friend Bigint operator - (const Bigint& obj1, const Bigint& obj2);
        friend Bigint operator * (const Bigint& obj1, const Bigint& obj2);
        friend Bigint operator / (const Bigint& obj1, const Bigint& obj2);
        friend Bigint operator % (const Bigint& obj1, const Bigint& obj2);

        Bigint operator - () const;
        Bigint operator += (const Bigint& obj);
        Bigint operator -= (const Bigint& obj);
        Bigint operator *= (const Bigint& obj);
        Bigint operator /= (const Bigint& obj);
        Bigint operator %= (const Bigint& obj);

        friend std::ostream& operator << (std::ostream& os, Bigint obj);
};


#endif //PYTHON_INTERPRETER_BIGINT_H
