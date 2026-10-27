/* SPDX-License-Identifier: MIT */
#include "NativeAdrenoTimer.h"
#include "NativeAdrenoProfile.h"
#include <IOKit/IOLib.h>

/* Derivation, not a guess:
 *   pinned a6xx_gmu.xml  GMU_ALWAYS_ON_COUNTER_L 0x1f888, _H 0x1f889
 *                        (dword offsets, variants A6XX-A7XX)
 *   byte offset = (offset << 2) - (gmu_base - gpu_base)
 *   SM8550 (gmu 0x03d6a000, gpu 0x03d00000): 0x7e220 - 0x6a000 = 0x14220
 *   SM7675 (gmu 0x03d68000, gpu 0x03d00000): 0x7e220 - 0x68000 = 0x16220
 *   pinned a6xx.xml      CP_ALWAYS_ON_COUNTER 0x0980 (64-bit, A6XX-A7XX)
 *     read as low at 0x0980 and high at 0x0981, as Linux gpu_read64 does.
 * The delta is a property of the board's register layout, so the GPU service
 * sets it from the two mapped window bases before a timer is attached. */
enum : uint32_t {
    TimerGmuDefault = 0x14220,
    TimerCpLow = 0x0980,
    TimerCpHigh = 0x0981,
    TimerRetries = 8
};
static uint32_t gmu_low_offset=TimerGmuDefault;
void NativeAdrenoTimer::setGmuBaseDelta(uint32_t delta)
{
    gmu_low_offset=NativeAdrenoProfileTable::timerGmuLowOffset(delta);
}
uint32_t NativeAdrenoTimer::gmuBaseDelta()
{
    return 0x1f888u*4u-gmu_low_offset;
}
uint32_t NativeAdrenoTimer::gmuLowOffset(){return gmu_low_offset;}
uint32_t NativeAdrenoTimer::gmuHighOffset(){return gmu_low_offset+4;}
uint32_t NativeAdrenoTimer::cpLowRegister(){return TimerCpLow;}
uint32_t NativeAdrenoTimer::cpHighRegister(){return TimerCpHigh;}
IOReturn NativeAdrenoTimer::attach(const NativeAdrenoTimeIo *io,uint32_t source)
{
    if(attached_)return kIOReturnBusy;
    if(!io || !io->read32)return kIOReturnBadArgument;
    if(source!=NativeAdrenoTimeGmu && source!=NativeAdrenoTimeCp)return kIOReturnBadArgument;
    io_=*io;source_=source;attached_=true;
    return kIOReturnSuccess;
}
IOReturn NativeAdrenoTimer::detach()
{
    if(!attached_)return kIOReturnNotReady;
    attached_=false;io_={};
    return kIOReturnSuccess;
}
IOReturn NativeAdrenoTimer::read(uint64_t *ticks) const
{
    if(!ticks)return kIOReturnBadArgument;*ticks=0;
    if(!attached_)return kIOReturnNotReady;
    const uint32_t low=source_==NativeAdrenoTimeGmu?gmu_low_offset:TimerCpLow;
    const uint32_t high=source_==NativeAdrenoTimeGmu?gmu_low_offset+4:TimerCpHigh;
    for(unsigned attempt=0;attempt<TimerRetries;++attempt) {
        uint32_t first=0,low_word=0,second=0;
        if(io_.read32(io_.cookie,high,&first) ||
            io_.read32(io_.cookie,low,&low_word) ||
            io_.read32(io_.cookie,high,&second))return kIOReturnIOError;
        if(first==second) {
            *ticks=(uint64_t(first)<<32)|low_word;
            return kIOReturnSuccess;
        }
    }
    return kIOReturnTimeout;
}
