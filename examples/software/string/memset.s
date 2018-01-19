/*
 *  memset.s
 *  Sets the first n bytes starting at ptr with value.
 *
 *  Inputs: void *ptr, unsigned char value, size_t n
 *  Returns: Pointer to ptr + n
 *  Modifies: A, B, C, H, L
 *
 *  void *memset(void *ptr, unsigned char value, size_t n) {
 *      while(n != 0) {
 *          *ptr = value;
 *          ++ptr;
 *          --n;
 *      }
 *  }
 *
 *  Bytecode: 28 bytes.

0xd0, 0xe0, 0xe0, 0x2f, 0xe0, 0x27,
0xe0, 0x37,
0xe0, 0xd2,
0x11, 0x7a, 0xba, 0x08,
0x1b, 0xe0,
0x9a, 0xb9, 0xf7, 0x99, 0xb5, 0xf4,
0xd0, 0xd9, 0x05, 0x00, 0xd2, 0xb4,

 */

memset:
        movx    hl, sp
        incx    hl          ; size_t n
        incx    hl
        mov     c, (hl)
        incx    hl
        mov     b, (hl)

        incx    hl          ; unsigned char value
        mov     d, (hl)

        incx    hl          ; void *ptr
        movx    hl, (hl)

$0:
        mov     a, b        ; while(n != 0)
        or      c
        jrz     $1

        mov     (hl), d     ; *ptr = value
        incx    hl          ; ++ptr

        dec     c           ; --n
        jrnc    $0
        dec     b
        jr      $0

$1:
        movx    hl, sp
        addx    hl, 5
        movx    hl, (hl)
        ret