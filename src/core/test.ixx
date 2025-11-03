module;

#include <utility>
#include <functional>
#include <string>
#include <algorithm>
#include <exception>
#include <unordered_map>
#include <format>
#include <print>
#include <source_location>
#include <sstream>

#define SRC_LOC_CURR std::source_location loc = std::source_location::current()

#define SUCCESS             Result(true)
#define FAILURE(checkType)  Result(false, #checkType, loc)

export module test;

import types;
import math;

namespace detail
{
    const char* LogIndent = "          ";
}

export namespace test
{
    class Registry; // registry of all tests
    class Suite;    // a test suite for grouping related tests
    class Test;     // a single test
    class Result;   // result of a single check
    class Report;   // combined results of several checks

    class Result
    {
        friend Registry;
        friend Report;

    public:
        Result(
            const bool success,
            std::string type = std::string(),
            const std::source_location& loc = std::source_location())
            : success(success)
        {
            message = std::format("Failed assert: '{}'; File: '{}'; Line: '{}'",
                std::move(type), loc.file_name(), loc.line());
        }

        constexpr operator bool() const
        {
            return success;
        }

    private:
        std::string message;
        bool success = false;
    };

    class Report
    {
        friend Registry;

    public:
        Report operator && (const Result& rhs) const
        {
            Report rep;
            rep.success = (success && rhs.success);
            rep.messages = messages;
            if (!rhs.success) rep.messages.push_back(rhs.message);
            return rep;
        }

        operator bool () const
        {
            return success;
        }

        [[nodiscard]]
        std::string Format() const
        {
            std::stringstream sstr;
            for (const auto& msg : messages)
                sstr << detail::LogIndent << msg << '\n';
            return sstr.str();
        }

    private:
        std::vector<std::string> messages;
        bool success = false;
    };

    class Test
    {
        friend Suite;
        friend Registry;

    public:
        Test(std::string name) : name(std::move(name)) {}

        // add test function
        auto Func(std::function<Report()> func) -> Test&
        {
            test = std::move(func);
            return *this;
        }

        // add setup function (will be called before the test)
        auto Setup(std::function<void()> func) -> Test&
        {
            setup = std::move(func);
            return *this;
        }

        // add teardown function (will be called after the test)
        auto Teardown(std::function<void()> func) -> Test&
        {
            teardown = std::move(func);
            return *this;
        }

    private:
        std::function<void()> setup;
        std::function<Report()> test;
        std::function<void()> teardown;
        std::string name;
    };

    class Suite
    {
        friend Test;
        friend Registry;

    public:
        Suite(std::string name) : name(std::move(name)) {}

        // add setup function (will be called before the start of this suite)
        auto Setup(std::function<void()> func) -> Suite&
        {
            setup = std::move(func);
            return *this;
        }

        // add teardown function (will be called after the end of this suite)
        auto Teardown(std::function<void()> func) -> Suite&
        {
            teardown = std::move(func);
            return *this;
        }

        // add tests to the registry
        template<typename T, typename... Ts>
        requires (std::same_as<std::decay_t<T>, Test> &&
                 (std::same_as<std::decay_t<Ts>, std::decay_t<T>> && ...))
        auto Add(T&& first, Ts&&... rest) -> void
        {
            tests.emplace_back(std::forward<T>(first));
            (tests.emplace_back(std::forward<Ts>(rest)), ...);
        }

    private:
        std::function<void()> setup;
        std::function<void()> teardown;

        std::string name;
        std::vector<Test> tests;
    };

    class Registry
    {
    public:
        // add a test suite to the registry
        auto Suite(std::string name) -> test::Suite&
        {
            return suites.emplace_back(std::move(name));
        }

        // run all tests in all suites and report the results
        auto Run() -> void
        {
            int total  = 0,
                failed = 0;
            std::vector<std::string> fails;

            for (auto& suite : suites)
            {
                if (suite.setup) suite.setup();
                std::println("[  SUITE] {}", suite.name);

                for (auto& test : suite.tests)
                {
                    auto& testName = test.name;

                    if (!test.test)
                    {
                        std::println("[! ERROR] Test '{}': missing the test function", testName);
                        continue;
                    }

                    // setup ----------
                    try
                    {
                        if (test.setup) test.setup();
                    }
                    catch (const std::exception& e)
                    {
                        std::println("[! ERROR] Test '{}': Setup function has thrown an unhandled exception '{}'",
                            testName, e.what());
                        continue;
                    }
                    catch (...)
                    {
                        std::println("[! ERROR] Test '{}': Setup function has thrown an unknown unhandled exception",
                            testName);
                        continue;
                    }

                    // test ----------
                    try
                    {
                        if (auto rep = test.test())
                        {
                            std::println("[   PASS] {}", testName);
                        }
                        else
                        {
                            std::println("[X  FAIL] {} :\n{}", testName, rep.Format());
                            ++failed;
                            fails.push_back(testName);
                        }
                    }
                    catch (const std::exception& e)
                    {
                        std::println("[! ERROR] Test '{}': Test function has thrown an unhandled exception '{}'",
                            testName, e.what());
                        continue;
                    }
                    catch (...)
                    {
                        std::println("[! ERROR] Test '{}': Test function has thrown an unknown unhandled exception '{}'",
                            testName);
                        continue;
                    }

                    // teardown ----------
                    try
                    {
                        if (test.teardown) test.teardown();
                    }
                    catch (const std::exception& e)
                    {
                        std::println("[! ERROR] Test '{}': Teardown function has thrown an unhandled exception '{}'",
                            testName, e.what());
                        continue;
                    }
                    catch (...)
                    {
                        std::println("[! ERROR] Test '{}': Teardown function has thrown an unknown unhandled exception",
                            testName);
                        continue;
                    }

                    ++total;
                }


                if (suite.teardown) suite.teardown();
            }

            std::println("[SUMMARY] Total: {}; Succeeded: {}; Failed: {}",
                total, total - failed, failed);

            if (failed > 0)
            {
                std::println("{}Failed tests:", detail::LogIndent);
                for (const auto& name : fails)
                    std::println("          - {}", name);
            }
        }

