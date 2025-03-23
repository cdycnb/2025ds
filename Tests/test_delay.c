#include "unity.h"
#include "mock_delay.h"

void setUp(void) {
    // 在每个测试之前运行的代码
}

void tearDown(void) {
    // 在每个测试之后运行的代码
}

void test_delay_function(void) {
    // 设置 mock 函数的期望值
    delay_Expect(1000);
    
    // 调用要测试的函数
    delay(1000);
    
    // 验证 mock 函数是否被正确调用
    delay_Verify();
}