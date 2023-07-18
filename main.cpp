#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#define TEST_LINE(x, y) test(x, y, __LINE__)
#define TEST_THROW_LINE(x) testThrow(x, __LINE__)

class BigInt final {
public:
    // constructors
    BigInt() : m_number{"0"} {}
    BigInt(int64_t num) : BigInt{std::to_string(num)} {}
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
        BigInt result{*this};
        result += rh;
        return result;
    }
    BigInt& operator+=(const BigInt& rh) & {
        if (m_isNegative == rh.m_isNegative) {
            const bool isNegative = m_isNegative;
            (*this) = sumOfNumbers(*this, rh);
            this->m_isNegative = isNegative;
        } else {
            const bool isNegative = (m_isNegative != (this->absolute() < rh.absolute()));
            (*this) = differenceOfNumbers(*this, rh);
            this->m_isNegative = isNegative;
            if (this->m_number == "0")
                m_isNegative = false;
        }
        return *this;
    }
    BigInt& operator++() {
        (*this) += BigInt{1};
        return *this;
    }
    BigInt operator++(int) {
        BigInt copy{*this};
        (*this) += BigInt{1};
        return copy;
    }

    BigInt operator-(const BigInt& rh) {
        auto result = BigInt{*this};
        result -= rh;
        return result;
    }
    BigInt& operator-=(const BigInt& rh) & {
        if (m_isNegative != rh.m_isNegative) {
            const bool isNegative = m_isNegative;
            (*this) = sumOfNumbers(*this, rh);
            this->m_isNegative = isNegative;
        } else {
            const bool isNegative = (m_isNegative != (this->absolute() < rh.absolute()));
            (*this) = differenceOfNumbers(*this, rh);
            this->m_isNegative = isNegative;
            if (this->m_number == "0")
                m_isNegative = false;
        }
        return *this;
    }
    BigInt& operator--() {
        (*this) -= BigInt{1};
        return *this;
    }
    BigInt operator--(int) {
        BigInt copy{*this};
        (*this) -= BigInt{1};
        return copy;
    }

    BigInt operator*(const BigInt& rh) {
        BigInt result{*this};
        result *= rh;
        return result;
    }
    BigInt& operator*=(const BigInt& rh) {
        if (m_number == "0" || rh.m_number == "0") {
            (*this) = BigInt{0};
            return (*this);
        }
        const bool isNegative = (m_isNegative != rh.m_isNegative);
        (*this) = multiplyNumbers(*this, rh);
        this->m_isNegative = isNegative;
        return (*this);
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
            if (exp % BigInt{2} == BigInt{1})
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
    auto sumOfNumbers(const BigInt& n1, const BigInt& n2) -> BigInt {
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
        result.m_number.resize(std::max(sizex, sizey), '0');
        auto remainder = 0;
        for (auto i = 0; (i < std::max(sizex, sizey)) || remainder; ++i) {
            char a = (i >= sizex) ? '0' : x[sizex - 1 - i];
            char b = (i >= sizey) ? '0' : y[sizey - 1 - i];
            auto sum = addDigits(a, b, remainder);
            if (i >= result.m_number.size())
                result.m_number.insert(0, 1, sum);
            else
                result.m_number[result.m_number.size() - 1 - i] = sum;
        }
        return result;
    }

    auto differenceOfNumbers(const BigInt& n1, const BigInt& n2) -> BigInt {
        auto subtractDigits = [](char a, char b, int& remainder) -> char {
            assert(remainder == 0 || remainder == 1);
            char sum = a - (b - '0' + remainder);
            remainder = (sum < '0');
            if (remainder)
                sum += 10;
            return sum;
        };

        auto n1IsGreater = (n1.absolute() > n2.absolute());
        const std::string& x = (n1IsGreater) ? n1.m_number : n2.m_number;
        const std::string& y = (n1IsGreater) ? n2.m_number : n1.m_number;
        const auto sizex = x.size();
        const auto sizey = y.size();
        auto result = BigInt{0};
        result.m_number.resize(std::max(sizex, sizey), '0');
        auto remainder = 0;
        auto i = 0;
        for (; i < std::max(sizex, sizey) || remainder; ++i) {
            char a = (i >= sizex) ? '0' : x.at(sizex - 1 - i);
            char b = (i >= sizey) ? '0' : y.at(sizey - 1 - i);
            auto sum = subtractDigits(a, b, remainder);
            // if (i >= result.m_number.size())
            //     result.m_number.insert(0, 1, sum);
            // else
            assert(i < result.m_number.size());
            result.m_number[result.m_number.size() - 1 - i] = sum;
        }

        // remove leading 0s
        for (auto it = result.m_number.begin(); *it == '0' && it != result.m_number.end() - 1;)
            it = result.m_number.erase(it);

        return result;
    }

    auto multiplyNumbers(const BigInt& a, const BigInt& b) -> BigInt {
        auto& x = a.m_number;
        auto& y = b.m_number;
        const auto sizex = x.size();
        const auto sizey = y.size();
        auto result = BigInt{0};
        result.m_number.resize(sizex + sizey, '0');

        std::vector<BigInt> sums(result.m_number.size(), 0);
        for (auto i = 0; i < sizex; ++i) {
            for (auto j = 0; j < sizey; ++j) {
                sums[i + j] = sums[i + j] + BigInt{(x[sizex - 1 - i] - '0') * (y[sizey - 1 - j] - '0')};
            }
        }

        BigInt remainder = 0;
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

    auto divideNumbers(BigInt a, BigInt b, BigInt& result, BigInt& remainder) -> void {
        assert(b != BigInt{"0"});

        a = a.absolute();
        b = b.absolute();
        if (a <= b) {
            result = (a == b) ? BigInt{"1"} : BigInt{"0"};
            remainder = (a == b) ? BigInt{"0"} : BigInt{a};
            return;
        }

        auto& numerator = a.m_number;
        const auto& denominator = b.m_number;
        result.m_number.clear();
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
            while (mult * b > remainder) {
                mult = mult - BigInt{"1"};
                assert(mult > BigInt{"0"});
            }

            result.m_number.push_back(mult.m_number[0]);
            remainder = remainder - (mult * b);
            firstPass = false;
            addZeros = false;
        }
    }

    bool m_isNegative{false};
    std::string m_number;
};

