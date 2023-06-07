#include <pico/stdlib.h>
#include "buffers.h"
#include "render.h"
#include "vga.h"

static void render_lores_line(uint line);

void __time_critical_func(render_lores)()
{
  vga_prepare_frame();

  // Skip 48 lines to center vertically
  struct vga_scanline* skip_sl = vga_prepare_scanline();
  for (int i = 0; i < 48; i++)
  {
    skip_sl->data[i] = (uint32_t)THEN_WAIT_HSYNC << 16;
  }
  skip_sl->length = 48;
  vga_submit_scanline(skip_sl);

  for (uint line = 0; line < 24; line++)
  {
    render_lores_line(line);
  }
}

void __time_critical_func(render_mixed_lores)()
{
  vga_prepare_frame();

  // Skip 48 lines to center vertically
  struct vga_scanline* skip_sl = vga_prepare_scanline();
  for (int i = 0; i < 48; i++)
  {
    skip_sl->data[i] = (uint32_t)THEN_WAIT_HSYNC << 16;
  }
  skip_sl->length = 48;
  vga_submit_scanline(skip_sl);

  for (uint line = 0; line < 20; line++)
  {
    render_lores_line(line);
  }
  if (soft_80col)
  {
    for (uint line = 20; line < 24; line++)
    {
      render_text80_line(line);
    }
  }
  else
  {
    for (uint line = 20; line < 24; line++)
    {
      render_text_line(line);
    }
  }
}

static void __time_critical_func(render_lores_line)(uint line)
{
  // Construct two scanlines for the two different colored cells at the same time
  struct vga_scanline* sl1 = vga_prepare_scanline();
  struct vga_scanline* sl2 = vga_prepare_scanline();
  uint sl_pos = 0;

  const uint8_t* page = ((soft_switches & SOFTSW_PAGE_2) && !soft_80store) ? text_memory + 1024 : text_memory;
  const uint8_t* line_buf = page + ((line & 0x7) << 7) + (((line >> 3) & 0x3) * 40);

  // Pad 40 pixels on the left to center horizontally
  sl1->data[sl_pos] = (0 | THEN_EXTEND_7) | ((0 | THEN_EXTEND_7) << 16);  // 16 pixels per word
  sl2->data[sl_pos] = (0 | THEN_EXTEND_7) | ((0 | THEN_EXTEND_7) << 16);  // 16 pixels per word
  sl_pos++;
  sl1->data[sl_pos] = (0 | THEN_EXTEND_7) | ((0 | THEN_EXTEND_7) << 16);  // 16 pixels per word
  sl2->data[sl_pos] = (0 | THEN_EXTEND_7) | ((0 | THEN_EXTEND_7) << 16);  // 16 pixels per word
  sl_pos++;
  sl1->data[sl_pos] = (0 | THEN_EXTEND_3) | ((0 | THEN_EXTEND_3) << 16);  // 8 pixels per word
  sl2->data[sl_pos] = (0 | THEN_EXTEND_3) | ((0 | THEN_EXTEND_3) << 16);  // 8 pixels per word
  sl_pos++;

  for (int i = 0; i < 40; i++)
  {
    uint32_t color1 = lores_palette[line_buf[i] & 0xf];
    uint32_t color2 = lores_palette[(line_buf[i] >> 4) & 0xf];

    // Each lores pixel is 7 hires pixels, or 14 VGA pixels wide
    sl1->data[sl_pos] = (color1 | THEN_EXTEND_6) | ((color1 | THEN_EXTEND_6) << 16);
    sl2->data[sl_pos] = (color2 | THEN_EXTEND_6) | ((color2 | THEN_EXTEND_6) << 16);
    sl_pos++;
  }
  sl1->data[sl_pos] = (0 | THEN_EXTEND_7) | ((0 | THEN_EXTEND_7) << 16);  // 16 pixels per word
  sl2->data[sl_pos] = (0 | THEN_EXTEND_7) | ((0 | THEN_EXTEND_7) << 16);  // 16 pixels per word
  sl_pos++;
  sl1->data[sl_pos] = (0 | THEN_EXTEND_7) | ((0 | THEN_EXTEND_7) << 16);  // 16 pixels per word
  sl2->data[sl_pos] = (0 | THEN_EXTEND_7) | ((0 | THEN_EXTEND_7) << 16);  // 16 pixels per word
  sl_pos++;
  sl1->data[sl_pos] = (0 | THEN_EXTEND_3) | ((0 | THEN_EXTEND_3) << 16);  // 8 pixels per word
  sl2->data[sl_pos] = (0 | THEN_EXTEND_3) | ((0 | THEN_EXTEND_3) << 16);  // 8 pixels per word
  sl_pos++;
  sl1->length = sl_pos;
  sl1->repeat_count = 7;
  vga_submit_scanline(sl1);

  sl2->length = sl_pos;
  sl2->repeat_count = 7;
  vga_submit_scanline(sl2);
}
