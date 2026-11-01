#include "unity.h"
#include "ring_buffer.h"

#define BUFFER_SIZE 4 // Size should be power of two 

static ring_buffer_t rb;
static char buffer_memory[BUFFER_SIZE];

void setUp(void){
  ring_buffer_init(&rb, buffer_memory, BUFFER_SIZE);
}
void tearDown(){}

void ring_buffer__should_init(void){
  TEST_ASSERT_EQUAL_UINT8(1, ring_buffer_empty(&rb));
  TEST_ASSERT_EQUAL_UINT8(0, ring_buffer_full(&rb));
  TEST_ASSERT_EQUAL_UINT(0, ring_buffer_num_items(&rb));
}

void ring_buffer_queue_dequeue(void){
  char data = 0;

  // Queue one item 
  TEST_ASSERT_EQUAL_UINT8(1, ring_buffer_queue(&rb, 'A'));
  TEST_ASSERT_EQUAL_UINT8(0, ring_buffer_empty(&rb));
  TEST_ASSERT_EQUAL_INT(1, ring_buffer_num_items(&rb));

  // Dequeue item 
  TEST_ASSERT_EQUAL_UINT8(1, ring_buffer_dequeue(&rb, &data));
  TEST_ASSERT_EQUAL_CHAR('A', data);
  TEST_ASSERT_EQUAL_UINT8(1, ring_buffer_empty(&rb));
  TEST_ASSERT_EQUAL_INT(0, ring_buffer_num_items(&rb));

  // Dequeue on an empty buffer will fail 
  TEST_ASSERT_EQUAL_UINT8(0, ring_buffer_dequeue(&rb, &data));
}

void test_ring_buffer_full(void) {
    // BUFFER_SIZE is 4, so maximum capacity is 3 items
    TEST_ASSERT_EQUAL_UINT8(1, ring_buffer_queue(&rb, '1'));
    TEST_ASSERT_EQUAL_UINT8(1, ring_buffer_queue(&rb, '2'));
    TEST_ASSERT_EQUAL_UINT8(1, ring_buffer_queue(&rb, '3'));
    
    TEST_ASSERT_EQUAL_UINT8(1, ring_buffer_full(&rb));
    TEST_ASSERT_EQUAL_INT(3, ring_buffer_num_items(&rb));

    // Queuing when full should fail and return 0
    TEST_ASSERT_EQUAL_UINT8(0, ring_buffer_queue(&rb, '4'));
}

void test_ring_buffer_queue_dequeue_arr(void) {

    const char *test_data = "XYZ";
    char read_data[4] = {0};

    // Queue array of bytes
    ring_buffer_queue_arr(&rb, test_data, 3);
    TEST_ASSERT_EQUAL_INT(3, ring_buffer_num_items(&rb));
    TEST_ASSERT_EQUAL_UINT8(1, ring_buffer_full(&rb));

    // Dequeue array of bytes
    ring_buffer_size_t count = ring_buffer_dequeue_arr(&rb, read_data, 3);
    TEST_ASSERT_EQUAL_INT(3, count);
    TEST_ASSERT_EQUAL_STRING_LEN("XYZ", read_data, 3);
    TEST_ASSERT_EQUAL_UINT8(1, ring_buffer_empty(&rb));
}

void test_ring_buffer_peek(void) {
    ring_buffer_queue(&rb, 'H');
    ring_buffer_queue(&rb, 'I');

    char peeked = 0;
    // Peek at index 0 (oldest item)
    TEST_ASSERT_EQUAL_UINT8(1, ring_buffer_peek(&rb, &peeked, 0));
    TEST_ASSERT_EQUAL_CHAR('H', peeked);

    // Peek at index 1
    TEST_ASSERT_EQUAL_UINT8(1, ring_buffer_peek(&rb, &peeked, 1));
    TEST_ASSERT_EQUAL_CHAR('I', peeked);

    // Out-of-bounds peek should fail
    TEST_ASSERT_EQUAL_UINT8(0, ring_buffer_peek(&rb, &peeked, 2));
}

void test_ring_buffer_wrap_around(void) {
    char data = 0;

    // Fill buffer to capacity (3 items)
    ring_buffer_queue(&rb, '1');
    ring_buffer_queue(&rb, '2');
    ring_buffer_queue(&rb, '3');

    // Dequeue two items to advance tail index
    ring_buffer_dequeue(&rb, &data);
    TEST_ASSERT_EQUAL_CHAR('1', data);
    ring_buffer_dequeue(&rb, &data);
    TEST_ASSERT_EQUAL_CHAR('2', data);

    // Add two more items to trigger wrap-around of head index
    TEST_ASSERT_EQUAL_UINT8(1, ring_buffer_queue(&rb, '4'));
    TEST_ASSERT_EQUAL_UINT8(1, ring_buffer_queue(&rb, '5'));

    // Verify items come out in correct FIFO order: '3', '4', '5'
    ring_buffer_dequeue(&rb, &data);
    TEST_ASSERT_EQUAL_CHAR('3', data);
    ring_buffer_dequeue(&rb, &data);
    TEST_ASSERT_EQUAL_CHAR('4', data);
    ring_buffer_dequeue(&rb, &data);
    TEST_ASSERT_EQUAL_CHAR('5', data);

    TEST_ASSERT_EQUAL_UINT8(1, ring_buffer_empty(&rb));
}

int main(void){
  UNITY_BEGIN();
  RUN_TEST(ring_buffer__should_init);
  RUN_TEST(ring_buffer_queue_dequeue);
  RUN_TEST(test_ring_buffer_full);
  RUN_TEST(test_ring_buffer_queue_dequeue_arr);
  RUN_TEST(test_ring_buffer_peek);
  RUN_TEST(test_ring_buffer_wrap_around);
  return UNITY_END();
}
