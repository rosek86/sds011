#ifndef SDS011_COMMON_H__
#define SDS011_COMMON_H__

#include <stdint.h>

#define SDS011_FRAME_BEG 0xAA
#define SDS011_FRAME_END 0xAB

#define SDS011_CMD_QUERY 0xB4
#define SDS011_CMD_REPLY 0xC5
#define SDS011_DAT_REPLY 0xC0

#define SDS011_META_DATA_SIZE     4
#define SDS011_QUERY_DATA_SIZE    15
#define SDS011_REPLY_DATA_SIZE    6
#define SDS011_MAX_DATA_SIZE      SDS011_QUERY_DATA_SIZE

#define SDS011_QUERY_PACKET_SIZE  (SDS011_META_DATA_SIZE+SDS011_QUERY_DATA_SIZE)
#define SDS011_REPLY_PACKET_SIZE  (SDS011_META_DATA_SIZE+SDS011_REPLY_DATA_SIZE)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  SDS011_PARSER_RES_RUNNING,
  SDS011_PARSER_RES_READY,
  SDS011_PARSER_RES_ERROR,
} sds011_parser_res_t;

typedef enum {
  SDS011_OK,
  SDS011_ERR_PARSER_FRAME_BEG,
  SDS011_ERR_PARSER_CMD,
  SDS011_ERR_PARSER_CRC,
  SDS011_ERR_PARSER_FRAME_END,
  SDS011_ERR_INVALID_MSG_TYPE,
  SDS011_ERR_INVALID_DATA,
  SDS011_ERR_INVALID_PARAM,
  SDS011_ERR_INVALID_SRC,
  SDS011_ERR_INVALID_REPLY,
  SDS011_ERR_MEM,
  SDS011_ERR_SEND_DATA,
  SDS011_ERR_BUSY,
  SDS011_ERR_TIMEOUT
} sds011_err_t;

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
} sds011_op_mode_type_t;

typedef struct {
  uint16_t pm2_5;
  uint16_t pm10;
} sds011_sample_t;

typedef struct {
  sds011_op_mode_type_t mode;
  uint8_t interval;
} sds011_op_mode_t;

typedef struct {
  uint8_t year;
  uint8_t month;
  uint8_t day;
} sds011_fw_ver_t;

typedef uint16_t sds011_dev_id_t;

typedef struct {
  uint16_t          dev_id;

  sds011_msg_type_t type;
  sds011_msg_op_t   op;
  sds011_msg_src_t  src;

  union {
    sds011_rep_mode_t rep_mode;
    sds011_sample_t   sample;
    sds011_dev_id_t   new_dev_id;
    sds011_sleep_t    sleep;
    sds011_op_mode_t  op_mode;
    sds011_fw_ver_t   fw_ver;
  } data;
} sds011_msg_t;

#ifdef __cplusplus
}
#endif

#endif // SDS011_COMMON_H__
