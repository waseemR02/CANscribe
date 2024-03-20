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

    uint8_t buf[sizeof(struct canscribe_msg) + 2];
    uint8_t expected[sizeof(struct canscribe_msg) + 2] = {3, 35, 1, 1, 3, 8, 1, 1, 1, 12, 1, 2, 3, 4, 5, 6, 7, 68, 51, 34, 17, 0};
    serialize(buf, &msg, sizeof(struct canscribe_msg));

    // Assert expected serialized data
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, buf, sizeof(buf));
}

void test_deserialize() {
    // Set up test data
    uint8_t serialized_buf[sizeof(struct canscribe_msg) + 2] = {3, 35, 1, 1, 3, 8, 1, 1, 1, 12, 1, 2, 3, 4, 5, 6, 7, 68, 51, 34, 17, 0};

    struct canscribe_msg expected_msg;
    for(uint8_t i = 0; i < 8; i++) {
        expected_msg.frame.data[i] = i;
    }
    expected_msg.frame.id = 0x123;
    expected_msg.frame.dlc = 8;
    expected_msg.frame.flags = 0x01;

    expected_msg.crc = 0x11223344;
    
    struct canscribe_msg msg;
    deserialize(serialized_buf, &msg, sizeof(msg));

    // Assert expected deserialized data
    TEST_ASSERT_EQUAL_UINT8_ARRAY (expected_msg.frame.data, msg.frame.data, 8);
    TEST_ASSERT_EQUAL_HEX32 (expected_msg.frame.id, msg.frame.id);
    TEST_ASSERT_EQUAL_UINT8 (expected_msg.frame.dlc, msg.frame.dlc);
    TEST_ASSERT_EQUAL_UINT8 (expected_msg.frame.flags, msg.frame.flags);
    
    TEST_ASSERT_EQUAL_HEX32 (expected_msg.crc, msg.crc);

}

void setUp() {}
void tearDown() {}


int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_serialize);
    RUN_TEST(test_deserialize);
    return UNITY_END();
}
