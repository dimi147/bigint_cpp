#include "bigint.h"

#include <cassert>
#include <cctype>
#include <sstream>
#include <vector>

BigInt::BigInt(int64_t num) : BigInt{std::to_string(num)} {}
BigInt::BigInt(const std::string& s) : m_number{s} {
    m_isNegative = (!m_number.empty() && m_number[0] == '-');
    if (m_isNegative)
        m_number.erase(m_number.begin());
    if (m_number.empty())
        throw std::invalid_argument("Invalid integer as string");
    for (auto it = m_number.begin(); it != m_number.end(); ++it)
        if (!std::isdigit(*it))
            throw std::invalid_argument("Invalid integer as string");
}

BigInt::BigInt(const BigInt& r) : m_number{r.m_number}, m_isNegative{r.m_isNegative} {}
BigInt::BigInt(BigInt&& r) : m_number{std::move(r.m_number)}, m_isNegative{r.m_isNegative} {}

auto BigInt::operator=(const BigInt& rh) & -> BigInt& {
    m_number = rh.m_number;
    m_isNegative = rh.m_isNegative;
    return *this;
}
auto BigInt::operator=(BigInt&& rh) & -> BigInt& {
    m_number = std::move(rh.m_number);
    m_isNegative = std::move(rh.m_isNegative);
    return *this;
}

BigInt::~BigInt() = default;

BigInt BigInt::operator+(const BigInt& rh) {
    if (m_isNegative == rh.m_isNegative) {
        auto result = sumOfNumbers(*this, rh);
        result.m_isNegative = m_isNegative;
        return result;
    } else {
        auto result = differenceOfNumbers(*this, rh);
        result.m_isNegative = (result.m_number == "0" ? false : (m_isNegative != (this->abs() < rh.abs())));
        return result;
    }
}
BigInt& BigInt::operator+=(const BigInt& rh) & {
    (*this) = (*this) + rh;
    return (*this);
}
BigInt& BigInt::operator++() & {
    (*this) += BigInt{1};
    return *this;
}
BigInt BigInt::operator++(int) & {
    BigInt copy{*this};
    (*this) += BigInt{1};
    return copy;
}

BigInt BigInt::operator-(const BigInt& rh) {
    if (m_isNegative != rh.m_isNegative) {
        auto result = sumOfNumbers(*this, rh);
        result.m_isNegative = m_isNegative;
        return result;
    } else {
        auto result = differenceOfNumbers(*this, rh);
        result.m_isNegative = (result.m_number == "0" ? false : (m_isNegative != (this->abs() < rh.abs())));
        return result;
    }
}
BigInt& BigInt::operator-=(const BigInt& rh) & {
    (*this) = (*this) - rh;
    return (*this);
}
BigInt& BigInt::operator--() & {
    (*this) -= BigInt{1};
    return *this;
}
BigInt BigInt::operator--(int) & {
    BigInt copy{*this};
    (*this) -= BigInt{1};
    return copy;
}

BigInt BigInt::operator*(const BigInt& rh) {
    if (m_number == "0" || rh.m_number == "0")
        return BigInt{0};
    auto result = multiplyNumbers(*this, rh);
    result.m_isNegative = (m_isNegative != rh.m_isNegative);
    return result;
}
BigInt& BigInt::operator*=(const BigInt& rh) & {
    (*this) = (*this) * rh;
    return (*this);
}

BigInt BigInt::operator/(const BigInt& rh) {
    if (rh == BigInt{"0"})
        throw(std::invalid_argument{"Division by 0"});
    const bool isNegative = (m_isNegative != rh.m_isNegative);
    auto [result, remainder] = divideNumbers(*this, rh);
    result.m_isNegative = isNegative;
    return result;
}
BigInt& BigInt::operator/=(const BigInt& rh) & {
    (*this) = (*this) / rh;
    return (*this);
}

BigInt BigInt::operator%(const BigInt& rh) {
    if (rh == BigInt{"0"})
        throw(std::invalid_argument{"Division by 0"});
    const bool isNegative = (m_isNegative != rh.m_isNegative);
    auto [result, remainder] = divideNumbers(*this, rh);
    remainder.m_isNegative = isNegative;
    return remainder;
}
BigInt& BigInt::operator%=(const BigInt& rh) & {
    (*this) = (*this) % rh;
    return (*this);
}

