#ifndef SDS011_FIFO_H__
#define SDS011_FIFO_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
  uint8_t *mem;
  size_t size;
  size_t elsize;

  size_t beg, end;
  size_t count;
} sds011_fifo_t;

bool sds011_fifo_init(sds011_fifo_t *fifo, size_t elsize, void *mem, size_t size);
bool sds011_fifo_push(sds011_fifo_t *fifo, void *el);
bool sds011_fifo_pop(sds011_fifo_t *fifo, void *el);

#endif // SDS011_FIFO_H__
