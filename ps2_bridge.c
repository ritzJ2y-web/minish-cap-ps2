#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <setjmp.h>
#include <tamtypes.h>

extern void PS2_PresentFrameAndPollInput(void);
extern void VBlankIntr(void);
extern void virtuappu_render_frame(void);

#define G_BUF(name, sz) __attribute__((aligned(16))) u8 name[sz]

G_BUF(gOam, 1024);
G_BUF(gVram, 98304);

G_BUF(gMain, 256);
G_BUF(gPlayerEntity, 512);
G_BUF(gPlayerState, 512);
G_BUF(gEntities, 32768);
G_BUF(gEntityLists, 512);
G_BUF(gEntityListsBackup, 512);
G_BUF(gAuxPlayerEntities, 2048);
G_BUF(gPlayerClones, 2048);
void* gCarriedEntity = NULL;
u8 gEntCount = 0;
u8 gManagerCount = 0;
u8 gCollidableCount = 0;
G_BUF(gCollidableList, 512);
G_BUF(gCollisionMtx, 256);

G_BUF(gRoomControls, 512);
G_BUF(gRoomTransition, 512);
G_BUF(gRoomVars, 512);
G_BUF(gCurrentRoomProperties, 256);
G_BUF(gRoomMemory, 4096);
G_BUF(gCurrentRoomMemory, 512);
G_BUF(gArea, 512);
G_BUF(gAreaTable, 1024);
G_BUF(gAreaRoomHeaders, 2048);
G_BUF(gAreaRoomMaps, 2048);
G_BUF(gAreaTileSets, 2048);
G_BUF(gAreaTiles, 2048);

G_BUF(gSave, 4096);
G_BUF(gScreen, 512);
G_BUF(gFadeControl, 256);
G_BUF(gMessage, 256);
G_BUF(gUI, 512);
G_BUF(gHUD, 512);
G_BUF(gInput, 128);
G_BUF(gOAMControls, 2048);
G_BUF(gOamCmd, 64);
G_BUF(gPaletteBuffer, 2048);
G_BUF(gPaletteBufferBackup, 2048);
G_BUF(gUsedPalettes, 256);
G_BUF(gPaletteList, 256);
G_BUF(gPalette_549, 256);
G_BUF(gGFXSlots, 512);
G_BUF(gVBlankDMA, 256);
u8 gUpdateVisibleTiles = 0;
u8 gUpdateContext = 0;
u32 gRand = 0x12345678;

G_BUF(gMapTop, 512);
G_BUF(gMapBottom, 512);
G_BUF(gMapData, 32768);
G_BUF(gMapDataTopSpecial, 8192);
G_BUF(gMapDataBottomSpecial, 8192);
G_BUF(gTilesForSpecialTiles, 2048);
G_BUF(gBgAnimations, 512);
G_BUF(gDiggingCaveEntranceTransition, 256);
G_BUF(gDungeonMap, 512);
G_BUF(gDungeonNames, 512);
G_BUF(gFuseInfo, 512);
G_BUF(gLilypadRails, 512);
G_BUF(gPauseMenuOptions, 256);
G_BUF(gPossibleInteraction, 256);
G_BUF(gPriorityHandler, 256);
G_BUF(gShakeOffsets, 256);
G_BUF(gSmallChests, 512);
G_BUF(gTextGfxBuffer, 8192);
G_BUF(gTextRender, 512);
G_BUF(gTranslations, 256);
G_BUF(gZeldaFollowerText, 256);
G_BUF(_gMenuSharedStorage, 8192);
G_BUF(gzHeap, 16384);

