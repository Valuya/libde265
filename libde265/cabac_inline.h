#ifndef DE265_CABAC_INLINE_H
#define DE265_CABAC_INLINE_H

#include "util.h"
#include <assert.h>
#include <stdio.h>

static const uint8_t LPS_table[64][4] =
  {
    { 128, 176, 208, 240},
    { 128, 167, 197, 227},
    { 128, 158, 187, 216},
    { 123, 150, 178, 205},
    { 116, 142, 169, 195},
    { 111, 135, 160, 185},
    { 105, 128, 152, 175},
    { 100, 122, 144, 166},
    {  95, 116, 137, 158},
    {  90, 110, 130, 150},
    {  85, 104, 123, 142},
    {  81,  99, 117, 135},
    {  77,  94, 111, 128},
    {  73,  89, 105, 122},
    {  69,  85, 100, 116},
    {  66,  80,  95, 110},
    {  62,  76,  90, 104},
    {  59,  72,  86,  99},
    {  56,  69,  81,  94},
    {  53,  65,  77,  89},
    {  51,  62,  73,  85},
    {  48,  59,  69,  80},
    {  46,  56,  66,  76},
    {  43,  53,  63,  72},
    {  41,  50,  59,  69},
    {  39,  48,  56,  65},
    {  37,  45,  54,  62},
    {  35,  43,  51,  59},
    {  33,  41,  48,  56},
    {  32,  39,  46,  53},
    {  30,  37,  43,  50},
    {  29,  35,  41,  48},
    {  27,  33,  39,  45},
    {  26,  31,  37,  43},
    {  24,  30,  35,  41},
    {  23,  28,  33,  39},
    {  22,  27,  32,  37},
    {  21,  26,  30,  35},
    {  20,  24,  29,  33},
    {  19,  23,  27,  31},
    {  18,  22,  26,  30},
    {  17,  21,  25,  28},
    {  16,  20,  23,  27},
    {  15,  19,  22,  25},
    {  14,  18,  21,  24},
    {  14,  17,  20,  23},
    {  13,  16,  19,  22},
    {  12,  15,  18,  21},
    {  12,  14,  17,  20},
    {  11,  14,  16,  19},
    {  11,  13,  15,  18},
    {  10,  12,  15,  17},
    {  10,  12,  14,  16},
    {   9,  11,  13,  15},
    {   9,  11,  12,  14},
    {   8,  10,  12,  14},
    {   8,   9,  11,  13},
    {   7,   9,  11,  12},
    {   7,   9,  10,  12},
    {   7,   8,  10,  11},
    {   6,   8,   9,  11},
    {   6,   7,   9,  10},
    {   6,   7,   8,   9},
    {   2,   2,   2,   2}
  };

static const uint8_t renorm_table[32] =
  {
    6,  5,  4,  4,
    3,  3,  3,  3,
    2,  2,  2,  2,
    2,  2,  2,  2,
    1,  1,  1,  1,
    1,  1,  1,  1,
    1,  1,  1,  1,
    1,  1,  1,  1
  };

static const uint8_t next_state_MPS[64] =
  {
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
    17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
    33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
    49,50,51,52,53,54,55,56,57,58,59,60,61,62,62,63
  };

static const uint8_t next_state_LPS[64] =
  {
    0,0,1,2,2,4,4,5,6,7,8,9,9,11,11,12,
    13,13,15,15,16,16,18,18,19,19,21,21,22,22,23,24,
    24,25,26,26,27,27,28,29,29,30,30,30,31,32,32,33,
    33,33,34,34,35,35,35,36,36,36,37,37,37,38,38,63
  };

