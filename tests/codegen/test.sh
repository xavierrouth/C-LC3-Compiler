#!/bin/bash

# 1) Copy all lc3tools-tests to LC3_TOOLS_ROOT/src/test/tests
# 2) Re-build lc3tools (cmake and make)
# 3) Run the unit tests with the corresponding input in codegen/input
# 4) yay!
# Must be a better way to do this
CODEGEN_TEST_DIR=$C_LC3_ROOT/tests/codegen

cp -a $CODEGEN_TEST_DIR/lc3tools-tests/. $LC3_TOOLS_ROOT/src/test/tests/

(cd $LC3_TOOLS_ROOT/build/ &&  cmake -DCMAKE_BUILD_TYPE=Release .. && make)

for filename in $CODEGEN_TEST_DIR/input/*.c; do
    # Run compiler on input to get asm
    $C_LC3_ROOT/build/main $filename "$C_LC3_ROOT/out/out.asm"

    # Run test on output asm
    $LC3_TOOLS_ROOT/build/bin/$(basename $filename .c) $C_LC3_ROOT/out/out.asm
done