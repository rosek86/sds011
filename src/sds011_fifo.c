#include "sds011_fifo.h"
#include <string.h>

static inline size_t increase(sds011_fifo_t *fifo, size_t iter) {
  return ++iter < fifo->count ? iter : 0;
}

static inline bool is_empty(sds011_fifo_t *fifo) {
  return fifo->beg == fifo->end;
}

static inline bool is_full(sds011_fifo_t *fifo) {
  return increase(fifo, fifo->end) == fifo->beg; 
}

static inline void* memloc(sds011_fifo_t *fifo, size_t offset) {
  return fifo->mem + (offset * fifo->elsize);
}

bool sds011_fifo_init(sds011_fifo_t *fifo, size_t elsize, void *mem, size_t size) {
  if (fifo == NULL || mem == NULL) { return false; }
  if (elsize == 0 || size == 0) { return false; }

  fifo->elsize = elsize;

  fifo->mem = mem;
  fifo->size = size;

  fifo->beg = fifo->end = 0;
  fifo->count = fifo->size / fifo->elsize;

  return fifo->count > 0;
}

bool sds011_fifo_push(sds011_fifo_t *fifo, void *el) {
  if (fifo == NULL || el == NULL) { return false; }

  if (is_full(fifo)) {
    return false;
  }

  memcpy(memloc(fifo, fifo->end), el, fifo->elsize);
  fifo->end = increase(fifo, fifo->end);

  return true;
}

bool sds011_fifo_pop(sds011_fifo_t *fifo, void *el) {
  if (fifo == NULL || el == NULL) { return false; }

  if (is_empty(fifo)) {
    return false;
  }

  memcpy(el, memloc(fifo, fifo->beg), fifo->elsize);
  fifo->beg = increase(fifo, fifo->beg);

  return true;
}