    private:
        std::vector<test::Suite> suites;
    };
}

namespace detail
{
    template<typename T>
    [[nodiscard]]
    constexpr auto EqualImpl(const T& actual, const T& expected) -> bool
    {
        if constexpr (types::FloatingPoint<T>)
            return math::AlmostEqual<T>(actual, expected);
        return (actual == expected);
    }

    template<typename F, typename Tuple = std::tuple<>>
    [[nodiscard]]
    constexpr auto ThrowsImpl(F&& func, Tuple&& argsTuple = {}) -> bool
    {
        try
        {
            std::apply(std::forward<F>(func), std::forward<Tuple>(argsTuple));
            return false;
        }
        catch (...)
        {
            return true;
        }
    }

    template<typename T>
    [[nodiscard]]
    constexpr auto NullImpl(T val) -> bool
    {
        return static_cast<bool>(!val);
    }

    template<typename T>
    [[nodiscard]]
    constexpr auto True(T val) -> bool
    {
        return val;
    }
}

export namespace test
{
    // succeeds if actual value is equal to the expected value.
    // for floats/doubles uses math::AlmostEqual function
    template<typename T>
    [[nodiscard]]
    constexpr auto Equal(const T& actual, const T& expected, SRC_LOC_CURR) -> Result
    {
        return detail::EqualImpl(actual, expected) ? SUCCESS : FAILURE(Equal);
    }

    // succeeds if actual value is NOT equal to the expected value.
    // for floats/doubles uses math::AlmostEqual function
    template<typename T>
    [[nodiscard]]
    constexpr auto NotEqual(const T& actual, const T& expected, SRC_LOC_CURR) -> Result
    {
        return detail::EqualImpl(actual, expected) ? FAILURE(NotEqual) : SUCCESS;
    }

    // succeeds if passed F (function, functor, lambda) throws any exception
    template<typename F, typename Tuple = std::tuple<>>
    [[nodiscard]]
    constexpr auto Throws(F&& func, Tuple&& argsTuple = {}, SRC_LOC_CURR) -> Result
    {
        return detail::ThrowsImpl(std::forward<F>(func), std::forward<Tuple>(argsTuple)) ?
            SUCCESS : FAILURE(Throws);
    }

    // succeeds if passed F (function / functor / lambda / ...) does not throw any exception
    template<typename F, typename Tuple = std::tuple<>>
    [[nodiscard]]
    constexpr auto DoesNotThrow(F&& func, Tuple&& argsTuple = {}, SRC_LOC_CURR) -> Result
    {
        return detail::ThrowsImpl(std::forward<F>(func), std::forward<Tuple>(argsTuple)) ?
            FAILURE(DoesNotThrow) : SUCCESS;
    }

    // succeeds if val is null
    template<typename T>
    [[nodiscard]]
    constexpr auto Null(T val, SRC_LOC_CURR) -> Result
    {
        return detail::NullImpl(val) ? SUCCESS : FAILURE(Null);
    }

    // succeeds if val is NOT null
    template<typename T>
    [[nodiscard]]
    constexpr auto NotNull(T val, SRC_LOC_CURR) -> Result
    {
        return detail::NullImpl(val) ? FAILURE(NotNull) : SUCCESS;
    }

    // succeeds if val is true
    template<typename T>
    [[nodiscard]]
    constexpr auto True(T val, SRC_LOC_CURR) -> Result
    {
        return detail::True(val) ? SUCCESS : FAILURE(True);
    }

    // succeeds if val is NOT true
    template<typename T>
    [[nodiscard]]
    constexpr auto False(T val, SRC_LOC_CURR) -> Result
    {
        return detail::True(val) ? FAILURE(False) : SUCCESS;
    }
}