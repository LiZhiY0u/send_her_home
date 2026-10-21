# 灯位数据来源

led-map-133.json 原样复制自工作区外层 docs/led-array-133/led-map-133.json（2026-10-16）。SHA256：

`2953DB35CCB678E9ECE812A2B834C67AB91A5A69E689237355EA77909D389DC7`

单位 mm，200×200mm，中心 33、四角各 25。文件内灯号为建议串接映射，不代表已核实的 PCB 连线。layout.js 为上下各建一份映射；高度 300mm 和顶部朝向仅供示意。

微信运行时通过 led-map-133.js 导出数据，不直接 require JSON。JS 副本由 JSON 生成，模块加载测试同时检查两者一致；更新坐标时必须同步生成该 JS 文件。

在小程序目录运行 `npm.cmd test` 验证数量、区域、唯一 ID 与坐标；主机测试存放于仓库 test/wechat-lighting，不进入小程序包。顶部实际安装方向后续校准。
