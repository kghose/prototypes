// clang++ capture.cpp -ocapture

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

int main() {
  // 1. Open the video device node
  int fd = open("/dev/video0", O_RDWR);
  if (fd < 0) {
    perror("Error: Cannot open /dev/video0. Is the camera plugged in?");
    return 1;
  }

  // 2. Negotiate video settings (640x480 resolution, MJPEG compression)
  struct v4l2_format fmt = {0};
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = 640;
  fmt.fmt.pix.height = 480;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG; // Native hardware JPEG stream
  fmt.fmt.pix.field = V4L2_FIELD_NONE;

  if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
    perror("Error: Device does not support native MJPEG format");
    close(fd);
    return 1;
  }

  // 3. Request memory buffers inside kernel space
  struct v4l2_requestbuffers req = {0};
  req.count = 1; // We only need 1 frame slot
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
    perror("Error: Could not request memory allocation from driver");
    close(fd);
    return 1;
  }

  // 4. Map the kernel space frame buffer into accessible user program memory
  struct v4l2_buffer buf = {0};
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index = 0;

  if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
    perror("Error: Failed to query internal buffer specs");
    close(fd);
    return 1;
  }

  void *buffer_start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, buf.m.offset);
  if (buffer_start == MAP_FAILED) {
    perror("Error: Memory-map (mmap) binding failed");
    close(fd);
    return 1;
  }

  // 5. Hand the empty buffer slot back to the device driver queue
  if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
    perror("Error: Failed to queue the operational memory buffer");
    munmap(buffer_start, buf.length);
    close(fd);
    return 1;
  }

  // 6. Turn on hardware sensor data streaming
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
    perror("Error: Stream activation failed");
    munmap(buffer_start, buf.length);
    close(fd);
    return 1;
  }

  std::cout << "[*] Stream active. Dequeuing the next available video frame..."
            << std::endl;

  // 7. Wait and extract the freshly captured frame data slot from the driver
  // queue
  if (ioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
    perror("Error: Failed to pull (dequeue) frame out of processing slots");
    ioctl(fd, VIDIOC_STREAMOFF, &type);
    munmap(buffer_start, buf.length);
    close(fd);
    return 1;
  }

  std::cout << "[!] Frame successfully intercepted: "
            << (buf.bytesused / 1024.0) << " KB gathered." << std::endl;

  // 8. Dump the raw memory data directly out into a JPEG file
  std::ofstream outFile("snapshot.jpg", std::ios::binary);
  if (outFile.is_open()) {
    outFile.write(reinterpret_cast<const char *>(buffer_start), buf.bytesused);
    outFile.close();
    std::cout << "[✓] Saved image as 'snapshot.jpg'" << std::endl;
  } else {
    std::cerr << "Error: Could not write output file to storage disk."
              << std::endl;
  }

  // 9. Shutdown components cleanly
  ioctl(fd, VIDIOC_STREAMOFF, &type);
  munmap(buffer_start, buf.length);
  close(fd);
  return 0;
}
