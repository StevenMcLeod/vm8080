;   void *memcpy(void *dest, void *src, size_t len) {
;       void *ret = dest;
;       while(len != 0) {
;           *dest = *src;
;           ++dest;
;           ++src;
;           --len;
;       }
;   }
;
; +6: len
; +4: src
; +2: dest
; +0: return addr

PUBLIC memcpy
memcpy:
        ; size_t len
        ld      hl, 7
        add     hl, sp
        ld      b, (hl)
        dec     hl
        ld      b, (hl)

        ; void *src
        dec     hl
        ld      d, (hl)
        dec     hl
        ld      e, (hl)

        ;void *dest
        dec     hl
        ld      a, (hl)
        dec     hl
        ld      l, (hl)
        ld      h, a

memcpy1:
        ; while(len != 0) {
        ld      a, b
        or      c
        jp      z, memcpy2

        ; *dest = *src;
        ld      a, (de)
        ld      (hl), a

        ; ++dest;
        ; ++src;
        ; --len;
        inc     hl
        inc     de
        dec     bc

        jp      nz, memcpy1

memcpy2:
        ; return ret;
        ex      (sp), hl
        ret

;memcpy2:
;        push    hl
;
;memcpy_loop:
;        ld      a, b
;        or      c
;        jp      z, memcpy_exit
;
;        ld      a, (de)
;        ld      (hl), a
;
;        inc     hl
;        inc     de
;        dec     bc
;        jp      memcpy_loop
;
;memcpy_exit:
;        pop     hl
;        ret