#include <stdio.h>
#include <stdint.h>


void inc_time2(float *curr_time, float step) {
    *curr_time+=step;
    if(*curr_time < 0 || *curr_time >= 24.0f) *curr_time = 0.0f;
}

int main() {
    float time=13.0f;
    inc_time2(&time, 10.5f);
    printf("Current time = %f\n", time);
    return 0;
}
