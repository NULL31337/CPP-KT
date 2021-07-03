#include "big_integer.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <vector>

big_integer::big_integer() : number(1, 0), sign(false) {}

big_integer::big_integer(big_integer const& other) = default;

big_integer::big_integer(uint32_t a, bool sign_) : number(1, a), sign(sign_) {}

big_integer::big_integer(uint32_t a, uint32_t b, bool sign_) : number(1, a), sign(sign_)
{
    number.push_back(b);
    fit();
}

big_integer::big_integer(unsigned long long a)
    : big_integer(static_cast<uint32_t>(a), static_cast<uint32_t>(a >> 32), false)
{
}
big_integer::big_integer(long long a) : big_integer(static_cast<uint32_t>(a), static_cast<uint32_t>(a >> 32), a < 0) {}

void big_integer::negate()
{
    *this = -(*this);
}

big_integer::big_integer(uint32_t a) : big_integer(static_cast<unsigned long long>(a)) {}
big_integer::big_integer(int a) : big_integer(static_cast<long long>(a)) {}
big_integer::big_integer(long a) : big_integer(static_cast<long long>(a)) {}
big_integer::big_integer(unsigned long a) : big_integer(static_cast<unsigned long long>(a)) {}

big_integer::~big_integer() = default;

static uint32_t parse_uint32(std::string const& str)
{
    uint32_t res = 0;
    for (size_t i = 0; i < str.size(); i++)
    {
        if ('0' <= str[i] && str[i] <= '9')
        {
            res *= 10;
            res += (str[i] - '0');
        }
        else
        {
            throw std::invalid_argument("Expected number");
        }
    }
    return res;
}

big_integer big_integer::abs() const
{
    return sign ? -(*this) : *this;
}

void big_integer::negate_no_copy()
{
    bool carry = true;
    for (size_t i = 0; i < number.size(); i++)
    {
        number[i] ^= UINT32_MAX;
        if (carry)
        {
            if (number[i] != UINT32_MAX)
                carry = false;
            number[i]++;
        }
    }
    sign ^= true;
    fit();
}

big_integer::big_integer(std::string const& str) : big_integer()
{
    for (size_t i = (str[0] == '-' || str[0] == '+'); i < str.size(); i += 9)
    {
        big_integer tmp;
        int mod = 1;
        if (i + 9 < str.size())
        {
            mod = 1000000000;
        }
        else
        {
            for (int j = i; j < str.size(); j++)
            {
                mod *= 10;
            }
        }
        *this = mul_long_short(mod);
        *this += parse_uint32(str.substr(i, 9));
    }
    if (str[0] == '-')
    {
        if (str.size() == 1)
        {
            throw std::invalid_argument("-");
        }
        negate();
    }
    if (str.size() == 0)
    {
        throw std::invalid_argument("");
    }
    fit();
}

big_integer& big_integer::operator=(big_integer const& other) = default;

uint32_t big_integer::add32c(uint32_t& first, uint32_t const& second, uint32_t const& carry)
{
    first += second + carry;
    return ((first < second + carry || (second == UINT32_MAX && carry)) ? 1 : 0);
}

big_integer big_integer::reserve(big_integer const& other, size_t size)
{
    if (other.number.size() < size)
    {
        big_integer tmp(other);
        while (size != tmp.number.size())
        {
            tmp.number.push_back((tmp.sign) ? UINT32_MAX : 0);
        }
        return tmp;
    }
    return other;
}

void big_integer::fit()
{
    if (number.size() == 1 && number[0] == 0)
    {
        sign = false;
        return;
    }
    while (number.size() > 1 && ((number.back() == UINT32_MAX && sign) || (!sign && number.back() == 0)))
    {
        number.pop_back();
    }
    if (sign && number.back() == 0)
    {
        number.push_back(UINT32_MAX);
    }
}

big_integer& big_integer::operator+=(big_integer const& rhs)
{
    *this = reserve(*this, std::max(number.size(), rhs.number.size()) + 1);
    big_integer right(reserve(rhs, std::max(number.size(), rhs.number.size() + 1)));
    uint32_t carry = 0;
    for (size_t i = 0; i < number.size(); i++)
    {
        carry = add32c(number[i], right.number[i], carry);
    }
    sign = number.back() >> 31;
    fit();
    return *this;
}

big_integer& big_integer::operator-=(big_integer const& rhs)
{
    return *this += -rhs;
}

