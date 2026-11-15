# Mosuan Player / 墨算播放器

跨平台教学视频播放器，目标平台：Windows 与 macOS。

## 当前阶段

V0.1.0：播放器核心骨架与 LibVLC 本地视频播放。

当前已加入：

- Qt 6 + C++20 工程骨架
- LibVLC 生命周期封装
- 本地视频打开、播放、暂停、停止、跳转、倍速接口
- Windows / macOS 视频输出接口
- 第一版简洁中文工具栏

## 后续模块

- 视频裁切与裁切配置记忆
- 播放列表 / 课程管理
- 智能反色
- 板书增强
- 头像、Logo 等保护区域
- 播放列表默认配置 + 单视频覆盖配置
- GPU 效果管线

## 构建

需要 Qt 6 Widgets 和 LibVLC SDK。

配置 `LIBVLC_ROOT` 指向 LibVLC SDK 根目录后使用 CMake 构建。

> 当前提交是工程源码骨架，尚未声称 Windows/macOS 安装包已经构建完成。下一步将补齐平台构建与 CI，再生成可测试版本。

## 学生模式配置接入（预览）

Windows 管家导出的 JSON 草案可用命令行 `MosuanPlayer.exe --student-policy "C:\\path\\MosuanPlayer-student-policy.preview.json"` 传入。播放器检查版本、字段和布尔值后，可禁用右键菜单、Ctrl+O/S/P/N、原生文件选择入口，并按配置保留播放快捷键。**仅 UI 级约束，命令行参数可被省略，学生仍可通过其他程序或系统功能绕过；不是安全沙箱或锁机方案。** 未接入管理员保护的策略目录、授权视频目录和系统白名单前，禁止作为无人值守学生终端使用。
