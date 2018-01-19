; HL = HL - DE
PUBLIC int_sub

int_sub:
        ld      a, e
        sub     l
        ld      l, a
        ld      a, d
        sbc     a, h
        ld      h, a
        ret