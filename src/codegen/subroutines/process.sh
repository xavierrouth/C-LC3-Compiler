xxd -i multiply.asm | sed 's/\([0-9a-f]\)$/\0, 0x00/' > multiply.h