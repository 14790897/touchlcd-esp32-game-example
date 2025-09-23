extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_random.h"
}

#include "game_common.hpp"
#include <algorithm>

uint32_t urand() { return esp_random(); }

int irand(int min_v, int max_v)
{
  uint32_t r = urand();
  return min_v + (int)(r % (uint32_t)(max_v - min_v + 1));
}

void fix_touch_coords(uint16_t &x, uint16_t &y, int screen_width, int screen_height)
{
  // Uncomment if your touch mapping is mirrored
  x = screen_width  - x;
  // y = screen_height - y;
  // take care out-of-bounds values
  if (x >= screen_width)  x = screen_width  - 1;
  if (y >= screen_height) y = screen_height - 1;
}

void spawn_ripple(Ripple ripples[], int sw, int sh, int x, int y, uint16_t color)
{
  for (int i = 0; i < MAX_RIPPLES; ++i) if (!ripples[i].active)
  {
    ripples[i].x = x; ripples[i].y = y;
    ripples[i].radius = 2; ripples[i].prev_rad = 0;
    int maxr = std::min(std::min(x, sw - x), std::min(y, sh - y));
    ripples[i].max_rad = std::max(12, std::min(maxr, 48));
    ripples[i].color = color; ripples[i].active = true;
    break;
  }
}

void spawn_particles(Particle parts[], int cx, int cy, uint16_t base_col)
{
  int spawned = 0;
  for (int i = 0; i < MAX_PARTICLES && spawned < 24; ++i) if (!parts[i].active)
  {
    int vx = irand(-3, 3), vy = irand(-3, 3);
    if (vx == 0 && vy == 0) vx = 1;
    parts[i].x = parts[i].px = cx;
    parts[i].y = parts[i].py = cy;
    parts[i].vx = vx; parts[i].vy = vy;
    parts[i].r  = parts[i].pr = irand(2, 4);
    parts[i].life = irand(14, 22);
    // slight color variation
    uint8_t r = (base_col >> 11) & 0x1F;
    uint8_t g = (base_col >> 5)  & 0x3F;
    uint8_t b =  base_col        & 0x1F;
    int dr = irand(-3, 3), dg = irand(-6, 6), db = irand(-3, 3);
    r = (uint8_t)std::max(0, std::min(31, (int)r + dr));
    g = (uint8_t)std::max(0, std::min(63, (int)g + dg));
    b = (uint8_t)std::max(0, std::min(31, (int)b + db));
    parts[i].color = (r << 11) | (g << 5) | b;
    parts[i].active = true; ++spawned;
  }
}

void draw_title(LGFX& gfx, const char* title, int sw)
{
  gfx.setTextColor(TFT_WHITE, TFT_BLACK);
  gfx.fillRect(0, 0, sw, 18, TFT_BLACK);
  gfx.setFont(&fonts::Font0);
  gfx.setTextSize(2);
  gfx.setCursor(4, 2);
  gfx.print(title);
}

#if ENABLE_GAME_SWITCH
// Button: right-aligned in the title bar
static constexpr int BTN_H = 16;
static constexpr int BTN_W = 50;
static constexpr int BTN_PAD = 2; // right/top padding

void draw_switch_button(LGFX &gfx, int sw, const char *label)
{
  int x = sw - BTN_W - BTN_PAD;
  int y = (18 - BTN_H) / 2;
  if (y < 0)
    y = 0;
  gfx.fillRoundRect(x, y, BTN_W, BTN_H, 3, TFT_DARKGREY);
  gfx.drawRoundRect(x, y, BTN_W, BTN_H, 3, TFT_WHITE);
  gfx.setTextColor(TFT_WHITE, TFT_DARKGREY);
  gfx.setFont(&fonts::Font0);
  gfx.setTextSize(1);
  int tw = gfx.textWidth(label);
  int tx = x + (BTN_W - tw) / 2;
  int ty = y + 3;
  gfx.setCursor(tx, ty);
  gfx.print(label);
}

bool is_in_switch_button(int sw, uint16_t x, uint16_t y)
{
  int bx = sw - BTN_W - BTN_PAD;
  int by = (18 - BTN_H) / 2;
  if (by < 0)
    by = 0;
  return (y < 18) && (x >= bx) && (x < bx + BTN_W) && (y >= by) && (y < by + BTN_H);
}
#endif

