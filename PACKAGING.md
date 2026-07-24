# 桌面安装包生成

## Linux
产物为单文件 AppImage，效果相当于 Linux 下的免安装应用：

```bash
chmod +x packaging/linux/build-appimage.sh
./packaging/linux/build-appimage.sh
```

生成到 `dist/*.AppImage`。使用时只需：

```bash
chmod +x 北京理工大学点对点通信-1.1.0-x86_64.AppImage
./北京理工大学点对点通信-1.1.0-x86_64.AppImage
```

## macOS
必须在 macOS 上生成 DMG：

```bash
./packaging/macos/build-dmg.sh
```

生成到 `dist/*.dmg`。

## 无本地环境时
将项目上传 GitHub，在 Actions 中运行 `Build desktop packages`，会同时产生：
- Linux x86_64 AppImage
- macOS Intel DMG
- macOS Apple Silicon DMG

当前自动构建产物未签名。首次打开 macOS 版本时，需要右键应用并选择“打开”。正式对外发布需要 Apple Developer ID 签名和公证。
