

#define API_VER 2
#include "framework.h"

int test() {
    if (2 > 1) {
        return 3;
    }
    return 6;
}

void IfConditionalTest(lc3::sim& sim, Tester& tester, double points) {
    sim.writePC(0x3000);
    sim.setRunInstLimit(50000);
    sim.run();

    tester.verify("Correct Value", sim.readMem(0xFDFF) == test(), 1);
}

void testBringup(lc3::sim & sim) { }

void testTeardown(lc3::sim & sim) { }

void setup(Tester & tester) { 
    tester.registerTest("Conditional Test", IfConditionalTest, 1, false);
}

void shutdown(void) { }