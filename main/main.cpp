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

// 触摸坐标修正函数 - 处理镜像问题
static void fix_touch_coords(uint16_t &x, uint16_t &y, int screen_width, int screen_height)
{
  // 常见的镜像修正方案，根据实际情况选择一种：

  // 方案1: X轴镜像（左右翻转）
  x = screen_width - x;

  // 方案2: Y轴镜像（上下翻转） - 如果需要可以取消注释
  // y = screen_height - y;

  // 方案3: XY都镜像 - 如果需要可以取消注释
  // x = screen_width - x;
  // y = screen_height - y;

  // 确保坐标在屏幕范围内
  if (x >= screen_width)
    x = screen_width - 1;
  if (y >= screen_height)
    y = screen_height - 1;
}

// ---- Touch effects ----
struct Particle
{
  int x, y;       // current position
  int px, py;     // previous position (for erase)
  int vx, vy;     // velocity (pixels/frame)
  int r, pr;      // radius and previous radius
  int life;       // remaining frames
  uint16_t color; // current color
  bool active;
};

struct Ripple
{
  int x, y;
  int radius;   // current radius
  int prev_rad; // previous radius (for erase)
  int max_rad;  // end radius
  uint16_t color;
  bool active;
};

static constexpr int MAX_PARTICLES = 48;
static constexpr int MAX_RIPPLES = 6;

static void spawn_ripple(Ripple ripples[], int sw, int sh, int x, int y, uint16_t color)
{
  for (int i = 0; i < MAX_RIPPLES; ++i)
  {
    if (!ripples[i].active)
    {
      ripples[i].x = x;
      ripples[i].y = y;
      ripples[i].radius = 2;
      ripples[i].prev_rad = 0;
      // limit max radius inside screen
      int maxr = std::min(std::min(x, sw - x), std::min(y, sh - y));
      ripples[i].max_rad = std::max(12, std::min(maxr, 48));
      ripples[i].color = color;
      ripples[i].active = true;
      break;
    }
  }
}

static void spawn_particles(Particle parts[], int cx, int cy, uint16_t base_col)
{
  int spawned = 0;
  for (int i = 0; i < MAX_PARTICLES && spawned < 24; ++i)
  {
    if (!parts[i].active)
    {
      int vx = irand(-3, 3), vy = irand(-3, 3);
      if (vx == 0 && vy == 0)
        vx = 1;
      parts[i].x = parts[i].px = cx;
      parts[i].y = parts[i].py = cy;
      parts[i].vx = vx;
      parts[i].vy = vy;
      parts[i].r = parts[i].pr = irand(2, 4);
      parts[i].life = irand(14, 22);
      // slight color variation
      uint8_t r = (base_col >> 11) & 0x1F;
      uint8_t g = (base_col >> 5) & 0x3F;
      uint8_t b = base_col & 0x1F;
      int dr = irand(-3, 3), dg = irand(-6, 6), db = irand(-3, 3);
      r = (uint8_t)std::max(0, std::min(31, (int)r + dr));
      g = (uint8_t)std::max(0, std::min(63, (int)g + dg));
      b = (uint8_t)std::max(0, std::min(31, (int)b + db));
      parts[i].color = (r << 11) | (g << 5) | b;
      parts[i].active = true;
      ++spawned;
    }
  }
}

