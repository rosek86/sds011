#ifndef SDS011_H__
#define SDS011_H__

#include <stdint.h>
#include <stdbool.h>

#include "sds011_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint8_t state;
  uint8_t cmd;
  uint8_t data_len;
  uint8_t data_iter;
  uint8_t data_crc;
  uint8_t data[SDS011_MAX_DATA_SIZE];
  uint32_t error;
  sds011_msg_t msg;
} sds011_parser_t;

/**
 * Initialize SDS011 parser
 * @param[in] parser SDS011 parser structure
 */
void sds011_parser_init(sds011_parser_t *parser);

/**
 * @brief Parse single byte coming from SDS011 device.
 *        This function returns SDS011_PARSER_RES_RUNNING until entire packet
 *        is parsed.
 *        Once packet parsing is completed function returns
 *        SDS011_PARSER_RES_READY and sds011_parser_get_msg can be used to
 *        retrieve data message.
 *        On error function returns SDS011_PARSER_RES_ERROR, additional
 *        information about the error can be retrieve using sds011_parser_get_error
 *        function.
 * @param[in] parser SDS011 parser structure
 * @param[in] byte data to be parsed
 * @return parser result
 */
sds011_parser_res_t sds011_parser_parse(sds011_parser_t *parser, uint8_t byte);

/**
 * Get latest message
 * @param[in]  parser SDS011 parser structure
 * @param[out] msg latest message
 */
void sds011_parser_get_msg(sds011_parser_t const *parser, sds011_msg_t *msg);

/**
 * Get latest error
 * @param[in] parser SDS011 parser structure
 * @return latest error
 */
sds011_parser_err_t sds011_parser_get_error(sds011_parser_t const *parser);

#ifdef __cplusplus
}
#endif

#endif // SDS011_H__
