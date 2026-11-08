#ifndef PROTOCOL_H
#define PROTOCOL_H

typedef enum {
  MSG_GET_SENSOR = 0x01,
  MSG_SET_RATE    = 0x02,
  MSG_STREAM_DATA = 0x03,
  MSG_ACK         = 0x04,
  MSG_NACK        = 0x05
} msg_type_t;

#define MAX_PAYLOAD_SIZE  64
#define MAX_FRAME_SIZE    128

#endif 
