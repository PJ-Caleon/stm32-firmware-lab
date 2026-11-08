#include "unity.h"
#include <stdint.h>
#include <string.h>
#include "parser.h"
#include "crc.h"

void setUp(void){}
void tearDown(void){}

static void test_parser_fuzz_garbage(void) {
    packet_parser parser;
    decode_parser pkt;
    parser_init(&parser);

    // Feed 1,000 random garbage bytes followed by a zero delimiter
    // Verification: Must never crash, lock up, or return a valid packet
    for (int i = 0; i < 1000; i++) {
        int res = parser_feed(&parser, (uint8_t)(i * 37), &pkt);
        TEST_ASSERT_TRUE(res <= 0); 
    }
    int res = parser_feed(&parser, 0x00, &pkt);
    TEST_ASSERT_TRUE(res <= 0);
}

static void test_parser_fuzz_million_bytes(void) {
    packet_parser parser;
    decode_parser pkt;
    parser_init(&parser);

    // 1 Million byte pseudo-random stream under ASan
    for (uint32_t i = 0; i < 1000000; i++) {
        int res = parser_feed(&parser, (uint8_t)(i * 37 + 13), &pkt);
        TEST_ASSERT_TRUE(res <= 0); 
    }
    
    // Inject delimiter to ensure state machine flushes cleanly
    int res = parser_feed(&parser, 0x00, &pkt);
    TEST_ASSERT_TRUE(res <= 0);
}

static void test_parser_truncated_frame(void) {
    packet_parser parser;
    decode_parser pkt;
    parser_init(&parser);

    // Partial valid frame cut off before completing CRC/delimiter
    uint8_t truncated[] = {0x04, 0x01, 0x05, 0x04, 'T', 'E'}; 
    for (size_t i = 0; i < sizeof(truncated); i++) {
        int res = parser_feed(&parser, truncated[i], &pkt);
        TEST_ASSERT_EQUAL_INT(0, res); // Incomplete frame accumulating
    }
    
    // Premature zero frame delimiter -> Should fail decode & reset cleanly
    int res = parser_feed(&parser, 0x00, &pkt);
    TEST_ASSERT_EQUAL_INT(-1, res); 
}

int main(void){
  UNITY_BEGIN();
  RUN_TEST(test_parser_fuzz_garbage);
  RUN_TEST(test_parser_fuzz_million_bytes);
  RUN_TEST(test_parser_truncated_frame);
  return UNITY_END();
}
