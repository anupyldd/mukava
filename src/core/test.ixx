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

export module test;

import types;
import math;

namespace detail
{
    auto LogIndent = "          ";
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
        Report& operator &= (const Result& rhs)
        {
            success = (success && rhs.success);
            if (!rhs.success) messages.push_back(rhs.message);
            return *this;
        }

        Report& operator & (const Result& rhs)
        {
            success = (success && rhs.success);
            if (!rhs.success) messages.push_back(rhs.message);
            return *this;
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
        bool success = true;
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
                failed = 0,
                errors = 0;
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
                        ++errors;
                        continue;
                    }
                    catch (...)
                    {
                        std::println("[! ERROR] Test '{}': Setup function has thrown an unknown unhandled exception",
                            testName);
                        ++errors;
                        continue;
                    }

                    // test ----------
                    try
                    {
                        auto rep = test.test();
                        if (rep)
                        {
                            std::println("[   PASS] {}", testName);
                        }
                        else
                        {
                            std::println("[X  FAIL] {}:\n{}", testName, rep.Format());
                            ++failed;
                            fails.push_back(testName);
                        }
                    }
                    catch (const std::exception& e)
                    {
                        std::println("[! ERROR] Test '{}': Test function has thrown an unhandled exception '{}'",
                            testName, e.what());
                        ++errors;
                        continue;
                    }
                    catch (...)
                    {
                        std::println("[! ERROR] Test '{}': Test function has thrown an unknown unhandled exception",
                            testName);
                        ++errors;
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
                        ++errors;
                        continue;
                    }
                    catch (...)
                    {
                        std::println("[! ERROR] Test '{}': Teardown function has thrown an unknown unhandled exception",
                            testName);
                        ++errors;
                        continue;
                    }

                    ++total;
                }


                if (suite.teardown) suite.teardown();
            }

            std::println("[SUMMARY] Total: {}; Succeeded: {}; Failed: {}; Errors: {}",
                total, total - failed, failed, errors);

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
    // succeeds if the actual value is equal to the expected value
    // works for floating point numbers as well
    template<typename T>
    [[nodiscard]]
    constexpr auto Equal(const T& actual, const T& expected, const std::source_location& loc = std::source_location::current()) -> Result
    {
        const auto res = (types::Number<T>) ? math::NumericEqual(actual, expected) : (actual == expected);
        return res ? Result(true) : Result(false, std::format("Equal [{}:{}]", actual, expected), loc);
    }

    // succeeds if actual value is NOT equal to the expected value
    // works for floating point numbers as well
    template<typename T>
    [[nodiscard]]
    constexpr auto NotEqual(const T& actual, const T& expected, const std::source_location& loc = std::source_location::current()) -> Result
    {
        const auto res = (types::Number<T>) ? math::NumericEqual(actual, expected) : (actual == expected);
        return res ? Result(false, std::format("NotEqual [{}:{}]", actual, expected), loc) : Result(true);
    }

    // succeeds if passed F (function, functor, lambda) throws any exception
    template<typename F, typename Tuple = std::tuple<>>
    [[nodiscard]]
    constexpr auto Throws(F&& func, Tuple&& argsTuple = {}, const std::source_location& loc = std::source_location::current()) -> Result
    {
        return detail::ThrowsImpl(std::forward<F>(func), std::forward<Tuple>(argsTuple)) ?
            Result(true) : Result(false, "Throws", loc);
    }

    // succeeds if passed F (function / functor / lambda / ...) does not throw any exception
    template<typename F, typename Tuple = std::tuple<>>
    [[nodiscard]]
    constexpr auto DoesNotThrow(F&& func, Tuple&& argsTuple = {}, const std::source_location& loc = std::source_location::current()) -> Result
    {
        return detail::ThrowsImpl(std::forward<F>(func), std::forward<Tuple>(argsTuple)) ?
            Result(false, "DoesNotThrow", loc) : Result(true);
    }

    // succeeds if val is null
    template<typename T>
    [[nodiscard]]
    constexpr auto Null(T val, const std::source_location& loc = std::source_location::current()) -> Result
    {
        return detail::NullImpl(val) ? Result(true) : Result(false, "Null", loc);
    }

    // succeeds if val is NOT null
    template<typename T>
    [[nodiscard]]
    constexpr auto NotNull(T val, const std::source_location& loc = std::source_location::current()) -> Result
    {
        return detail::NullImpl(val) ? Result(false, "NotNull", loc) : Result(true);
    }

    // succeeds if val is true
    template<typename T>
    [[nodiscard]]
    constexpr auto True(T val, const std::source_location& loc = std::source_location::current()) -> Result
    {
        return detail::True(val) ? Result(true) : Result(false, "True", loc);
    }

    // succeeds if val is NOT true
    template<typename T>
    [[nodiscard]]
    constexpr auto False(T val, const std::source_location& loc = std::source_location::current()) -> Result
    {
        return detail::True(val) ? Result(false, "False", loc) : Result(true);
    }
}