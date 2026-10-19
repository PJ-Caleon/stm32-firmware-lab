#include <stdio.h>
#include <assert.h>

unsigned int extract_field(unsigned int val, int start, int width){
  unsigned int mask = (1U << width) -1;
  return (val >> start) & mask;
}

unsigned int insert_field(unsigned int val, int start, int width, unsigned int field){
  unsigned int mask = ((1U << width) - 1) << start;
  unsigned int cleared_val = val & ~mask;
  unsigned int shifted_field = (field & ((1U << width) - 1)) << start;
  return cleared_val | shifted_field;
}

unsigned int set_bits(unsigned int val, int start, int width){
  unsigned int mask = ((1U << width) - 1) << start;
  return val | mask;
}

int main(){
  /* 
     looks at a chunk of the bit and reads it
     val = 0b1101,start = 1, end = 2, it will read the bits from index 1 to 3 
     which is 0b10
   */
  assert(extract_field(0b1101,1,2) == 0b10);

  /*
    it replace the value before that index 
    val = 0b0000
    insert: 0b11 at position 1 with width of 2
    then output is: 0b0110 
  */
  assert(insert_field(0b0000, 1, 2, 0b11) == 0b0110);

  /*
    it turns a specific range of bits into 1 
    val = 0b0000
    Set 2 bits to '1' at position 2-3
    Output: 0b1100
  */
  assert(set_bits(0b0000, 2, 2) == 0b1100);

  printf("All test pass");

  return 0;
}
