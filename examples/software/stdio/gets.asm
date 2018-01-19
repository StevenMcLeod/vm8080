;   char *gets(char *str) {
;       char c;
;       char *ret = str;
;       while((c = getchar()) != '\n') {
;           *str = c;
;           ++str;
;       }
;       return ret;
;   }
;
; +2: str
; +0: return addr

PUBLIC gets
gets:
        ; char *str
        ld      hl, 2
        add     hl, sp
        ld      e, (hl)
        inc     hl
        ld      d, (hl)

        ; char *ret
        ld      l, e
        ld      h, d

gets1:
        ; while((c = getchar()) != '\n') {
        in      a, (0)
        cp      a, 0x0a
        jp      z, gets2

        ; *str = c
        ; ++str
        ld      (de), a
        inc     de
        jp      gets1

gets2:
        xor     a, a
        ld      (de), a
        ret