extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
}

#include "game_common.hpp"
#include <algorithm>

static const char *TAG_GAME4 = "GAME4";

struct Drop
{
  bool active;
  float x;
  float y;
  float vy;
  int radius;
  uint16_t color;
  int prev_y;
};

void game_rain_catch(LGFX &gfx)
{
  const int sw = gfx.width();
  const int sh = gfx.height();

  gfx.fillScreen(TFT_BLACK);

  int score = 0;
  int miss = 0;

  const int paddle_w = std::max(48, sw / 5);
  const int paddle_h = 12;
  const int paddle_y = sh - 28;
  int paddle_x = sw / 2 - paddle_w / 2;
  int target_x = paddle_x;
  int last_drawn_px = -1000;

  static Particle particles[MAX_PARTICLES] = {};
  static Ripple ripples[MAX_RIPPLES] = {};

  Drop drops[6];
  for (auto &d : drops)
  {
    d.active = false;
    d.prev_y = -1;
  }

  auto draw_hud = [&]() {
    gfx.setTextColor(TFT_WHITE, TFT_BLACK);
    gfx.fillRect(0, 0, sw, 18, TFT_BLACK);
    gfx.setCursor(4, 2);
    gfx.printf("Game 4  Score:%d  Miss:%d", score, miss);
#if ENABLE_GAME_SWITCH
    draw_switch_button(gfx, sw, "SWITCH");
#endif
  };

  auto draw_paddle = [&](int px) {
    if (px == last_drawn_px)
      return;
    gfx.fillRect(0, paddle_y - 2, sw, paddle_h + 6, TFT_BLACK);
    uint16_t fill = gfx.color888(70, 200, 140);
    uint16_t shadow = gfx.color888(40, 120, 90);
    gfx.fillRoundRect(px, paddle_y, paddle_w, paddle_h, 4, fill);
    gfx.fillRoundRect(px, paddle_y + paddle_h - 4, paddle_w, 4, 3, shadow);
    gfx.drawRoundRect(px - 1, paddle_y - 1, paddle_w + 2, paddle_h + 2, 5, TFT_WHITE);
    last_drawn_px = px;
  };

  auto spawn_drop = [&]() {
    for (auto &d : drops)
    {
      if (d.active)
        continue;
      d.active = true;
      d.x = static_cast<float>(irand(12, sw - 12));
      d.y = 24.0f;
      float speed = 1.4f + std::min(20, score) * 0.08f;
      d.vy = speed;
      d.radius = 8;
      d.color = gfx.color888(irand(120, 255), irand(120, 255), irand(120, 255));
      d.prev_y = -1;
      return;
    }
  };

  auto update_effects = [&]() {
    for (int i = 0; i < MAX_PARTICLES; ++i)
    {
      if (!particles[i].active)
        continue;
      gfx.fillCircle(particles[i].px, particles[i].py, particles[i].pr + 1, TFT_BLACK);
      particles[i].px = particles[i].x;
      particles[i].py = particles[i].y;
      particles[i].pr = particles[i].r;
      particles[i].x += particles[i].vx;
      particles[i].y += particles[i].vy;
      if ((particles[i].life & 1) == 0 && particles[i].r > 0)
        particles[i].r--;
      particles[i].life--;
      if (particles[i].life <= 0 || particles[i].r <= 0)
      {
        particles[i].active = false;
        continue;
      }
      gfx.fillCircle(particles[i].x, particles[i].y, particles[i].r, particles[i].color);
    }

    for (int i = 0; i < MAX_RIPPLES; ++i)
    {
      if (!ripples[i].active)
        continue;
      if (ripples[i].prev_rad > 0)
      {
        gfx.drawCircle(ripples[i].x, ripples[i].y, ripples[i].prev_rad, TFT_BLACK);
        gfx.drawCircle(ripples[i].x, ripples[i].y, ripples[i].prev_rad - 1, TFT_BLACK);
      }
      ripples[i].prev_rad = ripples[i].radius;
      ripples[i].radius += 2;
      if (ripples[i].radius >= ripples[i].max_rad)
      {
        ripples[i].active = false;
        continue;
      }
      gfx.drawCircle(ripples[i].x, ripples[i].y, ripples[i].radius, ripples[i].color);
      gfx.drawCircle(ripples[i].x, ripples[i].y, ripples[i].radius - 1, ripples[i].color);
    }
  };

  draw_hud();
  draw_paddle(paddle_x);

  gfx.setTextColor(TFT_DARKGREY, TFT_BLACK);
  gfx.setCursor(8, 22);
  gfx.print("Catch the falling orbs!");

  uint32_t next_spawn_ms = lgfx::v1::millis() + 400;

  while (true)
  {
    uint32_t now = lgfx::v1::millis();

    if ((int32_t)(now - next_spawn_ms) >= 0)
    {
      spawn_drop();
      int base_delay = 1100;
      int accel = std::min(600, score * 35);
      int delay_ms = std::max(320, base_delay - accel);
      next_spawn_ms = now + delay_ms;
    }

    uint16_t tx, ty;
    if (gfx.getTouch(&tx, &ty))
    {
      fix_touch_coords(tx, ty, sw, sh);
#if ENABLE_GAME_SWITCH
      if (is_in_switch_button(sw, tx, ty))
      {
        gfx.fillScreen(TFT_BLACK);
        ESP_LOGI(TAG_GAME4, "Switch button");
        return;
      }
#endif
      if (ty > 22)
      {
        int desired = static_cast<int>(tx) - paddle_w / 2;
        if (desired < 0)
          desired = 0;
        else if (desired > sw - paddle_w)
          desired = sw - paddle_w;
        target_x = desired;
      }
    }

    if (paddle_x < target_x)
      paddle_x = std::min(target_x, paddle_x + 6);
    else if (paddle_x > target_x)
      paddle_x = std::max(target_x, paddle_x - 6);

    for (auto &d : drops)
    {
      if (!d.active)
        continue;

      if (d.prev_y >= 0)
        gfx.fillCircle(static_cast<int>(d.x), d.prev_y, d.radius + 1, TFT_BLACK);

      d.y += d.vy;
      d.vy += 0.05f;
      int iy = static_cast<int>(d.y + 0.5f);

      bool caught = false;
      if (iy + d.radius >= paddle_y && iy - d.radius <= paddle_y + paddle_h)
      {
        int drop_x = static_cast<int>(d.x);
        if (drop_x >= paddle_x - d.radius && drop_x <= paddle_x + paddle_w + d.radius)
        {
          caught = true;
          score++;
          draw_hud();
          spawn_particles(particles, drop_x, paddle_y, d.color);
          spawn_ripple(ripples, sw, sh, drop_x, paddle_y, d.color);
        }
      }

      if (caught)
      {
        d.active = false;
        d.prev_y = -1;
        continue;
      }

      if (iy - d.radius > sh)
      {
        miss++;
        draw_hud();
        ESP_LOGI(TAG_GAME4, "Miss (dropped)");
        d.active = false;
        d.prev_y = -1;
        continue;
      }

      gfx.fillCircle(static_cast<int>(d.x), iy, d.radius, d.color);
      d.prev_y = iy;
    }

    update_effects();
    draw_paddle(paddle_x);

    if (miss >= 6)
    {
      gfx.setTextColor(TFT_YELLOW, TFT_BLACK);
      gfx.setCursor(10, sh - 16);
      gfx.print("Miss >= 6");
    }

    vTaskDelay(pdMS_TO_TICKS(16));
  }
}
