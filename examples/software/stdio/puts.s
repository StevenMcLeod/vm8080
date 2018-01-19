/*
 *  puts.s
 *  Outputs a null-terminated string to the console.
 *
 *  Inputs: Pointer to string
 *  Returns: Int of characters written in HL
 *  Modifies: A, DE, HL
 *
 *  int puts(const char *str) {
 *      const char *start = str;
 *      while(*str != '\0') {
 *          OUT(*str);
 *          ++str;
 *      }
 *
 *      return (int) (str - start);
 *  }
 *
 *  Bytecode: 22 bytes.

0xd0, 0xe0, 0xe0, 0xd2,
0x3e, 0x35,
0x17, 0x78, 0xba, 0x03,
0x07, 0xe0, 0xb5, 0xf8
0x16, 0x64, 0x48,
0x15, 0x6b, 0x40, 0xb4

 */

puts:
        movx    hl, sp
        incx    hl
        incx    hl
        movx    hl, (hl)        ;Get string pointer

        mov     e, l            ;Start address
        mov     d, h
puts_loop:
        mov     a, (hl)
        or      a, a            ;Check for terminator
        jrz     puts_ret

        out     a
        inc     hl              ;Next character
        jr      puts_loop

puts_ret:
        mov     a, l            ;Calculate size
        sub     a, e
        mov     l, a

        mov     a, h
        sbc     a, d
        mov     h, a
        ret