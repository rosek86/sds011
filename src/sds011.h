#ifndef SDS011_H__
#define SDS011_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SDS011_MAX_DATA_SIZE 15

typedef struct {
  uint8_t state;
  uint8_t data_len;
  uint8_t data_iter;
  uint8_t data_crc;
  uint8_t data[SDS011_MAX_DATA_SIZE];
  uint32_t error;
} sds011_parser_t;

typedef enum {
  SDS011_PARSER_RES_RUNNING,
  SDS011_PARSER_RES_READY,
  SDS011_PARSER_RES_ERROR,
} sds011_parser_ret_t;

typedef enum {
  SDS011_PARSER_ERR_FRAME_BEG,
  SDS011_PARSER_ERR_CMD,
  SDS011_PARSER_ERR_CRC,
  SDS011_PARSER_ERR_FRAME_END,
} sds011_parser_err_t;

sds011_parser_ret_t sds011_parser_parse(sds011_parser_t *parser, uint8_t byte);
void sds011_parser_clear(sds011_parser_t *parser);

#ifdef __cplusplus
}
#endif

#endif // SDS011_H__
