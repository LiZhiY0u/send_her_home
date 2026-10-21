# 双层灯阵 3D 原型 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans (本会话直接实现) or superpowers:subagent-driven-development (用户选择后) to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** 在现有微信小程序增加离线双层灯阵 3D 预览，不改变设备行为。

**Architecture:** CommonJS 纯函数负责坐标映射和灯效采样；渲染模块负责 WebGL、相机及资源；Page 负责控件和生命周期。保留原遥控默认入口，添加 navigator 进入预览页。

**Tech Stack:** 现有 Node.js、node:test、微信 Canvas WebGL、threejs-miniprogram 候选适配库；不新增开发环境。

**Spec:** 资料/docs/superpowers/specs/2026-10-16-led-3d-prototype-design.md

## Global Constraints

- 顶部始终显示“离线预览 · 不控制设备”。
- 266 颗灯；源数据 200×200mm；盒高 300mm 仅示意。
- 不修改 ESP32、蓝牙协议、AppID；不发送控制、不烧录、不推送。
- 保留 main 分支和用户 project.config.json 修改；未经另行要求不提交。
- Three.js 依赖先核验版本、许可证和体积，锁版本及 lockfile；无法验证兼容性则报告，不偷偷降级成伪 3D。
- 不安装新的 SDK/runtime；应用依赖如需安装，明确说明范围，不使用全局安装。

## Review Focus

- 数据重复或非法坐标：拒绝渲染并显示错误，不能悄悄漏灯。
- WebGL 创建失败：显示可返回的错误页面。
- 隐藏/显示及快速退出重进：单动画循环，无退出后继续绘制。
- 多指切换/触摸取消：相机无 NaN 或突然跳变，缩放有上下限。
- 原遥控页回归：新增入口不触发断连或任何灯控指令。

## Task 1：灯位与效果纯函数

**Files:** Create wechat-mini-program/lighting/{led-map-133.json,layout.js,effects.js}; tests/lighting/{layout.test.js,effects.test.js}。

**Interfaces:** `buildLayout(source, {heightMm,topRotation,topMirror}) -> Point[]`；Point 为 `{id,layer,region,x,y,z,radius,angle}`，layer 为 top/bottom。`sampleEffect(point, settings, elapsedSeconds) -> [r,g,b]`，RGB 0..1；settings 包含 layer、region、effect、color、brightness、speed、linkedMode。

- [ ] 编写失败测试：单层 133、双层 266、每层中心 33/每角 25、id 全唯一、上下 Y 为 ±150、输入 X/Y 映射到 X/Z；非法/重复坐标拒绝。
- [ ] 运行 `node --test wechat-mini-program/tests/lighting/layout.test.js`，确认因缺失映射行为失败。
- [ ] 从外层源 JSON 引入副本，记录来源与 SHA256；实现校验、坐标变换、顶部旋转与镜像。
- [ ] 编写效果失败测试：零亮度全黑、常亮给定颜色、旋转对径线同时点亮、圆环按半径传播、选区外为黑、反向改变相位、相同输入确定性；非法参数归一化且不得产生 NaN。
- [ ] 实现常亮、旋转线、扩散环，时间来自调用参数，不访问时钟或 wx。
- [ ] 运行 `node --test wechat-mini-program/tests/lighting/*.test.js`；全部通过才进入渲染。

## Task 2：3D 场景与触摸

**Files:** Create wechat-mini-program/{package.json,package-lock.json}; lighting/{scene.js,gestures.js}; tests/lighting/{scene.test.js,gestures.test.js}。

**Interfaces:** `createScene(canvas, THREE, points, {width,height,pixelRatio}) -> {render(settings,time),resize(width,height),setView(name),touchStart(event),touchMove(event),touchEnd(event),dispose()}`。不由 scene 自行创建动画循环。

