;   char *strcat(char *dest, const char *src) {
;       char *ret = dest;
;       while(*dest != '\0') {
;           ++dest;
;       }
;
;       do {
;           *dest = *src;
;           ++dest;
;           ++src;
;       } while(*src != '\0');
;
;       return ret;
;   }
;
; +4: src
; +2: dest
; +0: return addr

PUBLIC strcat
strcat:
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

strcat1:
        ; Find the end of the dest string
        ; while(*dest != '\0') {
        ld      a, (de)
        or      a, a
        jp      z, strcat2

        ; ++dest
        inc     de
        jp      strcat1

strcat2:
        ; Append the src string
        ; do {
        ; *dest = *src
        ld      a, (hl)
        ld      (de), a

        ; ++dest
        ; ++src
        inc     de
        inc     hl

        ; } while(*src != '\0')
        or      a, a
        jp      nz, strcat2

        ; return ret
        ld      hl, 2
        add     hl, sp
        ld      a, (hl)
        inc     hl
        ld      h, (hl)
        ld      l, a
        ret