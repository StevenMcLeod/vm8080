/*
 *  gets.s
 *  Inputs a string from the console ended by a newline.
 *
 *  Inputs: Pointer to string
 *  Returns: Int of characters written in HL
 *  Modifies: A, DE, HL
 *
 *  int puts(char *str) {
 *      const char *start = str;
 *      do {
 *          register char c = IN();
 *          *str = c;
 *          ++str;
 *      } while(c != '\n');
 *
 *      return (int) (str - start);
 *  }
 *
 *  Bytecode: 26 bytes.

0xd0, 0xd9, 0x02, 0x00, 0xd2,
0x3e, 0x35,
0x06, 0x18, 0xe0,
0xac, 0x0a, 0xbb, 0xf9, 0xa7, 0x00,
0x16, 0x64, 0x48,
0x15, 0x6b, 0x40, 0xb4

 */

.equ    NEWLINE     0x0A

gets:
        movx    hl, sp
        addx    hl, 3
        movx    hl, (hl)        ;Load address of buffer

        mov     e, l
        mov     d, h            ;Load start

gets_loop1:
        in      a               ;Next character
        mov     (hl), a
        inc     hl              ;Increment pointer

gets_loop2:
        cmp     NEWLINE
        jnz     gets_loop1
        mov     (hl), 0         ;Null Terminate String

        mov     a, l            ;Calculate size
        sub     a, e
        mov     l, a

        mov     a, h
        sbc     a, d
        mov     h, a
        ret