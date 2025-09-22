# ESP32-S3 触摸游戏项目# _Sample project_



基于ESP32-S3和LovyanGFX库开发的触摸屏游戏，使用ILI9341显示屏和XPT2046触摸控制器。(See the README.md file in the upper level 'examples' directory for more information about examples.)



## 🎮 游戏特色This is the simplest buildable example. The example is used by command `idf.py create-project`

that copies the project to user specified path and set it's name. For more information follow the [docs page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project)

- 🎯 **触摸球游戏**：点击移动的彩色球获得分数

- 🌈 **随机彩色球**：每个球都有随机的鲜艳颜色

- ✨ **视觉特效**：

  - 触摸波纹效果## How to use example

  - 击中时的粒子爆炸特效We encourage the users to use the example as a template for the new projects.

  - 平滑的球体移动动画A recommended way is to follow the instructions on a [docs page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project).

- 📊 **实时计分**：显示当前得分

- ⏰ **自动换球**：5秒未击中自动生成新球## Example folder contents



## 🔧 硬件配置The project **sample_project** contains one source file in C language [main.c](main/main.c). The file is located in folder [main](main).



### 主控板ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`

- **ESP32-S3-DevKitM-1**files that provide set of directives and instructions describing the project's source files and targets

(executable, library, or both). 

### 显示屏

- **型号**：ILI9341 240x320 TFT LCDBelow is short explanation of remaining files in the project folder.

- **接口**：SPI

- **引脚连接**：```

  ```├── CMakeLists.txt

  MOSI: GPIO 1├── main

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