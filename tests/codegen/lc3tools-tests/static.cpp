#define RET_LOCATION 0xFDFF
#define API_VER 2
#include "framework.h"

int accumulate() {
    static int x = 4;
    x = x + 1;
    return x;
}

void StaticTest(lc3::sim& sim, Tester& tester, double points) {
    accumulate();
    accumulate();
    accumulate();
    accumulate();
    int result = accumulate();

    sim.writePC(0x3000);
    sim.setRunInstLimit(50000);
    sim.run();

    tester.verify("Returned correct value?", sim.readMem(0xFDFF) == result, 1);
}

void testBringup(lc3::sim & sim) { }

void testTeardown(lc3::sim & sim) { }

void setup(Tester & tester) { 
    tester.registerTest("Static Test", StaticTest, 1, false);
}

void shutdown(void) { }