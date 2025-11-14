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
                    Test{"AlmostEqual"}
                    .Func([]
                    {
                        auto r = Report{}
                        // sanity checks
                        & True(AlmostEqual(1.0f, 1.0f))
                        & True(AlmostEqual(1.0, 1.0))
                        & True(AlmostEqual(0.0f, 0.0f))
                        & True(AlmostEqual(0.0, 0.0))

                        & False(AlmostEqual(1.0f, 2.0f))
                        & False(AlmostEqual(1.0, 2.0))

                        // small representable differences
                        & True(AlmostEqual(1.0f, 1.0f + 1e-6f))
                        & False(AlmostEqual(1.0, 1.0 + 1e-12))

                        // slightly too large differences
                        & False(AlmostEqual(1.0f, 1.0f + 1e-3f))
                        & False(AlmostEqual(1.0, 1.0 + 1e-8))

                        // very small numbers
                        & True(AlmostEqual(1e-40f, 2e-40f))
                        & True(AlmostEqual(1e-320, 2e-320))
                        & False(AlmostEqual(1e-40f, 1e-30f))

                        // opposite signs but near zero
                        & False(AlmostEqual(1e-9f, -1e-9f))
                        & False(AlmostEqual(1e-2f, -1e-2f))

                        // large magnitude numbers
                        & True(AlmostEqual(1e8f, 1e8f + 1.0f))
                        & False(AlmostEqual(1e8f, 1e8f + 1e4f))
                        & True(AlmostEqual(1e16, 1e16 + 1.0))
                        & False(AlmostEqual(1e16, 1e16 + 1e8))

                        // nan and infinity
                        & True(AlmostEqual(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()))
                        & False(AlmostEqual(std::numeric_limits<float>::infinity(), 1e30f))
                        & False(AlmostEqual(std::nanf(""), std::nanf("")));

                        return r;
                    }),

                    Test{"Overlap"}
                    .Func([]
                    {
                        return Report{}
                        & True(Overlap(0, 10, 5, 15))
                        & True(Overlap(0, 10, 10, 20))
                        & False(Overlap(0, 10, 11, 20))
                        & True(Overlap(5, 10, 0, 15))
                        & False(Overlap(-5, -1, -10, -6))
                        & True(Overlap(-5, 5, 4, 6));
                    }),

                    Test{"ScalingFactor"}
                    .Func([]
                    {
                        return Report{}
                        & Equal(ScalingFactor(0.0), 1.0)
                        & Equal(ScalingFactor(0.5), 1.5)
                        & Equal(ScalingFactor(-0.5), (1 / (1 - (-0.5))))
                        & Equal(ScalingFactor(1.0), 2.0)
                        & Equal(ScalingFactor(-0.75), (1 / 1.75))
                        & Equal(ScalingFactor(10.0), 11.0)
                        & Equal(ScalingFactor(-0.9999), (1 / 1.9999))
                        & Equal(ScalingFactor(-1.0), 0.5);
                    }),

                    Test{"LinearValue"}
                    .Func([]
                    {
                        return Report{}
                        & Equal(LinearValue(1.0), 0.0)
                        & Equal(LinearValue(1.5), 0.5)
                        & Equal(LinearValue(2.0), 1.0)
                        & Equal(LinearValue(0.5), -1.0)
                        & Equal(LinearValue(11.0), 10.0);
                    }),

                    Test{"Average"}
                    .Func([]
                    {
                        return Report{}
                        // average of two
                        & Equal(Average(2, 4), 3.0f)
                        & Equal(Average(2.0, 4.0), 3.0)
                        & Equal(Average(2.0f, 4.0f), 3.0f)
                        & Equal(Average(-2, 2), 0.0f)
                        & Equal(Average(0, 1), 0.5f)
                        & Equal(Average(1'000'000'000, 1'000'000'002), 1'000'000'001.0f)
                        & Equal(Average(1e30, 1e30), 1e30)

                        // average of three
                        & Equal(Average(1, 2, 3), 2.0f)
                        & Equal(Average(1.0, 2.0, 3.0), 2.0)
                        & Equal(Average(-3, 0, 3), 0.0f)
                        & Equal(Average(0u, 2u, 4u), 2.0f)
                        & Equal(Average(1.5f, 2.5f, 3.5f), 2.5f)
                        & Equal(Average(1e12, 1e12, 1e12), 1e12)

                        // average of list
                        & Equal(Average({1, 2, 3, 4}), 2.5f)
                        & Equal(Average({1.0, 2.0, 3.0, 4.0}), 2.5)
                        & Equal(Average({0}), 0.0f)
                        & Equal(Average({-1, -2, -3}), -2.0f)
                        & Equal(Average({100, 200}), 150.0f)
                        & Equal(Average({1.5f, 2.5f, 3.5f}), 2.5f)
                        & Equal(Average({1e9, 2e9, 3e9}), 2e9)
                        & Equal(Average({1e300, 1e300, 1e300}), 1e300);
                    }),

                    Test{"Any"}
                    .Func([]
                    {
                        return Report{}
                        & True(Any({ 1 }))
                        & False(Any({ 0 }))
                        & True(Any({ 1, 0 }))
                        & True(Any({ 0, 1 }))
                        & True(Any({ 1, 1 }))
                        & False(Any({ 0, 0 }))
                        & True(Any({ 0.0, 0.0, 0.0, 0.0, 0.1, 0.0 }));
                    }),

                    Test{"Vector2"}
                    .Func([]
                    {
                        Report rep;

                        // constructors
                        {
                            const Vector2i v1;
                            rep & Equal(v1.x, 0)
                                & Equal(v1.y, 0);

                            const Vector2f v2(1.0f, 2.0f);
                            rep & Equal(v2.x, 1.0f)
                                & Equal(v2.y, 2.0f);

                            const Vector2d v3(1.0);
                            rep & Equal(v3.x, 1.0)
                                & Equal(v3.y, 1.0);

                            const Vector2i v4(v1);
                            rep & Equal(v4.x, v1.x)
                                & Equal(v4.y, v1.y);
                        }

                        // set, zero
                        {
                            Vector2i v1;

                            v1.Set(1, 2);
                            rep & Equal(v1.x, 1)
                                & Equal(v1.y, 2);

                            v1.Set(4);
                            rep & Equal(v1.x, 4)
                                & Equal(v1.y, 4);

                            v1.Zero();
                            rep & Equal(v1.x, 0)
                                & Equal(v1.y, 0);
                        }

                        // magnitude and normalization
                        {
                            Vector2d v1{3.0, 4.0};
                            rep & Equal(v1.MagnitudeSq(), 25.0)
                                & Equal(v1.Magnitude(), 5.0);

                            v1.Normalize();
                            rep & Equal(v1.x, 0.6)
                                & Equal(v1.y, 0.8)
                                & Equal(v1.Magnitude(), 1.0);

                            Vector2f v2{1.0f, 0.0f};
                            rep & Equal(v2.MagnitudeSq(), 1.0f)
                                & Equal(v2.Magnitude(), 1.0f);

                            v2.Normalize();
                            rep & Equal(v2.x, 1.0f)
                                & Equal(v2.y, 0.0f)
                                & Equal(v2.Magnitude(), 1.0f);

                            Vector2d v3{0.0, 0.0};
                            rep & Equal(v3.MagnitudeSq(), 0.0)
                                & Equal(v3.Magnitude(), 0.0);

                            v2.Normalize();
                            rep & Equal(v3.x, 0.0)
                                & Equal(v3.y, 0.0);
                        }

                        // yx
                        {
                            const Vector2i v1{1, 2};
                            const auto v2 = v1.YX();
                            rep & Equal(v2.x, 2)
                                & Equal(v2.y, 1);
                        }

                        // any, min, max, diff
                        {
                            const Vector2i v1{0, 0};
                            rep & False(v1.Any())
                                & Equal(v1.Min(), 0)
                                & Equal(v1.Max(), 0)
                                & False(v1.Different());

                            const Vector2i v2{1, 0};
                            rep & True(v2.Any())
                                & Equal(v2.Min(), 0)
                                & Equal(v2.Max(), 1)
                                & True(v2.Different());

                            const Vector2i v3{1, 1};
                            rep & True(v3.Any())
                                & Equal(v3.Min(), 1)
                                & Equal(v3.Max(), 1)
                                & False(v3.Different());

                            const Vector2i v4{-1, 1};
                            rep & True(v4.Any())
                                & Equal(v4.Min(), -1)
                                & Equal(v4.Max(),  1)
                                & True(v4.Different());
                        }

                        // average
                        {
                            const Vector2f v1{2.0f, 4.0f};
                            rep & Equal(v1.Average(), 3.0f);

                            const Vector2i v2{-2, 2};
                            rep & Equal(v2.Average(), 0.0f);
                        }

                        // dot
                        {
                            const Vector2f v1{1.0f, 0.0f};
                            rep & Equal(v1.Dot(Vector2f{0.0f, 1.0f}), 0.0f);

                            const Vector2i v2{1, 2};
                            rep & Equal(v2.Dot(Vector2i{2, 3}), 8.0f);

                            const Vector2i v3{3, 4};
                            rep & Equal(v3.Dot(Vector2i{3, 4}), 25.0f);

                            const Vector2i v4{-1, 1};
                            rep & Equal(v4.Dot(Vector2i{1, 1}), 0.0f);
                        }

                        // compound assignment
                        {
                            Vector2f v1{1.0f, 2.0f};

                            v1 += 3.0f;
                            rep & Equal(v1.x, 4.0f)
                                & Equal(v1.y, 5.0f);

                            v1 -= 1.0f;
                            rep & Equal(v1.x, 3.0f)
                                & Equal(v1.y, 4.0f);

                            v1 *= 2.0f;
                            rep & Equal(v1.x, 6.0f)
                                & Equal(v1.y, 8.0f);

                            v1 /= 2.0f;
                            rep & Equal(v1.x, 3.0f)
                                & Equal(v1.y, 4.0f);

                            Vector2f v2{2.0f, 3.0f};
                            Vector2f v3{1.0f, 2.0f};

                            v2 += v3;
                            rep & Equal(v2.x, 3.0f)
                                & Equal(v2.y, 5.0f);

                            v2 -= v3;
                            rep & Equal(v2.x, 2.0f)
                                & Equal(v2.y, 3.0f);

                            v2 *= v3;
                            rep & Equal(v2.x, 2.0f)
                                & Equal(v2.y, 6.0f);

                            v2 /= v3;
                            rep & Equal(v2.x, (2.0f / 1.0f))
                                & Equal(v2.y, (6.0f / 2.0f));
                        }

                        // arithmetic
                        {
                            Vector2i v1{2, 4};
                            Vector2i v2;

                            v2 = v1 + 1;
                            rep & Equal(v2.x, 3)
                                & Equal(v2.y, 5);

                            v2 = v1 - 1;
                            rep & Equal(v2.x, 1)
                                & Equal(v2.y, 3);

                            v2 = v1 * 2;
                            rep & Equal(v2.x, 4)
                                & Equal(v2.y, 8);

                            v2 = v1 / 2;
                            rep & Equal(v2.x, 1)
                                & Equal(v2.y, 2);

                            Vector2i v3;

                            v3 = v1 + v2;
                            rep & Equal(v3.x, 3)
                                & Equal(v3.y, 6);

                            v3 = v1 - v2;
                            rep & Equal(v3.x, 1)
                                & Equal(v3.y, 2);

                            v3 = v1 * v2;
                            rep & Equal(v3.x, 2)
                                & Equal(v3.y, 8);

                            v3 = v1 / v2;
                            rep & Equal(v3.x, 2)
                                & Equal(v3.y, 2);
                        }

                        // comparison
                        {
                            Vector2i v1{2, 3};
                            Vector2i v2{2, 3};
                            Vector2i v3{3, 4};

                            rep & True(v1 == v2)
                                & False(v1 != v2)
                                & True(v1 != v3)
                                & True(v1 < v3)
                                & True(v1 <= v2)
                                & True(v3 > v1)
                                & True(v2 >= v1);
                        }

                        return rep;
                    })
/*
                    Test{"Vector3"}
                    .Func([]
                    {
                        Report rep;

                        // constructors
                        {
                            const Vector3i v1;
                            rep & Equal(v1.x, 0)
                                & Equal(v1.y, 0)
                                & Equal(v1.z, 0);

                            const Vector3f v2(1.0f, 2.0f, 3.0f);
                            rep & Equal(v2.x, 1.0f)
                                & Equal(v2.y, 2.0f)
                                & Equal(v2.y, 3.0f);

                            const Vector3d v3(1.0);
                            rep & Equal(v3.x, 1.0)
                                & Equal(v3.y, 1.0)
                                & Equal(v3.z, 1.0);

                            const Vector3i v4(v1);
                            rep & Equal(v4.x, v1.x)
                                & Equal(v4.y, v1.y)
                                & Equal(v4.z, v1.z);
                        }

                        // set, zero
                        {
                            Vector3i v1;

                            v1.Set(1, 2, 3);
                            rep & Equal(v1.x, 1)
                                & Equal(v1.y, 2)
                                & Equal(v1.y, 3);

                            v1.Set(4);
                            rep & Equal(v1.x, 4)
                                & Equal(v1.y, 4)
                                & Equal(v1.z, 4);

                            v1.Zero();
                            rep & Equal(v1.x, 0)
                                & Equal(v1.y, 0)
                                & Equal(v1.z, 0);
                        }

                        // magnitude and normalization
                        {
                            Vector3f v1{0.0f};
                            rep & Equal(v1.MagnitudeSq(), 0.0f)
                                & Equal(v1.Magnitude(), 0.0f);

                            v1.Normalize();
                            rep & Equal(v1.x, 0.0f)
                                & Equal(v1.y, 0.0f)
                                & Equal(v1.z, 0.0f);

                            // ---
                            Vector3d v2{1.0, 0.0, 0.0};
                            rep & Equal(v2.MagnitudeSq(), 1.0)
                                & Equal(v2.Magnitude(), 1.0);

                            v2.Normalize();
                            rep & Equal(v2.x, 1.0)
                                & Equal(v2.y, 0.0)
                                & Equal(v2.z, 0.0);

                            // ---
                            Vector3d v3{3.0, 4.0, 0.0};
                            rep & Equal(v3.MagnitudeSq(), 25.0)
                                & Equal(v3.Magnitude(), 5.0);

                            v2.Normalize();
                            rep & Equal(v3.x, 0.6)
                                & Equal(v3.y, 0.8)
                                & Equal(v3.z, 0.0);

                            // ---
                            Vector3d v4{0.5};
                            rep & Equal(v4.MagnitudeSq(), 0.75)
                                & Equal(v4.Magnitude(), 0.866);

                            v2.Normalize();
                            rep & Equal(v4.x, 0.5773)
                                & Equal(v4.y, 0.5773)
                                & Equal(v4.z, 0.5773);
                        }

                        // any, min, max, diff
                        {
                            const Vector3i v1{0};
                            rep & False(v1.Any())
                                & Equal(v1.Min(), 0)
                                & Equal(v1.Max(), 0)
                                & False(v1.Different());

                            const Vector3i v2{1, 0, 0};
                            rep & True(v2.Any())
                                & Equal(v2.Min(), 0)
                                & Equal(v2.Max(), 1)
                                & True(v2.Different());

                            const Vector3i v3{1};
                            rep & True(v3.Any())
                                & Equal(v3.Min(), 1)
                                & Equal(v3.Max(), 1)
                                & False(v3.Different());

                            const Vector3i v4{-1, 1, 0};
                            rep & True(v4.Any())
                                & Equal(v4.Min(), -1)
                                & Equal(v4.Max(),  1)
                                & True(v4.Different());
                        }

                        // average
                        {
                            const Vector3f v1{2.0f, 4.0f, 6.0f};
                            rep & Equal(v1.Average(), 4.0f);

                            const Vector3i v2{-2, 2, 0};
                            rep & Equal(v2.Average(), 0.0f);
                        }

                        // dot
                        {
                            // parallel
                            const Vector3f v1{1.0f, 0.0f, 0.0f};
                            rep & Equal(v1.Dot(Vector3f{1.0f, 0.0f, 0.0f}), 1.0f);

                            // orthogonal
                            const Vector3f v2{1.0f, 0.0f, 0.0f};
                            rep & Equal(v2.Dot(Vector3f{0.0f, 1.0f, 0.0f}), 0.0f);

                            // opposite
                            const Vector3i v3{-1, -2, -3};
                            rep & Equal(v3.Dot(Vector3i{1, 2, 3}), 14.0f);
                        }

                        // compound assignment
                        {
                            Vector3f v1{1.0f, 2.0f, 3.0f};

                            v1 += 3.0f;
                            rep & Equal(v1.x, 4.0f)
                                & Equal(v1.y, 5.0f)
                                & Equal(v1.z, 6.0f);

                            v1 -= 1.0f;
                            rep & Equal(v1.x, 3.0f)
                                & Equal(v1.y, 4.0f)
                                & Equal(v1.z, 5.0f);

                            v1 *= 2.0f;
                            rep & Equal(v1.x, 6.0f)
                                & Equal(v1.y, 8.0f)
                                & Equal(v1.z, 10.0f);

                            v1 /= 2.0f;
                            rep & Equal(v1.x, 3.0f)
                                & Equal(v1.y, 4.0f)
                                & Equal(v1.z, 5.0f);

                            Vector3f v2{2.0f, 3.0f, 4.0f};
                            Vector3f v3{1.0f, 2.0f, 3.0f};

                            v2 += v3;
                            rep & Equal(v2.x, 3.0f)
                                & Equal(v2.y, 5.0f)
                                & Equal(v2.z, 7.0f);

                            v2 -= v3;
                            rep & Equal(v2.x, 2.0f)
                                & Equal(v2.y, 3.0f)
                                & Equal(v2.z, 4.0f);

                            v2 *= v3;
                            rep & Equal(v2.x, 2.0f)
                                & Equal(v2.y, 6.0f)
                                & Equal(v2.z, 12.0f);

                            v2 /= v3;
                            rep & Equal(v2.x, (2.0f / 1.0f))
                                & Equal(v2.y, (5.0f / 2.0f))
                                & Equal(v2.z, (5.0f / 2.0f));
                        }

                        // arithmetic
                        {
                            Vector3i v1{2, 4};
                            Vector3i v2;

                            v2 = v1 + 1;
                            rep & Equal(v2.x, 3)
                                & Equal(v2.y, 5);

                            v2 = v1 - 1;
                            rep & Equal(v2.x, 2)
                                & Equal(v2.y, 4);

                            v2 = v1 * 2;
                            rep & Equal(v2.x, 4)
                                & Equal(v2.y, 8);

                            v2 = v1 / 2;
                            rep & Equal(v2.x, 2)
                                & Equal(v2.y, 4);

                            Vector3i v3;

                            v3 = v1 + v2;
                            rep & Equal(v3.x, 4)
                                & Equal(v3.y, 8);

                            v3 = v1 - v2;
                            rep & Equal(v3.x, 0)
                                & Equal(v3.y, 0);

                            v3 = v1 * v2;
                            rep & Equal(v3.x, 4)
                                & Equal(v3.y, 16);

                            v3 = v1 / v2;
                            rep & Equal(v3.x, 1)
                                & Equal(v3.y, 1);
                        }

                        // comparison
                        {
                            Vector3i v1{2, 3};
                            Vector3i v2{2, 3};
                            Vector3i v3{3, 4};

                            rep & True(v1 == v2)
                                & False(v1 != v2)
                                & True(v1 != v3)
                                & True(v1 < v3)
                                & True(v1 <= v2)
                                & True(v3 > v1)
                                & True(v2 >= v1);
                        }

                        return rep;
                    })
                    */
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