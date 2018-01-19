/*
 *  memcpy.s
 *  Copies one buffer at dest to another buffer at src.
 *
 *  Inputs: void *dest, void *src, size_t n
 *  Returns: ptr
 *  Modifies: A, B, C, H, L
 *
 *  void *memcpy(void *dest, const void *src, size_t n) {
 *      while(n != 0) {
 *          *ptr = *src;
 *          ++dest;
 *          ++src;
 *          --n;
 *      }
 *  }
 *
 *  Bytecode: 40 bytes.

0xd0, 0xe0, 0xe0, 0x2f, 0xe0, 0x27,
0xe0, 0x3e, 0x35,
0x11, 0x7a, 0xba, 0x15,
0x4c, 0x43, 0xd2, 0x17,
0x4c, 0x43, 0xe0, 0xe0, 0xd2, 0x18,
0xe0, 0x94, 0xb9, 0x01, 0x93,
0x9a, 0xb9, 0xea, 0x99, 0xb5, 0xe7,
0xd0, 0xd9, 0x05, 0x00, 0xd2, 0xb4,

 */

//KNOWN ERROR: *dest AND *src ARE NOT BEING INCREMENTED PROPERLY!!!

memset:
        movx    hl, sp
        incx    hl          ; size_t n
        incx    hl
        mov     c, (hl)
        incx    hl
        mov     b, (hl)

        incx    hl          ; Prepare for load
        mov     e, l
        mov     d, h

$0:
        mov     a, b        ; while(n != 0)
        or      c
        jrz     $2

        mov     l, e        ; *src
        mov     h, d
        movx    hl, (hl)
        mov     a, (hl)

        mov     l, e        ; *dest
        mov     h, d
        incx    hl
        incx    hl
        movx    hl, (hl)
        mov     (hl), a     ; *dest = *src

        incx    hl          ; ++dest
        inc     e           ; ++src
        jrnc    $1
        inc     d

$1:
        dec     c           ; --n
        jrnc    $0
        dec     b
        jr      $0

$2:
        movx    hl, sp
        addx    hl, 5
        movx    hl, (hl)
        ret