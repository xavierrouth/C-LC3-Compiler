#define API_VER 2
#include "framework.h"


void ReturnTest(lc3::sim& sim, Tester& tester, double points) {
    sim.writePC(0x3000);
    sim.setRunInstLimit(50000);
    sim.run();

    tester.verify("Returned 10?", sim.readMem(0xFDFF) == 10, 1);
}

void testBringup(lc3::sim & sim) { }

void testTeardown(lc3::sim & sim) { }

void setup(Tester & tester) { 
    tester.registerTest("Return Test", ReturnTest, 1, false);
}

void shutdown(void) { }
