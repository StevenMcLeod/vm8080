;   int puts(const char *str) {
;       int c = 0;
;       while(*str != '\0') {
;           putchar(*str);
;           ++str;
;           ++c;
;       }
;       return c;
;   }
;
; +2: str
; +0: return addr

PUBLIC puts
puts:
        ; char *str
        ld      hl, 2
        add     hl, sp
        ld      e, (hl)
        inc     hl
        ld      d, (hl)

        ; int c = 0
        ld      hl, 0

puts1:
        ; while(*str != '\0') {
        ld      a, (de)
        or      a, a
        jp      z, puts2

        ; putchar(*str)
        ; ++str
        ; ++c
        out     (0), a
        inc     de
        inc     hl
        jp      puts1

puts2:
        ld      a, 0x0a
        out     (0), a
        ret