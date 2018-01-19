;   int strcmp(const char *dest, const char *src) {
;       while(*dest != '\0' && *src != '\0') {
;           if(*dest != *src) {
;               break;
;           }
;           ++dest;
;           ++src;
;       }
;
;       return (int) (*dest) - (int) (*src);
;   }
;
; +4: src
; +2: dest
; +0: return addr

PUBLIC strcmp
EXTERN  int_sub

strcmp:
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

strcmp1:
        ; while(
        ; *dest != '\0'
        ld      a, (de)
        or      a, a
        jp      z, strcmp2
        ld      b, a

        ; && *src != '\0') {
        ld      a, (hl)
        or      a, a
        jp      z, strcmp2

        ; if(*dest != *src) {
        cp      a, b
        jp      nz, strcmp2
        ; ++dest
        ; ++src
        inc     de
        inc     hl
        jp      strcmp1

        ;break
strcmp2:
        ; (int) (*dest)
        ld      l, (hl)
        ld      h, 0

        ; (int) (*src)
        ld      a, (de)
        ld      e, a
        ld      d, h

        ; return (int) (*dest) - (int) (*src)
        call    int_sub
        ret