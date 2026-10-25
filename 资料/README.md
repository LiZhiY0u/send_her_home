# 项目资料

本目录保存 [send_her_home](https://github.com/LiZhiY0u/send_her_home) 的硬件归档、软件说明及历史验收记录。当前功能总览见[仓库 README](../README.md)。

## 目录

- `hardware/`：2026-10-10 提供的四个原理图/PCB 文件，原始内容未修改。
- `docs/`：功能说明、验证记录、设计与实施计划。
- `images/`：旧 README 学习配图，保留备查。
- 根目录 PDF：XW06A 触摸芯片规格书。

## 当前功能说明

| 主题 | 文档 |
|---|---|
| 双层灯盘 IO17 / IO5 | [dual-strip-io5.md](docs/dual-strip-io5.md) |
| 22 种空间灯效 | [spatial-effect-library.md](docs/spatial-effect-library.md) |
| OLED 效果菜单 | [oled-effect-selection.md](docs/oled-effect-selection.md) |
| OLED 亮度滑条 | [oled-brightness.md](docs/oled-brightness.md) |
| 随机颜色渐变 | [random-color.md](docs/random-color.md) |
| 绘画协议与验证 | [led-paint-validation.md](docs/led-paint-validation.md) |
| 小程序电机控制与保持策略 | [motor-mini-program.md](docs/motor-mini-program.md) |
| 本地电机 UI | [motor-ui-validation.md](docs/motor-ui-validation.md) |
| DRV8837 驱动记录 | [drv8837-validation.md](docs/drv8837-validation.md) |
| 灯位与空间测试 | [led-spatial-validation.md](docs/led-spatial-validation.md) |
| 3D 原型记录 | [led-3d-prototype-validation.md](docs/led-3d-prototype-validation.md) |

固件重构的输入、蓝牙、App、OLED 验收记录分别位于 `firmware-*-validation.md`；`superpowers/specs/`、`superpowers/plans/` 保存分阶段设计及计划。

## 硬件文件

- [原理图 PDF](hardware/SCH_Schematic1_2_2026-10-10.pdf)
- [PCB PDF](hardware/PCB_PCB2_2026-10-10.pdf)
- [原理图工程](hardware/ProDoc_Schematic1_2_2026-10-10.epro2)
- [PCB 工程](hardware/ProDoc_PCB2_2026-10-10.epro2)

归档不等于硬件电气审核或实板验收完成。

## 阅读历史记录

历史 `2.controller_test` 对应当前 `esp32-firmware`；旧 `docs/` 路径对应 `资料/docs/`。专题文档中的“单层”“离线原型”“待接入”等描述可能是当时阶段的记录，不应直接当作现在的功能结论。

当前上下灯盘复制同一画面，小程序分层联动仍仅预览；新电机联动通过代码测试但仍需真机验收。硬件反馈、主机测试、固件编译和安全验证属于不同层次，不能相互替代。
