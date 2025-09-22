// LGFX SPI panel setup using generic pin macros from platformio.ini
#pragma once

#ifndef LGFX_USE_V1
#define LGFX_USE_V1
#endif

#include <LovyanGFX.hpp>

// ----- Project-local pin and panel configuration (embedded, no env needed) -----
// Display panel geometry and SPI frequency
#undef  TFT_WIDTH
#define TFT_WIDTH  240
#undef  TFT_HEIGHT
#define TFT_HEIGHT 320
#undef  TFT_OFFSET_X
#define TFT_OFFSET_X 0
#undef  TFT_OFFSET_Y
#define TFT_OFFSET_Y 0
#undef  TFT_FREQ
#define TFT_FREQ 40000000

// ESP32-S3 SPI wiring (3-wire when MISO = -1)
#undef  TFT_MOSI
#define TFT_MOSI 1
#undef  TFT_SCLK
#define TFT_SCLK 2
#undef  TFT_MISO
#define TFT_MISO 8
#undef  TFT_CS
#define TFT_CS   3
#undef  TFT_DC
#define TFT_DC   4
#undef  TFT_RST
#define TFT_RST  5
#undef  TFT_BL
#define TFT_BL   6
#undef  TFT_BL_ACTIVE
#define TFT_BL_ACTIVE 1   // 1: HIGH turns backlight on

//  touch (XPT2046 over SPI)
#undef  TOUCH_XPT2046
#define TOUCH_XPT2046 1
#undef  TOUCH_CS
#define TOUCH_CS   7
#undef  TOUCH_IRQ
#define TOUCH_IRQ  -1
#undef  TOUCH_SCLK
#define TOUCH_SCLK 42
#undef  TOUCH_MOSI
#define TOUCH_MOSI 45
#undef  TOUCH_MISO
#define TOUCH_MISO 46
#undef  TOUCH_FREQ
#define TOUCH_FREQ 1000000
#undef  TOUCH_ROTATION
#define TOUCH_ROTATION 0

// If touch must be on a separate SPI host, define it here
#ifndef TOUCH_SPI_HOST
#define TOUCH_SPI_HOST SPI2_HOST
#endif

#ifndef TFT_WIDTH
#define TFT_WIDTH 240
#endif
#ifndef TFT_HEIGHT
#define TFT_HEIGHT 240
#endif
#ifndef TFT_OFFSET_X
#define TFT_OFFSET_X 0
#endif
#ifndef TFT_OFFSET_Y
#define TFT_OFFSET_Y 0
#endif
#ifndef TFT_FREQ
#define TFT_FREQ 40000000
#endif

#ifndef TFT_MOSI
#define TFT_MOSI -1
#endif
#ifndef TFT_MISO
#define TFT_MISO -1
#endif
#ifndef TFT_SCLK
#define TFT_SCLK -1
#endif
#ifndef TFT_CS
#define TFT_CS -1
#endif
#ifndef TFT_DC
#define TFT_DC -1
#endif
#ifndef TFT_RST
#define TFT_RST -1
#endif
#ifndef TFT_BL
#define TFT_BL -1
#endif
#ifndef TFT_BL_ACTIVE
#define TFT_BL_ACTIVE 1
#endif

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Bus_SPI _bus;
  lgfx::Panel_ILI9341 _panel;
  lgfx::Light_PWM _light;
#if TOUCH_XPT2046
  lgfx::Touch_XPT2046 _touch;
#endif

