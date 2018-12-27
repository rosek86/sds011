#ifndef SDS011_H__
#define SDS011_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "sds011_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*sds011_cb_t)(sds011_err_t, sds011_msg_t const *);
typedef void (*sds011_sample_cb_t)(sds011_msg_sample_t);

typedef struct {
  struct {
    size_t (*bytes_available)(void);
    uint8_t (*read_byte)(void);
    bool (*send_byte)(uint8_t);
  } serial;

  sds011_sample_cb_t on_sample;
} sds011_init_t;

sds011_err_t sds011_init(sds011_init_t const *init);

sds011_err_t sds011_query_data(uint16_t dev_id, sds011_cb_t cb);

sds011_err_t sds011_set_device_id(uint16_t dev_id, uint16_t new_id, sds011_cb_t cb);

sds011_err_t sds011_set_reporting_mode_active(uint16_t dev_id, sds011_cb_t cb);
sds011_err_t sds011_set_reporting_mode_query(uint16_t dev_id, sds011_cb_t cb);

sds011_err_t sds011_get_reporting_mode(uint16_t dev_id, sds011_cb_t cb);

sds011_err_t sds011_set_sleep_on(uint16_t dev_id, sds011_cb_t cb);
sds011_err_t sds011_set_sleep_off(uint16_t dev_id, sds011_cb_t cb);
sds011_err_t sds011_get_sleep(uint16_t dev_id, sds011_cb_t cb);

sds011_err_t sds011_set_op_mode_continous(uint16_t dev_id, sds011_cb_t cb);
sds011_err_t sds011_set_op_mode_periodic(uint16_t dev_id, uint8_t ival, sds011_cb_t cb);
sds011_err_t sds011_get_op_mode(uint16_t dev_id, sds011_cb_t cb);

sds011_err_t sds011_get_fw_ver(uint16_t dev_id, sds011_cb_t cb);

sds011_err_t sds011_process(void);

#ifdef __cplusplus
}
#endif

#endif // SDS011_H__
