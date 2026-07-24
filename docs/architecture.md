# 系统结构

```mermaid
flowchart LR
    A[Linux 节点 A\nQt/C++] <-->|TCP 5000\n长度前缀 JSON| B[Linux 节点 B\nQt/C++]
    V1[RTSP/H.264 视频源] -->|RTSP 8554| A
    V2[RTSP/H.264 视频源] -->|RTSP 8554| B
```

## 文本协议

每个数据帧：`4 字节大端长度 + UTF-8 JSON`。

```json
{
  "type": "text",
  "id": "uuid",
  "timestamp": 1784909400000,
  "content": "收到，可以开始通信。"
}
```

支持 `hello`、`text`、`ack`、`heartbeat` 四种消息。
