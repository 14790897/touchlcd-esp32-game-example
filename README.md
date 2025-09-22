# ESP32-S3 触摸游戏示例

这是一个基于ESP32-S3和LovyanGFX库的触摸屏游戏项目，展示了如何在ESP32上实现触摸检测、图形绘制和游戏逻辑。

## 🎮 游戏特性

- **触摸球游戏**：点击移动的彩色球来得分
- **炫酷视觉效果**：
  - 粒子爆炸特效
  - 波纹扩散效果
  - 随机彩色球
- **触摸镜像修正**：自动修正触摸坐标偏移问题
- **实时计分**：显示当前得分

## 🔧 硬件要求

### 主控板
- **ESP32-S3-DevKitM-1** 或兼容开发板

### 显示屏
- **ILI9341** 2.8寸 TFT LCD (240x320)
- SPI接口

### 触摸屏
- **XPT2046** 触摸控制器
- 与显示屏共享SPI总线

## 📋 引脚连接

| 功能 | ESP32-S3 引脚 | 说明 |
|------|---------------|------|
| **显示屏 SPI** |  |  |
| MOSI | GPIO 1 | SPI数据输出 |
| SCLK | GPIO 2 | SPI时钟 |
| MISO | GPIO 8 | SPI数据输入 |
| CS | GPIO 3 | 显示屏片选 |
| DC | GPIO 4 | 数据/命令选择 |
| RST | GPIO 5 | 复位 |
| BL | GPIO 6 | 背光控制 |
| **触摸屏** |  |  |
| T_CS | GPIO 7 | 触摸片选 |
| T_SCLK | GPIO 42 | 触摸时钟 |
| T_MOSI | GPIO 41 | 触摸数据输出 |
| T_MISO | GPIO 40 | 触摸数据输入 |

## 🚀 快速开始

### 1. 环境准备

```bash
# 安装ESP-IDF (v5.0+)
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
source ./export.sh
```

### 2. 克隆项目

```bash
git clone https://github.com/14790897/touchlcd-esp32-game-example.git
cd touchlcd-esp32-game-example
```

### 3. 初始化子模块

```bash
git submodule update --init --recursive
```

### 4. 编译和烧录

```bash
# 配置项目
idf.py menuconfig

# 编译
idf.py build

# 烧录并监控
idf.py flash monitor
```

## 📁 项目结构

```
├── main/
│   ├── main.cpp              # 主程序和游戏逻辑
│   ├── lgfx_setup.hpp        # LovyanGFX配置
│   └── CMakeLists.txt        # 主要组件配置
├── components/
│   └── LovyanGFX/            # LovyanGFX库
├── CMakeLists.txt            # 项目配置
├── sdkconfig                 # ESP-IDF配置
└── README.md                 # 项目说明
```

## � 游戏玩法

1. **启动游戏**：程序启动后会显示"Touch Game"界面
2. **击球得分**：触摸屏幕上移动的彩色球来得分
3. **视觉特效**：
   - 击中球时会产生粒子爆炸效果
   - 触摸时会产生波纹扩散效果
   - 每个新球都有随机颜色
4. **自动换球**：如果5秒内未击中，球会自动变换位置和颜色

## 🔧 触摸校准

项目内置了触摸坐标镜像修正功能：

- **X轴镜像**：自动修正左右镜像问题
- **坐标范围限制**：确保触摸坐标在屏幕范围内

如需调整，可在 `main.cpp` 中的 `fix_touch_coords()` 函数中修改：

```cpp
// X轴镜像（左右翻转）
x = screen_width - x;

// Y轴镜像（上下翻转） - 如果需要
// y = screen_height - y;
```

## 📊 技术特性

### 图形系统
- **LovyanGFX**：高性能图形库
- **16位颜色**：RGB565格式
- **硬件加速**：ESP32 SPI DMA

### 触摸检测
- **XPT2046控制器**：12位精度
- **轮询模式**：无需中断引脚
- **坐标修正**：自动镜像修正

