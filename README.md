# send_her_home · FigureCase 展示柜控制系统

ESP32 展示柜固件与微信小程序，通过蓝牙手柄、手机和编码器控制灯光、电机及 OLED 菜单。

仓库：[LiZhiY0u/send_her_home](https://github.com/LiZhiY0u/send_her_home)。项目运行时名称及蓝牙广播名仍为 **FigureCase**，仅维护 `main` 分支。

## 当前进度

| 功能 | 当前实现 |
|---|---|
| OLED 与输入 | SH1106 128×64；Light、Editor、Volt、Setting、Motor；编码器、手柄和小程序五向遥控 |
| 双灯盘 | 底面 GPIO17、顶面 GPIO5，各 133 颗 WS2812B，共 266 颗；当前两层复制同一画面 |
| 空间灯效 | 22 种可选效果，20 种自动轮播；OLED 和小程序均可选择 |
| 亮度与颜色 | OLED 亮度滑条、小程序亮度/速度/颜色控制；可开启每 5 秒一次的随机颜色渐变 |
| 手绘 | 平面灯板画布、软边补光、笔宽、颜色、橡皮和清空；蓝牙同步到上下灯盘 |
| 3D 预览 | 上下双层、手势旋转/缩放、灯层/区域选择；横屏左右分区，竖屏模型与控制区分区 |
| 电机 | DRV8837：启停、停机换向、速度滑条；OLED 与小程序共用状态；退出保持/停止策略 |
| 连接提示 | 手柄与小程序独立连接通知，OLED 右侧滑入/滑出 |
| 验证 | 最近代码验证：33 组固件主机测试、迁移安全检查、40 项小程序测试及 ESP32 编译通过 |

已收到用户对灯位、双灯盘显示及前期本地电机控制的实物反馈；**新小程序电机联动及保持策略仍需完整真机回归**。编译和主机测试不代表电气、供电或运动安全验收。Editor 的部分功能仍为占位项。

## 快速使用

1. 用 VS Code / PlatformIO 打开 `esp32-firmware`，编译并由用户手动烧录。
2. 微信开发者工具导入 `wechat-mini-program`，配置自己的 AppID。
3. 3D 页面需要先安装工程 npm 依赖，再在开发者工具中执行“工具 → 构建 npm”。
4. 手机开启蓝牙，从遥控首页连接广播名为 `FigureCase` 的设备。
5. 灯效页先预览，再点“应用到底面灯板”开启联动；**此按钮目前实际同步上下两层**。
6. 手绘页点“连接并同步绘画”；电机页读取设备状态后，必须手动点击启动。

不要连接后直接假定设备处于停止状态；尤其是电机保持模式，应以设备回传状态和现场情况为准。

## 目录

```text
esp32-firmware/
  platformio.ini           ESP32 编译配置
  src/
    App/                   启动与任务组织
    Bluetooth/             Bluepad32/BTstack、手机 ATT 服务
    Input/                 统一输入事件
    Lighting/              灯效、绘画协议、双灯盘、命令桥
    MotorControl/          共享电机状态、命令队列、超时策略
    OledUI/                OLED 页面、渲染、导航与弹窗
    Configs/               引脚、任务、蓝牙及测试开关
  test/                    C++ 主机测试与测试替身
  tools/                   测试脚本
wechat-mini-program/
  pages/                   遥控、3D 灯效、手绘、电机页面
  ble/                     共用蓝牙连接与串行写队列
  lighting/                3D 场景、空间效果与手势
  painting/                软画笔、灯位、传输与确认
  motor/                   电机协议与页面客户端
test/wechat-lighting/       小程序 Node 测试
资料/
  hardware/                原理图、PCB 工程及 PDF
  docs/                    功能说明、设计和验收记录
  images/                  历史学习配图
```

固件目录历史名为 `2.controller_test`。旧 OLED 示例与 Bad Apple 示例可从 Git 历史查找；历史文档不一定代表当前功能。

## 灯光功能与边界

单层灯位为中心 33 颗、四角各 25 颗，共 133 颗。效果按物理坐标采样，不按接线顺序简单跑马；图形采用软边，改善稀疏灯点下的连续感。没有灯珠的区域不会实际发光。

22 种效果：常亮、旋转光线、圆形扩散、水平波浪、斜向波浪、正弦光带、海浪叠加、方框平移、方形扩散、方框旋转、雷达扫描、螺旋光带、四角接力、极光流动、向心圆环、双向对撞、偏心涟漪、菱形脉冲、弹跳光块、双臂风车、花瓣开合、对角呼吸。自动轮播不含常亮和旋转光线，每种名义约 12 秒，实际包含计算及两条灯带发送耗时。

- OLED：`Light → Effect` 选择效果或 `Auto Playlist`；`Brightness` 按 5% 步进调节；`Random Color` 开关随机渐变。
- 亮度上限为 `26/255`，页面的 100% 对应该上限，**不是灯珠全功率输出**；当前驱动存在整数亮度分档。
- 随机色为伪随机调色板序列：保持约 4 秒，最后 1 秒渐变，不提高亮度上限。
- 小程序灯层、区域、同步/反向/镜像/延迟选项目前只影响预览；**实物仍为上下同画面，不支持分层独立控制**。
- 手绘收到完整帧并应用后才确认；退出/断连保留最后画面，“恢复自动灯效”返回轮播。草稿只保留在本次小程序运行内。
- 灯光没有跨端完整状态读回；OLED 数值及小程序选项可能是各自最后的设置，不等于设备实时快照。
- 3D 盒体尺寸 200×200×300mm 为示意，非真实照度/反射模拟，也不是机械制造模型。

当前 `LightingTestConfig.h` 中 `ENABLE_LED_STRIP_TEST=1`、`ENABLE_LED_SPATIAL_TEST=1`，启用 GPIO17/5 空间灯效路径；关闭前者恢复 GPIO19 原演示，关闭后者进入链路/区域测试，并非正常灯效播放配置。

## 电机控制与保持策略

电机为 DRV8837 双路 PWM，IN1 GPIO4、IN2 GPIO16，1kHz；上电停止，初始速度设置 30%。速度百分比表示 PWM 比例，**不是实测转速**。

小程序电机页和 OLED 共用一份设备状态；运行中禁止换向，先停止再切换方向。连接、查询、重新进入页面不会自动启动。小程序前台约每秒读取状态；调速及保持策略采用字段更新，不隐式重发缓存启动状态。

**“退出后保持运行”默认开启，包含返回页面、微信后台、锁屏和蓝牙断开。启动后可能持续运行，必须确保可以重新连接、本地停止或断电。**

关闭保持时，离页发送停止指令；固件另对远程控制执行断连/3 秒心跳超时停止。该保护依赖 MCU 与任务正常运行，不是硬件级急停。

OLED 的明确控制操作可以接管远程控制。退出 OLED Motor 菜单会停止本地拥有的运行，但不会仅因导航就停止远程拥有的运行。暂不保存掉电前运行状态。

## 软件架构

- `main.cpp → App` 组织 HAL、输入、蓝牙及任务启动；保留采样、灯光、编码器、显示四个任务。
- 手柄、编码器、小程序五向键和串口输入提交统一语义事件；固定容量 16 的输入队列满时拒绝新事件。
- Lighting 任务是灯带唯一输出者，消费 OLED / BLE 灯光命令，处理双层输出、绘画与效果播放。
- Display 任务处理电机命令与 PWM，再运行 OLED 页面；蓝牙回调只校验/入队，不直接驱动电机。
- 小程序所有页面共用连接，写操作串行化；设备确认、过期序列拒绝、旧连接隔离用于区分“已发送”和“已应用”。
- 蓝牙栈采用 Bluepad32/BTstack，不应再混用另一套 `BLEDevice` 初始化。

手柄地址过滤为 `C0:D6:D5:EE:DB:8A`；首次配对需手柄进入配对模式。左摇杆和十字键支持按住滚动，延迟 400ms 后每 150ms 一项；A 键按下只确认一次。手柄按键日志默认关闭；串口 115200。当前 `platformio.ini` 开启串口输入调试，与手柄日志开关不同。

## 开发环境与命令

- 原版 ESP32 / `esp32dev`，Arduino C++，PlatformIO `espressif32@6.10.0`。
- Bluepad32 自定义框架由 `platformio.ini` 的 `platform_packages` 指定，当前下载地址跟踪 `main`，**未锁定提交**；此前本机解析为 4.0.2。
- 库声明：Adafruit NeoPixel `^1.15.1`、U8g2 `^2.36.12`；此前本机解析为 1.15.5、2.36.18，不代表所有机器的解析结果。
- 小程序：微信开发者工具、已有 Node/npm；`threejs-miniprogram@0.0.8` 固定版本（内部 Three.js r108）。
- Windows 优先使用 VS Code PlatformIO IDE 已有环境，不安装重复 SDK/工具链。

以下命令均从仓库根目录，在 PowerShell 执行；烧录命令会改写设备固件，请核对 COM 口并由用户主动执行：

```powershell
# 固件编译
& 'C:\Users\Cainiao\.platformio\penv\Scripts\pio.exe' run -d .\esp32-firmware

# 手动烧录：COM3 替换为实际端口
& 'C:\Users\Cainiao\.platformio\penv\Scripts\pio.exe' run -d .\esp32-firmware -t upload --upload-port COM3

# 串口监视
& 'C:\Users\Cainiao\.platformio\penv\Scripts\pio.exe' device monitor -p COM3 -b 115200

# 已有 g++ / clang++ 时运行固件主机测试
& .\esp32-firmware\tools\run-host-tests.ps1

# 已安装 Node/npm 时运行小程序测试
npm.cmd test --prefix .\wechat-mini-program
```

如 PlatformIO 环境尚未初始化，用 VS Code 打开 `esp32-firmware`，等待扩展初始化后再编译。`.pio/`、生成的 `.vscode/`、`node_modules/`、`miniprogram_npm/` 不提交。AppID 用自己的配置，不应被示例配置覆盖。详见[小程序 README](wechat-mini-program/README.md)。

## 接线配置

| 用途 | GPIO / 配置 |
|---|---|
| SH1106 OLED SDA / SCL | 21 / 22 |
| 编码器 A / B / 按键 | 14 / 33 / 32 |
| 底面 / 顶面 WS2812B | 17 / 5；每层 133，GRB / 800kHz |
| 原 RGB3 演示灯带 | 19；当前双灯盘模式不运行该演示 |
| DRV8837 IN1 / IN2 | 4 / 16 |
| 电池采样 / 充电检测 / 电源保持 | 27 / 25 / 26 |

引脚依据 `BoardConfig.h`、`LightingTestConfig.h`；归档硬件资料不等于已逐项核对网表。断电接线，核对模块供电及共地；双层负载不能沿用单层供电验收结论。

## 蓝牙接口

广播名：`FigureCase`，服务 UUID：`0000FF10-0000-1000-8000-00805F9B34FB`。

| 特征 | UUID 尾部 | 用途 |
|---|---|---|
| 遥控写入 | FF11 | 单字节五向键：01上、02下、03左、04右、05确定 |
| 扩展写入 | FF12 | A7 灯光/绘画、B8 电机命令 |
| 状态通知/读取 | FF13 | 灯光应用确认、电机命令状态通知 |

UUID 完整格式与服务相同，仅将 FF10 替换为相应尾部。五向键的上/左映射上一项，下/右映射下一项。扩展包有独立会话、序列及参数校验；电机通知为命令应用状态，不是转速/电流传感器反馈。

## 资料与验证记录

| 主题 | 文档 |
|---|---|
| 资料目录与历史路径 | [资料索引](资料/README.md) |
| 双灯盘 IO17 / IO5 | [双层输出说明](资料/docs/dual-strip-io5.md) |
| 22 种效果及联动边界 | [空间灯效库](资料/docs/spatial-effect-library.md) |
| OLED 效果 / 亮度 | [灯效菜单](资料/docs/oled-effect-selection.md)、[亮度滑条](资料/docs/oled-brightness.md) |
| 随机颜色 | [渐变说明](资料/docs/random-color.md) |
| 手绘传输 | [绘画验证](资料/docs/led-paint-validation.md) |
| 电机控制及安全边界 | [小程序电机说明](资料/docs/motor-mini-program.md)、[DRV8837 记录](资料/docs/drv8837-validation.md) |
| 固件架构历史 | [架构设计](资料/docs/superpowers/specs/2026-10-10-firmware-architecture-design.md) |

部分专题文档保留了首批实现时的限制，以当前源码和本 README 汇总为准。没有云端服务、独立分层实物输出、完整灯光状态同步或实测转速闭环。

## 硬件归档

2026-10-10 提供的原始文件：

- [原理图 PDF](资料/hardware/SCH_Schematic1_2_2026-10-10.pdf)
- [PCB PDF](资料/hardware/PCB_PCB2_2026-10-10.pdf)
- [原理图工程](资料/hardware/ProDoc_Schematic1_2_2026-10-10.epro2)
- [PCB 工程](资料/hardware/ProDoc_PCB2_2026-10-10.epro2)

`.epro2` 使用兼容的嘉立创 EDA 专业版打开。文件归档不代表电气设计评审通过。

## 参考与署名

[Bluepad32](https://github.com/ricardoquesada/bluepad32)、[WouoUI](https://github.com/RQNG/WouoUI)、[ESP32_connect_XboxController](https://github.com/xiaocainiao11111/ESP32_connect_XboxController)、[super_knob](https://gitee.com/wenzhengclub/super_knob)。灯效思路参考 WLED / FastLED，具体说明见空间灯效文档；保留原代码作者署名及依赖许可。

## 开源许可证

本项目自有软件代码采用 **GNU General Public License v3.0（GPL-3.0-only）**，完整条款见 [LICENSE](LICENSE)。无任何担保，具体权利、义务与限制以许可证全文为准。

第三方依赖、引用代码及非本项目持有权利的资料保留其原有许可证与署名，本声明不替代其许可，也不表示已完成所有第三方许可兼容性审计。硬件工程、数据手册及图片等资料若另有原始权利声明，以其声明为准。
