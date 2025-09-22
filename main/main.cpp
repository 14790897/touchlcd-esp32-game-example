extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "sdkconfig.h"
}

#include <cmath>
#include <cstdlib>
#include "lgfx_setup.hpp"

static const char* TAG = "TOUCH_GAME";

static uint32_t urand() {
  return esp_random();
}

static int irand(int min_v, int max_v) {
  uint32_t r = urand();
  return min_v + (int)(r % (uint32_t)(max_v - min_v + 1));
}

extern "C" void app_main(void)
{
  LGFX gfx;
  if (!gfx.init()) {
    ESP_LOGE(TAG, "LGFX init failed");
    vTaskDelay(portMAX_DELAY);
    return;
  }

  gfx.setRotation(1); // 根据屏幕安装方向调整
  gfx.setColorDepth(16);
  gfx.fillScreen(TFT_BLACK);

  const int sw = gfx.width();
  const int sh = gfx.height();

  // 游戏元素
  int score = 0;
  int radius = 22;
  int cx = irand(radius, sw - radius);
  int cy = irand(radius, sh - radius);
  int vx = (irand(0, 1) ? 1 : -1) * irand(2, 4);
  int vy = (irand(0, 1) ? 1 : -1) * irand(2, 4);
  uint32_t last_spawn_ms = 0;

  auto millis = [](){ return (uint32_t)(esp_timer_get_time() / 1000ULL); };

  auto draw_hud = [&](){
    gfx.setTextColor(TFT_WHITE, TFT_BLACK);
    gfx.setFont(&fonts::Font0);
    gfx.setTextSize(2);
    gfx.fillRect(0, 0, sw, 18, TFT_BLACK);
    gfx.setCursor(4, 2);
    gfx.printf("Score: %d", score);
  };

  auto draw_ball = [&](int x, int y, uint16_t col){
    gfx.fillCircle(x, y, radius, col);
  };

  draw_hud();
  draw_ball(cx, cy, TFT_GREEN);

  // 主循环
  while (true) {
    // 移动小球
    int nx = cx + vx;
    int ny = cy + vy;
    if (nx - radius < 0 || nx + radius >= sw) { vx = -vx; nx = cx + vx; }
    if (ny - radius < 18 || ny + radius >= sh) { vy = -vy; ny = cy + vy; } // 顶部 HUD 区域

    // 擦除旧位置、绘制新位置
    gfx.fillCircle(cx, cy, radius + 1, TFT_BLACK);
    cx = nx; cy = ny;
    draw_ball(cx, cy, TFT_GREEN);

    // 触摸检测
    uint16_t tx, ty;
    if (gfx.getTouch(&tx, &ty)) {
      int dx = (int)tx - cx;
      int dy = (int)ty - cy;
      if (dx * dx + dy * dy <= radius * radius) {
        // 命中：加分并重新生成
        gfx.fillCircle(cx, cy, radius + 2, TFT_BLACK);
        score++;
        draw_hud();

        radius = irand(16, 28);
        cx = irand(radius, sw - radius);
        cy = irand(radius + 18, sh - radius);
        vx = (irand(0, 1) ? 1 : -1) * irand(2, 5);
        vy = (irand(0, 1) ? 1 : -1) * irand(2, 5);
        uint16_t col = gfx.color888(irand(0,255), irand(0,255), irand(0,255));
        draw_ball(cx, cy, col);
        last_spawn_ms = millis();
      }
    }

    // 超时未命中则自动换一个
    if (millis() - last_spawn_ms > 5000) {
      gfx.fillCircle(cx, cy, radius + 2, TFT_BLACK);
      radius = irand(16, 28);
      cx = irand(radius, sw - radius);
      cy = irand(radius + 18, sh - radius);
      vx = (irand(0, 1) ? 1 : -1) * irand(2, 5);
      vy = (irand(0, 1) ? 1 : -1) * irand(2, 5);
      uint16_t col = gfx.color888(irand(0,255), irand(0,255), irand(0,255));
      draw_ball(cx, cy, col);
      last_spawn_ms = millis();
    }

    vTaskDelay(pdMS_TO_TICKS(16)); // ~60 FPS
  }
}
