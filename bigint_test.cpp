#include "bigint.h"

#include <sstream>

#define TEST_LINE(x, y) test(x, y, __LINE__)
#define TEST_THROW_LINE(x) testThrow(x, __LINE__)

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
            std::cout << "test " << line << ": error :: no exception raised" << std::endl;
            allTestsPassed = false;
        } catch (std::exception& e) {
            std::cout << "test " << line << ": passed :: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "test " << line << ": error :: unknown exception raised" << std::endl;
            allTestsPassed = false;
        }
    };

    // construction
    TEST_THROW_LINE([]() { BigInt{""}; });
    TEST_THROW_LINE([]() { BigInt{"-"}; });
    TEST_THROW_LINE([]() { BigInt{"227f"}; });

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
    TEST_LINE(200_bi - 99_bi, 101_bi);

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
    {
        BigInt a{3};
        TEST_LINE(a *= 2_bi, 6_bi);
        TEST_LINE(a *= -3_bi, -18_bi);
        TEST_LINE(a, -18_bi);
    }

    // division
    TEST_LINE("100"_bi / "10"_bi, "10"_bi);
    TEST_LINE(-100_bi / 10_bi, -10_bi);
    TEST_LINE(100_bi / -10_bi, -10_bi);
    TEST_LINE(-100_bi / -10_bi, 10_bi);
    TEST_LINE(10_bi / 100_bi, 0_bi);
    TEST_LINE(8005672_bi / 2000_bi, 4002_bi);
    TEST_LINE(100_bi / 2_bi, 50_bi);
    TEST_THROW_LINE([]() { 100_bi / 0_bi; });
    {
        BigInt a{100};
        TEST_LINE(a /= 2_bi, 50_bi);
        TEST_LINE(a, 50_bi);
        TEST_THROW_LINE([&a]() { a /= 0_bi; });
        TEST_LINE(a /= 100_bi, 0_bi);
    }

    // modulo
    TEST_LINE(8005672_bi % 2000_bi, 1672_bi);
    TEST_LINE(100_bi % 100_bi, 0_bi);
    TEST_LINE(50_bi % 100_bi, 50_bi);
    {
        BigInt a{100};
        TEST_LINE(a %= 60_bi, 40_bi);
        TEST_LINE(a %= 50_bi, 40_bi);
        TEST_LINE(a, 40_bi);
        TEST_THROW_LINE([&a]() { a %= 0_bi; });
    }

    // power
    TEST_LINE(12_bi ^ 2_bi, 144_bi);
    TEST_LINE(3_bi ^ 3_bi, 27_bi);
    TEST_LINE(3_bi ^ 7_bi, 2187_bi);
    TEST_LINE(12_bi ^ -2_bi, 0_bi);
    TEST_LINE(-3_bi ^ 2_bi, 9_bi);
    TEST_LINE(-3_bi ^ 3_bi, -27_bi);
    TEST_LINE(-3_bi ^ 12_bi, 531441_bi);
    TEST_LINE(-3_bi ^ 13_bi, -1594323_bi);
    {
        BigInt a{-2};
        TEST_LINE(a ^= 3_bi, -8_bi);
        TEST_LINE(a ^= 2_bi, 64_bi);
        TEST_LINE(a, 64_bi);
    }

    // stream
    {
        std::stringstream ss{};
        ss << "-100"_bi;
        auto passed = (std::stoi(ss.str()) == -100);
        std::cout << "test " << __LINE__ << (passed ? ": passed" : ": failed") << std::endl;
        allTestsPassed |= passed;
    }
    {
        std::stringstream ss1{"5678"};
        std::stringstream ss2{"-4242"};
        auto n = "1234"_bi;
        ss1 >> n;
        auto passed = (n == "12345678"_bi);
        std::cout << "test " << __LINE__ << (passed ? ": passed" : ": failed") << std::endl;
        ss2 >> n;
        passed |= (n == "-4242"_bi);
        std::cout << "test " << __LINE__ << (passed ? ": passed" : ": failed") << std::endl;
        allTestsPassed |= passed;
    }
    TEST_THROW_LINE([]() {
        auto n = "123"_bi;
        std::stringstream ss{"45f"};
        ss >> n;
    });

    std::cout << "\n\n" << std::boolalpha << "All tests passed: " << allTestsPassed << "\n\n";
}

void main() { testBigInt(); }