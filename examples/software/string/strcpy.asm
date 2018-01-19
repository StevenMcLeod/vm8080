;   char *strcpy(char *dest, const char *src) {
;       void *ret = dest;
;       do {
;           *dest = *src;
;           ++dest;
;           ++src;
;       } while(*src != '\0');
;       return ret;
;   }
;
; +4: src
; +2: dest
; +0: return addr

PUBLIC strcpy
strcpy:
        ; char *dest
        ld      hl, 2
        add     hl, sp
        ld      e, (hl)
        inc     hl
        ld      d, (hl)

        ; char *src
        inc     hl
        ld      a, (hl)
        inc     hl
        ld      h, (hl)
        ld      l, a

strcpy1:
        ; do {
        ; *dest = *src;
        ld      a, (hl)
        ld      (de), a

        ; ++dest;
        ; ++src;
        inc     de
        inc     hl

        ; while(*src != '\0')
        or      a, a
        jp      nz, strcpy1

        ; return ret;
        ld      hl, 2
        add     hl, sp
        ld      a, (hl)
        inc     hl
        ld      h, (hl)
        ld      l, a
        ret