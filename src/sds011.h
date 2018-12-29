#ifndef SDS011_H__
#define SDS011_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "sds011_common.h"
#include "sds011_parser.h"
#include "sds011_builder.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*sds011_cb_t)(sds011_err_t, sds011_msg_t const *);
typedef void (*sds011_sample_cb_t)(sds011_msg_t const *, void *);

typedef struct {
  uint32_t msg_timeout;
  uint32_t msg_send_timeout;

  uint32_t (*millis)(void);

  struct {
    size_t (*bytes_available)(void *user_data);
    uint8_t (*read_byte)(void *user_data);
    bool (*send_byte)(uint8_t, void *user_data);
    void *user_data;
  } serial;
} sds011_init_t;

typedef struct {
  sds011_msg_type_t msg_type;
  uint32_t start_time;
  sds011_cb_t cb;
} sds011_query_req_t;

typedef struct {
  sds011_sample_cb_t callback;
  void *user_data;
} sds011_on_sample_t;

typedef struct {
  sds011_init_t cfg;
  sds011_parser_t parser;
  sds011_query_req_t req;
  sds011_on_sample_t on_sample;
} sds011_t;

/**
 * Initialize dust sensor module
 * @param self pointer to the sensor instance
 * @param init initialization structure
 * @return SDS011_OK on success, otherwise error code
 */
sds011_err_t sds011_init(sds011_t *self, sds011_init_t const *init);

/**
 * Set on sample callback, the callback is called when new sample is received.
 * @param self pointer to the sensor instance
 * @param cb on sample callback
 * @param user_data pointer to a data passed with each call.
 */
sds011_err_t sds011_set_sample_callback(sds011_t *self, sds011_sample_cb_t cb, void *user_data);

/**
 * Query dust sensor data
 * @param self pointer to the sensor instance
 * @param dev_id sensor id
 * @param cb callback executed on sensor response or when error occurs
 * @return SDS011_OK on success, otherwise error code
 */
sds011_err_t sds011_query_data(sds011_t *self, uint16_t dev_id, sds011_cb_t cb);

/**
 * Set dust sensor device id
 * @param self pointer to the sensor instance
 * @param dev_id sensor id
 * @param new_id new sensor id
 * @param cb callback executed on sensor response or when error occurs
 * @return SDS011_OK on success, otherwise error code
 */
sds011_err_t sds011_set_device_id(sds011_t *self, uint16_t dev_id, uint16_t new_id, sds011_cb_t cb);

/**
 * Set automatic (active) reporting mode
 * @param self pointer to the sensor instance
 * @param dev_id sensor id
 * @param cb callback executed on sensor response or when error occurs
 * @return SDS011_OK on success, otherwise error code
 */
sds011_err_t sds011_set_reporting_mode_active(sds011_t *self, uint16_t dev_id, sds011_cb_t cb);

/**
 * Set manual (query) reporting mode
 * @param self pointer to the sensor instance
 * @param dev_id sensor id
 * @param cb callback executed on sensor response or when error occurs
 * @return SDS011_OK on success, otherwise error code
 */
sds011_err_t sds011_set_reporting_mode_query(sds011_t *self, uint16_t dev_id, sds011_cb_t cb);

/**
 * Get reporting mode. The reporting mode can be retrieved from the callback message parameter.
 * @param self pointer to the sensor instance
 * @param dev_id sensor id
 * @param cb callback executed on sensor response or when error occurs
 * @return SDS011_OK on success, otherwise error code
 */
sds011_err_t sds011_get_reporting_mode(sds011_t *self, uint16_t dev_id, sds011_cb_t cb);

/**
 * Turn on the sleep mode
 * @param self pointer to the sensor instance
 * @param dev_id sensor id
 * @param cb callback executed on sensor response or when error occurs
 * @return SDS011_OK on success, otherwise error code
 */
sds011_err_t sds011_set_sleep_on(sds011_t *self, uint16_t dev_id, sds011_cb_t cb);

/**
 * Turn off the sleep mode
 * @param self pointer to the sensor instance
 * @param dev_id sensor id
 * @param cb callback executed on sensor response or when error occurs
 * @return SDS011_OK on success, otherwise error code
 */
sds011_err_t sds011_set_sleep_off(sds011_t *self, uint16_t dev_id, sds011_cb_t cb);

/**
 * Get current sleep state. The sleep state can be retrieved from the callback message parameter.
 * @param self pointer to the sensor instance
 * @param dev_id sensor id
 * @param cb callback executed on sensor response or when error occurs
 * @return SDS011_OK on success, otherwise error code
 */
sds011_err_t sds011_get_sleep(sds011_t *self, uint16_t dev_id, sds011_cb_t cb);

/**
 * Set continous operation mode
 * @param self pointer to the sensor instance
 * @param dev_id sensor id
 * @param cb callback executed on sensor response or when error occurs
 * @return SDS011_OK on success, otherwise error code
 */
sds011_err_t sds011_set_op_mode_continous(sds011_t *self, uint16_t dev_id, sds011_cb_t cb);

/**
 * Set periodic operation mode
 * @param self pointer to the sensor instance
 * @param dev_id sensor id
 * @param ival sample interval in minutes, value should be between 1 and 30
 * @param cb callback executed on sensor response or when error occurs
 * @return SDS011_OK on success, otherwise error code
 */
sds011_err_t sds011_set_op_mode_periodic(sds011_t *self, uint16_t dev_id, uint8_t ival, sds011_cb_t cb);

/**
 * Get current operation mode. The operation mode can be retrieved from the callback message parameter.
 * @param self pointer to the sensor instance
 * @param dev_id sensor id
 * @param cb callback executed on sensor response or when error occurs
 * @return SDS011_OK on success, otherwise error code
 */
sds011_err_t sds011_get_op_mode(sds011_t *self, uint16_t dev_id, sds011_cb_t cb);

/**
 * Get firmware version. The firmware version can be retrieved from the callback message parameter.
 * @param self pointer to the sensor instance
 * @param dev_id sensor id
 * @param cb callback executed on sensor response or when error occurs
 * @return SDS011_OK on success, otherwise error code
 */
sds011_err_t sds011_get_fw_ver(sds011_t *self, uint16_t dev_id, sds011_cb_t cb);

/**
 * Processing function. This function should be called periodically,
 * in the main loop.
 * @param self pointer to the sensor instance
 */
sds011_err_t sds011_process(sds011_t *self);

#ifdef __cplusplus
}
#endif

#endif // SDS011_H__