#if defined(__x86_64__) && defined(__GNUC__) && !defined(DE265_LOG_TRACE)
#define DE265_CABAC_ASM_X86_64 1
// Combined state-transition table (folds the MPS-flip at state 0), indexed by
// the packed context byte (state*2+MPSbit) for MPS and (packed ^ 0xFF) for LPS.
static const uint8_t cabac_transition[256] =
  {
      2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
     18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
     34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
     50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
     66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81,
     82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97,
     98, 99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,
    114,115,116,117,118,119,120,121,122,123,124,125,124,125,126,127,
    127,126, 77, 76, 77, 76, 75, 74, 75, 74, 75, 74, 73, 72, 73, 72,
     73, 72, 71, 70, 71, 70, 71, 70, 69, 68, 69, 68, 67, 66, 67, 66,
     67, 66, 65, 64, 65, 64, 63, 62, 61, 60, 61, 60, 61, 60, 59, 58,
     59, 58, 57, 56, 55, 54, 55, 54, 53, 52, 53, 52, 51, 50, 49, 48,
     49, 48, 47, 46, 45, 44, 45, 44, 43, 42, 43, 42, 39, 38, 39, 38,
     37, 36, 37, 36, 33, 32, 33, 32, 31, 30, 31, 30, 27, 26, 27, 26,
     25, 24, 23, 22, 23, 22, 19, 18, 19, 18, 17, 16, 15, 14, 13, 12,
     11, 10,  9,  8,  9,  8,  5,  4,  5,  4,  3,  2,  1,  0,  0,  1,
  };
#endif

