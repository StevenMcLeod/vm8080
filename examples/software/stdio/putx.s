/*
 *  putx.s
 *  Outputs HL to the console in hex.
 *  Similar to printf("%x", int);
 *
 *  Inputs: H, L
 *  Returns: None
 *  Modifies: A
 *
 *  void puthx(unsigned int n) {
 *      unsigned short hb, lb;
 *
 *      hb = (n & 0xF000) >> 12;
 *      temp += (temp < 10) ? '0' : 'A' - 10;
 *      OUT(temp);
 *
 *      hb = (n & 0x0F00) >> 8;
 *      temp += (temp < 10) ? '0' : 'A' - 10;
 *      OUT(temp);
 *
 *      lb = (n & 0x00F0) >> 4;
 *      temp += (temp < 10) ? '0' : 'A' - 10;
 *      OUT(temp);
 *
 *      lb = (n & 0x000F);
 *      temp += (temp < 10) ? '0' : 'A' - 10;
 *      OUT(temp);
 *  }
 *
 *  Bytecode: 56 bytes.

0x15, 0xad, 0xf0, 0x09, 0x09, 0x09, 0x09,
0xac, 0x0a, 0xb8, 0x02, 0xa8, 0x07,
0xa8, 0x30, 0x07,
0x15, 0xad, 0x0f,
0xac, 0x0a, 0xb8, 0x02, 0xa8, 0x07,
0xa8, 0x30, 0x07,

0x16, 0xad, 0xf0, 0x09, 0x09, 0x09, 0x09,
0xac, 0x0a, 0xb8, 0x02, 0xa8, 0x07,
0xa8, 0x30, 0x07,
0x16, 0xad, 0x0f,
0xac, 0x0a, 0xb8, 0x02, 0xa8, 0x07,
0xa8, 0x30, 0x07

 */

puthx:
        mov     a, h            ; unsigned short hb;
        and     a, 0xF0         ; temp = (n & 0xF0) >> 4;
        srl     a
        srl     a
        srl     a
        srl     a

        cmp     a, 10           ; (temp < 10)
        jrc     $0
        add     a, 0x07         ; First part of add: 'A' - '0' - 0x0A

$0:
        add     a, 0x30         ; Second part of add: '0'
        out     a

        mov     a, h
        and     a, 0x0F

        cmp     a, 10
        jrc     $1
        add     a, 0x07

$1:
        add     a, 0x30
        out     a

        mov     a, l            ; unsigned short lb;
        and     a, 0xF0         ; temp = (n & 0xF0) >> 4;
        srl     a
        srl     a
        srl     a
        srl     a

        cmp     a, 10           ; (temp < 10)
        jrc     $2
        add     a, 0x07         ; First part of add: 'A' - '0' - 0x0A

$2:
        add     a, 0x30         ; Second part of add: '0'
        out     a

        mov     a, l
        and     a, 0x0F

        cmp     a, 10
        jrc     $3
        add     a, 0x07

$3:
        add     a, 0x30
        out     a