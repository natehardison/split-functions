#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

int

Winky()
{
    return 7;
} float
Dinky()
{
    return (float)Winky();
}

void Binky(char *s) { float f = Dinky(); printf("%g %s\n", f, s); } void Slinky() { printf("WOOOO\n"); }

int
main(int argc, const char *argv[])
{
    char *s = "This code has the OLD version of main()";
    Binky(s);
    return 0;
}