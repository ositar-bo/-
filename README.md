# 北京理工大学点对点通信系统（Linux 原型正式版）

Qt 6 / C++17 实现，可在两台 Linux 电脑上直接测试双向文字通信，并接收 RTSP/HTTP/本地视频流。

## 已实现

- 任一节点都能作为 TCP 服务端监听，也能作为客户端连接
- 双向文字发送与接收
- 长度前缀 JSON 协议，解决 TCP 粘包与拆包
- 消息 UUID、接收确认、5 秒心跳
- 单对端连接限制和异常处理
- 正式深色界面
- Qt Multimedia 视频播放入口

## Ubuntu 22.04/24.04 安装依赖

```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build \
  qt6-base-dev qt6-multimedia-dev \
  libgl1-mesa-dev gstreamer1.0-plugins-base \
  gstreamer1.0-plugins-good gstreamer1.0-plugins-bad \
  gstreamer1.0-plugins-ugly gstreamer1.0-libav
```

## 编译

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/bit-p2p-comm
```

## 两台 Linux 测试

假设：

- 节点 A：`192.168.1.100`
- 节点 B：`192.168.1.200`
- TCP 端口：`5000`

节点 B 点击“开始监听”。节点 A 输入 `192.168.1.200` 和 `5000`，点击“连接对端”。连接后双方均可发送文字。

同一台电脑测试时，打开两个程序：一个监听 `5000`，另一个连接 `127.0.0.1:5000`。

防火墙放行：

```bash
sudo ufw allow 5000/tcp
sudo ufw allow 8554/tcp
```

## 视频测试

界面的视频接收依赖系统的 GStreamer/FFmpeg 后端支持。可以先用本地 MP4 文件验证：

```text
/home/user/test.mp4
```

也可以填写：

```text
rtsp://192.168.1.200:8554/live
```

快速建立 RTSP 服务建议使用 MediaMTX。服务启动后，在视频发送端执行：

```bash
ffmpeg -re -stream_loop -1 -i test.mp4 \
  -c:v libx264 -preset ultrafast -tune zerolatency \
  -an -f rtsp rtsp://127.0.0.1:8554/live
```

另一台机器在软件中填写：

```text
rtsp://发送端IP:8554/live
```

注意：不同 Linux 发行版的 Qt Multimedia 对 RTSP 支持取决于其媒体后端。若 Qt 播放 RTSP 不稳定，正式部署建议把视频模块替换为 GStreamer C API 管线，文本模块无需改动。

## 后续 Android 迁移

保持当前 TCP JSON 协议不变，Android 端用 Kotlin 实现同样的 `4 字节长度 + JSON` 编解码，即可与本程序互通。
