extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
}

#include "game_common.hpp"

static const char* TAG_GAME1 = "GAME1";

void game_tap_ball(LGFX& gfx)
{
  const int sw = gfx.width();
  const int sh = gfx.height();

  int score = 0;
  int radius = 22;
  int cx = irand(radius, sw - radius);
  int cy = irand(radius + 18, sh - radius);
  int vx = (irand(0, 1) ? 1 : -1) * irand(2, 4);
  int vy = (irand(0, 1) ? 1 : -1) * irand(2, 4);
  uint32_t last_spawn_ms = 0;
  uint32_t last_touch_ms = 0;
#if ENABLE_GAME_SWITCH
  uint32_t hold_switch_ms = 0;
#endif

  uint16_t ball_color = gfx.color888(irand(100,255), irand(100,255), irand(100,255));
  static Particle particles[MAX_PARTICLES] = {};
  static Ripple   ripples[MAX_RIPPLES]     = {};

  gfx.fillScreen(TFT_BLACK);
  draw_title(gfx, "Game 1  Score: 0", sw);

  auto draw_hud = [&](){
    gfx.setTextColor(TFT_WHITE, TFT_BLACK);
    gfx.fillRect(0, 0, sw, 18, TFT_BLACK);
    gfx.setCursor(4, 2);
    gfx.printf("Game 1  Score: %d", score);
#if ENABLE_GAME_SWITCH
    draw_switch_button(gfx, sw, "SWITCH");
#endif
  };
  auto draw_ball = [&](int x, int y, uint16_t col){ gfx.fillCircle(x, y, radius, col); };

  draw_ball(cx, cy, ball_color);

  while (true)
  {
    // erase previous effects
    for (int i = 0; i < MAX_PARTICLES; ++i) if (particles[i].active)
      gfx.fillCircle(particles[i].px, particles[i].py, particles[i].pr + 1, TFT_BLACK);
    for (int i = 0; i < MAX_RIPPLES; ++i) if (ripples[i].active && ripples[i].prev_rad > 0) {
      gfx.drawCircle(ripples[i].x, ripples[i].y, ripples[i].prev_rad,     TFT_BLACK);
      gfx.drawCircle(ripples[i].x, ripples[i].y, ripples[i].prev_rad - 1, TFT_BLACK);
    }

    // move ball
    int nx = cx + vx;
    int ny = cy + vy;
    if (nx - radius < 0 || nx + radius >= sw) { vx = -vx; nx = cx + vx; }
    if (ny - radius < 18 || ny + radius >= sh) { vy = -vy; ny = cy + vy; }
    gfx.fillCircle(cx, cy, radius + 1, TFT_BLACK);
    cx = nx; cy = ny;
    draw_ball(cx, cy, ball_color);

    // touch input
    uint16_t tx, ty;
    if (gfx.getTouch(&tx, &ty)) {
      fix_touch_coords(tx, ty, sw, sh);
      // top-right switch button tap
#if ENABLE_GAME_SWITCH
      if (is_in_switch_button(sw, tx, ty))
      {
        gfx.fillScreen(TFT_BLACK);
        ESP_LOGI(TAG_GAME1, "Switch button");
        return;
      }
#endif
      int dx = (int)tx - cx;
      int dy = (int)ty - cy;
      uint32_t now = lgfx::v1::millis();
      if (now - last_touch_ms > 80) { spawn_ripple(ripples, sw, sh, tx, ty, TFT_DARKGREY); last_touch_ms = now; }
      if (dx * dx + dy * dy <= radius * radius) {
        gfx.fillCircle(cx, cy, radius + 2, TFT_BLACK);
        score++; draw_hud();
        radius = irand(16, 28);
        cx = irand(radius, sw - radius);
        cy = irand(radius + 18, sh - radius);
        vx = (irand(0, 1) ? 1 : -1) * irand(2, 5);
        vy = (irand(0, 1) ? 1 : -1) * irand(2, 5);
        uint16_t col = gfx.color888(irand(0,255), irand(0,255), irand(0,255));
        ball_color = gfx.color888(irand(100,255), irand(100,255), irand(100,255));
        spawn_particles(particles, tx, ty, col);
        spawn_ripple(ripples, sw, sh, tx, ty, col);
        draw_ball(cx, cy, ball_color);
        last_spawn_ms = lgfx::v1::millis();
      }
    }

    // auto-respawn if idle
    if (lgfx::v1::millis() - last_spawn_ms > 5000) {
      gfx.fillCircle(cx, cy, radius + 2, TFT_BLACK);
      radius = irand(16, 28);
      cx = irand(radius, sw - radius);
      cy = irand(radius + 18, sh - radius);
      vx = (irand(0, 1) ? 1 : -1) * irand(2, 5);
      vy = (irand(0, 1) ? 1 : -1) * irand(2, 5);
      ball_color = gfx.color888(irand(100,255), irand(100,255), irand(100,255));
      draw_ball(cx, cy, ball_color);
      last_spawn_ms = lgfx::v1::millis();
    }

    // update effects
    for (int i = 0; i < MAX_PARTICLES; ++i) if (particles[i].active) {
      particles[i].px = particles[i].x; particles[i].py = particles[i].y; particles[i].pr = particles[i].r;
      particles[i].x += particles[i].vx; particles[i].y += particles[i].vy;
      if ((particles[i].life & 1) == 0 && particles[i].r > 0) particles[i].r--;
      particles[i].life--;
      if (particles[i].life <= 0 || particles[i].r <= 0) { gfx.fillCircle(particles[i].px, particles[i].py, particles[i].pr + 1, TFT_BLACK); particles[i].active = false; continue; }
      gfx.fillCircle(particles[i].x, particles[i].y, particles[i].r, particles[i].color);
    }
    for (int i = 0; i < MAX_RIPPLES; ++i) if (ripples[i].active) {
      ripples[i].prev_rad = ripples[i].radius;
      ripples[i].radius += 2;
      if (ripples[i].radius >= ripples[i].max_rad) { gfx.drawCircle(ripples[i].x, ripples[i].y, ripples[i].prev_rad, TFT_BLACK); ripples[i].active = false; continue; }
      gfx.drawCircle(ripples[i].x, ripples[i].y, ripples[i].radius, ripples[i].color);
      gfx.drawCircle(ripples[i].x, ripples[i].y, ripples[i].radius - 1, ripples[i].color);
    }

    vTaskDelay(pdMS_TO_TICKS(16));
  }
}
