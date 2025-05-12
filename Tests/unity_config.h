#define UNITY_INCLUDE_DOUBLE
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_delay);
    return UNITY_END();
}