public:
  LGFX()
  {
    // ---- SPI bus config ----
    auto bus_cfg = _bus.config();

// ESP32-C3 uses SPI2_HOST, ESP32-S3 uses SPI3_HOST (VSPI)
#if defined(ARDUINO_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C3)
    bus_cfg.spi_host = SPI2_HOST;
#elif defined(ARDUINO_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S3)
    bus_cfg.spi_host = SPI3_HOST;  // ESP32-S3 uses SPI3 (VSPI)
#else
#ifdef VSPI_HOST
    bus_cfg.spi_host = VSPI_HOST;
#else
    bus_cfg.spi_host = SPI3_HOST;
#endif
#endif

    // bus_cfg.spi_mode = 0;
    bus_cfg.freq_write = TFT_FREQ;
    bus_cfg.freq_read = 16000000;
    // bus_cfg.spi_mode = 3;
    bus_cfg.pin_sclk = TFT_SCLK;
    bus_cfg.pin_mosi = TFT_MOSI;
    bus_cfg.pin_dc = TFT_DC;

    // If MISO is provided, use 4-wire; otherwise 3-wire
    if (TFT_MISO >= 0)
    {
      bus_cfg.pin_miso = TFT_MISO;
      bus_cfg.spi_3wire = false;
    }
    else
    {
      bus_cfg.pin_miso = -1;
      bus_cfg.spi_3wire = true;
    }

    bus_cfg.use_lock = true;
    bus_cfg.dma_channel = 1;

    _bus.config(bus_cfg);
    _panel.setBus(&_bus);

    // ---- Panel config ----
    auto panel_cfg = _panel.config();
    panel_cfg.pin_cs = TFT_CS;   // -1 if not used
    panel_cfg.pin_rst = TFT_RST; // -1 if not used
    panel_cfg.pin_busy = -1;

    panel_cfg.memory_width = TFT_WIDTH;
    panel_cfg.memory_height = TFT_HEIGHT;
    panel_cfg.panel_width = TFT_WIDTH;
    panel_cfg.panel_height = TFT_HEIGHT;
    panel_cfg.offset_x = TFT_OFFSET_X;
    panel_cfg.offset_y = TFT_OFFSET_Y;
    panel_cfg.offset_rotation = 0;

    panel_cfg.readable = false;
    panel_cfg.invert   = false;   // Many ST7789 need invert=on; change if colors look wrong
    panel_cfg.rgb_order = false; // Change to true if your colors are swapped
    panel_cfg.dlen_16bit = false;
    panel_cfg.bus_shared = false;

    _panel.config(panel_cfg);

    // ---- Backlight (optional) ----
    if (TFT_BL >= 0)
    {
      auto light_cfg = _light.config();
      light_cfg.pin_bl = TFT_BL;
      light_cfg.freq = 44100;
      light_cfg.pwm_channel = 7;
      light_cfg.invert = (TFT_BL_ACTIVE == 0);
      _light.config(light_cfg);
      _panel.setLight(&_light);
    }

#if TOUCH_XPT2046
    // ---- Touch (XPT2046 over SPI, optional) ----
    // Only configure if CS is valid. Pins default to TFT_* if not provided.
    if (TOUCH_CS >= 0)
    {
      auto tcfg = _touch.config();
      // Use dedicated SPI host for touch if required
      tcfg.spi_host = TOUCH_SPI_HOST;
      tcfg.pin_sclk = TOUCH_SCLK;
      tcfg.pin_mosi = TOUCH_MOSI;
      tcfg.pin_miso = TOUCH_MISO;
      tcfg.pin_cs = TOUCH_CS;
      tcfg.pin_int = TOUCH_IRQ; // optional, can be -1
      tcfg.freq = TOUCH_FREQ;
      tcfg.bus_shared = false;               // use separate bus from display
      tcfg.offset_rotation = TOUCH_ROTATION; // adjust if mapping is rotated
      
      // XPT2046 校准参数 - 根据实际测试调整
      // 这些值需要根据您的硬件进行微调
      tcfg.x_min = 300;     // X轴最小原始值
      tcfg.x_max = 3900;    // X轴最大原始值  
      tcfg.y_min = 300;     // Y轴最小原始值
      tcfg.y_max = 3900;    // Y轴最大原始值
      
      // 触摸变换和旋转
      tcfg.offset_rotation = TOUCH_ROTATION;
      

      _touch.config(tcfg);
      _panel.setTouch(&_touch);
    }
#endif

    setPanel(&_panel);
  }
};
