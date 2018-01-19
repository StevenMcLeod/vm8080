; Dest in HL, Source in DE, Count in BC
memcpy:
        ld      a, b
        or      c
        ret     z
        push    hl      ; For return

memcpy_loop:
        ld      a, (de)
        ld      (hl), a

        inc     hl
        inc     de
        dec     bc

        ld      a, b
        or      c
        jp      nz, memcpy_loop

        pop     hl
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