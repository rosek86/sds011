#ifndef SDS011_H__
#define SDS011_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SDS011_MAX_DATA_SIZE 15

typedef enum {
  SDS011_PARSER_RES_RUNNING,
  SDS011_PARSER_RES_READY,
  SDS011_PARSER_RES_ERROR,
} sds011_parser_res_t;

typedef enum {
  SDS011_PARSER_OK,
  SDS011_PARSER_ERR_FRAME_BEG,
  SDS011_PARSER_ERR_CMD,
  SDS011_PARSER_ERR_CRC,
  SDS011_PARSER_ERR_FRAME_END,
  SDS011_PARSER_ERR_INVALID_MSG_TYPE,
  SDS011_PARSER_ERR_INVALID_DATA,
  SDS011_PARSER_ERR_INVALID_CMD
} sds011_parser_err_t;

typedef enum {
  SDS011_MSG_TYPE_REP_MODE  = 2,
  SDS011_MSG_TYPE_DATA      = 4,
  SDS011_MSG_TYPE_DEV_ID    = 5,
  SDS011_MSG_TYPE_SLEEP     = 6,
  SDS011_MSG_TYPE_FW_VER    = 7,
  SDS011_MSG_TYPE_OP_MODE   = 8,
  SDS011_MSG_TYPE_COUNT
} sds011_msg_type_t;

typedef enum {
  SDS011_MSG_OP_GET,
  SDS011_MSG_OP_SET,
} sds011_msg_op_t;

typedef enum {
  SDS011_MSG_SRC_SENSOR,
  SDS011_MSG_SRC_HOST,
} sds011_msg_src_t;

typedef enum {
  SDS011_REP_MODE_ACTIVE,
  SDS011_REP_MODE_QUERY,
} sds011_rep_mode_t;

typedef enum {
  SDS011_SLEEP_ON,
  SDS011_SLEEP_OFF,
} sds011_sleep_t;

typedef enum {
  SDS011_OP_MODE_CONTINOUS,
  SDS011_OP_MODE_INTERVAL
} sds011_op_mode_t;

typedef struct {
  uint8_t value;
} sds011_msg_rep_mode_t;

typedef struct {
  uint16_t pm2_5;
  uint16_t pm10;
} sds011_msg_sample_t;

typedef struct {
  uint16_t new_id;
} sds011_msg_dev_id_t;

typedef struct {
  uint8_t value;
} sds011_msg_sleep_t;

typedef struct {
  uint8_t mode;
  uint8_t interval;
} sds011_msg_op_mode_t;

typedef struct {
  uint8_t year;
  uint8_t month;
  uint8_t day;
} sds011_msg_fw_ver_t;

typedef struct {
  uint16_t          dev_id;
  sds011_msg_type_t type;
  sds011_msg_op_t   op;
  sds011_msg_src_t  src;

  union {
    sds011_msg_rep_mode_t rep_mode;
    sds011_msg_sample_t   sample;
    sds011_msg_dev_id_t   dev_id;
    sds011_msg_sleep_t    sleep;
    sds011_msg_fw_ver_t   fw_ver;
    sds011_msg_op_mode_t  op_mode;
  } data;
} sds011_msg_t;

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
