#define API_VER 2
#include "framework.h"

int fib(int n) {
    if (2 > n) {
        return n;
    }
    int a = fib(n-1);
    int b = fib(n-2);
    return a + b;
}

void FibonacciTest(lc3::sim& sim, Tester& tester, double points) {
    sim.writePC(0x3000);
    sim.setRunInstLimit(50000);
    sim.run();

    tester.verify("Correct Value", sim.readMem(0xFDFF) == fib(11), 1);
}

void testBringup(lc3::sim & sim) { }

void testTeardown(lc3::sim & sim) { }

void setup(Tester & tester) { 
    tester.registerTest("Fibonacci Test", FibonacciTest, 1, false);
}

void shutdown(void) { }
