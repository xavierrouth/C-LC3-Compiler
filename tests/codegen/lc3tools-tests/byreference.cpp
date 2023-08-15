#define API_VER 2
#include "framework.h"


void ChangeFiveTest(lc3::sim& sim, Tester& tester, double points) {
    sim.writePC(0x3000);
    sim.setRunInstLimit(50000);
    sim.run();

    tester.verify("Returned 5?", sim.readMem(0xFDFF) == 5, 1);
}

void testBringup(lc3::sim & sim) { }

void testTeardown(lc3::sim & sim) { }

void setup(Tester & tester) { 
    tester.registerTest("Change To Five Test", ChangeFiveTest, 1, false);
}

void shutdown(void) { }