- [ ] 核验候选包实际版本、许可证、Canvas 绑定接口；优先采用微信适配库的基础几何能力，不引入模型 loader。使用精确版本，记录来源；若该库不适配所需基础能力，停止并报告替代选择。
- [ ] 先测试手势：单指旋转、双指缩放、零距离、手指数变化、cancel、上下限，保证相机参数有限。
- [ ] 先测试场景生命周期：使用边界 fake Canvas/THREE 检查尺寸、像素比上限 2、dispose 幂等、销毁后不绘制；不把 fake 自身当作业务证明。
- [ ] 实现盒体线框、半透明面、简化手办占位几何体；灯珠共享几何/材质或批量绘制，禁止创建 266 个真实点光源。
- [ ] 实现整体/俯视/复位视角，未选层与区域保留暗轮廓。控制相机距离与俯仰，像素比限制 2。
- [ ] 运行全部纯函数与渲染边界测试；记录这不能替代真机 WebGL 验收。

## Task 3：页面及生命周期集成

**Files:** Create pages/lighting/lighting.{js,json,wxml,wxss}; lighting/preview-loop.js; tests/lighting/{page.test.js,preview-loop.test.js}。Modify app.json、pages/remote/remote.wxml（仅 navigator 入口）。

**Interfaces:** `createPreviewLoop({requestFrame,cancelFrame,render,now}) -> {start(),stop(),dispose()}`；start 幂等，暂停冻结效果时间，隐藏停绘，显示不重复循环；30fps 目标节流。Page 调用 Task 1、2 接口。

- [ ] 测试循环 start 两次只有一条链、暂停时间不前进、隐藏后停止、dispose 后回调不续订。
- [ ] 测试 Page：Canvas 无法获取/WebGL 不可用时显错；onHide 停止、onUnload 释放、异步初始化后已退出不创建遗留场景。
- [ ] 页面加入层、区域（全部/中心/四角汇总及各角）、效果、颜色预设、亮度、速度、同步/反向、播放/暂停；所有控件只更新本地预览。
- [ ] 增加拖动/缩放提示、300mm 示意尺寸标签、顶部朝向未校准说明。用 navigator 新增入口，不改 remote.js 蓝牙代码。
- [ ] 运行 `node --test wechat-mini-program/tests/lighting/*.test.js` 和新增 JS 文件 `node --check`；核对 BLE 写入代码 diff 为零。

## Task 4：工具与设备验收记录

**Files:** Modify wechat-mini-program/README.md；Create 资料/docs/led-3d-prototype-validation.md。

- [ ] 写明现有 Node/npm 环境、精确依赖、开发者工具“构建 npm”和打开页面步骤，记录未测试边界。
- [ ] 若有可访问的微信开发者工具，检查渲染、三种灯效、区域选择、触摸和页面往返；否则明确交给用户验证，不能声称通过。
- [ ] Android/iOS 真机分别记录帧率、切后台恢复、退出重进，不以开发者工具结果代替真机结果。
- [ ] 最终跑完整 JS 测试及 diff 检查，确认固件、AppID、BLE 逻辑无改动；交付原型，不自动提交、推送或控制设备。

## 自审

四项任务覆盖设计范围及五类主要失败模式；纯算法与页面生命周期有独立验证。性能、实际显示、手机兼容和朝向校准保留真实验收边界。执行前用户审阅本计划并选择直接执行或子代理执行。

## 执行状态（2026-10-16）

- [x] 用户确认并选择本会话直接执行。
- [x] Task 1 灯位/灯效实现与自动测试。
- [x] Task 2 场景/手势实现与真实 Three 几何边界测试。
- [x] Task 3 页面/循环实现与主机生命周期测试。
- [ ] Task 4 开发者工具和 Android/iOS 实机验收待执行；文档、代码检查和独立审查已完成（Canvas 触摸问题已修复）；CLI 本地构建因服务端口关闭未完成。

测试路径调整为仓库 test/wechat-lighting（不进入小程序包），统一命令是在 wechat-mini-program 目录运行 npm.cmd test。其他原计划测试路径为设计阶段记录。

详细测试、偏差及审查记录见 资料/docs/led-3d-prototype-validation.md；不能将上述代码测试视为 GPU 或手机验收通过。
