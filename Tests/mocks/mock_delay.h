#include "unity.h"
#include "mock_delay.h"

void setUp(void) {
}

void tearDown(void) {
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_delay_function);
    return UNITY_END();
}