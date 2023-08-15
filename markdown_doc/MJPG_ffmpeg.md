# 视频监控项目（MJPG 方案 和 ffmpeg 方案）

推流端 -> 流媒体服务器 -> 拉流端

**常见的流媒体协议**

HTTP-FLV（http流）、RTMP（TCP流）、HLS（http流）

## 基于 MJPG-Streamer 的方案（Buildroot配置）

- 只能实现局域网的视频监控（要实现互联网监控需要内网穿透）
- 对处理器的性能要求较低；很多摄像头都支持 MJPG
- 没有实现声音传输
- 只是多个 JPEG 图片的组合，不考虑前后两帧数据的变化，总是传输一帧一帧，传输带宽要求高

```shell
mjpg_streamer -i "/usr/lib/mjpg-streamer/input_uvc.so -d /dev/video1 -f 30 -q 90 -n" -o "/usr/lib/mjpg-streamer/output_http.so -w /usr/share/mjpg-streamer/www"

http://192.168.5.9:8080/stream.html
```

![img](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/v2-913018fec9045d2772567d8ae383b0a0_r.jpg)

MJPG-Streamer 工作时，除了 main 线程，还会起 2 个线程（输入插件和输出插件）；main 线程会调用 dlopen() 函数打开指定的库文件，并运行里面的 init() 和 run() 函数

input_uvc.so init()会打开初始化摄像头；run() 会创建一个摄像头线程，摄像头线程会调用 uvcGrab() 函数，会读取摄像头，得到一帧的数据；然后调用 covert_to_JPEG 将图像转为 JPEG 格式，转换结束后，使用 copy_to_global_buffer() 将数据存放进一个全局 buffer 中

output_http init() 处理用户传入的参数，run() 会创建一个 server 线程，用来等待连接；浏览器来连接开发板 ip 后，server 端会创建一个 client 线程，用来从 buffer 中读出一帧的数据，传给浏览器

## ffmpeg 方案

![1692098956850](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1692098956850.png)

> 推流端：ffmpeg 使用 RTMP 协议向 Nginx 流媒体服务器推流
>
> 拉流端：VLC 播放器使用 RTMP 或者 HTTPFLV 协议向 Nginx 拉流；浏览器使用 HTTPFLV 协议从 Nginx 拉流（安装 flv.js）

- 原生 Nginx 不支持 RTMP 以及 HTTPFLV，需要移植第三方模块

- nginx-rtmp-module：实现了 RTMP 协议
- nginx-http-flv-module：在 RTMP 的基础上，实现了 HTTPFLV 功能

协议转换：流媒体服务器通过将 RTMP 推流数据解码，并以 FLV 格式封装，然后通过 HTTP 协议传输给客户端

![1692099405204](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1692099405204.png)

> 协议层：将标准封装格式的数据用流媒体协议进行打包；常用流媒体协议：RTMP、HTTP
>
> 容器层（封装）：将音频流压缩数据和视频流压缩数据进行封装，按一定格式放在一起；常用封装格式：MP4、FLV、AVI
>
> 编码层：将原始的音频/视频数据 压缩成 音频/视频 压缩数据；常用视频压缩协议H264；常用音频压缩标准 AAC、MP3
>
> 原始数据层（视音频同步问题）

配置 /etc/nginx/nginx.conf；添加 rtmp 节点

![1692103094870](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1692103094870.png)

```shell
ffmpeg -f v4l2 -framerate 10 -i /dev/video1 -q 10 -f flv rtmp://127.0.0.1/live.wei   # 推流
ffmpeg -f alsa -ac 1 -ar 11025 -i hw:0,0 -acodec aac -f v4l2 -framerate 10 -i /dev/video1 -q 10 -vcodec h264  -f flv rtmp://127.0.0.1/live/wei		# 推流（声音+视频）

rtmp://192.168.5.9/live/wei					   # RTMP 拉流
http://192.168.5.9/test?app=live&stream=wei		# http-flv 拉流
```

## 内网穿透

ffmpeg 和 Nginx 都部署在开发板上，拉流端只能在局域网内，不能通过外网访问 Nginx，解决方法：

- 把 Nginx 放到公网服务器上
- 使用内网穿透技术，把开发板暴露到公网

![1692102526883](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1692102526883.png)

内网穿透的两种实现方法：

![1692102789158](https://raw.githubusercontent.com/LQF376/Linux-arm-mystudy/main/markdown_pic/1692102789158.png)