### 游戏引擎
- **60FPS**：流畅的游戏体验
- **粒子系统**：最多48个粒子
- **波纹效果**：最多6个同时波纹

## 🛠️ 自定义配置

### 修改显示屏参数

在 `main/lgfx_setup.hpp` 中：

```cpp
#define TFT_WIDTH  240      // 屏幕宽度
#define TFT_HEIGHT 320      // 屏幕高度
#define TFT_FREQ   40000000 // SPI频率
```

### 修改触摸参数

```cpp
#define TOUCH_FREQ 1000000  // 触摸SPI频率
// 校准参数
tcfg.x_min = 300;
tcfg.x_max = 3900;
tcfg.y_min = 300;
tcfg.y_max = 3900;
```

### 修改游戏参数

在 `main/main.cpp` 中：

```cpp
static constexpr int MAX_PARTICLES = 48;  // 最大粒子数
static constexpr int MAX_RIPPLES = 6;     // 最大波纹数
// 球的大小范围
radius = irand(16, 28);
```

## 📝 故障排除

### 显示问题
- **黑屏**：检查SPI连接和电源
- **颜色异常**：检查颜色格式配置
- **花屏**：降低SPI频率

### 触摸问题
- **无触摸响应**：检查触摸SPI连接
- **坐标偏移**：调整镜像修正参数
- **检测不稳定**：降低触摸SPI频率

### 编译问题
- **LovyanGFX错误**：确保子模块正确初始化
- **链接错误**：检查CMakeLists.txt配置

## 📄 许可证

本项目基于MIT许可证开源。详见LICENSE文件。

## 🤝 贡献

欢迎提交Issue和Pull Request！

## 📞 联系方式

