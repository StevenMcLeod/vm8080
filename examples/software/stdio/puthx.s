/*
 *  puthx.s
 *  Outputs L to the console in hex.
 *  Similar to printf("%hx", short);
 *
 *  Inputs: L
 *  Returns: None
 *  Modifies: A
 *
 *  void puthx(unsigned short n) {
 *      unsigned short temp;
 *
 *      temp = (n & 0xF0) >> 4;
 *      temp += (temp < 10) ? '0' : 'A' - 10;
 *      OUT(temp);
 *
 *      temp = n & 0x0F;
 *      temp += (temp < 10) ? '0' : 'A' - 10;
 *      OUT(temp);
 *  }
 *
 *  Bytecode: 28 bytes.

0x16, 0xad, 0xf0, 0x09, 0x09, 0x09, 0x09,
0xac, 0x0a, 0xb8, 0x02, 0xa8, 0x07,
0xa8, 0x30, 0x07,
0x16, 0xad, 0x0f,
0xac, 0x0a, 0xb8, 0x02, 0xa8, 0x07,
0xa8, 0x30, 0x07

 */

puthx:
        mov     a, l            ; unsigned short temp;
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

        mov     a, l
        and     a, 0x0F

        cmp     a, 10
        jrc     $1
        add     a, 0x07

$1:
        add     a, 0x30
        out     a