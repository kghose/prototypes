// clang++ -O3 -Wall capture_and_show.cpp -o capture_and_show -lX11

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <fcntl.h>
#include <iostream>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

// Initialize the single-header image decoder
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main() {
  // ==========================================
  // 1. CAPTURE THE FRAME VIA V4L2 (WEBCAM)
  // ==========================================
  int fd = open("/dev/video0", O_RDWR);
  if (fd < 0) {
    perror("Error: Cannot open /dev/video0");
    return 1;
  }

  struct v4l2_format fmt = {0};
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = 640;
  fmt.fmt.pix.height = 480;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;

  if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
    perror("Error: Hardware MJPEG not supported");
    close(fd);
    return 1;
  }

  struct v4l2_requestbuffers req = {0};
  req.count = 1;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  ioctl(fd, VIDIOC_REQBUFS, &req);

  struct v4l2_buffer buf = {0};
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index = 0;
  ioctl(fd, VIDIOC_QUERYBUF, &buf);

  void *buffer_start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, buf.m.offset);
  ioctl(fd, VIDIOC_QBUF, &buf);

  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ioctl(fd, VIDIOC_STREAMON, &type);
  ioctl(fd, VIDIOC_DQBUF, &buf); // Capture frame

  std::cout << "[*] Frame captured dynamically inside memory buffers."
            << std::endl;

  // ==========================================
  // 2. DECOMPRESS JPEG MEMORY TO RAW RGB PIXELS
  // ==========================================
  int img_w, img_h, img_channels;
  // Decode the JPEG buffer straight from system RAM back into uncompressed
  // pixels
  unsigned char *rgb_data = stbi_load_from_memory(
      reinterpret_cast<unsigned char *>(buffer_start), buf.bytesused, &img_w,
      &img_h, &img_channels, 4 // Force 4 channels (RGBA) for X11 compatibility
  );

  if (!rgb_data) {
    std::cerr << "Error: Failed to decompress JPEG memory layout." << std::endl;
    ioctl(fd, VIDIOC_STREAMOFF, &type);
    munmap(buffer_start, buf.length);
    close(fd);
    return 1;
  }

  // Convert standard RGBA pixels to the BGRA format that the X11 engine
  // natively expects
  for (int i = 0; i < img_w * img_h * 4; i += 4) {
    unsigned char r = rgb_data[i];
    rgb_data[i] = rgb_data[i + 2]; // Move Blue to Red channel slot
    rgb_data[i + 2] = r;           // Move Red to Blue channel slot
  }

  // Clean up V4L2 handles since the raw pixels are now copied to our local
  // pointer
  ioctl(fd, VIDIOC_STREAMOFF, &type);
  munmap(buffer_start, buf.length);
  close(fd);

  // ==========================================
  // 3. NATIVE DISPLAY RENDERING (X11 WINDOW)
  // ==========================================
  Display *display = XOpenDisplay(NULL);
  if (!display) {
    std::cerr << "Error: Unable to open X11 graphical display environment."
              << std::endl;
    stbi_image_free(rgb_data);
    return 1;
  }

  int screen = DefaultScreen(display);
  Window root = RootWindow(display, screen);

  // Create the system window structure
  Window win = XCreateSimpleWindow(display, root, 100, 100, img_w, img_h, 1,
                                   BlackPixel(display, screen),
                                   WhitePixel(display, screen));

  // Register interest in keyboard presses and window exposure events
  XSelectInput(display, win, ExposureMask | KeyPressMask);
  XMapWindow(display, win);
  XStoreName(display, win, "Minimalist Edge AI Preview");

  // Bind the raw pixel pointer data directly to an X11 Image Matrix structure
  XImage *ximage =
      XCreateImage(display, DefaultVisual(display, screen), 24, ZPixmap, 0,
                   reinterpret_cast<char *>(rgb_data), img_w, img_h, 32, 0);

  GC gc = DefaultGC(display, screen);
  std::cout << "[!] Window open! Click window and press any key to exit."
            << std::endl;

  // Simple structural graphics event execution loop
  bool running = true;
  while (running) {
    XEvent event;
    XNextEvent(display, &event);

    if (event.type == Expose) {
      // Draw the image matrix onto the window surface
      XPutImage(display, win, gc, ximage, 0, 0, 0, 0, img_w, img_h);
    }
    if (event.type == KeyPress) {
      running = false; // Exit window wrapper loop on any keypress
    }
  }

  // Clean up graphics pointers and exit
  XDestroyImage(ximage); // Automatically calls stbi_image_free internally
  XDestroyWindow(display, win);
  XCloseDisplay(display);
  std::cout << "[✓] Pipeline closed cleanly." << std::endl;

  return 0;
}
