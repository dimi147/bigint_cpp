#include <cstdint>
#include <iostream>
#include <string>
#include <utility>

class BigInt final {
public:
    // constructors
    explicit BigInt(int64_t num);
    explicit BigInt(const std::string& s);

    // copy/move
    BigInt(const BigInt& r);
    BigInt(BigInt&& r);
    auto operator=(const BigInt& rh) & -> BigInt&;
    auto operator=(BigInt&& rh) & -> BigInt&;

    // destructors
    ~BigInt();

    // Arithmetic operators
    BigInt operator+(const BigInt& rh);
    BigInt& operator+=(const BigInt& rh) &;
    BigInt& operator++() &;
    BigInt operator++(int) &;

    BigInt operator-(const BigInt& rh);
    BigInt& operator-=(const BigInt& rh) &;
    BigInt& operator--() &;
    BigInt operator--(int) &;

    BigInt operator*(const BigInt& rh);
    BigInt& operator*=(const BigInt& rh) &;

    BigInt operator/(const BigInt& rh);
    BigInt& operator/=(const BigInt& rh) &;

    BigInt operator%(const BigInt& rh);
    BigInt& operator%=(const BigInt& rh) &;

    BigInt operator^(const BigInt& rh);
    BigInt& operator^=(const BigInt& rh) &;

    // Unary operators
    auto operator-() -> BigInt;

    // Comparison operators
    auto operator==(const BigInt& rh) const -> bool;
    auto operator!=(const BigInt& rh) const -> bool;
    auto operator<(const BigInt& rh) const -> bool;
    auto operator>(const BigInt& rh) const -> bool;
    auto operator<=(const BigInt& rh) const -> bool;
    auto operator>=(const BigInt& rh) const -> bool;

    // Stream operators
    friend std::ostream& operator<<(std::ostream& os, const BigInt& n);
    friend std::istream& operator>>(std::istream& is, BigInt& n);

    // Absolute
    auto abs() const -> BigInt;

private:
    auto sumOfNumbers(const BigInt& n1, const BigInt& n2) -> BigInt;
    auto differenceOfNumbers(const BigInt& n1, const BigInt& n2) -> BigInt;
    auto multiplyNumbers(const BigInt& a, const BigInt& b) -> BigInt;
    auto divideNumbers(BigInt a, BigInt b) -> std::pair<BigInt, BigInt>;

    bool m_isNegative = false;
    std::string m_number;
};

auto operator""_bi(const char* s, size_t size) -> BigInt;
auto operator""_bi(unsigned long long int number) -> BigInt;
