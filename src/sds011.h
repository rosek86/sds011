#ifndef SDS011_H__
#define SDS011_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SDS011_MAX_PAYLOAD_SIZE 15

typedef struct {
  uint8_t state;
  uint8_t payload_len;
  uint8_t payload[SDS011_MAX_PAYLOAD_SIZE];
} sds011_parser_t;

bool sds011_parser_parse(sds011_parser_t *parser, uint8_t byte);
void sds011_parser_clear(sds011_parser_t *parser);

#ifdef __cplusplus
}
#endif

#endif // SDS011_H__
