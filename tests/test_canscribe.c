#include "unity.h"
#include "canscribe.h"
#include <stdio.h>
#include <string.h>

void test_serialize() {

    // Set up test data
    struct canscribe_msg msg;
    for(uint8_t i = 0; i < 8; i++) {
        msg.frame.data[i] = i;
    }
    msg.frame.id = 0x123;
    msg.frame.dlc = 8;
    msg.frame.flags = 0x01;

    msg.crc = 0x11223344;

    uint8_t buf[23];
    uint8_t expected[23] = {3, 35, 1, 1, 3, 8, 1, 1, 1, 12, 1, 2, 3, 4, 5, 6, 7, 68, 51, 34, 17, 1, 0};
    serialize(buf, &msg, 21);

    // Assert expected serialized data
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, buf, 23);
}

void setUp() {}
void tearDown() {}


int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_serialize);
    return UNITY_END();
}