G_BUF(gActiveItems, 1024);
G_BUF(gActiveScriptInfo, 256);
G_BUF(gScriptExecutionContextArray, 2048);
G_BUF(gPlayerScriptExecutionContext, 512);
G_BUF(gEntityScriptCtxTable, 4096);
G_BUF(gExtraFrameOffsets, 512);
G_BUF(gFixedTypeGfxData, 512);
G_BUF(gFrameObjLists, 512);
G_BUF(gMoreSpritePtrs, 512);
G_BUF(gNPCData, 512);
G_BUF(gSpritePtrs, 16384);
u32 gSpritePtrsLoadedCount = 0;
const u32 kMaxSpritePtrs = 2048;
G_BUF(gSpriteAnimations_322, 512);
G_BUF(gSpriteAnimations_GhostBrothers, 512);
G_BUF(gUnk_02000010, 128);
G_BUF(gUnk_02000040, 256);
G_BUF(gUnk_020000C0, 512);
G_BUF(gUnk_02006F00, 1024);
G_BUF(gUnk_0200B640, 1024);
G_BUF(gUnk_02017830, 512);
G_BUF(gUnk_02017AA0, 4096);
G_BUF(gUnk_02018EA0, 64);
G_BUF(gUnk_02018EB0, 64);
G_BUF(gUnk_02018EE0, 512);
G_BUF(gUnk_02021F00, 2048);
G_BUF(gUnk_020227DC, 64);
G_BUF(gUnk_020227E8, 64);
G_BUF(gUnk_020227F0, 64);
G_BUF(gUnk_020227F8, 64);
G_BUF(gUnk_02022800, 128);
G_BUF(gUnk_02022830, 512);
G_BUF(gUnk_02024048, 2048);
G_BUF(gUnk_020246B0, 4096);
G_BUF(gUnk_020342F8, 512);
G_BUF(gUnk_02034480, 64);
G_BUF(gUnk_02034492, 64);
G_BUF(gUnk_020344A0, 4096);
G_BUF(gUnk_020354C0, 4096);
G_BUF(gUnk_02036BB8, 4096);
G_BUF(gUnk_03000C30, 1024);
G_BUF(gUnk_03001020, 1024);

G_BUF(gMPlayInfos, 512);
G_BUF(gMPlayInfos2, 512);
G_BUF(gMPlayMemAccArea, 512);
G_BUF(gMPlayTracks, 2048);
G_BUF(gSoundPlayingInfo, 256);

u16 gPortIntrCheck = 0;
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
    if (gPortSoftResetArmed) longjmp(gPortSoftResetJmp, 1);
}

void BgAffineSet(const void *src, void *dst, int count) {}
void ObjAffineSet(const void *src, void *dst, int count, int offset) {}

int Port_Config_GetAnalogDeadzone(void) { return 40; }
int Port_Config_GetHoldToAdvanceText(void) { return 0; }
void Port_Config_GetLeftStick(s16 *x, s16 *y) { if (x) *x = 0; if (y) *y = 0; }
int Port_Config_GetRollAttackMacroEnabled(void) { return 0; }
int Port_Config_InputEdgePressed(int action) { return 0; }
int Port_Config_InputPressed(int action) { return 0; }
int Port_Config_PortSettingsMenuEnabled(void) { return 0; }
int Port_Config_PreferredLanguage(void) { return 0; }
int Port_Config_PreferredRegion(void) { return 0; }
void Port_Config_SetActiveSaveProfile(int p) {}
int Port_Config_TargetFps(void) { return 60; }
void Port_Config_CycleTargetFps(void) {}
int Port_DebugVerbose(void) { return 0; }
int Port_Debug_NoclipEnabled(void) { return 0; }
void Port_Debug_OnFlagSet(u32 f) {}
void Port_BugReport_NoteBadHitbox(void) {}
void Port_LevelEditor_OnRoomLoad(void) {}
void Port_LogAssetLoaderStatus(void) {}
void Port_LogTextLookup(void) {}

