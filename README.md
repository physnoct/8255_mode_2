# 8255_mode_2
Example code to use the PIO 8255 in mode 2, using interrupts.

There are not much examples on the web to use the 8255 PIO in mode 2. My particular setup uses the PIO in mode 2 for the port A and mode 1 for the port B as input.
I use a Z80 on the bus side, using the Z80 interrupt mode 2.

On the peripheral side, a keyboard is plugged on the port B, and a /STB pulse is generated when a key is pressed.
A bidirectional "terminal" is plugged on the port A. The terminal is another Z80 which can receive and transmit bytes to the host Z80.

I had difficulties getting this to work without interrupts, as I couldn't receive bytes from the terminal or the keyboard without first writing to the terminal.
By using interrupts, this solved the problem.

Arduino side: .cpp, .h, .ino files
Z80 side: .pre files (assemble and link with SDCC assembler: sdasz80 -glos *.pre)
