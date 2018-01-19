/*
 *  mul8_slow.s
 *  Multiplies B by C and puts the result in L.
 *  Uses the repeated add algorithm.
 *
 *  Inputs: B, C
 *  Returns: H, L
 *  Modifies: A, B, C = 0, H, L
 *
 *  Bytecode: 14 bytes.

0xa5, 0x00, 0x80, 0x92,
0x9a, 0xba, 0x06,
0x51, 0xb9, 0xfa,
0x95, 0xb5, 0xf7,
0x48

 */

mul8_slow:
        mov     h, 0       ;Clear result
        xor     a, a
        inc     c

$0:
        dec     c
        jrz     $1

        add     a, b
        jrnc    $0

        inc     h
        jr      $0

$1:
        mov     l, a
