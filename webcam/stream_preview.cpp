// clang++ -O3 -Wall stream_preview.cpp -o stream_preview -lX11


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <fcntl.h>
#include <iostream>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main() {
  const int WIDTH = 640;
  const int HEIGHT = 480;

  // ==========================================
  // 1. INITIALIZE V4L2 HARDWARE STREAM
  // ==========================================
  int fd = open("/dev/video0", O_RDWR);
  if (fd < 0) {
    perror("Error: Cannot open /dev/video0");
    return 1;
  }

  struct v4l2_format fmt = {0};
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = WIDTH;
  fmt.fmt.pix.height = HEIGHT;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;

  if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
    perror("Error: Hardware MJPEG not supported");
    close(fd);
    return 1;
  }

  // Allocate 2 buffer slots for fluid ping-pong streaming overhead
  struct v4l2_requestbuffers req = {0};
  req.count = 2;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  ioctl(fd, VIDIOC_REQBUFS, &req);

  // Map both buffers into our user address space
  void *buffers[2];
  size_t buffer_lengths[2];

  for (int i = 0; i < 2; ++i) {
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    ioctl(fd, VIDIOC_QUERYBUF, &buf);

    buffers[i] = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                      buf.m.offset);
    buffer_lengths[i] = buf.length;

    // Queue the empty buffer slot back to the kernel
    ioctl(fd, VIDIOC_QBUF, &buf);
  }

  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ioctl(fd, VIDIOC_STREAMON, &type);

  // ==========================================
  // 2. INITIALIZE NATIVE DISPLAY RENDERING (X11)
  // ==========================================
  Display *display = XOpenDisplay(NULL);
  if (!display) {
    std::cerr << "Error: Unable to open X11 graphical display environment."
              << std::endl;
    ioctl(fd, VIDIOC_STREAMOFF, &type);
    close(fd);
    return 1;
  }

  int screen = DefaultScreen(display);
  Window root = RootWindow(display, screen);

  Window win = XCreateSimpleWindow(display, root, 100, 100, WIDTH, HEIGHT, 1,
                                   BlackPixel(display, screen),
                                   WhitePixel(display, screen));

  // Listen for graphics refreshing, keyboard hits, and window destruction
  // events
  XSelectInput(display, win, ExposureMask | KeyPressMask | StructureNotifyMask);
  XMapWindow(display, win);
  XStoreName(display, win, "Minimalist Edge AI Live Stream");

  // Create a listener to intercept window frame close requests safely
  Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, win, &wmDeleteMessage, 1);

  GC gc = DefaultGC(display, screen);

  // Allocate memory for our display buffer (X11 maps directly to this memory
  // block)
  unsigned char *display_pixels = (unsigned char *)malloc(WIDTH * HEIGHT * 4);
  XImage *ximage = XCreateImage(
      display, DefaultVisual(display, screen), 24, ZPixmap, 0,
      reinterpret_cast<char *>(display_pixels), WIDTH, HEIGHT, 32, 0);

  std::cout
      << "[!] Live stream active! Press any key or close the window to exit."
      << std::endl;

  // ==========================================
  // 3. THE HIGH-SPEED EXECUTION LOOP
  // ==========================================
  bool running = true;
  while (running) {
    // --- A. Non-blocking UI Input Handlers ---
    while (XPending(display)) {
      XEvent event;
      XNextEvent(display, &event);

      if (event.type == KeyPress) {
        running = false;
      }
      if (event.type == ClientMessage &&
          (Atom)event.xclient.data.l[0] == wmDeleteMessage) {
        running = false; // User clicked the 'X' button on the window frame
      }
    }

    if (!running)
      break;

    // --- B. Pull Fresh Hardware Camera Frame ---
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    // Wait for hardware to deliver a frame slot
    if (ioctl(fd, VIDIOC_DQBUF, &buf) < 0)
      continue;

    // --- C. In-Memory Decompress ---
    int img_w, img_h, img_channels;
    unsigned char *decoded_raw = stbi_load_from_memory(
        reinterpret_cast<unsigned char *>(buffers[buf.index]), buf.bytesused,
        &img_w, &img_h, &img_channels, 4);

    if (decoded_raw) {
      // Re-order RGB bytes into X11 native BGRA scheme directly into display
      // memory
      for (int i = 0; i < WIDTH * HEIGHT * 4; i += 4) {
        display_pixels[i] = decoded_raw[i + 2];     // Blue
        display_pixels[i + 1] = decoded_raw[i + 1]; // Green
        display_pixels[i + 2] = decoded_raw[i];     // Red
        display_pixels[i + 3] = 255;                // Alpha
      }
      stbi_image_free(decoded_raw);

      // Force repaint onto screen canvas surface
      XPutImage(display, win, gc, ximage, 0, 0, 0, 0, WIDTH, HEIGHT);
      XFlush(
          display); // Push pixel stack changes to X Display Server immediately
    }

    // Return the used frame slot back to the V4L2 camera queue driver
    ioctl(fd, VIDIOC_QBUF, &buf);

    // Micro-sleep (approx 30fps baseline timing adjustment)
    usleep(16000);
  }

  // ==========================================
  // 4. CLEAN DISMANTLE PROCESS
  // ==========================================
  std::cout << "\n[*] Shutting down window environments cleanly..."
            << std::endl;
  XDestroyImage(
      ximage); // Automatically handles freeing display_pixels buffer internally
  XDestroyWindow(display, win);
  XCloseDisplay(display);

  ioctl(fd, VIDIOC_STREAMOFF, &type);
  for (int i = 0; i < 2; ++i) {
    munmap(buffers[i], buffer_lengths[i]);
  }
  close(fd);

  std::cout << "[✓] Pipeline terminated safely." << std::endl;
  return 0;
}
