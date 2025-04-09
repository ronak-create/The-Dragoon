#include <stdio.h>

int main() {
    int t0 = 0, t1 = 0, num = 0, hex = 0, bin = 0;
    const char* str = "";

    num = 42;
    hex = 0x1F;
    bin = 0b01010;
    str = "Hello World";
    t0 = num;
    if (!t0) { };
    printf("Hello World");
L0:
    return 0;
}
