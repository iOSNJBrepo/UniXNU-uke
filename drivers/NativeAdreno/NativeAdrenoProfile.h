/* SPDX-License-Identifier: MIT */
#ifndef NATIVE_ADRENO_PROFILE_H
#define NATIVE_ADRENO_PROFILE_H
#include <stdint.h>

/* Immutable per-SoC facts the driver needs before it may touch a register.
 * Every value names the public source it was taken from; nothing here is read
 * from the device. The register tables that are shared by the whole A7xx
 * family stay in NativeAdrenoA740HwInit.h and NativeAdrenoPwrupRegs.h. */
struct NativeAdrenoProfile {
    uint32_t chip_id;             /* qcom,chipid */
    const char *name;             /* qcom,gpu-model */
    uint32_t gmem_bytes;          /* GMEM visible to the UCHE */
    uint32_t gmu_chip_id;         /* a6xx_info.gmu_chipid, written to GMU_GENERAL_10 */
    uint32_t gmu_cgc_mode;        /* a6xx_info.gmu_cgc_mode */
    uint32_t preempt_record_bytes;
    uint32_t ubwc_version;        /* 4 for UBWC 4.0/4.3 */
    uint32_t highest_bank_bit;
    uint32_t ubwc_swizzle;        /* bit0 level 1, bit1 level 2, bit2 level 3 */
    uint32_t gmu_gpu_delta;       /* gmu reg base - gpu reg base, for the always-on counter */
    const char *sqe_firmware;
    const char *gmu_firmware;
    const char *zap_firmware;
    bool acd;
    bool preemption;
    bool ifpc;
};

class NativeAdrenoProfileTable final {
    NativeAdrenoProfileTable() = delete;
public:
    /* The A740 (SM8550, 0x43050a01) and A732 (SM7675, 0x43030b00) entries.
     * An unknown chip id returns nullptr; a driver that cannot name every
     * value for a chip must not touch it. */
    static const NativeAdrenoProfile *a740();
    static const NativeAdrenoProfile *a732();
    static const NativeAdrenoProfile *find(uint32_t chip_id);
    /* GMU always-on counter low word, inside the GPU register window, for a
     * profile whose GMU window starts `delta` bytes after the GPU window. */
    static uint32_t timerGmuLowOffset(uint32_t delta);
};

#endif
