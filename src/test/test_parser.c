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

int main(void){
  UNITY_BEGIN();
  RUN_TEST(test_parser_fuzz_garbage);
  return UNITY_END();
}