BigInt BigInt::operator^(const BigInt& rh) {
    if (rh < BigInt{0})
        return BigInt{0};
    if (rh == BigInt{0})
        return BigInt{1};
    BigInt result{1}, base{*this}, exp{rh};
    while (exp > BigInt{0}) {
        if (exp % BigInt{2} == BigInt{1})
            result = result * base;
        base = base * base;
        exp = exp / BigInt{2};
    }
    return result;
}
BigInt& BigInt::operator^=(const BigInt& rh) & {
    (*this) = (*this) ^ rh;
    return (*this);
}

auto BigInt::operator-() -> BigInt {
    if (m_number == "0")
        return *this;
    BigInt result{*this};
    result.m_isNegative = !this->m_isNegative;
    return result;
}

auto BigInt::operator==(const BigInt& rh) const -> bool {
    return (m_isNegative == rh.m_isNegative && m_number == rh.m_number);
}
auto BigInt::operator!=(const BigInt& rh) const -> bool { return !(*this == rh); }
auto BigInt::operator<(const BigInt& rh) const -> bool {
    if (m_isNegative != rh.m_isNegative)
        return m_isNegative;
    if (m_number.size() != rh.m_number.size()) {
        if (m_isNegative)
            return m_number.size() > rh.m_number.size();
        else
            return m_number.size() < rh.m_number.size();
    } else {
        if (m_isNegative)
            return m_number > rh.m_number;
        else
            return m_number < rh.m_number;
    }
}
auto BigInt::operator>(const BigInt& rh) const -> bool { return !(*this == rh || *this < rh); }
auto BigInt::operator<=(const BigInt& rh) const -> bool { return *this == rh || *this < rh; }
auto BigInt::operator>=(const BigInt& rh) const -> bool { return *this == rh || *this > rh; }

std::ostream& operator<<(std::ostream& os, const BigInt& n) { return os << (n.m_isNegative ? "-" : "") << n.m_number; }
std::istream& operator>>(std::istream& is, BigInt& n) {
    if (is.peek() && is.eof())
        return is;

    auto size = is.seekg(0, std::ios::end).tellg() - is.seekg(0, std::ios::beg).tellg();

    if (is.peek() == '-') {
        is.get();
        n.m_number.clear();
        n.m_isNegative = true;
        size--;
    }

    auto currentSize = n.m_number.size();
    n.m_number.resize(currentSize + size);
    is.read(n.m_number.data() + currentSize, size);

    for (auto i = currentSize; i < n.m_number.size(); ++i)
        if (!std::isdigit(n.m_number[i]))
            throw std::exception("Invalid integer as string");

    return is;
}

auto BigInt::abs() const -> BigInt {
    BigInt result{*this};
    result.m_isNegative = false;
    return result;
}

auto BigInt::sumOfNumbers(const BigInt& n1, const BigInt& n2) -> BigInt {
    auto addDigits = [](char a, char b, int& remainder) -> char {
        assert(remainder == 0 || remainder == 1);
        char sum = a + b - '0' + remainder;
        remainder = (sum > '9');
        if (remainder)
            sum -= 10;
        return sum;
    };

    auto& x = n1.m_number;
    auto& y = n2.m_number;
    const auto sizex = x.size();
    const auto sizey = y.size();
    auto result = BigInt{0};
    result.m_number.resize(std::max(sizex, sizey) + 1, '0');
    auto remainder = 0;

    for (auto i = 0; (i < std::max(sizex, sizey)) || remainder; ++i) {
        char a = (i >= sizex) ? '0' : x[sizex - 1 - i];
        char b = (i >= sizey) ? '0' : y[sizey - 1 - i];
        auto sum = addDigits(a, b, remainder);
        assert(i < result.m_number.size());
        result.m_number[result.m_number.size() - 1 - i] = sum;
    }

    if (result.m_number[0] == '0')
        result.m_number.erase(result.m_number.begin());

    return result;
}

