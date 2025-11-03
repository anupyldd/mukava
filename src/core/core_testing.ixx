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

    Registry coreReg;

    auto TestMath() -> void
    {
        using namespace math;

        coreReg .Suite("Core::Math")
                .Add(
                    Test("AlmostEqual")
                    .Func([]
                    {
                        return Report{}

                        // sanity checks
                        && True(AlmostEqual(1.0f, 1.0f))
                        && True(AlmostEqual(1.0, 1.0))
                        && True(AlmostEqual(0.0f, 0.0f))
                        && True(AlmostEqual(0.0, 0.0))

                        && False(AlmostEqual(1.0f, 2.0f))
                        && False(AlmostEqual(1.0, 2.0))

                        // small representable differences
                        && True(AlmostEqual(1.0f, 1.0f + 1e-6f))
                        && False(AlmostEqual(1.0, 1.0 + 1e-12))

                        // slightly too large differences
                        && False(AlmostEqual(1.0f, 1.0f + 1e-3f))
                        && False(AlmostEqual(1.0, 1.0 + 1e-8))

                        // very small numbers
                        && True(AlmostEqual(1e-40f, 2e-40f))
                        && True(AlmostEqual(1e-320, 2e-320))
                        && False(AlmostEqual(1e-40f, 1e-30f))

                        // opposite signs but near zero
                        && False(AlmostEqual(1e-9f, -1e-9f))
                        && False(AlmostEqual(1e-2f, -1e-2f))

                        // large magnitude numbers
                        && True(AlmostEqual(1e8f, 1e8f + 1.0f))
                        && False(AlmostEqual(1e8f, 1e8f + 1e4f))
                        && True(AlmostEqual(1e16, 1e16 + 1.0))
                        && False(AlmostEqual(1e16, 1e16 + 1e8))

                        // nan and infinity
                        && True(AlmostEqual(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()))
                        && False(AlmostEqual(std::numeric_limits<float>::infinity(), 1e30f))
                        && False(AlmostEqual(std::nanf(""), std::nanf("")));
                    })
                );
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

        TestMath();
        coreReg.Run();

        std::println("Core testing finished");
    }
}