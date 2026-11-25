#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <setjmp.h>
#include <tamtypes.h>

extern void PS2_PresentFrameAndPollInput(void);
extern void VBlankIntr(void);
extern void virtuappu_render_frame(void);
extern u8 gPaletteBuffer[];
extern u8 gVram[];
extern u8 gOam[];

#define G_BUF(name, sz) __attribute__((aligned(16))) u8 name[sz]

G_BUF(gOam, 1024);
G_BUF(gVram, 98304);

int gPortSoftResetArmed = 0;
jmp_buf gPortSoftResetJmp;
int n64_post = 0;

void VBlankIntrWait(void) {
    VBlankIntr();
    virtuappu_render_frame();
    PS2_PresentFrameAndPollInput();
}

void CpuSet(const void *src, void *dst, u32 mode) {
    if (!src || !dst) return;
    u32 count = mode & 0x1FFFFF;
    int is32 = (mode & (1 << 26)) != 0;
    int isFill = (mode & (1 << 24)) != 0;
    if (is32) {
        const u32 *s = (const u32 *)src;
        u32 *d = (u32 *)dst;
        for (u32 i = 0; i < count; i++) d[i] = isFill ? s[0] : s[i];
    } else {
        const u16 *s = (const u16 *)src;
        u16 *d = (u16 *)dst;
        for (u32 i = 0; i < count; i++) d[i] = isFill ? s[0] : s[i];
    }
}

void LZ77UnCompWram(const void *src, void *dst) {
    if (!src || !dst) return;
    const u8 *s = (const u8 *)src;
    u8 *d = (u8 *)dst;
    u32 header = s[0] | (s[1] << 8) | (s[2] << 16) | (s[3] << 24);
    u32 destLen = header >> 8;
    if (destLen == 0 || destLen > 0x100000) return;
    u32 srcIdx = 4, dstIdx = 0;
    while (dstIdx < destLen) {
        u8 flags = s[srcIdx++];
        for (int i = 0; i < 8 && dstIdx < destLen; i++) {
            if (flags & (0x80 >> i)) {
                u8 b1 = s[srcIdx++];
                u8 b2 = s[srcIdx++];
                u32 len = (b1 >> 4) + 3;
                u32 disp = (((b1 & 0x0F) << 8) | b2) + 1;
                for (u32 j = 0; j < len && dstIdx < destLen; j++) {
                    d[dstIdx] = (dstIdx >= disp) ? d[dstIdx - disp] : 0;
                    dstIdx++;
                }
            } else {
                d[dstIdx++] = s[srcIdx++];
            }
        }
    }
}

void LZ77UnCompVram(const void *src, void *dst) { LZ77UnCompWram(src, dst); }
u16 Sqrt(u32 num) { return (u16)sqrt((double)num); }
s32 Div(s32 num, s32 denom) { return denom ? (num / denom) : 0; }
s32 Mod(s32 num, s32 denom) { return denom ? (num % denom) : 0; }
u64 DivAndModCombined(s32 num, s32 denom) {
    if (!denom) return 0;
    u32 q = (u32)(num / denom);
    u32 r = (u32)(num % denom);
    return ((u64)r << 32) | q;
}

void RegisterRamReset(u32 flags) {
    if (flags & 0x04) memset(gPaletteBuffer, 0, 1024);
    if (flags & 0x08) memset(gVram, 0, 0x18000);
    if (flags & 0x10) memset(gOam, 0, 0x400);
}

void SoftReset(u32 resetFlags) {
    (void)resetFlags;
    if (gPortSoftResetArmed) longjmp(gPortSoftResetJmp, 1);
}

void BgAffineSet(const void *src, void *dst, int count) {
    (void)src; (void)dst; (void)count;
}

void ObjAffineSet(const void *src, void *dst, int count, int offset) {
    (void)src; (void)dst; (void)count; (void)offset;
}