inline __attribute__((always_inline)) int CABAC_decoder::decode_bit(context_model* model)
{
#if defined(DE265_CABAC_ASM_X86_64) && !defined(DE265_CABAC_C)
  // The asm's branch-free decision in plain C++, so that the compiler can keep the
  // engine in registers instead of reloading memory after every bin ("memory" clobber).
  uint8_t* sp = reinterpret_cast<uint8_t*>(model);
  const uint32_t b = *sp;                                   // state*2 + MPS
  const uint32_t lps = LPS_table[b >> 1][(range >> 6) - 4];
  const uint32_t r_mps = range - lps;
  const uint32_t scaled = r_mps << 7;
  const uint32_t lps_mask = ~uint32_t(int32_t(value - scaled) >> 31);  // all ones if LPS
  uint32_t v = value - (scaled & lps_mask);
  uint32_t r = r_mps + ((lps - r_mps) & lps_mask);
  const uint32_t idx = b ^ (lps_mask & 0xFF);
  *sp = cabac_transition[idx];
  const int shift = __builtin_clz(r) - 23;                  // back to 9 bits
  v <<= shift;
  r <<= shift;
  int bn = bits_needed + shift;
  if (likely(bitstream_curr < bitstream_end)) {
    // branch-free refill: the next byte merged under a mask when bits are needed
    const uint32_t need = uint32_t(int32_t(~bn) >> 31);   // all ones when bn >= 0
    v |= (uint32_t(*bitstream_curr) << (bn & 7)) & need;
    bitstream_curr += need & 1;
    bn -= 8 & need;
  }
  else if (bn >= 0) {
    bn -= 8;
  }
  range = r; value = v; bits_needed = int16_t(bn);
  return idx & 1;
#elif defined(DE265_CABAC_ASM_X86_64)
  // x86-64 branchless arithmetic decoder. Bit-identical to the C fallback below.
  uint32_t range_l = range, value_l = value;
  int      bn_l    = bits_needed;
  uint8_t* curr_l = bitstream_curr;
  uint8_t* sp = reinterpret_cast<uint8_t*>(model);
  int bit_l;

  __asm__ (
    "movzbl (%[sp]), %%eax            \n\t" // eax = b (state*2 + MPS)
    "mov    %[range], %%ecx           \n\t"
    "shr    $6, %%ecx                 \n\t"
    "sub    $4, %%ecx                 \n\t" // ecx = (range>>6)-4
    "mov    %%eax, %%edx              \n\t"
    "shr    $1, %%edx                 \n\t" // edx = state
    "lea    (%%rcx,%%rdx,4), %%rcx    \n\t" // rcx = state*4 + ridx
    "lea    %[lps], %%rdx             \n\t" // rdx = &LPS_table[0][0]
    "movzbl (%%rdx,%%rcx,1), %%edx    \n\t" // edx = RangeLPS
    "sub    %%edx, %[range]           \n\t" // range = range - RangeLPS (MPS range)
    "mov    %[range], %%r8d           \n\t"
    "shl    $7, %%r8d                 \n\t" // r8d = scaled_range
    "mov    %[value], %%r9d           \n\t"
    "sub    %%r8d, %%r9d              \n\t"
    "sar    $31, %%r9d                \n\t" // r9d = mps_mask (-1 if MPS)
    "not    %%r9d                     \n\t" // r9d = lps_mask (-1 if LPS)
    "mov    %%r8d, %%ecx              \n\t"
    "and    %%r9d, %%ecx              \n\t"
    "sub    %%ecx, %[value]           \n\t" // value -= scaled & lps_mask
    "mov    %%edx, %%ecx              \n\t" // RangeLPS
    "sub    %[range], %%ecx           \n\t"
    "and    %%r9d, %%ecx              \n\t"
    "add    %%ecx, %[range]           \n\t" // range = RangeLPS if LPS, else unchanged
    "and    $0xFF, %%r9d              \n\t"
    "xor    %%r9d, %%eax              \n\t" // eax = idx
    "mov    %%eax, %[bit]             \n\t"
    "and    $1, %[bit]                \n\t" // bit = idx & 1
    "lea    %[trans], %%rcx           \n\t" // rcx = &cabac_transition[0]
    "movzbl (%%rcx,%%rax,1), %%edx    \n\t"
    "mov    %%dl, (%[sp])             \n\t" // *sp = cabac_transition[idx]
    "bsr    %[range], %%ecx           \n\t" // ecx = MSB index
    "mov    $8, %%edx                 \n\t"
    "sub    %%ecx, %%edx              \n\t" // edx = renorm shift = 8 - bsr
    "mov    %%edx, %%ecx              \n\t"
    "shl    %%cl, %[value]            \n\t"
    "shl    %%cl, %[range]            \n\t"
    "add    %%edx, %[bn]              \n\t" // bits_needed += shift
    "test   %[bn], %[bn]              \n\t"
    "js     1f                        \n\t" // bits_needed < 0: no refill
    "cmp    %[end], %[curr]           \n\t"
    "jae    2f                        \n\t" // curr >= end: no read
    "movzbl (%[curr]), %%eax          \n\t"
    "mov    %[bn], %%ecx              \n\t"
    "shl    %%cl, %%eax              \n\t"
    "or     %%eax, %[value]           \n\t" // value |= (*curr) << bits_needed
    "inc    %[curr]                   \n\t"
    "2:                               \n\t"
    "sub    $8, %[bn]                 \n\t"
    "1:                               \n\t"
    : [range]"+r"(range_l), [value]"+r"(value_l), [bn]"+r"(bn_l),
      [curr]"+r"(curr_l), [bit]"=&r"(bit_l)
    : [sp]"r"(sp), [end]"r"(bitstream_end),
      [lps]"m"(LPS_table[0][0]), [trans]"m"(cabac_transition[0])
    : "rax","rcx","rdx","r8","r9","cc","memory"
  );

  range = range_l; value = value_l; bits_needed = (int16_t)bn_l; bitstream_curr = curr_l;
  return bit_l;
#else
  logtrace(LogCABAC,"[%3d] decodeBin r:%x v:%x state:%d\n",logcnt,range, value, model->state);

  int decoded_bit;
  int LPS = LPS_table[model->state][ ( range >> 6 ) - 4 ];
  range -= LPS;

  uint32_t scaled_range = range << 7;

  logtrace(LogCABAC,"[%3d] sr:%x v:%x\n",logcnt,scaled_range, value);

  if (value < scaled_range)
    {
      logtrace(LogCABAC,"[%3d] MPS\n",logcnt);

      // MPS path

      decoded_bit = model->MPSbit;
      model->state = next_state_MPS[model->state];

      if (scaled_range < ( 256 << 7 ) )
        {
          // scaled range, highest bit (15) not set

          range = scaled_range >> 6; // shift range by one bit
          value <<= 1;               // shift value by one bit
          bits_needed++;

          if (bits_needed == 0)
            {
              bits_needed = -8;
              if (bitstream_curr < bitstream_end)
                { value |= *bitstream_curr++; }
            }
        }
    }
  else
    {
      logtrace(LogCABAC,"[%3d] LPS\n",logcnt);
      //printf("%d %d\n", model->state, 0);

      // LPS path

      value = (value - scaled_range);

      uint8_t num_bits = renorm_table[ LPS >> 3 ];
      value <<= num_bits;
      range   = LPS << num_bits;  /* this is always >= 0x100 except for state 63,
                                              but state 63 is never used */

#ifndef NDEBUG
      int num_bitsTab = renorm_table[ LPS >> 3 ];
      assert(num_bits == num_bitsTab);
#endif

      decoded_bit      = 1 - model->MPSbit;

      if (model->state==0) { model->MPSbit = 1-model->MPSbit; }
      model->state = next_state_LPS[model->state];

      bits_needed += num_bits;

      if (bits_needed >= 0)
        {
          logtrace(LogCABAC,"bits_needed: %d\n", bits_needed);
          if (bitstream_curr < bitstream_end)
            { value |= (*bitstream_curr++) << bits_needed; }

          bits_needed -= 8;
        }
    }

  logtrace(LogCABAC,"[%3d] -> bit %d  r:%x v:%x\n", logcnt, decoded_bit, range, value);
#ifdef DE265_LOG_TRACE
  logcnt++;
#endif

  return decoded_bit;
#endif
}

