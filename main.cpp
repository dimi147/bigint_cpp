#include <cassert>
#include <iostream>
#include <string>
#include <vector>

class BigInt final {
public:
    // constructors
    BigInt() : m_number{"0"} {}
    BigInt(int64_t num) : m_number{std::to_string(num)}, m_isNegative{num < 0} {}
    BigInt(const std::string& s) : m_number{s} {
        if (m_number.empty())
            m_number = "0";
        m_isNegative = (m_number[0] == '-');
        if (m_isNegative)
            m_number.erase(m_number.begin());
    }

    // copy/move
    BigInt(const BigInt& r) : m_number{r.m_number}, m_isNegative{r.m_isNegative} {}
    BigInt(BigInt&& r) : m_number{std::move(r.m_number)}, m_isNegative{r.m_isNegative} {}
    auto operator=(const BigInt& rh) -> BigInt& {
        m_number = rh.m_number;
        m_isNegative = rh.m_isNegative;
        return *this;
    }
    auto operator=(BigInt&& rh) -> BigInt& {
        m_number = std::move(rh.m_number);
        m_isNegative = std::move(rh.m_isNegative);
        return *this;
    }

    // destructors
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
            if (this->absolute() > rh.absolute()) {
                subtractNumbers(*this, rh, result);
                result.m_isNegative = m_isNegative;
            } else if (*this == rh)
                return BigInt{"0"};
            else {
                subtractNumbers(rh, *this, result);
                result.m_isNegative = rh.m_isNegative;
            }
        }
        return result;
    }

    BigInt operator*(const BigInt& rh) {
        BigInt result{};
        if (m_number == "0" || rh.m_number == "0")
            return BigInt{"0"};
        result.m_isNegative = (m_isNegative != rh.m_isNegative);
        multiplyNumbers(*this, rh, result);
        return result;
    }

    BigInt operator/(const BigInt& rh) {
        BigInt result{}, remainder{};
        if (rh == BigInt{"0"})
            throw(std::invalid_argument{"Division by 0"});
        result.m_isNegative = (m_isNegative != rh.m_isNegative);
        divideNumbers(*this, rh, result, remainder);
        return result;
    }

    BigInt operator%(const BigInt& rh) {
        BigInt result{}, remainder{};
        if (rh == BigInt{"0"})
            throw(std::invalid_argument{"Division by 0"});
        result.m_isNegative = (m_isNegative != rh.m_isNegative);
        divideNumbers(*this, rh, result, remainder);
        return remainder;
    }

    BigInt operator^(const BigInt& rh) {
        if (rh < BigInt{0})
            return BigInt{0};
        if (rh == BigInt{0})
            return BigInt{1};
        BigInt result{1}, base{*this}, exp{rh};
        while (exp > BigInt{0}) {
            if (exp.m_number[exp.m_number.size() - 1] == '1')
                result = result * base;
            base = base * base;
            exp = exp / BigInt{2};
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

    auto multiplyNumbers(const BigInt& a, const BigInt& b, BigInt& result) -> void {
        auto& x = a.m_number;
        auto& y = b.m_number;
        auto& z = result.m_number;
        const auto sizex = x.size();
        const auto sizey = y.size();
        z.insert(0, sizex + sizey - z.size(), '0');
        const auto sizez = z.size();

        // TODO: use vector of BigInts here instead
        std::vector<BigInt> sums(sizez, 0);
        for (auto i = 0; i < sizex; ++i) {
            for (auto j = 0; j < sizey; ++j) {
                sums[i + j] = sums[i + j] + BigInt{(x[sizex - 1 - i] - '0') * (y[sizey - 1 - j] - '0')};
            }
        }

        BigInt remainder = 0;
        for (auto i = 0; i < sizez; ++i) {
            auto sum = sums[i];
            sum = sum + remainder;

            if (sum.m_number.size() > 1) {
                remainder = sum;
                remainder.m_number.erase(remainder.m_number.end() - 1);
            }

            if (sum > BigInt{9})
                sum = BigInt{sum.m_number[sum.m_number.size() - 1] - '0'};
            assert(sum.m_number.size() == 1);
            z[sizez - 1 - i] = sum.m_number[0];
        }

        // O((M+N)*M*N)
        // auto digitRemainder = 0;
        // for (auto d = 0; d < sizez; ++d) {
        //     auto digitSum = 0;
        //     for (auto i = 0; i <= d; ++i) {
        //         auto remainder = 0;
        //         auto sum = 0;
        //         for (auto j = 0; j <= d - i; ++j) {
        //             sum = (getDigit(y, sizey - 1 - i) - '0') * (getDigit(x, sizex - 1 - j) - '0') + remainder;
        //             remainder = sum / 10;
        //             if (sum >= 10)
        //                 sum %= 10;
        //             if (d == (j + i))
        //                 digitSum += sum;
        //         }
        //     }
        //     z[sizez - 1 - d] = digitSum + digitRemainder + '0';
        //     digitRemainder = (z[sizez - 1 - d] > '9');
        //     if (digitRemainder)
        //         z[sizez - 1 - d] -= 10;
        // }

        // remove leading 0s
        for (auto it = z.begin(); *it == '0' && it != z.end() - 1;)
            it = z.erase(it);
    }

    auto divideNumbers(const BigInt& a, const BigInt& b, BigInt& result, BigInt& remainder) -> void {
        assert(b != BigInt{"0"});

        // TODO handle remainder here
        if (a <= b) {
            result = (a == b) ? BigInt{"1"} : BigInt{"0"};
            return;
        }

        auto numerator = a.m_number;
        const auto& denominator = b.m_number;

        // TODO there is no point to this, while using push_back below
        result.m_number.resize(numerator.size() - denominator.size() + 1, '0');

        remainder = BigInt{std::string{*numerator.begin()}};
        numerator.erase(numerator.begin());

        bool firstPass = true;
        bool addZeros = false;
        while (remainder.m_number.size() + numerator.size() >= denominator.size()) {
            if (remainder == BigInt{"0"})
                addZeros = true;

            while (remainder < b && !numerator.empty()) {
                remainder.m_number.push_back(*numerator.begin());
                numerator.erase(numerator.begin());
                if (addZeros)
                    result.m_number.push_back('0');
                if (!firstPass)
                    addZeros = true;
            }

            if (remainder < b)
                break;

            auto mult = BigInt{"9"};
            while (mult * b > remainder)
                mult = mult - BigInt{"1"};

            assert(mult > BigInt{"0"});

            result.m_number.push_back(mult.m_number[0]);
            remainder = remainder - (mult * b);
            firstPass = false;
            addZeros = false;
        }

        // remove leading 0s
        for (auto it = result.m_number.begin(); *it == '0' && it != result.m_number.end() - 1;)
            it = result.m_number.erase(it);
    }

    auto placeDigit(std::string& s, int digit, int index) -> void {
        assert(index < (int)s.size());
        if (index < 0)
            s.insert(0, 1, digit);
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
auto operator""_bi(unsigned long long int number) -> BigInt { return BigInt{std::to_string(number)}; }

void testBigInt() {
    auto testCount = 0;
    auto allTestsPassed = true;
    auto test = [&testCount, &allTestsPassed](auto cond, auto res) {
        try {
            std::cout << "test " << ++testCount << (cond == res ? ": passed" : ": error") << std::endl;
        } catch (std::exception& e) {
            std::cout << "test " << ++testCount << ": error :: exception raised :: " << e.what() << std::endl;
            allTestsPassed = false;
        } catch (...) {
            std::cout << "test " << ++testCount << ": error :: unknown exception raised" << std::endl;
            allTestsPassed = false;
        }
    };

    auto testThrow = [&testCount, &allTestsPassed](auto func) {
        try {
            func();
        } catch (std::exception& e) {
            std::cout << "test " << ++testCount << ": passed :: " << e.what() << std::endl;
            return;
        } catch (...) {
            std::cout << "test " << ++testCount << ": error :: unknown exception raised" << std::endl;
            allTestsPassed = false;
            return;
        }
        std::cout << "test " << ++testCount << ": error :: no exception raised" << std::endl;
        allTestsPassed = false;
    };

    // comparison
    test("1234"_bi == "1234"_bi, true);
    test("1234"_bi == "-1234"_bi, false);
    test("125"_bi != "120"_bi, true);
    test("125"_bi != "125"_bi, false);
    test("12312"_bi > "998"_bi, true);
    test("998"_bi > "12312"_bi, false);
    test("998"_bi < "12312"_bi, true);
    test("12312"_bi < "998"_bi, false);
    test("123"_bi >= "123"_bi, true);
    test("123"_bi <= "123"_bi, true);
    test("123"_bi >= "124"_bi, false);
    test("124"_bi <= "123"_bi, false);

    // unary
    test(-"100"_bi, "-100"_bi);
    test(-"-100"_bi, "100"_bi);

    // addition
    test("991723947"_bi + "2342342"_bi, "994066289"_bi);
    test("-991723947"_bi + "-2342342"_bi, "-994066289"_bi);
    test("100"_bi + "-30"_bi, "70"_bi);
    test("-100"_bi + "30"_bi, "-70"_bi);
    test(999_bi + 1_bi, 1000_bi);

    // subtraction
    test("100"_bi - "30"_bi, "70"_bi);
    test("100"_bi - "-30"_bi, "130"_bi);
    test("-100"_bi - "-30"_bi, "-70"_bi);
    test("-100"_bi - "30"_bi, "-130"_bi);

    // multiplication
    test("100"_bi * "10"_bi, "1000"_bi);
    test("10"_bi * "100"_bi, "1000"_bi);
    test("99"_bi * "999"_bi, "98901"_bi);
    test("-99"_bi * "-999"_bi, "98901"_bi);
    test("99"_bi * "-999"_bi, "-98901"_bi);

    // division
    test("100"_bi / "10"_bi, "10"_bi);
    test(8005672_bi / 2000_bi, 4002_bi);
    testThrow([]() { 100_bi / 0_bi; });

    // modulo
    test(8005672_bi % 2000_bi, 1672_bi);

    // power
    test(12_bi ^ 2_bi, 144_bi);
    test(12_bi ^ -2_bi, 0_bi);

    std::cout << std::boolalpha << "all tests passed: " << allTestsPassed << std::endl;
}

void main() { testBigInt(); }