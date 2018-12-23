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
  uint8_t cmd;
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

typedef enum {
  SDS011_MSG_TYPE_REP_MODE          = 2,
  SDS011_MSG_TYPE_DATA              = 4,
  SDS011_MSG_TYPE_DEV_ID            = 5,
  SDS011_MSG_TYPE_SLEEP             = 6,
  SDS011_MSG_TYPE_FIRMWARE_VERSION  = 7,
  SDS011_MSG_TYPE_ON_PERIOD         = 8,
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
  SDS011_MSG_REP_ACTIVE,
  SDS011_MSG_REP_QUERY,
} sds011_msg_rep_t;

typedef struct {
  uint16_t pm2_5;
  uint16_t pm10;
} sds011_msg_sample_t;

typedef struct {
  uint16_t dev_id;
} sds011_msg_dev_id_t;

typedef struct {
  uint8_t sleep;
} sds011_msg_sleep_t;

typedef struct {
  uint8_t op_mode;
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
    uint8_t               rep_mode;
    sds011_msg_sample_t   sample;
    sds011_msg_dev_id_t   dev_id;
    sds011_msg_sleep_t    sleep;
    sds011_msg_op_mode_t  op_mode;
    sds011_msg_fw_ver_t   fw_ver;
  } data;
} sds011_msg_t;

sds011_parser_ret_t sds011_parser_parse(sds011_parser_t *parser, uint8_t byte);
bool sds011_parser_get_msg(sds011_parser_t const *parser, sds011_msg_t *msg);

void sds011_parser_clear(sds011_parser_t *parser);

sds011_parser_err_t sds011_parser_get_error(sds011_parser_t const *parser);

#ifdef __cplusplus
}
#endif

#endif // SDS011_H__
