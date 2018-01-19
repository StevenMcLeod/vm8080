;   size_t strlen(const char *str) {
;       size_t count = 0;
;       while(*str != '\0') {
;           ++str;
;           ++count;
;       }
;       return count;
;   }
;
; +2: str
; +0: return addr

PUBLIC strlen
strlen:
        ld      hl, 2
        add     hl, sp
        ld      e, (hl)
        inc     hl
        ld      d, (hl)

        ; size_t count = 0
        ld      hl, 0

strlen1:
        ; while(*str != '\0') {
        ld      a, (de)
        or      a, a
        ; return count
        ret     z

        ; ++str
        ; ++count
        inc     de
        inc     hl

        jp      strlen1