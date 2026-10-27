/* SPDX-License-Identifier: MIT */
#include "NativeAdrenoProfile.h"

/* a6xx_catalog.c (mainline, 42f3b40c): 0x43050a01 is ADRENO_7XX_GEN2 with
 * a740_hwcg/a730_protect, 3 MiB GMEM, a7xx_pwrup_reglist, gmu_chipid
 * 0x7020100, gmu_cgc_mode 0x00020202 and a 4192 KiB preemption record.
 *
 * 0x43030b00 (SM7675, gen7_11_0) is the Adreno 732 entry from
 * MCC45TR/senemos-uke-kernel-mainline patch 0036: same family and register
 * tables, gen71100 firmware, 1.5 MiB GMEM, gmu_chipid 0x70b0000,
 * gmu_cgc_mode 0x00020202 and a 2196 KiB preemption record. That patch
 * deliberately leaves preemption and IFPC unqualified; ACD levels are part of
 * the live pwrlevel table, so the ACD vote stays enabled. */
static const NativeAdrenoProfile kA740 = {
    /*chip_id*/0x43050a01u,/*name*/"Adreno740",/*gmem_bytes*/0x300000u,
    /*gmu_chip_id*/0x7020100u,/*gmu_cgc_mode*/0x00020202u,
    /*preempt_record_bytes*/0x418000u,/*ubwc_version*/4u,
    /*highest_bank_bit*/16u,/*ubwc_swizzle*/3u,/*gmu_gpu_delta*/0x6a000u,
    /*sqe_firmware*/"a740_sqe.fw",/*gmu_firmware*/"gmu_gen70200.bin",
    /*zap_firmware*/"a740_zap.mbn",/*acd*/true,/*preemption*/true,/*ifpc*/true,
};
static const NativeAdrenoProfile kA732 = {
    /*chip_id*/0x43030b00u,/*name*/"Adreno732",/*gmem_bytes*/0x180000u,
    /*gmu_chip_id*/0x70b0000u,/*gmu_cgc_mode*/0x00020202u,
    /*preempt_record_bytes*/0x225000u,/*ubwc_version*/4u,
    /*highest_bank_bit*/16u,/*ubwc_swizzle*/3u,/*gmu_gpu_delta*/0x68000u,
    /*sqe_firmware*/"gen71100_sqe.fw",/*gmu_firmware*/"gen71100_gmu.bin",
    /*zap_firmware*/"gen71100_zap.mbn",/*acd*/true,/*preemption*/false,/*ifpc*/false,
};

const NativeAdrenoProfile *NativeAdrenoProfileTable::a740() {return &kA740;}
const NativeAdrenoProfile *NativeAdrenoProfileTable::a732() {return &kA732;}

const NativeAdrenoProfile *NativeAdrenoProfileTable::find(uint32_t chip_id)
{
    if(chip_id==kA740.chip_id)return &kA740;
    if(chip_id==kA732.chip_id)return &kA732;
    return nullptr;
}

uint32_t NativeAdrenoProfileTable::timerGmuLowOffset(uint32_t delta)
{
    /* a6xx_gmu.xml GMU_ALWAYS_ON_COUNTER_L is dword 0x1f888 (A6XX..A7XX).
     * The counter is read through the GPU window, so subtract the distance
     * from the GPU base to the GMU base. */
    const uint32_t counter=0x1f888u*4u;
    return delta<counter?counter-delta:0u;
}