inline __attribute__((always_inline)) int CABAC_decoder::decode_bypass()
{
#ifdef DE265_CABAC_ASM_X86_64
  uint32_t v = value << 1;
  int bn = bits_needed + 1;
  if (likely(bitstream_curr < bitstream_end)) {
    const uint32_t need = uint32_t(int32_t(~bn) >> 31);
    v |= uint32_t(*bitstream_curr) & need;
    bitstream_curr += need & 1;
    bn -= 8 & need;
  }
  else if (bn >= 0) {
    bn = -8;
  }
  const uint32_t scaled_range = range << 7;
  const uint32_t one = uint32_t(int32_t(scaled_range - 1 - v) >> 31);   // all ones when v >= scaled_range
  value = v - (scaled_range & one);
  bits_needed = int16_t(bn);
  return int(one & 1);
#else
  logtrace(LogCABAC,"[%3d] bypass r:%x v:%x\n",logcnt,range, value);

  value <<= 1;
  bits_needed++;

  if (bits_needed >= 0)
    {
      if (bitstream_end > bitstream_curr) {
        bits_needed = -8;
        value |= *bitstream_curr++;
      }
      else {
        // we read past the end of the bitstream, fill with 0
        bits_needed = -8;
      }
    }

  int bit;
  uint32_t scaled_range = range << 7;
  if (value >= scaled_range)
    {
      value -= scaled_range;
      bit=1;
    }
  else
    {
      bit=0;
    }

  logtrace(LogCABAC,"[%3d] -> bit %d  r:%x v:%x\n", logcnt, bit, range, value);
#ifdef DE265_LOG_TRACE
  logcnt++;
#endif

  return bit;
#endif
}

inline __attribute__((always_inline)) int CABAC_decoder::decode_FL_bypass_parallel(int nBits)
{
  logtrace(LogCABAC,"[%3d] bypass group r:%x v:%x (nBits=%d)\n",logcnt,
           range, value, nBits);

  value <<= nBits;
  bits_needed+=nBits;

  if (bits_needed >= 0)
    {
      if (bitstream_end > bitstream_curr) {
        int input = *bitstream_curr++;
        input <<= bits_needed;

        bits_needed -= 8;
        value |= input;
      }
    }

  uint32_t scaled_range = range << 7;
  int v = value / scaled_range;
  if (unlikely(v>=(1<<nBits))) { v=(1<<nBits)-1; } // may happen with broken bitstreams
  value -= v * scaled_range;

  logtrace(LogCABAC,"[%3d] -> value %d  r:%x v:%x\n", logcnt+nBits-1,
           v, range, value);

#ifdef DE265_LOG_TRACE
  logcnt+=nBits;
#endif

  return v;
}

// Up to 8 bins per division (decode_FL_bypass_parallel), also beyond 8 bins.
inline __attribute__((always_inline)) uint32_t CABAC_decoder::decode_FL_bypass(int nBits)
{
  uint32_t v = 0;
  while (nBits > 0) {
    int k = nBits < 8 ? nBits : 8;
    v = (v << k) | decode_FL_bypass_parallel(k);
    nBits -= k;
  }
  return v;
}

#endif
