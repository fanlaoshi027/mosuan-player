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
