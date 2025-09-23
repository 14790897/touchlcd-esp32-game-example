extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
}

#include "game_common.hpp"

static const char* TAG_GAME2 = "GAME2";

void game_whack(LGFX& gfx)
{
  const int sw = gfx.width();
  const int sh = gfx.height();
  gfx.fillScreen(TFT_BLACK);

  int score = 0, miss = 0;
  int radius = 16;
  int txc = irand(radius, sw - radius);
  int tyc = irand(radius + 18, sh - radius);
  uint32_t ttl_ms = 1200;
  uint32_t spawn_ms = lgfx::v1::millis();
  uint32_t last_fx_ms = 0;
#if ENABLE_GAME_SWITCH
  uint32_t hold_switch_ms = 0;
#endif

  static Ripple   ripples[MAX_RIPPLES]     = {};
  static Particle particles[MAX_PARTICLES] = {};

  auto draw_hud = [&](){
    gfx.setTextColor(TFT_WHITE, TFT_BLACK);
    gfx.fillRect(0, 0, sw, 18, TFT_BLACK);
    gfx.setCursor(4, 2);
    gfx.printf("Game 2  Score:%d  Miss:%d", score, miss);
#if ENABLE_GAME_SWITCH
    draw_switch_button(gfx, sw, "SWITCH");
#endif
  };
  draw_hud();

  auto spawn_target = [&](){
    txc = irand(radius, sw - radius);
    tyc = irand(radius + 18, sh - radius);
    spawn_ms = lgfx::v1::millis();
  };
  auto draw_target = [&](){
    gfx.fillCircle(txc, tyc, radius, TFT_GREEN);
    gfx.drawCircle(txc, tyc, radius + 1, TFT_DARKGREEN);
  };
  draw_target();

  while (true) {
    uint32_t now = lgfx::v1::millis();
    if (now - spawn_ms > ttl_ms) {
      gfx.fillCircle(txc, tyc, radius + 2, TFT_BLACK);
      miss++; draw_hud(); spawn_target(); draw_target();
    }

    // touch
    uint16_t x, y;
    if (gfx.getTouch(&x, &y)) {
      fix_touch_coords(x, y, sw, sh);
      // top-right switch button tap
#if ENABLE_GAME_SWITCH
      if (is_in_switch_button(sw, x, y)) { gfx.fillScreen(TFT_BLACK); ESP_LOGI(TAG_GAME2, "Switch button"); return; }
#endif
      int dx = (int)x - txc, dy = (int)y - tyc;
      if ((dx*dx + dy*dy) <= radius*radius) {
        gfx.fillCircle(txc, tyc, radius + 2, TFT_BLACK);
        score++; draw_hud();
        uint16_t col = gfx.color888(irand(64,255), irand(64,255), irand(64,255));
        spawn_particles(particles, txc, tyc, col);
        spawn_ripple(ripples, sw, sh, x, y, col);
        spawn_target(); draw_target();
      } else if (now - last_fx_ms > 80) {
        spawn_ripple(ripples, sw, sh, x, y, TFT_DARKGREY);
        last_fx_ms = now;
      }
    }

    // update effects
    for (int i = 0; i < MAX_PARTICLES; ++i) if (particles[i].active) {
      gfx.fillCircle(particles[i].px, particles[i].py, particles[i].pr + 1, TFT_BLACK);
      particles[i].px = particles[i].x; particles[i].py = particles[i].y; particles[i].pr = particles[i].r;
      particles[i].x += particles[i].vx; particles[i].y += particles[i].vy;
      if ((particles[i].life & 1) == 0 && particles[i].r > 0) particles[i].r--;
      particles[i].life--; if (particles[i].life <= 0 || particles[i].r <= 0) { particles[i].active = false; continue; }
      gfx.fillCircle(particles[i].x, particles[i].y, particles[i].r, particles[i].color);
    }
    for (int i = 0; i < MAX_RIPPLES; ++i) if (ripples[i].active) {
      if (ripples[i].prev_rad > 0) {
        gfx.drawCircle(ripples[i].x, ripples[i].y, ripples[i].prev_rad,     TFT_BLACK);
        gfx.drawCircle(ripples[i].x, ripples[i].y, ripples[i].prev_rad - 1, TFT_BLACK);
      }
      ripples[i].prev_rad = ripples[i].radius; ripples[i].radius += 2;
      if (ripples[i].radius >= ripples[i].max_rad) { ripples[i].active = false; continue; }
      gfx.drawCircle(ripples[i].x, ripples[i].y, ripples[i].radius,     ripples[i].color);
      gfx.drawCircle(ripples[i].x, ripples[i].y, ripples[i].radius - 1, ripples[i].color);
    }

    if (miss >= 5) {
      gfx.setTextColor(TFT_YELLOW, TFT_BLACK);
      gfx.setCursor(10, sh - 20);
      gfx.print("Miss >= 5");
    }
    vTaskDelay(pdMS_TO_TICKS(16));
  }
}