- GitHub: [@14790897](https://github.com/14790897)
- 项目地址: [touchlcd-esp32-game-example](https://github.com/14790897/touchlcd-esp32-game-example)

---

**享受编程的乐趣！** 🎉

  SCLK: GPIO 2│   ├── CMakeLists.txt

  MISO: GPIO 8│   └── main.c

  CS:   GPIO 3└── README.md                  This is the file you are currently reading

  DC:   GPIO 4```

  RST:  GPIO 5Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 

  BL:   GPIO 6They are not used or needed when building with CMake and idf.py.

  ```

### 触摸屏
- **型号**：XPT2046 触摸控制器
- **接口**：独立SPI总线
- **引脚连接**：
  ```
  SCLK: GPIO 42
  MOSI: GPIO 41
  MISO: GPIO 40
  CS:   GPIO 7
  IRQ:  GPIO -1 (禁用，使用轮询模式)
  ```

## 📁 项目结构

```
test-lgfx/
├── CMakeLists.txt              # 项目配置
├── README.md                   # 项目说明
├── sdkconfig                   # ESP-IDF配置
├── .gitignore                  # Git忽略文件
├── main/
│   ├── CMakeLists.txt          # 主组件配置
│   ├── main.cpp                # 游戏主程序
│   └── lgfx_setup.hpp          # LovyanGFX配置
└── components/
    └── LovyanGFX/              # LovyanGFX库
```

## 🚀 开始使用

### 环境要求

- **ESP-IDF**: v5.5 或更高版本
- **开发环境**: VS Code + ESP-IDF 扩展 或 ESP-IDF 命令行
- **编译器**: xtensa-esp32s3-elf-gcc

### 安装步骤

1. **克隆项目**
   ```bash
   git clone <your-repo-url>
   cd test-lgfx
   ```

2. **初始化子模块（如果有）**
   ```bash
   git submodule update --init --recursive
   ```

3. **配置ESP-IDF环境**
   ```bash
   # Windows PowerShell
   $env:IDF_PATH = 'C:\git-program\esp-idf'  # 根据您的安装路径调整
   & $env:IDF_PATH\export.ps1
   
   # Linux/Mac
   . $HOME/esp/esp-idf/export.sh
   ```

4. **编译项目**
   ```bash
   idf.py build
   ```

5. **烧录到设备**
   ```bash
   idf.py -p COM13 flash monitor  # Windows
   idf.py -p /dev/ttyUSB0 flash monitor  # Linux
   ```

## 🎯 触摸校准

项目包含自动镜像修正功能，适用于触摸坐标与显示坐标不匹配的情况：

- **X轴镜像修正**：自动处理左右翻转问题
- **可选Y轴修正**：如需要可在代码中启用
- **坐标边界检查**：确保触摸坐标在屏幕范围内

如需调整，请修改 `main.cpp` 中的 `fix_touch_coords()` 函数。

## ⚙️ 配置文件说明

### `lgfx_setup.hpp`
- 显示屏和触摸屏的引脚配置
- SPI频率设置
- 触摸校准参数

### `main.cpp`
- 游戏逻辑和特效
- 触摸事件处理
- 坐标镜像修正

## 🎨 自定义配置

### 修改游戏参数
```cpp
// 在 main.cpp 中调整这些值：
static constexpr int MAX_PARTICLES = 48;  // 粒子特效数量
static constexpr int MAX_RIPPLES = 6;     // 波纹特效数量
int radius = 22;                          // 球的半径
```

### 修改颜色方案
```cpp
// 球的颜色范围（RGB 100-255 确保颜色鲜艳）
ball_color = gfx.color888(irand(100,255), irand(100,255), irand(100,255));
```

### 修改触摸灵敏度
在 `lgfx_setup.hpp` 中调整：
```cpp
tcfg.x_min = 300;    // X轴最小值
tcfg.x_max = 3900;   // X轴最大值
tcfg.y_min = 300;    // Y轴最小值
tcfg.y_max = 3900;   // Y轴最大值
```

## 🐛 故障排除

### 编译错误
- 确保ESP-IDF环境正确设置
- 检查LovyanGFX库是否正确添加到 `components/` 目录

### 显示问题
- 检查SPI引脚连接
- 确认显示屏型号为ILI9341
- 验证电源供应（3.3V/5V）

### 触摸不响应
- 检查触摸屏引脚连接
- 确认CS引脚设置正确
- 查看串口输出的触摸初始化信息

### 触摸位置偏移
- 项目已包含镜像修正功能
- 如仍有偏移，可调整 `fix_touch_coords()` 函数
- 考虑启用Y轴镜像或双轴镜像

## 📊 性能优化

- 使用静态内存分配避免栈溢出
- 60FPS游戏循环优化
- 高效的特效渲染算法
- SPI总线共享优化

## 🎮 游戏玩法

1. **启动游戏**：设备上电后自动启动
2. **点击球体**：触摸屏幕上移动的彩色球
3. **获得分数**：成功击中球体得分+1
4. **新球生成**：击中后生成新的随机颜色球
5. **自动换球**：5秒未击中自动生成新球

## 🔄 版本历史

- **v1.0** - 基础触摸游戏功能
- **v1.1** - 添加视觉特效（粒子爆炸、触摸波纹）
- **v1.2** - 镜像修正功能，解决触摸偏移问题
- **v1.3** - 彩色球功能，随机颜色生成

## 🤝 贡献

欢迎提交Issues和Pull Requests！

### 贡献指南
1. Fork 项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开Pull Request

## 📄 许可证

本项目基于MIT许可证开源。

## 🙏 致谢

- [LovyanGFX](https://github.com/lovyan03/LovyanGFX) - 优秀的ESP32图形库
- [ESP-IDF](https://github.com/espressif/esp-idf) - Espressif官方开发框架
- ESP32社区的所有贡献者

## 📞 联系方式

如有问题，请通过以下方式联系：
- GitHub Issues: [项目Issues页面]
- 技术讨论: 欢迎在Issues中提问

---

**享受游戏吧！** 🎮✨

> 这是一个学习ESP32开发和图形编程的绝佳项目，适合初学者和进阶开发者。