# ESP32-S3 触控小游戏示例（LovyanGFX）

本项目基于 ESP32-S3 与 LovyanGFX，演示触摸屏输入与基本绘图，包含两款可切换的小游戏，并已模块化拆分，便于扩展与复用。

## 功能

- 点球（Game 1）：点击移动小球得分，带粒子/波纹效果
- 打地鼠（Game 2）：在时限内点击目标得分，超时计 Miss
- 公共特效：粒子喷射、同心圆波纹、标题栏绘制
- 右上角按钮“SWITCH”切换（可选，需开启宏）
- 编译期选择默认游戏

## 目录结构

```bash
main/
  main.cpp             # 入口，仅初始化与调度
  game_common.hpp/.cpp # 公共工具：随机、触摸修正、特效、标题栏/按钮
  game_tap_ball.cpp    # Game 1：点球
  game_whack.cpp       # Game 2：打地鼠
  lgfx_setup.hpp       # 显示与触摸硬件配置（LovyanGFX）
  CMakeLists.txt       # 组件构建配置
CMakeLists.txt         # 顶层构建
```

## 构建与下载

```
idf.py build
idf.py flash monitor
```

## 游戏选择与切换

- 编译期选择默认进入的游戏：

  - `GAME_MODE=1` 点球，`GAME_MODE=2` 打地鼠
  - 命令行：`idf.py -D GAME_MODE=2 build`

- 运行时切换（右上角按钮）
  - 开启宏：`ENABLE_GAME_SWITCH=1`
  - 命令行：`idf.py -D ENABLE_GAME_SWITCH=1 build`
  - 游戏界面标题栏右上角会显示“SWITCH”按钮，点击即切换到另一款游戏

## 硬件与映射

- 屏幕：ILI9341 240x320，配置见 `main/lgfx_setup.hpp`
- 触摸：XPT2046，如坐标方向不符，可在 `fix_touch_coords` 中调整镜像逻辑

### 📋 引脚连接

| 功能           | ESP32-S3 引脚 | 说明          |
| -------------- | ------------- | ------------- |
| **显示屏 SPI** |               |               |
| MOSI           | GPIO 1        | SPI 数据输出  |
| SCLK           | GPIO 2        | SPI 时钟      |
| MISO           | GPIO 8        | SPI 数据输入  |
| CS             | GPIO 3        | 显示屏片选    |
| DC             | GPIO 4        | 数据/命令选择 |
| RST            | GPIO 5        | 复位          |
| BL             | GPIO 6        | 背光控制      |
| **触摸屏**     |               |               |
| T_CS           | GPIO 7        | 触摸片选      |
| T_SCLK         | GPIO 42       | 触摸时钟      |
| T_MOSI         | GPIO 41       | 触摸数据输出  |
| T_MISO         | GPIO 40       | 触摸数据输入  |

## 关键实现点

- 公共接口：`game_common.hpp/.cpp`
  - `spawn_particles`, `spawn_ripple` 特效复用
  - `draw_title` 标题栏
  - `draw_switch_button`, `is_in_switch_button` 按钮绘制与点按检测（需 `ENABLE_GAME_SWITCH=1`）

## 常见问题

- 颜色异常或方向不对：`lgfx_setup.hpp` 中调整面板参数；触摸方向在 `fix_touch_coords` 调整
- 性能帧率：已使用 16bpp 与 SPI DMA，若闪烁可降低刷新或特效数量

## 许可证

MIT