int Port_AreSpritePtrsLoadedFromAssets(void) { return 0; }
int Port_IsAreaTablePtrFromAssets(const void *p) { return 0; }
int Port_IsLoadedAssetBytes(const void *p) { return 0; }
int Port_IsRoomHeaderPtrReadable(const void *p) { return p != NULL; }
void* Port_GetMapAssetDataByIndex(u32 idx) { return NULL; }
void* Port_GetSpriteAnimationData(u32 idx) { return NULL; }
void Port_LoadAreaTablesFromAssets(void) {}
void Port_LoadGfxGroupFromAssets(u32 g) {}
void Port_LoadPaletteGroupFromAssets(u32 g) {}
void Port_LoadSpritePtrsFromAssets(void) {}
void Port_LoadTextsFromAssets(void) {}
void Port_RefreshAreaDataFromAssets(void) {}

int Port_AudioMute_ShouldSuppress(int cat) { return 1; }
void Port_M4A_Backend_ContinuePlayer(int p) {}
int Port_M4A_Backend_IsPlayerActive(int p) { return 0; }
void Port_M4A_Backend_SetSoundMode(u32 m) {}
void Port_M4A_Backend_SetTrackPan(int p, int t, s8 pan) {}
void Port_M4A_Backend_SetTrackVolume(int p, int t, u16 vol) {}
void Port_M4A_Backend_SetVSyncEnabled(int e) {}
void Port_M4A_Backend_SoundInit(void) {}
void Port_M4A_Backend_StartSongById(u16 id, int p) {}
void Port_M4A_Backend_StopPlayer(int p) {}

void Port_PPU_CyclePresentationMode(void) {}
void Port_PPU_CycleWindowScale(void) {}
int Port_PPU_IsFullscreen(void) { return 1; }
const char* Port_PPU_PresentationModeName(void) { return "PS2"; }
void Port_PPU_ToggleFullscreen(void) {}
int Port_PPU_WindowScale(void) { return 1; }
int Port_Widescreen_EffectiveViewWidth(void) { return 240; }
void Port_Widescreen_UpdateShadows(void) {}
int Port_TTS_GetEnabled(void) { return 0; }
void Port_TTS_Speak(const char *s) {}
void Port_TTS_Stop(void) {}
int Port_Voxel_IsDrawing(void) { return 0; }

int Port_SoftSlots_GetActiveSlot(void) { return 0; }
int Port_SoftSlots_GetAssignment(int s) { return 0; }
int Port_SoftSlots_GetEffectiveBItem(void) { return 0; }
int Port_SoftSlots_IsBHeld(void) { return 0; }
void Port_SoftSlots_NotifyPauseActive(int a) {}
void Port_SoftSlots_SetAssignment(int s, int i) {}

int Port_Reborn_ConsumeJustResumed(void) { return 0; }
int Port_Reborn_IsEnabled(void) { return 0; }
void Port_Reborn_NotifyJustResumed(void) {}

int Port_RandoFileMenu_IsOpen(void) { return 0; }
void Port_RandoFileMenu_Open(void) {}
int Port_RandoFileMenu_ShouldOpenForNewFile(void) { return 0; }
void Port_RandoFileMenu_ToggleSidebar(void) {}
void Port_RandoSave_ClearSlot(int s) {}
void Port_RandoSave_CopySlot(int a, int b) {}
void Port_RandoSave_LoadSlot(int s) {}
void Port_RandoSave_SaveActiveSlot(void) {}
void* RandoLogic_FindLocationByKey(const char *k) { return NULL; }
int RandoLogic_IsLoaded(void) { return 0; }
int Rando_Entrance_RemapExit(int e) { return e; }
int Rando_Entrance_RemapGreenWarp(int w) { return w; }
int Rando_Entrance_RemapHole(int h) { return h; }
void Rando_Homewarp_Request(void) {}
int Rando_IsActive(void) { return 0; }
int Rando_Music_Remap(int m) { return m; }
int Rando_OverrideItem(int i) { return i; }
const char* Rando_OverrideLocationKey(const char *k) { return k; }
void Rando_Reset(void) {}
int Rando_RouteDungeonItem(int i) { return i; }
int Rando_Runtime_DamageMultiplier(void) { return 1; }
void Rando_Runtime_OnNewFile(void) {}
void Rando_Runtime_OpenTingleBrothers(void) {}
void Rando_Runtime_Refresh(void) {}
