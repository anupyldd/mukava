module;

#include <cassert>
#include <print>

export module core_testing;

import types;
import math;
import test;

namespace detail
{
    using namespace test;

    constexpr auto AssertSuccess(const Result& res) noexcept -> void
    {
        return assert(res == true);
    }

    constexpr auto AssertFailure(const Result& res) noexcept -> void
    {
        return assert(res == false);
    }

    auto TestAsserts() -> void
    {
        std::println("Assert testing started");

        AssertSuccess(Equal(1, 1));
        AssertFailure(Equal(1, 2));
        AssertSuccess(Equal(1.1, 1.1));

        AssertFailure(NotEqual(1, 1));
        AssertSuccess(NotEqual(1, 2));
        AssertSuccess(NotEqual(1.12, 1.1));

        AssertSuccess(True(true));
        AssertFailure(True(false));

        AssertFailure(False(true));
        AssertSuccess(False(false));

        {
            constexpr auto a = 10;
            const auto ptr = &a;

            AssertSuccess(Null(nullptr));
            AssertFailure(Null(ptr));

            AssertSuccess(NotNull(ptr));
            AssertFailure(NotNull(nullptr));
        }

        std::println("Assert testing finished successfully");
    }


}

// testing the testing framework
export namespace internal_testing
{
    auto TestCore() -> void
    {
        using namespace detail;

        std::println("Core testing started");

        TestAsserts();

        std::println("Core testing finished");
    }
}