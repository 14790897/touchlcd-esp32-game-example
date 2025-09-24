extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
}

#include "lgfx_setup.hpp"
#include "game_common.hpp"
#include "games.hpp"

// Build-time options
#ifndef GAME_MODE
#define GAME_MODE 1  // 1: Tap Ball, 2: Whack-a-Mole, 3: Memory Grid, 4: Rain Catch
#endif
#ifndef ENABLE_GAME_SWITCH
#define ENABLE_GAME_SWITCH 0
#endif

static const char* TAG = "TOUCH_GAME";

extern "C" void app_main(void)
{
  ESP_LOGI(TAG, "Starting touch game (compile-time switch)...");
  vTaskDelay(pdMS_TO_TICKS(100));

  static LGFX gfx;
  if (!gfx.init()) { ESP_LOGE(TAG, "LGFX init failed"); while (1) vTaskDelay(pdMS_TO_TICKS(1000)); }
  gfx.setRotation(1);
  gfx.setColorDepth(16);
  gfx.fillScreen(TFT_BLACK);

#if ENABLE_GAME_SWITCH
  int mode = (GAME_MODE >= 1 && GAME_MODE <= 4) ? GAME_MODE : 1;
  while (true) {
    if (mode == 1) {
      ESP_LOGI(TAG, "GAME_MODE=1 (Tap Ball)");
      game_tap_ball(gfx);
      mode = 2;
    } else if (mode == 2) {
      ESP_LOGI(TAG, "GAME_MODE=2 (Whack-a-Mole)");
      game_whack(gfx);
      mode = 3;
    } else if (mode == 3) {
      ESP_LOGI(TAG, "GAME_MODE=3 (Memory Grid)");
      game_memory_grid(gfx);
      mode = 4;
    } else {
      ESP_LOGI(TAG, "GAME_MODE=4 (Rain Catch)");
      game_rain_catch(gfx);
      mode = 1;
    }
  }
#else
  #if GAME_MODE == 1
    ESP_LOGI(TAG, "GAME_MODE=1 (Tap Ball)");
    game_tap_ball(gfx);
  #elif GAME_MODE == 2
    ESP_LOGI(TAG, "GAME_MODE=2 (Whack-a-Mole)");
    game_whack(gfx);
  #elif GAME_MODE == 3
    ESP_LOGI(TAG, "GAME_MODE=3 (Memory Grid)");
    game_memory_grid(gfx);
  #elif GAME_MODE == 4
    ESP_LOGI(TAG, "GAME_MODE=4 (Rain Catch)");
    game_rain_catch(gfx);
  #else
    #error "Invalid GAME_MODE (use 1, 2, 3 or 4)"
  #endif
#endif
}
