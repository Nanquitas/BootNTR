#include "main.h"
#include "config.h"

extern char         *g_primary_error;
extern char         *g_secondary_error;
extern ntrConfig_t  *ntrConfig;

u32     protectRemoteMemory(Handle hProcess, u32 addr, u32 size)
{
    return (svcControlProcessMemory(hProcess, addr, addr, size, 6, 7));
}

u32     copyRemoteMemory(Handle hDst, u32 ptrDst, Handle hSrc, u32 ptrSrc, u32 size)
{
    u32     dmaConfig[20] = { 0 };
    u32     hdma = 0;
    u32     state;
    u32     i;
    u32     result;
    bool    firstError = true;

    if ((result = svcFlushProcessDataCache(hSrc, (void *)ptrSrc, size)) != 0)
        goto error;
    if ((result = svcFlushProcessDataCache(hDst, (void *)ptrDst, size)) != 0)
        goto error;
again:
    if ((result = svcStartInterProcessDma(&hdma, hDst, (void *)ptrDst, hSrc, (void *)ptrSrc, size, dmaConfig)) != 0)
        goto error;
    state = 0;
    if (ntrConfig->InterProcessDmaFinishState == 0)
    {
        while (1)
        {
            svc_getDmaState(&state, hdma);
            svcSleepThread(10000);
            result = state;
            svcGetDmaState(&state, hdma);
            if (result == state)
                break;
        }
        ntrConfig->InterProcessDmaFinishState = state;
    }
    else
    {
        for (i = 0; i < 1000000; i++)
        {           
            result = svcGetDmaState(&state, hdma);
            if (state == ntrConfig->InterProcessDmaFinishState)
                break;
            svc_sleepThread(10000);
        }
        if (i >= 1000000)
        {
            g_primary_error = READREMOTEMEMORY_TIMEOUT;
            svcCloseHandle(hdma);
            if (firstError)
            {
                newAppStatus(DEFAULT_COLOR, TINY | CENTER, "An error occurred");
                newAppStatus(DEFAULT_COLOR, TINY | CENTER, "Retry in 2 seconds");
                updateUI();
                svcSleepThread(2000000000);
                firstError = false;
                removeAppStatus();
                removeAppStatus();
                updateUI();
                goto again;
            }
            goto error;
        }
    }
    svcCloseHandle(hdma);
    if ((result = svcInvalidateProcessDataCache(hDst, (void *)ptrDst, size)) != 0)
        goto error;
    return (0);
error:
    return (RESULT_ERROR);
}

u32     patchRemoteProcess(u32 pid, u32 addr, u8 *buf, u32 len)
{
    u32     hProcess;
    u32     ret;

    ret = svc_openProcess(&hProcess, pid);
    check_prim(ret, OPENPROCESS_FAILURE);
    ret = protectRemoteMemory(hProcess, ((addr / 0x1000) * 0x1000), 0x1000);
    check_prim(ret, PROTECTMEMORY_FAILURE);
    ret = copyRemoteMemory(hProcess, addr, 0xffff8001, (u32)buf, len);
    check_sec(ret, REMOTECOPY_FAILURE);
    if (hProcess)
        svcCloseHandle(hProcess);
    return (ret);
error:
    return (RESULT_ERROR);
}

u32     rtAlignToPageSize(u32 size)
{
    return (((size / 0x1000) + 1) * 0x1000);
}

u32     rtGetPageOfAddress(u32 addr)
{
    return ((addr / 0x1000) * 0x1000);
}


u32     rtCheckRemoteMemoryRegionSafeForWrite(Handle hProcess, u32 addr, u32 size)
{
    u32     ret;
    u32     startPage;
    u32     endPage;
    u32     page;

    startPage = rtGetPageOfAddress(addr);
    endPage = rtGetPageOfAddress(addr + size - 1);
    for (page = startPage; page <= endPage; page += 0x1000)
    {
        ret = protectRemoteMemory(hProcess, page, 0x1000);
        if (ret != 0)
            return (ret);
    }
    return (0);
}

#define ALPHABET_LEN 256

// Quick Search algorithm, adapted from
// http://igm.univ-mlv.fr/~lecroq/string/node19.html#SECTION00190
u32     memfind(u8 *startPos, u32 size, const void *pattern, u32 patternSize)
{
    u32     i;
    u32     j;
    u32     table[ALPHABET_LEN];

    const u8 *patternc = (const u8 *)pattern;

    // Preprocessing
    for (i = 0; i < ALPHABET_LEN; ++i)
        table[i] = patternSize + 1;
    for (i = 0; i < patternSize; ++i)
        table[patternc[i]] = patternSize - i;

    // Searching
    j = 0;
    while (j <= size - patternSize)
    {
        if (memcmp(patternc, startPos + j, patternSize) == 0)
            return (j);
        j += table[startPos[j + patternSize]];
    }
    return (0);
}

u32     findNearestSTMFD(u32 base, u32 pos) 
{
    if (pos < base)
    {
        return 0;
    }
    pos = pos - pos % 4;
    u32 term = pos - 0x1000;
    if (term < base)
    {
        term = base;
    }
    while (pos >= term) {
        if (*(u16*)(pos + 2) == 0xe92d){
            return pos;
        }
        pos -= 4;
    }
    return 0;
}

u32     searchBytes(u32 startAddr, u32 endAddr, u8* pat, int patlen, int step)
{
    u32 pat0 = ((u32*)pat)[0];

    while (1)
    {
        if (startAddr + patlen >= endAddr)
        {
                return 0;
        }
        if (*((u32*)(startAddr)) == pat0)
        {
            if (memcmp((u32*) startAddr, pat, patlen) == 0)
            {
                return startAddr;
            }
        }
        startAddr += step;
    }
    return 0;
}

