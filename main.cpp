#include <cassert>
#include <iostream>
#include <string>

class BigInt {
public:
    BigInt() : m_number{"0"} {}
    BigInt(const std::string& s) : m_number{s} {
        m_isNegative = m_number[0] == '-';
        if (m_isNegative)
            m_number.erase(m_number.begin());
    }
    BigInt(const BigInt& r) : m_number{r.m_number}, m_isNegative{r.m_isNegative} {}
    BigInt(BigInt&& r) : m_number{std::move(r.m_number)}, m_isNegative{r.m_isNegative} {}
    ~BigInt() = default;

    // Arithmetic operators
    BigInt operator+(const BigInt& rh) {
        BigInt result{};
        if (m_isNegative == rh.m_isNegative) {
            addNumbers(*this, rh, result);
            result.m_isNegative = m_isNegative;
        } else {
            if (this->absolute() > rh.absolute()) {
                subtractNumbers(*this, rh, result);
                result.m_isNegative = m_isNegative;
            } else if (this->absolute() == rh.absolute())
                return BigInt{"0"};
            else {
                subtractNumbers(rh, *this, result);
                result.m_isNegative = rh.m_isNegative;
            }
        }
        return result;
    }

    BigInt operator-(const BigInt& rh) {
        auto result = BigInt{};
        if (m_isNegative != rh.m_isNegative) {
            addNumbers(*this, rh, result);
            result.m_isNegative = m_isNegative;
        } else {
            if (this->absolute() > rh.absolute())
                subtractNumbers(*this, rh, result);
            else if (*this == rh)
                return BigInt{"0"};
            else
                subtractNumbers(rh, *this, result);
        }
        return result;
    }

    // Unary operators
    auto operator-() -> BigInt {
        if (m_number == "0")
            return *this;
        BigInt result{*this};
        result.m_isNegative = !this->m_isNegative;
        return result;
    }

    // Comparison operators
    auto operator==(const BigInt& rh) const -> bool {
        return (m_isNegative == rh.m_isNegative && m_number == rh.m_number);
    }
    auto operator!=(const BigInt& rh) const -> bool { return !(*this == rh); }
    auto operator<(const BigInt& rh) const -> bool {
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
    auto operator>(const BigInt& rh) const -> bool { return !(*this == rh || *this < rh); }
    auto operator<=(const BigInt& rh) const -> bool { return *this == rh || *this < rh; }
    auto operator>=(const BigInt& rh) const -> bool { return *this == rh || *this > rh; }

    // Stream operators
    friend std::ostream& operator<<(std::ostream& os, const BigInt& n) {
        if (n.m_isNegative)
            os << '-';
        os << n.m_number;
        return os;
    }

    auto absolute() const -> BigInt {
        BigInt result{*this};
        result.m_isNegative = false;
        return result;
    }

private:
    auto addNumbers(const BigInt& a, const BigInt& b, BigInt& result) -> void {
        assert(a.m_isNegative == b.m_isNegative);
        result.m_isNegative = a.m_isNegative;
        auto& x = a.m_number;
        auto& y = b.m_number;
        auto& z = result.m_number;

        if (z.size() < std::max(x.size(), y.size()))
            z.insert(0, std::max(x.size(), y.size()) - z.size(), '0');

        const auto sizex = x.size();
        const auto sizey = y.size();
        const auto sizez = z.size();

        auto remainder = 0;
        for (auto i = 0; i < sizez || remainder; ++i) {
            char a = getDigit(x, sizex - 1 - i);
            char b = getDigit(y, sizey - 1 - i);
            auto sum = addDigits(a, b, remainder);
            placeDigit(z, sum, sizez - 1 - i);
        }
    }

    auto subtractNumbers(const BigInt& a, const BigInt& b, BigInt& result) -> void {
        assert(a.absolute() > b.absolute());
        result.m_isNegative = a.m_isNegative;
        auto& x = a.m_number;
        auto& y = b.m_number;
        auto& z = result.m_number;

        if (z.size() < std::max(x.size(), y.size()))
            z.insert(0, std::max(x.size(), y.size()) - z.size(), '0');

        const auto sizex = x.size();
        const auto sizey = y.size();
        const auto sizez = z.size();

        auto remainder = 0;
        for (auto i = 0; i < sizez || remainder; ++i) {
            char a = getDigit(x, sizex - 1 - i);
            char b = getDigit(y, sizey - 1 - i);
            auto sum = subtractDigits(a, b, remainder);
            placeDigit(z, sum, sizez - 1 - i);
        }

        // remove leading 0s
        for (auto it = z.begin(); *it == '0' && it != z.end() - 1;)
            it = z.erase(it);
    }

    auto placeDigit(std::string& s, int digit, int index) -> void {
        assert(index < s.size());
        if (index < 0)
            s.append(0, 1, digit);
        else
            s[index] = digit;
    }

    auto addDigits(char a, char b, int& remainder) -> char {
        assert(remainder == 0 || remainder == 1);
        char sum = a + b - '0' + remainder;
        remainder = (sum > '9');
        if (remainder)
            sum -= 10;
        return sum;
    }

    auto subtractDigits(char a, char b, int& remainder) -> char {
        assert(remainder == 0 || remainder == 1);
        char sum = a - (b - '0' + remainder);
        remainder = (sum < '0');
        if (remainder)
            sum += 10;
        return sum;
    }

    auto getDigit(const std::string& s, int index) -> char {
        if (index < 0 || index > s.size() - 1)
            return '0';
        return s[index];
    }

    bool m_isNegative{false};
    std::string m_number;
};

auto operator""_bi(const char* s, size_t size) -> BigInt { return BigInt{s}; }

void testBigInt() {
    auto testCount = 0;
    auto check = [&testCount](auto cond, auto res) {
        std::cout << "test " << ++testCount << (cond == res ? ": passed" : ": error") << std::endl;
        return cond == res;
    };

    check("1234"_bi == "1234"_bi, true);
    check("1234"_bi == "-1234"_bi, false);
    check("125"_bi != "120"_bi, true);
    check("125"_bi != "125"_bi, false);
    check("12312"_bi > "998"_bi, true);
    check("998"_bi > "12312"_bi, false);
    check("998"_bi < "12312"_bi, true);
    check("12312"_bi < "998"_bi, false);
    check("123"_bi >= "123"_bi, true);
    check("123"_bi <= "123"_bi, true);
    check("123"_bi >= "124"_bi, false);
    check("124"_bi <= "123"_bi, false);
    check("991723947"_bi + "2342342"_bi, "994066289"_bi);
    check("-991723947"_bi + "-2342342"_bi, "-994066289"_bi);

    check("100"_bi + "-30"_bi, "70"_bi);
    check("-100"_bi + "30"_bi, "-70"_bi);
}

void main() { testBigInt(); }