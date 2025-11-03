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
                    }),

                    Test("Overlap")
                    .Func([]
                    {
                        return Report{}
                        && True(Overlap(0, 10, 5, 15))
                        && True(Overlap(0, 10, 10, 20))
                        && False(Overlap(0, 10, 11, 20))
                        && True(Overlap(5, 10, 0, 15))
                        && False(Overlap(-5, -1, -10, -6))
                        && True(Overlap(-5, 5, 4, 6));
                    }),

                    Test("ScalingFactor")
                    .Func([]
                    {
                        return Report{}
                        && Equal(ScalingFactor(0.0), 1.0)
                        && Equal(ScalingFactor(0.5), 1.5)
                        && Equal(ScalingFactor(-0.5), (1 / (1 - (-0.5))))
                        && Equal(ScalingFactor(1.0), 2.0)
                        && Equal(ScalingFactor(-0.75), (1 / 1.75))
                        && Equal(ScalingFactor(10.0), 11.0)
                        && Equal(ScalingFactor(-0.9999), (1 / 1.9999))
                        && Equal(ScalingFactor(-1.0), 0.5);
                    }),

                    Test("LinearValue")
                    .Func([]
                    {
                        return Report{}
                        && Equal(LinearValue(1.0), 0.0)
                        && Equal(LinearValue(1.5), 0.5)
                        && Equal(LinearValue(2.0), 1.0)
                        && Equal(LinearValue(0.5), -1.0)
                        && Equal(LinearValue(11.0), 10.0);
                    }),

                    Test("Average")
                    .Func([]
                    {
                        return Report{}
                        // average of two
                        && Equal(Average(2, 4), 3.0f)
                        && Equal(Average(2.0, 4.0), 3.0)
                        && Equal(Average(2.0f, 4.0f), 3.0f)
                        && Equal(Average(-2, 2), 0.0f)
                        && Equal(Average(0, 1), 0.5f)
                        && Equal(Average(1'000'000'000, 1'000'000'002), 1'000'000'001.0f)
                        && Equal(Average(1e30, 1e30), 1e30)

                        // average of three
                        && Equal(Average(1, 2, 3), 2.0f)
                        && Equal(Average(1.0, 2.0, 3.0), 2.0)
                        && Equal(Average(-3, 0, 3), 0.0f)
                        && Equal(Average(0u, 2u, 4u), 2.0f)
                        && Equal(Average(1.5f, 2.5f, 3.5f), 2.5f)
                        && Equal(Average(1e12, 1e12, 1e12), 1e12)

                        // average of list
                        && Equal(Average({1, 2, 3, 4}), 2.5f)
                        && Equal(Average({1.0, 2.0, 3.0, 4.0}), 2.5)
                        && Equal(Average({0}), 0.0f)
                        && Equal(Average({-1, -2, -3}), -2.0f)
                        && Equal(Average({100, 200}), 150.0f)
                        && Equal(Average({1.5f, 2.5f, 3.5f}), 2.5f)
                        && Equal(Average({1e9, 2e9, 3e9}), 2e9)
                        && Equal(Average({1e300, 1e300, 1e300}), 1e300);
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