extern "C" void app_main(void)
{
  ESP_LOGI(TAG, "Starting touch game...");

  // 延迟一下让系统稳定
  vTaskDelay(pdMS_TO_TICKS(100));

  static LGFX gfx; // 使用静态分配避免栈溢出

  ESP_LOGI(TAG, "Initializing LGFX...");
  if (!gfx.init()) {
    ESP_LOGE(TAG, "LGFX init failed");
    while (1)
    {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
  ESP_LOGI(TAG, "LGFX initialized successfully");

  gfx.setRotation(1); // 根据屏幕安装方向调整
  gfx.setColorDepth(16);
  gfx.fillScreen(TFT_BLACK);

  const int sw = gfx.width();
  const int sh = gfx.height();

  ESP_LOGI(TAG, "Screen size: %dx%d", sw, sh);

  // 检查触摸功能是否可用
  ESP_LOGI(TAG, "Checking touch capability...");
  if (gfx.touch())
  {
    ESP_LOGI(TAG, "Touch controller initialized successfully");
  }
  else
  {
    ESP_LOGI(TAG, "Touch controller not available, continuing without touch");
  }

  // 测试基本绘图功能
  ESP_LOGI(TAG, "Testing basic drawing...");
  gfx.fillScreen(TFT_BLACK);
  gfx.setTextColor(TFT_WHITE);
  gfx.setTextSize(2);
  gfx.setCursor(10, 10);
  gfx.println("Touch Game");
  gfx.setCursor(10, 40);
  gfx.println("Loading...");

  vTaskDelay(pdMS_TO_TICKS(1000)); // 等待1秒确保稳定

  // 跳过校准，直接进入游戏
  ESP_LOGI(TAG, "Skipping calibration, using mirror coordinate fix...");
  gfx.fillScreen(TFT_BLACK);
  gfx.setTextColor(TFT_WHITE);
  gfx.setTextSize(2);
  gfx.setCursor(10, 10);
  gfx.println("Touch Game");
  gfx.setCursor(10, 40);
  gfx.println("Mirror Fix Mode");
  gfx.setTextSize(1);
  gfx.setCursor(10, 70);
  gfx.println("Touch coordinates will be");
  gfx.setCursor(10, 85);
  gfx.println("automatically mirrored");

  vTaskDelay(pdMS_TO_TICKS(2000)); // 显示2秒

  // 游戏元素
  int score = 0;
  int radius = 22;
  int cx = irand(radius, sw - radius);
  int cy = irand(radius, sh - radius);
  int vx = (irand(0, 1) ? 1 : -1) * irand(2, 4);
  int vy = (irand(0, 1) ? 1 : -1) * irand(2, 4);
  uint32_t last_spawn_ms = 0;
  uint32_t last_touch_ms = 0;

  // 效果缓存 - 使用静态分配避免栈溢出
  static Particle particles[MAX_PARTICLES] = {};
  static Ripple ripples[MAX_RIPPLES] = {};

  ESP_LOGI(TAG, "Starting main game loop...");

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
    // 先擦除上帧的特效，以免覆盖新绘制的小球
    for (int i = 0; i < MAX_PARTICLES; ++i)
      if (particles[i].active)
      {
        gfx.fillCircle(particles[i].px, particles[i].py, particles[i].pr + 1, TFT_BLACK);
      }
    for (int i = 0; i < MAX_RIPPLES; ++i)
      if (ripples[i].active && ripples[i].prev_rad > 0)
      {
        // 擦除上一帧的波纹（用黑色描边）
        gfx.drawCircle(ripples[i].x, ripples[i].y, ripples[i].prev_rad, TFT_BLACK);
        gfx.drawCircle(ripples[i].x, ripples[i].y, ripples[i].prev_rad - 1, TFT_BLACK);
      }

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
      // 应用镜像修正
      fix_touch_coords(tx, ty, sw, sh);

      int dx = (int)tx - cx;
      int dy = (int)ty - cy;
      uint32_t now = lgfx::v1::millis();
      // 点击波纹（限频）
      if (now - last_touch_ms > 80)
      {
        spawn_ripple(ripples, sw, sh, tx, ty, TFT_DARKGREY);
        last_touch_ms = now;
      }

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
        // 粒子爆炸特效
        spawn_particles(particles, tx, ty, col);
        // 强调波纹
        spawn_ripple(ripples, sw, sh, tx, ty, col);
        draw_ball(cx, cy, col);
        last_spawn_ms = lgfx::v1::millis();
      }
    }

    // 超时未命中则自动换一个
    if (lgfx::v1::millis() - last_spawn_ms > 5000)
    {
      gfx.fillCircle(cx, cy, radius + 2, TFT_BLACK);
      radius = irand(16, 28);
      cx = irand(radius, sw - radius);
      cy = irand(radius + 18, sh - radius);
      vx = (irand(0, 1) ? 1 : -1) * irand(2, 5);
      vy = (irand(0, 1) ? 1 : -1) * irand(2, 5);
      uint16_t col = gfx.color888(irand(0,255), irand(0,255), irand(0,255));
      draw_ball(cx, cy, col);
      last_spawn_ms = lgfx::v1::millis();
    }

    // 更新并绘制特效
    for (int i = 0; i < MAX_PARTICLES; ++i)
      if (particles[i].active)
      {
        // 更新
        particles[i].px = particles[i].x;
        particles[i].py = particles[i].y;
        particles[i].pr = particles[i].r;
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        if ((particles[i].life & 1) == 0 && particles[i].r > 0)
          particles[i].r--; // 隔帧缩小
        particles[i].life--;
        if (particles[i].life <= 0 || particles[i].r <= 0)
        {
          // 擦除尾迹
          gfx.fillCircle(particles[i].px, particles[i].py, particles[i].pr + 1, TFT_BLACK);
          particles[i].active = false;
          continue;
        }
        // 绘制
        gfx.fillCircle(particles[i].x, particles[i].y, particles[i].r, particles[i].color);
      }

    for (int i = 0; i < MAX_RIPPLES; ++i)
      if (ripples[i].active)
      {
        ripples[i].prev_rad = ripples[i].radius;
        ripples[i].radius += 2; // 扩散速度
        if (ripples[i].radius >= ripples[i].max_rad)
        {
          // 擦除最后一圈
          gfx.drawCircle(ripples[i].x, ripples[i].y, ripples[i].prev_rad, TFT_BLACK);
          ripples[i].active = false;
          continue;
        }
        // 用两条线画出稍厚的波纹
        gfx.drawCircle(ripples[i].x, ripples[i].y, ripples[i].radius, ripples[i].color);
        gfx.drawCircle(ripples[i].x, ripples[i].y, ripples[i].radius - 1, ripples[i].color);
      }

    vTaskDelay(pdMS_TO_TICKS(16)); // ~60 FPS
  }
}
