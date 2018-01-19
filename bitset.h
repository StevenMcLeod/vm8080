//
// Created by Steven on 10-Dec-17.
//

#ifndef VM8080_BITSET_H
#define VM8080_BITSET_H

//Reference: https://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching

#define EVAL_BOOL(n)                        ((n)?1:0)

#define BITS_SET(val, bits)                 (val) |= (bits)
#define BITS_RESET(val, bits)               (val) &= ~(bits)
#define BITS_FLIP(val, bits)                (val) ^= (bits)
#define BITS_TEST(val, bits)                ((val) & (bits))
#define BITS_TESTNOT(val, bits)             (~(val) & (bits))

#define BITS_SET_BIT(val, bitpos)           BITS_SET(val, 1 << (bitpos))
#define BITS_RESET_BIT(val, bitpos)         BITS_RESET(val, 1 << (bitpos))
#define BITS_FLIP_BIT(val, bitpos)          BITS_FLIP(val, 1 << (bitpos))
#define BITS_TEST_BIT(val, bitpos)          EVAL_BOOL(BITS_TEST(val, 1 << (bitpos)))
#define BITS_TESTNOT_BIT(val, bitpos)       EVAL_BOOL(!BITS_TESTNOT(val, 1 << (bitpos)))
#define BITS_COND_BIT(val, bitpos, bit)     do { if(bit) BITS_SET(val, 1 << (bitpos)); else BITS_RESET(val, 1 << (bitpos)); } while(0)


#endif //VM8080_BITSET_H
