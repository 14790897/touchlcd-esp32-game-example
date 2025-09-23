extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
}

#include "game_common.hpp"
#include <algorithm>

static const char *TAG_GAME3 = "GAME3";

void game_memory_grid(LGFX &gfx)
{
  const int sw = gfx.width();
  const int sh = gfx.height();

  gfx.fillScreen(TFT_BLACK);

  int score = 0;
  int miss = 0;

  static constexpr int COLS = 3;
  static constexpr int ROWS = 3;
  static constexpr int TOTAL = COLS * ROWS;

  const int grid_top = 24;
  const int grid_left = 12;
  const int grid_width = sw - grid_left * 2;
  const int grid_height = sh - grid_top - 12;
  const int cell_w = grid_width / COLS;
  const int cell_h = grid_height / ROWS;

  const uint16_t idle_fill = gfx.color888(45, 45, 45);
  const uint16_t active_fill = gfx.color888(80, 170, 255);

  int active_idx = -1;
  uint32_t appear_ms = 0;
  uint32_t ttl_ms = 1500;
  uint32_t next_spawn_ms = 0;

  int feedback_idx = -1;
  uint32_t feedback_until = 0;

  auto draw_hud = [&]() {
    gfx.setTextColor(TFT_WHITE, TFT_BLACK);
    gfx.fillRect(0, 0, sw, 18, TFT_BLACK);
    gfx.setCursor(4, 2);
    gfx.printf("Game 3  Score:%d  Miss:%d", score, miss);
#if ENABLE_GAME_SWITCH
    draw_switch_button(gfx, sw, "SWITCH");
#endif
  };

  auto cell_bounds = [&](int idx, int &x, int &y, int &w, int &h) {
    int row = idx / COLS;
    int col = idx % COLS;
    x = grid_left + col * cell_w;
    y = grid_top + row * cell_h;
    int remaining_w = grid_width - col * cell_w;
    int remaining_h = grid_height - row * cell_h;
    w = (col == COLS - 1) ? remaining_w : cell_w;
    h = (row == ROWS - 1) ? remaining_h : cell_h;
  };

  auto draw_idle_cell = [&](int idx) {
    int x, y, w, h;
    cell_bounds(idx, x, y, w, h);
    gfx.fillRect(x, y, w, h, TFT_BLACK);
    if (w > 4 && h > 4)
    {
      gfx.fillRoundRect(x + 2, y + 2, w - 4, h - 4, 4, idle_fill);
      gfx.drawRoundRect(x + 1, y + 1, w - 2, h - 2, 4, TFT_DARKGREY);
    }
    else
    {
      gfx.fillRect(x, y, w, h, idle_fill);
    }
  };

  auto draw_active_cell = [&](int idx, uint16_t fill) {
    int x, y, w, h;
    cell_bounds(idx, x, y, w, h);
    gfx.fillRect(x, y, w, h, TFT_BLACK);
    if (w > 6 && h > 6)
    {
      gfx.fillRoundRect(x + 2, y + 2, w - 4, h - 4, 4, fill);
      gfx.drawRoundRect(x + 1, y + 1, w - 2, h - 2, 4, TFT_WHITE);
    }
    else
    {
      gfx.fillRect(x, y, w, h, fill);
    }
  };

  auto touch_to_index = [&](uint16_t tx, uint16_t ty) -> int {
    if (tx < grid_left || ty < grid_top)
      return -1;
    if (tx >= grid_left + grid_width || ty >= grid_top + grid_height)
      return -1;
    int rel_x = tx - grid_left;
    int rel_y = ty - grid_top;
    int col = std::min(COLS - 1, rel_x / std::max(1, cell_w));
    int row = std::min(ROWS - 1, rel_y / std::max(1, cell_h));
    return row * COLS + col;
  };

  auto flash_cell = [&](int idx, bool good, uint32_t now) {
    if (idx < 0)
      return;
    uint16_t fill = good ? TFT_GREEN : gfx.color888(200, 50, 50);
    uint16_t border = good ? TFT_WHITE : TFT_RED;
    int x, y, w, h;
    cell_bounds(idx, x, y, w, h);
    gfx.fillRect(x, y, w, h, TFT_BLACK);
    if (w > 6 && h > 6)
    {
      gfx.fillRoundRect(x + 2, y + 2, w - 4, h - 4, 4, fill);
      gfx.drawRoundRect(x + 1, y + 1, w - 2, h - 2, 4, border);
    }
    else
    {
      gfx.fillRect(x, y, w, h, fill);
    }
    feedback_idx = idx;
    feedback_until = now + 220;
  };

  auto spawn_target = [&](uint32_t now) {
    int next = irand(0, TOTAL - 1);
    if (next == active_idx)
      next = (next + 1) % TOTAL;
    if (active_idx >= 0)
      draw_idle_cell(active_idx);
    active_idx = next;
    appear_ms = now;
    draw_active_cell(active_idx, active_fill);
  };

  draw_hud();
  for (int i = 0; i < TOTAL; ++i)
    draw_idle_cell(i);
  next_spawn_ms = lgfx::v1::millis();

  while (true)
  {
    uint32_t now = lgfx::v1::millis();

    if (feedback_idx >= 0 && (int32_t)(now - feedback_until) >= 0)
    {
      if (active_idx >= 0 && feedback_idx == active_idx)
        draw_active_cell(active_idx, active_fill);
      else
        draw_idle_cell(feedback_idx);
      feedback_idx = -1;
    }

    if (active_idx < 0)
    {
      if ((int32_t)(now - next_spawn_ms) >= 0)
      {
        spawn_target(now);
      }
    }
    else if ((int32_t)(now - appear_ms) > (int32_t)ttl_ms)
    {
      miss++;
      draw_hud();
      flash_cell(active_idx, false, now);
      ESP_LOGI(TAG_GAME3, "Miss (timeout)");
      active_idx = -1;
      next_spawn_ms = now + 350;
    }

    uint16_t tx, ty;
    if (gfx.getTouch(&tx, &ty))
    {
      fix_touch_coords(tx, ty, sw, sh);
#if ENABLE_GAME_SWITCH
      if (is_in_switch_button(sw, tx, ty))
      {
        gfx.fillScreen(TFT_BLACK);
        ESP_LOGI(TAG_GAME3, "Switch button");
        return;
      }
#endif
      if (ty >= grid_top)
      {
        int idx = touch_to_index(tx, ty);
        if (idx >= 0)
        {
          if (idx == active_idx)
          {
            score++;
            draw_hud();
            flash_cell(active_idx, true, now);
            active_idx = -1;
            next_spawn_ms = now + 300;
            if (ttl_ms > 650)
              ttl_ms -= 20;
          }
          else
          {
            miss++;
            draw_hud();
            flash_cell(idx, false, now);
            ESP_LOGI(TAG_GAME3, "Miss (wrong cell)");
          }
        }
      }
    }

    if (miss >= 8)
    {
      gfx.setTextColor(TFT_YELLOW, TFT_BLACK);
      gfx.setCursor(10, sh - 20);
      gfx.print("Miss >= 8");
    }

    vTaskDelay(pdMS_TO_TICKS(16));
  }
}
