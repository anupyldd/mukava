
#include <iostream>
#include <print>

import test;

int main()
{
    using namespace std;
    using namespace test;

    Registry regs;

    regs.Suite("Test")
        .Add(
            Test("loc")
            .Func([]
            {
                return Report{} &&
                    Equal(1, 1) &&
                    Equal(1, 2) &&
                    Equal(1.2, 1.4) &&
                    Throws([]{ throw "a"; }) &&
                    Throws([]{}) &&
                    DoesNotThrow([]{}) &&
                    DoesNotThrow([]{ throw "a"; });
            })
        );

    regs.Run();

    return 0;
}