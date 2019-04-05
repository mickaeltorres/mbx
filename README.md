# MBX

Base converter with ability to take bit slice.

Enter your value in the first field:
- 0x.... for hexa
- 0b.... for binary
- 0..... for octal
- ...... for decimal

Then you can enter a bit slice to use on the second field like:
- start:end
- end:start

It will use only the bits between start and end inclusive.

Ctrl+C or Ctrl+Q to quit.

# Why ?

Made while developping/debugging verilog code, after spending way too many hours counting bits on the screen in bc.

# Compiling

Might need to add -lbsd to LDFLAGS to compile on linux, not tested.