auto operator""_bi(const char* s, size_t size) -> BigInt { return BigInt{s}; }
auto operator""_bi(unsigned long long int number) -> BigInt { return BigInt{std::to_string(number)}; }

void testBigInt() {
    auto allTestsPassed = true;

    auto test = [&allTestsPassed](auto cond, auto res, auto line) {
        try {
            std::cout << "test " << line << (cond == res ? ": passed" : ": error") << std::endl;
            if (cond != res)
                allTestsPassed = false;
        } catch (std::exception& e) {
            std::cout << "test " << line << ": error :: exception raised :: " << e.what() << std::endl;
            allTestsPassed = false;
        } catch (...) {
            std::cout << "test " << line << ": error :: unknown exception raised" << std::endl;
            allTestsPassed = false;
        }
    };

    auto testThrow = [&allTestsPassed](auto func, auto line) {
        try {
            func();
        } catch (std::exception& e) {
            std::cout << "test " << line << ": passed :: " << e.what() << std::endl;
            return;
        } catch (...) {
            std::cout << "test " << line << ": error :: unknown exception raised" << std::endl;
            allTestsPassed = false;
            return;
        }
        std::cout << "test " << line << ": error :: no exception raised" << std::endl;
        allTestsPassed = false;
    };

    // comparison
    TEST_LINE("1234"_bi == "1234"_bi, true);
    TEST_LINE("1234"_bi == "-1234"_bi, false);
    TEST_LINE("125"_bi != "120"_bi, true);
    TEST_LINE("125"_bi != "125"_bi, false);
    TEST_LINE("12312"_bi > "998"_bi, true);
    TEST_LINE("998"_bi > "12312"_bi, false);
    TEST_LINE("998"_bi < "12312"_bi, true);
    TEST_LINE("12312"_bi < "998"_bi, false);
    TEST_LINE("123"_bi >= "123"_bi, true);
    TEST_LINE("123"_bi <= "123"_bi, true);
    TEST_LINE("123"_bi >= "124"_bi, false);
    TEST_LINE("124"_bi <= "123"_bi, false);

    // unary
    TEST_LINE(-"100"_bi, "-100"_bi);
    TEST_LINE(-"-100"_bi, "100"_bi);
    TEST_LINE(-"0"_bi, 0_bi);

    // addition
    TEST_LINE("991723947"_bi + "2342342"_bi, "994066289"_bi);
    TEST_LINE("-991723947"_bi + "-2342342"_bi, "-994066289"_bi);
    TEST_LINE("100"_bi + "-30"_bi, "70"_bi);
    TEST_LINE("-100"_bi + "30"_bi, "-70"_bi);
    TEST_LINE(-30_bi + 100_bi, 70_bi);
    TEST_LINE(999_bi + 1_bi, 1000_bi);

    {
        BigInt a{-30};
        TEST_LINE(a += 100_bi, 70_bi);
        TEST_LINE(a++, 70_bi);
        TEST_LINE(++a, 72_bi);
    }

    // subtraction
    TEST_LINE("100"_bi - "30"_bi, "70"_bi);
    TEST_LINE("100"_bi - "-30"_bi, "130"_bi);
    TEST_LINE(30_bi - 100_bi, -70_bi);
    TEST_LINE(30_bi - -100_bi, 130_bi);
    TEST_LINE("-100"_bi - "-30"_bi, "-70"_bi);
    TEST_LINE("-100"_bi - "30"_bi, "-130"_bi);

    {
        BigInt a{50};
        TEST_LINE(a -= 70_bi, -20_bi);
        TEST_LINE(a--, -20_bi);
        TEST_LINE(--a, -22_bi);
    }

    // multiplication
    TEST_LINE("100"_bi * "10"_bi, "1000"_bi);
    TEST_LINE("10"_bi * "100"_bi, "1000"_bi);
    TEST_LINE(-10_bi * 100_bi, -1000_bi);
    TEST_LINE(10_bi * -100_bi, -1000_bi);
    TEST_LINE(-10_bi * -100_bi, 1000_bi);
    TEST_LINE("99"_bi * "999"_bi, "98901"_bi);
    TEST_LINE(999_bi * 0_bi, 0_bi);

    // division
    TEST_LINE("100"_bi / "10"_bi, "10"_bi);
    TEST_LINE(-100_bi / 10_bi, -10_bi);
    TEST_LINE(100_bi / -10_bi, -10_bi);
    TEST_LINE(-100_bi / -10_bi, 10_bi);
    TEST_LINE(10_bi / 100_bi, 0_bi);
    TEST_LINE(8005672_bi / 2000_bi, 4002_bi);
    TEST_THROW_LINE([]() { 100_bi / 0_bi; });

    // modulo
    TEST_LINE(8005672_bi % 2000_bi, 1672_bi);
    TEST_LINE(100_bi % 100_bi, 0_bi);
    TEST_LINE(50_bi % 100_bi, 50_bi);

    // power
    TEST_LINE(12_bi ^ 2_bi, 144_bi);
    TEST_LINE(3_bi ^ 3_bi, 27_bi);
    TEST_LINE(3_bi ^ 7_bi, 2187_bi);
    TEST_LINE(12_bi ^ -2_bi, 0_bi);

    std::cout << std::boolalpha << "all tests passed: " << allTestsPassed << std::endl;
}

void main() { testBigInt(); }