# MAX7219x4

Arduino sketch to drive four MAX7219 chips connected to four 8x8
pixel LED display modules.
The modules are arranged to make a 16x16 pixel display, and the
frame buffer in the code treats this as a 256-bit (32-byte) bitmap.

The main loop draws a simple animation.
