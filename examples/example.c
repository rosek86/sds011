
#include "../src/sds011.h"

#include <stdio.h>

// Mock prototypes
static uint32_t mock_millis(void);
static size_t mock_bytes_available(void *user_data);
static uint8_t mock_read_byte(void *user_data);
static bool mock_send_byte(uint8_t byte, void *user_data);
// --

// General query callback
static bool _cb_exec = false;
static sds011_err_t _cb_err_code;
static sds011_msg_t _cb_msg;

static void sds011_cb(sds011_err_t err_code, sds011_msg_t const *msg, void *user_data) {
  (void)user_data;
  _cb_exec = true;
  _cb_err_code = err_code;

  if (msg != NULL) {
    _cb_msg = *msg;
  }
}

// Sample event handler
static bool _new_sample = false;
static sds011_msg_t _new_sample_msg;

static void on_sample(sds011_msg_t const *msg, void *user_data) {
  (void)user_data;
  if (msg) {
    _new_sample = true;
    _new_sample_msg = *msg;
  }
}

int main(void) {
  sds011_err_t err_code;
  sds011_t sds011;

  err_code = sds011_init(&sds011, &(sds011_init_t) {
    .retries      = 2,
    .msg_timeout  = 2000, // set reply timeout to 2s
    .millis       = mock_millis,
    .serial = {
      .bytes_available  = mock_bytes_available,
      .read_byte        = mock_read_byte,
      .send_byte        = mock_send_byte
    },
  });
  if (err_code != SDS011_OK) {
    return 1;
  }

  err_code = sds011_set_sample_callback(&sds011, (sds011_on_sample_t) {
    .callback = on_sample,
    .user_data = NULL,
  });
  if (err_code != SDS011_OK) {
    return 1;
  }

  _cb_exec = false;

  // Request firware version
  err_code = sds011_get_fw_ver(&sds011, 0xA160, (sds011_cb_t) {
    .callback = sds011_cb,
    .user_data = NULL
  });
  if (err_code != SDS011_OK) {
    return 1;
  }

  while (_cb_exec == false) {
    sds011_process(&sds011);

    if (_new_sample == true) {
      _new_sample = false;

      // Sample received asynchronously 
      printf("Device ID: %4X\n", _new_sample_msg.dev_id);
      printf("PM2.5:     %.1f ug/m3\n", _new_sample_msg.data.sample.pm2_5 / 10.0F);
      printf("PM10:      %.1f ug/m3\n", _new_sample_msg.data.sample.pm10 / 10.0F);
      printf("\n");
    }
  }

  if (_cb_err_code != SDS011_OK) {
    printf("callback error: %d\n", _cb_err_code);
    return 1;
  }

  printf("Device ID: %4X\nFirmware version: %d/%d/%d\n",
    _cb_msg.dev_id,
    _cb_msg.data.fw_ver.day,
    _cb_msg.data.fw_ver.month,
    _cb_msg.data.fw_ver.year + 2000
  );
  return 0;
}

// Mock implementation
// These function should be implemented for the target device
static uint8_t _serial_buffer[] = {
  0xAA, 0xC0, 0xD4, 0x04, 0x3A, 0x0A, 0xA1, 0x60, 0x1D, 0xAB,
  0xAA, 0xC5, 0x07, 0x0F, 0x07, 0x0A, 0xA1, 0x60, 0x28, 0xAB
};
static uint32_t _serial_buffer_iter = sizeof(_serial_buffer);

static uint32_t mock_millis(void) {
  // This function returns number of milliseconds since the
  // system start and is used to implement communication timeouts
  return 0;
}

static size_t mock_bytes_available(void *user_data) {
  (void)user_data;
  // This function returns number of bytes
  // in the serial input queue
  return sizeof(_serial_buffer) - _serial_buffer_iter;
}

static uint8_t mock_read_byte(void *user_data) {
  (void)user_data;
  // This function retunrs single byte from the
  // serial input queue
  return _serial_buffer[_serial_buffer_iter++];
}

static bool mock_send_byte(uint8_t byte, void *user_data) {
  (void)byte;
  (void)user_data;
  // This function will send data via serial interface
  // and return true on success, false otherwise.
  _serial_buffer_iter = 0; // NOTE: reply available after send
  return true;
}
