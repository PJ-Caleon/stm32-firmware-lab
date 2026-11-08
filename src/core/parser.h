#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint8_t buffer[128];
  size_t index;
} packet_parser;

typedef struct {
  uint8_t type;
  uint8_t seq;
  uint8_t length;
  uint8_t payload[64];
} decode_parser;

void parser_init(packet_parser *parser);
int parser_feed(packet_parser *parser, uint8_t byte, decode_parser *pkt);

#endif 
