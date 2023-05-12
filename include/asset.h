#pragma once

#ifndef _ASSET_H_
#define _ASSET_H_

#include "api.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef V5_API_H_

typedef struct __attribute__((__packed__)) _v5_image {
  uint16_t width;
  uint16_t height;
  uint32_t *data;
  uint32_t *p;
} v5_image;

uint32_t vexImageBmpRead(const uint8_t *ibuf, v5_image *oBuf, uint32_t maxw, uint32_t maxh);
uint32_t vexImagePngRead(const uint8_t *ibuf, v5_image *oBuf, uint32_t maxw, uint32_t maxh, uint32_t ibuflen);
void vexDisplayCopyRect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t *pSrc, int32_t srcStride);

#endif

typedef struct __attribute__((__packed__)) _asset {
  uint8_t *buf;
  size_t size;
} asset;

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#define ASSET(x)                                                                    \
  extern "C" {                                                                      \
  extern uint8_t _binary_static_##x##_start[], _binary_static_##x##_size[];         \
  static asset x = {_binary_static_##x##_start, (size_t)_binary_static_##x##_size}; \
  }

#else

#define ASSET(x)                                                            \
  extern uint8_t _binary_static_##x##_start[], _binary_static_##x##_size[]; \
  static asset x = {_binary_static_##x##_start, (size_t)_binary_static_##x##_size};

#endif

#define DECODE_PNG(var, asset, width, height) \
  var.data = new uint32_t[width * height];    \
  vexImagePngRead(asset.buf, &var, width, height, asset.size);

#define DECODE_BMP(var, asset, width, height) \
  var.data = new uint32_t[width * height];    \
  vexImageBmpRead(asset.buf, &var, width, height);


static void draw_image(int x, int y, v5_image *image) {
  int x2 = x + image->width - 1;
  int y2 = y + image->height - 1;
  vexDisplayCopyRect(x, y, x2, y2, image->data, x2 - x + 1);
}

static void draw_image(lv_obj_t *canvas, int x, int y, v5_image *image) {
  int start_x = x;
  int start_y = y;

  for (int i = 0; i < image->height; i++) {
    for (int j = 0; j < image->width; j++) {
      lv_color_t c = lv_color_hex(image->data[i * image->width + j]);
      lv_color_t d = lv_canvas_get_px(canvas, start_x + j, start_y + i);

      float a_float = c.alpha / 2.55 / 100;
      float a_m_float = 1 - a_float;

      d.red = d.red * a_m_float + c.red * a_float;
      d.green = d.green * a_m_float + c.green * a_float;
      d.blue = d.blue * a_m_float + c.blue * a_float;

      lv_canvas_set_px(canvas, start_x + j, start_y + i, d);
    }
  }
}

#endif // _ASSET_H_