big_integer big_integer::mul_long_short(uint32_t second) const
{
    uint32_t carry = 0;
    uint64_t tmp;
    big_integer ans;
    ans.number.pop_back();
    for (size_t i = 0; i < number.size(); i++)
    {
        tmp = static_cast<uint64_t>(number[i]) * static_cast<uint64_t>(second) + carry;
        ans.number.push_back(static_cast<uint32_t>(tmp));
        carry = static_cast<uint32_t>((tmp >> 32));
    }
    ans.number.push_back(carry);
    return ans;
}

void big_integer::myMultiply(big_integer const& left, big_integer const& right) {
    *this = 0;
    for (size_t i = right.number.size(); i > 0; i--)
    {
        *this <<= 32;
        *this += left.mul_long_short(right.number[i - 1]);
    }
}

big_integer& big_integer::operator*=(big_integer const& rhs)
{
    bool ans_sign = (sign ^ rhs.sign);
    myMultiply((sign ? this->abs() : *this), (rhs.sign ? rhs.abs() : rhs));
    if (ans_sign)
    {
        negate();
    }
    fit();
    return *this;
}

uint32_t big_integer::myAt(size_t pos) {
    if (pos < number.size()) {
        return number[pos];
    }
    return (sign ? UINT32_MAX : 0);
}


big_integer& big_integer::operator/=(big_integer const& rhs)
{
    bool ans_sign = (sign ^ rhs.sign);
    big_integer left = abs();
    big_integer right = rhs.abs();
    if (left < right)
    {
        *this = 0;
        return *this;
    }
    if (right.number.size() == 1)
    {
        *this = left;
        div_long_short(right.number[0]);
    }
    else
    {
        *this = 0;
        uint32_t f = static_cast<uint32_t>((static_cast<uint64_t>(UINT32_MAX) + 1) /
                                          (static_cast<uint64_t>(right.number.back()) + 1));
        left = left.mul_long_short(f);
        right = right.mul_long_short(f);
        right.fit();
        size_t ls = left.number.size(), rs = right.number.size();
        number.resize(ls - rs);
        for (size_t i = ls - rs; i > 0; i--)
        {
            number[i - 1] = static_cast<uint32_t>(std::min(
                ((static_cast<uint64_t>(left.myAt(i - 1 + rs)) << 32) +
                 static_cast<uint64_t>(left.myAt(i - 2 + rs))) /
                static_cast<uint64_t>(right.number.back()), static_cast<uint64_t>(UINT32_MAX)));
            left -= right * number[i - 1] << ((i - 1) * 32);
            // В пдф написано что это не будет выпоняться много раз на тестах иногда 1 раз но не больше 2 так что O(nm)
            while (left < 0)
            {
                number[i - 1]--;
                left += right << ((i - 1) * 32);
            }
        }
    }
    if (ans_sign)
    {
        negate_no_copy();
    }
    fit();
    return *this;
}

big_integer& big_integer::operator%=(big_integer const& rhs)
{
    return *this -= (*this / rhs) * rhs;
}

big_integer& big_integer::bitOp(big_integer const& rhs, std::function<uint32_t(uint32_t, uint32_t)> f)
{
    *this = reserve(*this, std::max(number.size(), rhs.number.size()));
    big_integer right(reserve(rhs, std::max(number.size(), rhs.number.size())));
    for (int i = 0; i < number.size(); i++)
    {
        number[i] = f(number[i], right.number[i]);
    }
    fit();
    sign = f(sign, rhs.sign);
    return *this;
}

namespace
{
    uint32_t myAnd(uint32_t a, uint32_t b)
    {
        return a & b;
    }

    uint32_t myOr(uint32_t a, uint32_t b)
    {
        return a | b;
    }

    uint32_t myXor(uint32_t a, uint32_t b)
    {
        return a ^ b;
    }
}

big_integer& big_integer::operator&=(big_integer const& rhs)
{
    return bitOp(rhs, myAnd);
}

big_integer& big_integer::operator|=(big_integer const& rhs)
{
    return bitOp(rhs, myOr);
}

big_integer& big_integer::operator^=(big_integer const& rhs)
{
    return bitOp(rhs, myXor);
}

big_integer& big_integer::operator<<=(int rhs)
{
    uint32_t offset = rhs % 32, carry = 0;
    if (offset != 0)
    {
        for (size_t i = 0; i < number.size(); i++)
        {
            uint64_t tmp = (static_cast<uint64_t>(number[i]) << offset) + carry;
            carry = static_cast<uint32_t>(tmp >> 32);
            number[i] = static_cast<uint32_t>(tmp);
        }
        number.push_back(((sign ? UINT32_MAX : 0) << offset) + carry);
    }
    number.insert(number.begin(), rhs / 32, 0);
    fit();
    return *this;
}