auto BigInt::differenceOfNumbers(const BigInt& n1, const BigInt& n2) -> BigInt {
    auto subtractDigits = [](char a, char b, int& remainder) -> char {
        assert(remainder == 0 || remainder == 1);
        char sum = a - (b - '0' + remainder);
        remainder = (sum < '0');
        if (remainder)
            sum += 10;
        return sum;
    };

    auto n1IsGreater = (n1.abs() > n2.abs());
    const std::string& x = (n1IsGreater) ? n1.m_number : n2.m_number;
    const std::string& y = (n1IsGreater) ? n2.m_number : n1.m_number;
    const auto sizex = x.size();
    const auto sizey = y.size();
    auto result = BigInt{0};
    result.m_number.resize(std::max(sizex, sizey), '0');

    for (auto i = 0, remainder = 0; i < std::min(sizex, sizey) || remainder; ++i) {
        char a = (i >= sizex) ? '0' : x.at(sizex - 1 - i);
        char b = (i >= sizey) ? '0' : y.at(sizey - 1 - i);
        auto sum = subtractDigits(a, b, remainder);
        assert(i < result.m_number.size());
        result.m_number[result.m_number.size() - 1 - i] = sum;
    }

    // remove leading 0s
    for (auto it = result.m_number.begin(); *it == '0' && it != result.m_number.end() - 1;)
        it = result.m_number.erase(it);

    return result;
}

auto BigInt::multiplyNumbers(const BigInt& a, const BigInt& b) -> BigInt {
    auto& x = a.m_number;
    auto& y = b.m_number;
    const auto sizex = x.size();
    const auto sizey = y.size();
    auto result = BigInt{0};
    result.m_number.resize(sizex + sizey, '0');

    std::vector<BigInt> sums(result.m_number.size(), BigInt{0});
    for (auto i = 0; i < sizex; ++i) {
        for (auto j = 0; j < sizey; ++j) {
            sums[i + j] = sums[i + j] + BigInt{(x[sizex - 1 - i] - '0') * (y[sizey - 1 - j] - '0')};
        }
    }

    BigInt remainder{0};
    for (auto i = 0; i < result.m_number.size(); ++i) {
        auto sum = sums[i];
        sum = sum + remainder;

        if (sum.m_number.size() > 1) {
            remainder = sum;
            remainder.m_number.erase(remainder.m_number.end() - 1);
        }

        if (sum > BigInt{9})
            sum = BigInt{sum.m_number[sum.m_number.size() - 1] - '0'};
        assert(sum.m_number.size() == 1);
        result.m_number[result.m_number.size() - 1 - i] = sum.m_number[0];
    }

    // remove leading 0s
    for (auto it = result.m_number.begin(); *it == '0' && it != result.m_number.end() - 1;)
        it = result.m_number.erase(it);

    return result;
}

auto BigInt::divideNumbers(BigInt a, BigInt b) -> std::pair<BigInt, BigInt> {
    assert(b != BigInt{"0"});
    a = a.abs();
    b = b.abs();

    if (a <= b)
        return (a == b ? std::make_pair(BigInt{1}, BigInt{0}) : std::make_pair(BigInt{0}, BigInt{a}));

    BigInt result{0}, remainder{0};
    result.m_number.clear();
    auto& numerator = a.m_number;
    remainder.m_number = numerator.substr(0, b.m_number.size());
    numerator.erase(numerator.begin(), numerator.begin() + b.m_number.size());

    while (remainder.m_number.size() + numerator.size() >= b.m_number.size()) {
        bool addZeros = (remainder.m_number == "0");
        while (remainder < b && !numerator.empty()) {
            if (remainder.m_number == "0")
                remainder.m_number[0] = *numerator.begin();
            else
                remainder.m_number.push_back(*numerator.begin());
            numerator.erase(numerator.begin());
            if (addZeros)
                result.m_number.push_back('0');
            addZeros = true;
        }

        if (remainder < b)
            break;

        auto mult = BigInt{"9"};
        while (mult * b > remainder) {
            mult = mult - BigInt{"1"};
            assert(mult > BigInt{"0"});
        }

        result.m_number.push_back(mult.m_number[0]);
        remainder = remainder - (mult * b);
    }

    return {result, remainder};
}

auto operator""_bi(const char* s, size_t size) -> BigInt { return BigInt{s}; }
auto operator""_bi(unsigned long long int number) -> BigInt { return BigInt{std::to_string(number)}; }
