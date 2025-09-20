#basics 
If we are creating a connection between client and server using TCP then it has a few functionalities like, TCP is suited for applications that require high reliability, and transmission time is relatively less critical. It is used by other protocols like HTTP, HTTPs, FTP, SMTP, Telnet. TCP rearranges data packets in the order specified. There is absolute guarantee that the data transferred remains intact and arrives in the same order in which it was sent. TCP does Flow Control and requires three packets to set up a socket connection before any user data can be sent. TCP handles reliability and congestion control. It also does error checking and error recovery. Erroneous packets are retransmitted from the source to the destination.

[[https://www.geeksforgeeks.org/c/tcp-server-client-implementation-in-c/]]

![[Socket_server-1.png]]

#ASIO Server 

#nodejs-NET
https://www.w3schools.com/nodejs/nodejs_net.asp?utm_source=chatgpt.com

https://www.w3schools.com/nodejs/nodejs_readline.asp - this is for IO on the client side 

## Socket Properties and Methods

The Socket object provided to the server connection callback and returned by `createConnection()` has many useful properties and methods:

|                                                 |                                                                                     |
| ----------------------------------------------- | ----------------------------------------------------------------------------------- |
| `socket.write(data[, encoding][, callback])`    | Writes data to the socket, optionally with the specified encoding                   |
| `socket.end([data][, encoding][, callback])`    | Closes the socket after all data is written and flushed                             |
| `socket.setEncoding(encoding)`                  | Sets the encoding for data received on the socket                                   |
| `socket.setTimeout(timeout[, callback])`        | Sets the socket to timeout after the specified number of milliseconds of inactivity |
| `socket.setKeepAlive([enable][, initialDelay])` | Enables/disables keep-alive functionality                                           |
| `socket.address()`                              | Returns an object with connection's address, family, and port                       |
| `socket.remoteAddress`                          | Remote IP address as a string                                                       |
| `socket.remotePort`                             | Remote port as a number                                                             |
| `socket.localAddress`                           | Local IP address the server is listening on                                         |
| `socket.localPort`                              | Local port the server is listening on                                               |
| `socket.bytesRead`                              | Number of bytes received                                                            |
| `socket.bytesWritten`                           | Number of bytes sent                                                                |

## Server Properties and Methods

The Server object returned by `createServer()` has these useful properties and methods:

|Property/Method|Description|
|---|---|
|`server.listen(port[, hostname][, backlog][, callback])`|Starts the server listening for connections|
|`server.close([callback])`|Stops the server from accepting new connections|
|`server.address()`|Returns an object with server's address info|
|`server.maxConnections`|Set this property to reject connections when the connection count exceeds it|
|`server.connections`|Number of concurrent connections|
|`server.listening`|Boolean indicating whether the server is listening|


#Boost
Is apart of the boost framework https://www.boost.org/doc/user-guide/task-networking.html
#SimpleChat 
https://www.geeksforgeeks.org/cpp/synchronous-chatting-application-using-c-boostasio/?utm_source=chatgpt.com

https://think-async.com/Asio/Download.html
- You will then download the ASIO version zip.
- After this:
- 
    - Extract it inside a lib or SDK folder.
    - **Note:** Don’t try to add this as an include path in VS Code — it’s a nightmare with `tasks.json`, etc.
    
- Once inside your IDE (Visual Studio preferred):
    - Right-click on the project root and go to Properties. Add the ASIO path (the one containing `asio.hpp`) to:
    
	    - Configuration Properties > VC++ Directories > Include Directories
	    - Configuration Properties > C/C++ > General > Additional Include Directories



Streaming
https://dev.epicgames.com/documentation/en-us/unreal-engine/media-framework-overview-for-unreal-engine?utm_source=chatgpt.com

https://ffmpeg.org/ffmpeg.html

https://www.wowza.com/blog/rtmp-vs-rtsp-which-protocol-should-you-choose
https://ably.com/blog/what-is-webrtc?

# Real Time Streaming Protocol (RTMP) 
# Real Time Messaging Protocol (RTMP)
# WebRTC (Real Time Communication)

- running the media mtx server on the laptop simply go to downloads path and use 
- **./mediamtx**

broad casting the media mtx server is going to be done with tail scale. this is has been installed on the machine and running it is done with
- **sudo tailscale up** 
- sudo tailscale down ----- this removes the machine from the mesh, ie it cant be seen by the other devices on the 'network'
  
sudo apt install ffmpeg -y has already been installed on the mint machine 

#example
```
ffmpeg -re -i sample.mp4 -c copy -f rtsp rtsp://localhost:8554/live.sdp
```
- `-re`: Read input at native frame rate (real time).
- `-i sample.mp4`: Input file.
- `-c copy`: Copy the streams without re-encoding.
- `-f rtsp`: Use RTSP as the output format.
- `rtsp://localhost:8554/live.sdp`: RTSP server URL and stream name.
### 4.4. Streaming from a Webcam (Linux) https://coderslegacy.com/ffmpeg-rtsp-streaming/?

To stream video from your webcam (on Linux, typically `/dev/video0`):

Bash

```
ffmpeg -f v4l2 -i /dev/video0 -c:v libx264 -preset veryfast -maxrate 3000k -bufsize 6000k -f rtsp rtsp://localhost:8554/webcam.sdp
```

For both audio and video:

Bash

```
ffmpeg -f v4l2 -i /dev/video0 -f alsa -i default -c:v libx264 -preset veryfast -maxrate 3000k -bufsize 6000k -c:a aac -b:a 128k -f rtsp rtsp://localhost:8554/camstream.sdp
```

**Explanation:**

- `-f v4l2`: Use Video4Linux2 for video capture.
- `-i /dev/video0`: Specify the webcam device.
- `-c:v libx264`: Encode video using H.264.
- `-preset veryfast`: Use the “veryfast” preset for lower latency.
- `-maxrate` and `-bufsize`: Control bitrate and buffering.
- `-f rtsp rtsp://localhost:8554/webcam.sdp`: Output stream to RTSP server.
- `-f alsa -i default`: Captures audio from the default ALSA device.



# what is being ran 

#without audio

ffmpeg -f v4l2 -i /dev/video0 -c:v libx264 -preset veryfast -tune zerolatency -maxrate 3000k -bufsize 6000k -f rtsp rtsp://127.0.0.1:8554/webcam.sdp

# with audio

ffmpeg -f v4l2 -i /dev/video0 \
  -c:v libx264 -preset veryfast -tune zerolatency -pix_fmt yuv420p \
  -c:a aac -ar 44100 -b:a 128k \
  -f rtsp rtsp://127.0.0.1:8554/webcam.sdp

# this is for RTMP 

ffmpeg -f v4l2 -i /dev/video0 \
  -c:v libx264 -preset veryfast -tune zerolatency \
  -maxrate 3000k -bufsize 6000k \
  -f flv rtmp://127.0.0.1:1935/live/webcam

# this is for WebRTC

ffmpeg -f v4l2 -i /dev/video0   -c:v libx264 -preset veryfast -tune zerolatency -pix_fmt yuv420p   -c:a aac -ar 44100 -b:a 128k   -f rtsp rtsp://127.0.0.1:8554/webcam.sdp


# terminal client check 
- ffplay rtsp://100.71.46.81:8554/webcam.sdp
- ffplay rtmp://100.71.46.81:1935/live/webcam
- http://100.71.46.81:8889/webcam.sdp

to run in browser after going inot yml and adding 
hls: yes
hlsAddress: ":8888"
hlsAlwaysRemux: yes
- http://<your_ip>:8888/webcam.sdp/index.m3u8
- http://100.71.46.81:8888/webcam.sdp/index.m3u8

# tailscale IP for Linux machine 100.71.46.81

Full Setup Guide: MediaMTX + FFmpeg + Tailscale on Linux Mint
1. Update System & Install FFmpeg
sudo apt update
sudo apt install ffmpeg v4l-utils -y

- ffmpeg → video/audio encoding/streaming.
- v4l-utils → tools to list and manage webcam devices.

Check camera:
v4l2-ctl --list-devices
2. Install MediaMTX
Download the latest amd64 build from MediaMTX releases (https://github.com/bluenviron/mediamtx/releases). Example:

wget https://github.com/bluenviron/mediamtx/releases/download/v1.15.0/mediamtx_v1.15.0_linux_amd64.tar.gz
tar -xvzf mediamtx_v1.15.0_linux_amd64.tar.gz
cd mediamtx_v1.15.0_linux_amd64
./mediamtx

You should see logs like:
[RTSP] listener opened on :8554
[RTMP] listener opened on :1935
[HLS] listener opened on :8888
3. Install & Configure Tailscale
curl -fsSL https://tailscale.com/install.sh | sh
sudo tailscale up

- Follow the login URL to authenticate.
- Get your Tailscale IP:
  tailscale ip -4

Example: 100.92.15.23

This IP is what your Unreal PC will use to connect.
4. Test Webcam Access
ffplay -f v4l2 -framerate 30 -video_size 1280x720 /dev/video0

If it shows video, good.
5. Push Webcam → MediaMTX
RTSP option:
ffmpeg -f v4l2 -i /dev/video0   -c:v libx264 -preset veryfast -tune zerolatency   -maxrate 3000k -bufsize 6000k   -f rtsp rtsp://127.0.0.1:8554/webcam.sdp

Publishes as rtsp://<tailscale-ip>:8554/webcam.sdp

RTMP option:
ffmpeg -f v4l2 -i /dev/video0   -c:v libx264 -preset veryfast -tune zerolatency   -maxrate 3000k -bufsize 6000k   -f flv rtmp://127.0.0.1:1935/live/webcam

Publishes as rtmp://<tailscale-ip>:1935/live/webcam
6. Test From Another Machine (Unreal PC via Tailscale)
RTSP:
ffplay rtsp://100.92.15.23:8554/webcam.sdp

RTMP:
ffplay rtmp://100.92.15.23:1935/live/webcam

Logs in MediaMTX should confirm a client is connected.
7. (Optional) Test in Browser (HLS)
MediaMTX auto-generates an HLS feed:
http://100.92.15.23:8888/webcam.sdp/index.m3u8

Open that in Chrome/Firefox.





USE THIS LATER WEB STREAMING GAME 
https://dev.epicgames.com/documentation/en-us/unreal-engine/getting-started-with-pixel-streaming-in-unreal-engine
