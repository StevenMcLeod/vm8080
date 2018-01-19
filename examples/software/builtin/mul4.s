/*
 *  mul4.s
 *  Multiplies B by C and puts the result in L.
 *  Multiplying with a number larger than 4 bits long is undefined behaviour.
 *
 *  Inputs: B, C
 *  Returns: L
 *  Modifies: A, B, C, D = 0, L
 *
 *  Bytecode: x bytes.

0xa6, 0x00, 0xa3, 0x04,
0x12, 0x0f, 0x28, 0xb9, 0x03,
0x11, 0x56, 0x48,
0x11, 0x08, 0x20,
0x9b, 0xbb, 0xf2

 */

mul4:
        mov     l, 0        ; Clear result
        mov     d, 4        ; Loop Counter

mul4_loop1:
        mov     a, c
        rrc     a
        mov     c, a
        jrnc    mul4_loop2  ; Skip add

        mov     a, b        ; Add factor
        add     a, l
        mov     l, a

mul4_loop2:
        mov     a, b
        sll     a
        mov     b, a

        dec     d
        jrnz    mul4_loop1


