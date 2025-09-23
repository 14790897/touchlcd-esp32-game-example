// Shared helpers and types for LGFX touch games
#pragma once

#ifndef LGFX_USE_V1
#define LGFX_USE_V1
#endif

#include "lgfx_setup.hpp"
#include <cstdint>

// ---- Build-time toggles ----
#ifndef ENABLE_GAME_SWITCH
#define ENABLE_GAME_SWITCH 0
#endif

// ---- Random helpers ----
uint32_t urand();
int      irand(int min_v, int max_v);

// ---- Touch helpers ----
void fix_touch_coords(uint16_t &x, uint16_t &y, int screen_width, int screen_height);

// ---- Effects ----
struct Particle {
  int x, y;
  int px, py;
  int vx, vy;
  int r, pr;
  int life;
  uint16_t color;
  bool active;
};

struct Ripple {
  int x, y;
  int radius;
  int prev_rad;
  int max_rad;
  uint16_t color;
  bool active;
};

constexpr int MAX_PARTICLES = 48;
constexpr int MAX_RIPPLES   = 6;

void spawn_particles(Particle parts[], int cx, int cy, uint16_t base_col);
void spawn_ripple(Ripple ripples[], int sw, int sh, int x, int y, uint16_t color);

// ---- UI Helpers ----
void draw_title(LGFX& gfx, const char* title, int sw);

#if ENABLE_GAME_SWITCH
// Switch button helpers (top-right within title bar height ~18px)
void draw_switch_button(LGFX& gfx, int sw, const char* label = "SW");
bool is_in_switch_button(int sw, uint16_t x, uint16_t y);
#endif
