# 双层灯阵 3D 原型验证记录

日期：2026-10-16。状态：离线原型代码验证，设备验收未完成。

## 环境与来源

- 使用已有 Node.js v24.14.0/npm，不新增运行时。
- threejs-miniprogram 精确 0.0.8，MIT，npm 声明解包大小 600888B；lockfile 锁定下载完整性。安装禁用 lifecycle scripts。
- 真实 r108 场景对象与灯点属性用于主机测试，仅 GPU renderer 边界使用 fake；这不能证明 shader 已在手机 GPU 编译。
- 133 灯 JSON 副本 SHA256 和来源记录在 wechat-mini-program/lighting/DATA-SOURCE.md。

## 自动验证

命令：在 wechat-mini-program 目录执行 `npm.cmd test`。15 项通过，覆盖几何计数/映射、区域遮罩、三类效果、方向、限幅、Canvas x/y 与浏览器 clientX/clientY 手势、单循环、暂停时间、退出清理与异步回调。主机测试放在仓库 test/wechat-lighting，避免 node:test 进入小程序打包目录。

## 未完成的环境验收

已发现微信开发者工具 2.02.2608080（E:/Tencent/微信web开发者工具），CLI build-npm 返回“服务端口已关闭”，虽退出码为 0 但没有完成构建。未更改安全设置。请在界面“工具 → 构建 npm”完成本地构建。未实测 Android/iOS 的 WebGL、触摸、切后台恢复、shader、帧率及页面往返内存，不能将 Node 测试视作这些项目通过。

独立代码审查已完成，修复 Canvas 触摸格式兼容问题。已知轻微问题：若 Canvas 查询在页面隐藏后才返回，初始化可能绘制一帧，但不会持续运行动画；暂缓优化，不影响前台预览的主要流程。

用户步骤：完成 README 的构建 npm；打开原遥控页下方“灯阵实验室”；查看上下各 133 点，测试层/区域及三个效果、视角旋转缩放、播放暂停；退出重进及切后台恢复；检查原遥控五键仍正常。Android/iOS 分别记录手机型号、微信/基础库版本和问题。

## 安全边界

## 2026-10-16 页面空白修复

用户提供开发者工具错误：`lighting/led-map-133.json.js is not defined`。原因是页面初始化直接 require JSON，微信模块加载器按 JS 路径解析而失败；原 Node 测试自动支持 JSON，未覆盖该差异。

改用生成的 JS 数据模块，保留 JSON 作为可追溯快照。新增 JS-only 模块加载回归，修复前复现相同 json.js 缺失、修复后通过，另核对 JS 与 JSON 内容一致。16 项自动测试通过；用户重新编译后的页面与 GPU 显示仍待确认。

本次不改固件、BLE 写入逻辑或 AppID。不烧录、不控制实物、不自动提交或推送。LED 预览仅示意，盒高 300mm 和顶部朝向待校准，真实供电和功率限制留待固件接入阶段。
