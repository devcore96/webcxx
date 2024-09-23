#include <unordered_map>
#include <stdexcept>
#include <iostream>
#include <concepts>
#include <memory>
#include <format>
#include <vector>

#include "../../build/Terminal.h"

// Add a compiler flag to these tests
#define FLAG(...)

// Add a .cpp file to these tests
#define SOURCE(...)

// Add a library to these tests
#define LIBRARY(...)

// Add a library path to these tests
#define LIBRARY_PATH(...)

// Add an include path to these tests
#define INCLUDE_PATH(...)

// Defines the test collection
#define COLLECTION(T) std::unique_ptr<TestSuite> init() { \
    auto suite = std::make_unique<T>(); \
     \

#define END() \
     \
    return suite; \
} \

// Defines a test
#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)
#define TESTCLASSNAME(line) CONCAT(TestClass_, line)
#define _TEST(_name, ...) class TESTCLASSNAME(__LINE__) : public Test { \
public: \
    virtual const char* name() override { return _name; } \
    virtual void run() override __VA_ARGS__ \
}; \
 \
suite->addTest(std::make_shared<TESTCLASSNAME(__LINE__)>()); \

#define TEST(name, ...) _TEST(name, __VA_ARGS__)
#define   IT(name, ...) _TEST(  "it " name, __VA_ARGS__)



// Thrown when an assertion fails
class AssertionFailedException : public std::runtime_error {
public:
    AssertionFailedException(std::string what) : std::runtime_error(what) { }

    AssertionFailedException() = default;
    AssertionFailedException(const AssertionFailedException&)  = default;
    AssertionFailedException(      AssertionFailedException&&) = default;

    AssertionFailedException& operator=(const AssertionFailedException&)  = default;
    AssertionFailedException& operator=(      AssertionFailedException&&) = default;
};



// Assertion helper class
template <class T>
class Expect {
private:
    static inline void throwIfNotTrue(bool condition, std::string message) { if (!condition) throw AssertionFailedException(message); }

    const T& actual;

public:
    Expect(const T& actual) : actual(actual) { }

    void     toBeTrue() requires(std::convertible_to<T, bool>) { throwIfNotTrue( actual, std::format("{} is true",      actual)); }
    void  toNotBeTrue() requires(std::convertible_to<T, bool>) { throwIfNotTrue(!actual, std::format("{} is not true",  actual)); }

    void    toBeFalse() requires(std::convertible_to<T, bool>) { throwIfNotTrue(!actual, std::format("{} is false",     actual)); }
    void toNotBeFalse() requires(std::convertible_to<T, bool>) { throwIfNotTrue( actual, std::format("{} is not false", actual)); }

    template<std::equality_comparable_with<T> U>
    void                   toBe(const U& expected) { throwIfNotTrue(actual == expected, std::format("{} and {} value are the same",      actual, expected)); }

    template<std::equality_comparable_with<T> U>
    void                toNotBe(const U& expected) { throwIfNotTrue(actual != expected, std::format("{} and {} value are not the same",  actual, expected)); }

    template<std::equality_comparable_with<T> U>
    void              toBeEqual(const U& expected) { throwIfNotTrue(actual == expected, std::format("{} and {} value are the same",      actual, expected)); }

    template<std::equality_comparable_with<T> U>
    void           toNotBeEqual(const U& expected) { throwIfNotTrue(actual != expected, std::format("{} and {} value are not the same",  actual, expected)); }

    template<std::totally_ordered_with<T> U>
    void        toBeGreaterThan(const U& expected) { throwIfNotTrue(actual >  expected, std::format("{} is greater than {}",             actual, expected)); }

    template<std::totally_ordered_with<T> U>
    void          toBelLessThan(const U& expected) { throwIfNotTrue(actual <  expected, std::format("{} is less than {}",                actual, expected)); }

    template<std::totally_ordered_with<T> U>
    void toBeGreaterThanOrEqual(const U& expected) { throwIfNotTrue(actual >= expected, std::format("{} is greater than or equal to {}", actual, expected)); }

    template<std::totally_ordered_with<T> U>
    void    toBeLessThanOrEqual(const U& expected) { throwIfNotTrue(actual <= expected, std::format("{} is less than or equal to {}",    actual, expected)); }
};



// A single test
class Test {
private:
    friend class TestSuite;
protected:
    virtual void run() = 0;
    virtual const char* name() = 0;
};

// The test's result
struct TestResult {
    bool success;
    std::string output;
};



// A collection of tests
class TestSuite {
private:
    std::vector<std::shared_ptr<Test>> tests;
    std::unordered_map<std::shared_ptr<Test>, TestResult> results;
    size_t totalFails = 0;

public:
    // Sets up the environment before the first test is run
    virtual void setup() { }

    // Cleans up the environment after the last test has completed
    virtual void cleanup() { }

    // Run before each test
    virtual void beforeEach() { }

    // Run after each test
    virtual void afterEach() { }

    void addTest(std::shared_ptr<Test> test) { tests.push_back(test); }

    size_t  size() { return tests.size(); }
    size_t fails() { return totalFails; }

    void printSummary() {
        bool printed = false;
        
        for (auto& result : results) {
            if (result.second.success) {
                std::cout << Terminal::Color::Green << "[Pass] " << Terminal::Color::Default << result.first->name() << std::endl;
            }
        }

        for (auto& result : results) {
            if (!result.second.success) {
                std::cout << Terminal::Color::Red << "[Fail] " << Terminal::Color::Default << result.first->name() << std::endl;
                std::cout << Terminal::Color::DarkGray << result.second.output << Terminal::Color::Default;
                std::cout << "\n\n";
            }
        }
    }

    int run() {
        setup();
        
        totalFails = 0;

        for (auto& test : tests) {
            beforeEach();

            try {
                test->run();

                results.insert(std::make_pair(test, TestResult { true, "" }));
            } catch (AssertionFailedException& e) {
                totalFails++;

                results.insert(std::make_pair(test, TestResult { false, std::format("Assertion that {} failed.", e.what()) }));
            } catch (std::exception& e) {
                totalFails++;

                results.insert(std::make_pair(test, TestResult { false, std::format("Test failed with exception \"{}\".", e.what()) }));
            } catch (...) {
                totalFails++;

                results.insert(std::make_pair(test, TestResult { false, "Test failed with unknown exception." }));
            }

            afterEach();
        }

        cleanup();

        return totalFails;
    }
};

std::unique_ptr<TestSuite> init();

int main(int argc, const char* argv[]) {
    auto suite = init();

    int fails = suite->run();

    suite->printSummary();

    return fails;
}
