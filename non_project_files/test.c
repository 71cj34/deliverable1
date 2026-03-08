#include <stdint.h>
#include <math.h>
#include <stdio.h>

int main(void) {
    float degrees = remainderf(191.25, 360.0f);

    printf("Result: %f\n", degrees);

    return 0;
}