big_integer& big_integer::operator>>=(int rhs)
{
    number.erase(number.begin(), number.begin() + std::min(static_cast<int>(number.size()), rhs / 32));
    if (number.size() == 0)
    {
        number.push_back(0);
        return *this;
    }
    if (sign)
    {
        number.push_back(UINT32_MAX);
    }
    rhs %= 32;
    uint64_t carry = 0;
    if (rhs != 0)
    {
        for (size_t i = number.size(); i > 0; i--)
        {
            uint64_t tmp = (static_cast<uint64_t>(number[i - 1]) << (32 - rhs)) + carry;
            carry = tmp << 32;
            number[i - 1] = static_cast<uint32_t>(tmp >> 32);
        }
    }
    if (sign)
        number.pop_back();
    fit();
    return *this;
}

big_integer big_integer::operator+() const
{
    return *this;
}

big_integer big_integer::operator-() const
{
    big_integer res(*this);
    res.negate_no_copy();
    return res;
}

big_integer big_integer::operator~() const
{
    big_integer res(*this);
    for (unsigned int& i : res.number)
    {
        i ^= UINT32_MAX;
    }
    res.sign = !sign;
    return res;
}

big_integer& big_integer::operator++()
{
    return *this += 1;
}

big_integer big_integer::operator++(int)
{
    big_integer res(*this);
    *this += 1;
    return res;
}

big_integer& big_integer::operator--()
{
    return *this -= 1;
}

big_integer big_integer::operator--(int)
{
    big_integer res(*this);
    *this -= 1;
    return res;
}

big_integer operator+(big_integer a, big_integer const& b)
{
    return a += b;
}

big_integer operator-(big_integer a, big_integer const& b)
{
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const& b)
{
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const& b)
{
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const& b)
{
    return a %= b;
}

big_integer operator&(big_integer a, big_integer const& b)
{
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const& b)
{
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const& b)
{
    return a ^= b;
}

big_integer operator<<(big_integer a, int b)
{
    return a <<= b;
}

big_integer operator>>(big_integer a, int b)
{
    return a >>= b;
}

bool operator==(big_integer const& a, big_integer const& b)
{
    return a.sign == b.sign && a.number.size() == b.number.size() &&
           std::equal(a.number.begin(), a.number.end(), b.number.begin());
}

bool operator!=(big_integer const& a, big_integer const& b)
{
    return !(a == b);
}

bool operator<(big_integer const& a, big_integer const& b)
{
    if (a.sign != b.sign)
    {
        return a.sign;
    }
    if (a.number.size() != b.number.size())
    {
        return a.sign ^ (a.number.size() < b.number.size());
    }
    for (size_t i = a.number.size(); i > 0; i--)
    {
        if (a.number[i - 1] != b.number[i - 1])
        {
            return a.number[i - 1] < b.number[i - 1];
        }
    }
    return false;
}

bool operator>(big_integer const& a, big_integer const& b)
{
    return b < a;
}

bool operator<=(big_integer const& a, big_integer const& b)
{
    return !(a > b);
}

bool operator>=(big_integer const& a, big_integer const& b)
{
    return !(a < b);
}

uint32_t big_integer::div_long_short(uint32_t right)
{
    uint32_t carry = 0;
    uint64_t tmp;
    for (size_t i = number.size(); i > 0; i--)
    {
        tmp = (static_cast<uint64_t>(carry) << 32) + number[i - 1];
        number[i - 1] = tmp / right;
        carry = tmp % right;
    }
    return carry;
}

std::string to_string(big_integer const& a)
{
    std::stringstream str;
    big_integer tmp(a);
    if (tmp.sign)
    {
        str << "-";
        tmp.negate();
    }
    if (tmp.number.size() == 1 && tmp.number[0] == 0)
    {
        str << "0";
    }
    else
    {
        std::vector<uint32_t> ans;
        while (tmp.number.size() > 1 || tmp.number[0] != 0)
        {
            ans.push_back(tmp.div_long_short(static_cast<uint32_t>(1000000000)));
            tmp.fit();
        }
        str << ans.back();
        for (size_t i = ans.size() - 1; i > 0; i--)
        {
            std::string num = std::to_string(ans[i - 1]);
            for (size_t j = 0; j < 9 - num.size(); j++)
            {
                str << "0";
            }
            str << num;
        }
    }
    return str.str();
}

std::ostream& operator<<(std::ostream& s, big_integer const& a)
{
    return s << to_string(a);
}
