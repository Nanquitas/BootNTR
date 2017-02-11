#include "main.h"
#include "config.h"

extern ntrConfig_t      *ntrConfig;
extern bootNtrConfig_t  *bnConfig;
extern u8               *tmpBuffer;
extern char             *g_primary_error;
extern char             *g_secondary_error;

#if DEBUG
    #define newAppTopDebug(...) newAppTop(...)
#else
    #define newAppTopDebug(...)
#endif

/*Result  bnInitParamsByHomeMenu(void)
{
    u32     hProcess = 0;
    u32     ret;
    vu32    t = 0x11111111;
    u8      region;

again:
    ret = svcOpenProcess(&hProcess, ntrConfig->HomeMenuPid);
    if (ret)
    {
        newAppStatus(DEFAULT_COLOR, TINY | CENTER, "An error occurred");
        newAppStatus(DEFAULT_COLOR, TINY | CENTER, "Retry in 2 seconds");
        updateUI();
        svcSleepThread(2000000000);
        removeAppStatus();
        removeAppStatus();
        updateUI();
        goto again;
    }
    check_prim(ret, OPENPROCESS_FAILURE);
    flushDataCache();
    *(u32 *)(tmpBuffer) = 0;
    ret = copyRemoteMemory(CURRENT_PROCESS_HANDLE, (u32)tmpBuffer, hProcess, 0x00200000, 4);
    check_sec(ret, REMOTECOPY_FAILURE);
    svcCloseHandle(hProcess);
    t = *(u32*)(tmpBuffer);
    ret = cfguInit();
    check_prim(ret, CFGU_INIT_FAILURE);
    ret = CFGU_SecureInfoGetRegion(&region);
    check_prim(ret, CFGU_SECURE_FAILURE);
    if (region >= 7)
    {
        g_primary_error = WRONGREGION;
        goto error;
    }
    cfguExit();
    if (t == 0xe3a08001)
    {
        // old 3ds 10.6.0-27K
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(10, 6, 0);
        ntrConfig->HomeMenuInjectAddr = 0x12ded0;
        ntrConfig->HomeFSReadAddr = 0x12c19c;
        ntrConfig->HomeCardUpdateInitAddr = 0x118d78;
        ntrConfig->HomeFSUHandleAddr = 0x32dfa4;
        ntrConfig->HomeAptStartAppletAddr = 0x12ea08;
    }
    else if (t == 0xe8960140)
    {
        // old 3ds 10.3 usa
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(10, 3, 0);
        ntrConfig->HomeMenuInjectAddr = 0x12ddc4;
        ntrConfig->HomeFSReadAddr = 0x12c090;
        ntrConfig->HomeCardUpdateInitAddr = 0x118cc0;
        ntrConfig->HomeFSUHandleAddr = 0x32dfa4;
        ntrConfig->HomeAptStartAppletAddr = 0x12e8fc;
    }
    else if (t == 0xe5c580f5)
    {
        // old 3ds 10.3 eur
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(10, 3, 0);
        ntrConfig->HomeMenuInjectAddr = 0x12ddc4;
        ntrConfig->HomeFSReadAddr = 0x12c090;
        ntrConfig->HomeCardUpdateInitAddr = 0x118cc0;
        ntrConfig->HomeFSUHandleAddr = 0x32dfa4;
        ntrConfig->HomeAptStartAppletAddr = 0x12e8fc;
    }
    else if (t == 0x0a000004)
    {
        // old 3ds 10.1 eur
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(10, 1, 0);
        ntrConfig->HomeMenuInjectAddr = 0x12ddc4;
        ntrConfig->HomeFSReadAddr = 0x12c090;
        ntrConfig->HomeCardUpdateInitAddr = 0x118cc0;
        ntrConfig->HomeFSUHandleAddr = 0x32dfa4;
        ntrConfig->HomeAptStartAppletAddr = 0x12e8fc;
    }
    else if (t == 0xe1530721)
    {
        // old 3ds 10.1 usa
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(10, 1, 0);
        ntrConfig->HomeMenuInjectAddr = 0x12ddc4;
        ntrConfig->HomeFSReadAddr = 0x12c090;
        ntrConfig->HomeCardUpdateInitAddr = 0x118cc0;
        ntrConfig->HomeFSUHandleAddr = 0x32dfa4;
        ntrConfig->HomeAptStartAppletAddr = 0x12e8fc;
    }
    else if (t == 0xe59f80f4)
    {
        // new3ds 9.2.0
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(9, 2, 0);;
        ntrConfig->HomeMenuInjectAddr = 0x131208;
        ntrConfig->HomeFSReadAddr = 0x0012F6EC;
        ntrConfig->HomeCardUpdateInitAddr = 0x139900;
        ntrConfig->HomeFSUHandleAddr = 0x002F0EFC;
        ntrConfig->HomeAptStartAppletAddr = 0x00131C98;

    }
    else if (t == 0xE28DD008)
    {
        // new3ds 9.1.0
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(9, 1, 0);;
        ntrConfig->HomeMenuInjectAddr = 0x00131208;
        ntrConfig->HomeFSReadAddr = 0x0012F6EC;
        ntrConfig->HomeCardUpdateInitAddr = 0x139900;
        ntrConfig->HomeFSUHandleAddr = 0x002F1EFC;
        ntrConfig->HomeAptStartAppletAddr = 0x00131C98;
    }
    else if (t == 0xE1B03F02)
    {
        // new3ds 9.0.0
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(9, 0, 0);;
        ntrConfig->HomeMenuInjectAddr = 0x00130CFC;
        ntrConfig->HomeFSReadAddr = 0x0012F224;
        ntrConfig->HomeCardUpdateInitAddr = 0x001393F4;
        ntrConfig->HomeFSUHandleAddr = 0x002EFEFC;
        ntrConfig->HomeAptStartAppletAddr = 0x0013178C;
    }
    else if (t == 0xE28F2E19)
    {
        // new3ds 8.1.0
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(8, 1, 0);;
        ntrConfig->HomeMenuInjectAddr = 0x00129098;
        ntrConfig->HomeFSReadAddr = 0x0011AAB8;
        ntrConfig->HomeCardUpdateInitAddr = 0x0013339C;
        ntrConfig->HomeFSUHandleAddr = 0x00278E4C;
        ntrConfig->HomeAptStartAppletAddr = 0x00129BFC;
    }
    else if (t == 0xe59f201c)
    {
        // iQue 9.3.0
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(9, 3, 0);
        ntrConfig->HomeMenuInjectAddr = 0x13b7b0;
        ntrConfig->HomeFSReadAddr = 0x1188e0;
        ntrConfig->HomeCardUpdateInitAddr = 0x13434c;
        ntrConfig->HomeFSUHandleAddr = 0x2240d4;
        ntrConfig->HomeAptStartAppletAddr = 0x128480;
    }
    else if (t == 0xe3a06001)
    {
        // iQue 4.4.0
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(4, 4, 0);
        ntrConfig->HomeMenuInjectAddr = 0x13c344;
        ntrConfig->HomeFSReadAddr = 0x118888;
        ntrConfig->HomeCardUpdateInitAddr = 0x134448;
        ntrConfig->HomeFSUHandleAddr = 0x2210cc;
        ntrConfig->HomeAptStartAppletAddr = 0x12844c;
    }
    else if (t == 0xeb0083b3)
    {
        // new3ds 9.5.0
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(9, 5, 0);
        ntrConfig->HomeMenuInjectAddr = 0x12e1f8;
        ntrConfig->HomeFSReadAddr = 0x12c624;
        ntrConfig->HomeCardUpdateInitAddr = 0x136a8c;
        ntrConfig->HomeFSUHandleAddr = 0x313f7c;
        ntrConfig->HomeAptStartAppletAddr = 0x12ec88;
    }
    else if (t == 0xe2053001)
    {
        // USA 9.9.0
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(9, 9, 0);
        ntrConfig->HomeMenuInjectAddr = 0x12ddc4;
        ntrConfig->HomeFSReadAddr = 0x12c090;
        ntrConfig->HomeCardUpdateInitAddr = 0x118cc0;
        ntrConfig->HomeFSUHandleAddr = 0x32dfa4;
        ntrConfig->HomeAptStartAppletAddr = 0x12e8fc;
    }
    else if (t == 0xe1a00000)
    {
        // TW 9.8.0
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(9, 8, 0);
        ntrConfig->HomeMenuInjectAddr = 0x13ba60;
        ntrConfig->HomeFSReadAddr = 0x1188e0;
        ntrConfig->HomeCardUpdateInitAddr = 0x13434c;
        ntrConfig->HomeFSUHandleAddr = 0x2240d4;
        ntrConfig->HomeAptStartAppletAddr = 0x128480;
    }
    else if (t == 0xe12fff1e)
    {
        if (region == 5)
        {
            // KR 10.1.0-23
            ntrConfig->HomeMenuVersion = SYSTEM_VERSION(10, 1, 0);
            ntrConfig->HomeMenuInjectAddr = 0x12ddc4;
            ntrConfig->HomeFSReadAddr = 0x12c090;
            ntrConfig->HomeCardUpdateInitAddr = 0x118cc0;
            ntrConfig->HomeFSUHandleAddr = 0x32dfa4;
            ntrConfig->HomeAptStartAppletAddr = 0x12e8fc;
        }
        else
        {
            // TW 9.9.0
            ntrConfig->HomeMenuVersion = SYSTEM_VERSION(9, 9, 0);
            ntrConfig->HomeMenuInjectAddr = 0x13c0ac;
            ntrConfig->HomeFSReadAddr = 0x118c04;
            ntrConfig->HomeCardUpdateInitAddr = 0x134794;
            ntrConfig->HomeFSUHandleAddr = 0x2250e4;
            ntrConfig->HomeAptStartAppletAddr = 0x1288c8;
        }
    }
    else if (t == 0x0032dde8)
    {
        // JP 9.9.0
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(9, 9, 0);
        ntrConfig->HomeMenuInjectAddr = 0x12ddc4;
        ntrConfig->HomeFSReadAddr = 0x12c090;
        ntrConfig->HomeCardUpdateInitAddr = 0x118cc0;
        ntrConfig->HomeFSUHandleAddr = 0x32dfa4;
        ntrConfig->HomeAptStartAppletAddr = 0x12e8fc;
    }
    else if (t == 0xe1530005)
    {
        // JP 9.6.0
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(9, 6, 0);
        ntrConfig->HomeMenuInjectAddr = 0x12ddf4;
        ntrConfig->HomeFSReadAddr = 0x12c0c0;
        ntrConfig->HomeCardUpdateInitAddr = 0x118cf0;
        ntrConfig->HomeFSUHandleAddr = 0x32efac;
        ntrConfig->HomeAptStartAppletAddr = 0x12e92c;
    }
    else if (t == 0xe1a02004)
    {
        // USA 9.4.0
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(9, 4, 0);
        ntrConfig->HomeMenuInjectAddr = 0x12e204;
        ntrConfig->HomeFSReadAddr = 0x12c630;
        ntrConfig->HomeCardUpdateInitAddr = 0x136a98;
        ntrConfig->HomeFSUHandleAddr = 0x313f7c;
        ntrConfig->HomeAptStartAppletAddr = 0x12ec94;
    }
    else if (t == 0xe1966009)
    {
        //europe 9.7.0
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(9, 7, 0);
        ntrConfig->HomeMenuInjectAddr = 0x12dd98;
        ntrConfig->HomeFSReadAddr = 0x12c064;
        ntrConfig->HomeCardUpdateInitAddr = 0x118c94;
        ntrConfig->HomeFSUHandleAddr = 0x32dfa4;
        ntrConfig->HomeAptStartAppletAddr = 0x12e8d0;
    }
    else if (t == 0xe28f3fde)
    {
        // USA 8.1.0-9U
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(8, 1, 0);
        ntrConfig->HomeMenuInjectAddr = 0x13f2d8;
        ntrConfig->HomeFSReadAddr = 0x11a994;
        ntrConfig->HomeCardUpdateInitAddr = 0x13719c;
        ntrConfig->HomeFSUHandleAddr = 0x238df4;
        ntrConfig->HomeAptStartAppletAddr = 0x12aac0;
    }
    else if (t == 0xe1a0231c)
    {
        // Korea 9.9.0
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(9, 9, 0);
        ntrConfig->HomeMenuInjectAddr = 0x12dd98;
        ntrConfig->HomeFSReadAddr = 0x12c064;
        ntrConfig->HomeCardUpdateInitAddr = 0x118c94;
        ntrConfig->HomeFSUHandleAddr = 0x32dfa4;
        ntrConfig->HomeAptStartAppletAddr = 0x12e8d0;
    }
    else if (t == 0xea00001f)
    {
        //  10.4.0-29J
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(10, 4, 0);
        ntrConfig->HomeMenuInjectAddr = 0x12ded0;
        ntrConfig->HomeFSReadAddr = 0x12c19c;
        ntrConfig->HomeCardUpdateInitAddr = 0x118d78;
        ntrConfig->HomeFSUHandleAddr = 0x32efa4;
        ntrConfig->HomeAptStartAppletAddr = 0x12ea08;
    }
    else if (t == 0xe1a00006)
    {
        // new3ds 10.5.0U
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(10, 5, 0);
        ntrConfig->HomeMenuInjectAddr = 0x12ded0;
        ntrConfig->HomeFSReadAddr = 0x12c19c;
        ntrConfig->HomeCardUpdateInitAddr = 0x118d78;
        ntrConfig->HomeFSUHandleAddr = 0x32dfa4;
        ntrConfig->HomeAptStartAppletAddr = 0x12ea08;
    }
    else if (t == 0xe7941100 || t == 0xea0020d2)
    {
        // old/new3ds 11.1.0U/k
        // old/new3ds 11.2.0U/k
        // old/new3DS 11.3.0 U
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(11, 1, 0);
        ntrConfig->HomeMenuInjectAddr = 0x12ded0;
        ntrConfig->HomeFSReadAddr = 0x12c19c;
        ntrConfig->HomeCardUpdateInitAddr = 0x118d78;
        ntrConfig->HomeFSUHandleAddr = 0x32dfa4;
        ntrConfig->HomeAptStartAppletAddr = 0x12ea08;
    }
    else if (t == 0xe0811101)
    {
        // old/new3ds 11.1.0E / J
        // old/new3ds 11.2.0E / J
        // old/new 11.3.0 E
        ntrConfig->HomeMenuVersion = SYSTEM_VERSION(11, 1, 0);
        ntrConfig->HomeMenuInjectAddr = 0x12ded0;
        ntrConfig->HomeFSReadAddr = 0x12c19c;
        ntrConfig->HomeCardUpdateInitAddr = 0x118d78;
        ntrConfig->HomeFSUHandleAddr = 0x32efa4;
        ntrConfig->HomeAptStartAppletAddr = 0x12ea08;
    }
    else 
    {
        goto unsupported;
    }
    return (0);
unsupported:
    ntrConfig->HomeMenuVersion = 0;
    check_sec(RESULT_ERROR, UNKNOWN_HOMEMENU);
error:
    return (RESULT_ERROR);
}*/


#define MAX_MAP_SIZE (0x00400000)

u32     translateAddr(u32 mappedAddr)
{
    if (mappedAddr < 0x0f000000)
    {
        return (0);
    }
    return (mappedAddr - 0x0f000000 + 0x00100000);
}

u32     locateSwapBuffer(u32 startAddr, u32 endAddr) 
{
    
    static u32 pat[] = { 0xe1833000, 0xe2044cff, 0xe3c33cff, 0xe1833004, 0xe1824f93 };
    static u32 pat2[] = { 0xe8830e60, 0xee078f9a, 0xe3a03001, 0xe7902104 };
    static u32 pat3[] = { 0xee076f9a, 0xe3a02001, 0xe7901104, 0xe1911f9f, 0xe3c110ff};

    u32 addr = searchBytes(startAddr, endAddr, (u8 *)pat, sizeof(pat), 4);
    if (!addr) 
    {
        addr = searchBytes(startAddr, endAddr, (u8 *)pat2, sizeof(pat2), 4);
    }
    if (!addr) 
    {
        addr = searchBytes(startAddr, endAddr, (u8 *)pat3, sizeof(pat3), 4);
    }
    return (findNearestSTMFD(startAddr, addr));
}

static const u8 patFsRead[] = { 0xC2, 0x00, 0x02, 0x08 };
static const u8 patFsHandle[] = { 0xf9, 0x67, 0xa0, 0x08 };
static const u8 patCartUpdate[] = { 0x42, 0x00, 0x07, 0x00 };
static const u8 patStartApplet[] = { 0x40, 0x01, 0x15, 0x00 };

Result  analyseHomeMenu(void)
{
    Result  ret = 0;
    u32     text = 0x0f000000;
    u32     mapSize = MAX_MAP_SIZE;

    MemInfo     meminfo;
    PageInfo    pageinfo;

    memset(&meminfo, 0, sizeof(meminfo));
    memset(&pageinfo, 0, sizeof(pageinfo));

    newAppTopDebug(DEFAULT_COLOR, SKINNY, "Starting analysis...");

    ret = svcQueryMemory(&meminfo, &pageinfo, (u32)text);
    if(R_FAILED(ret))
    {
        newAppTopDebug(RED, SKINNY, "svcQueryMemory failed: 0x%08x", (unsigned int)ret);
        return (RESULT_ERROR);
    }

    if(meminfo.size < mapSize) 
        mapSize = meminfo.size;

    newAppTopDebug(DEFAULT_COLOR, SKINNY, "mapSize: %08x, size: %08x", mapSize, meminfo.size);

    
    ntrConfig->HomeFSReadAddr = translateAddr(findNearestSTMFD(text ,searchBytes(text, text + mapSize, (u8 *)patFsRead, sizeof(patFsRead), 4)));
    newAppTopDebug(DEFAULT_COLOR, SKINNY, "HomeFSReadAddr: %08x", ntrConfig->HomeFSReadAddr);

    
    u32 t = searchBytes(text, text + mapSize, (u8 *)patFsHandle, sizeof(patFsHandle), 4);
    if (t > 0)
    {
        ntrConfig->HomeFSUHandleAddr = *(u32*)(t - 4);
    }
    newAppTopDebug(DEFAULT_COLOR, SKINNY,"HomeFSUHandleAddr: %08x", ntrConfig->HomeFSUHandleAddr);


    ntrConfig->HomeCardUpdateInitAddr = translateAddr(findNearestSTMFD(text, searchBytes(text, text + mapSize, (u8 *)patCartUpdate, sizeof(patCartUpdate), 4)));
    newAppTopDebug(DEFAULT_COLOR, SKINNY,"HomeCardUpdateInitAddr: %08x", ntrConfig->HomeCardUpdateInitAddr);

    
    ntrConfig->HomeAptStartAppletAddr = translateAddr(findNearestSTMFD(text, searchBytes(text, text + mapSize, (u8 *)patStartApplet, sizeof(patStartApplet), 4)));
    newAppTopDebug(DEFAULT_COLOR, SKINNY,"HomeAptStartAppletAddr: %08x", ntrConfig->HomeAptStartAppletAddr);

    ntrConfig->HomeMenuInjectAddr = translateAddr(locateSwapBuffer(text, text + mapSize));
    newAppTopDebug(DEFAULT_COLOR, SKINNY,"HomeMenuInjectAddr: %08x", ntrConfig->HomeMenuInjectAddr);

    newAppTopDebug(GREEN, SKINNY, "Analysis finished.");

    return (0);
}

Result bnInitParamsByHomeMenu(void)
{
    u32     hProcess = 0;
    u32     ret;

again:
    ret = svcOpenProcess(&hProcess, ntrConfig->HomeMenuPid);
    if (ret)
    {
        newAppStatus(DEFAULT_COLOR, TINY | CENTER, "An error occurred");
        newAppStatus(DEFAULT_COLOR, TINY | CENTER, "Retry in 2 seconds");
        updateUI();
        svcSleepThread(2000000000);
        removeAppStatus();
        removeAppStatus();
        updateUI();
        goto again;
    }

    //Map .text+0(0x00100000) up to 0x40000, to vaddr 0x0f000000 in the current process.
    ret = svcMapProcessMemory(hProcess, 0x0f000000, MAX_MAP_SIZE);
    if (ret != 0)
    {
        newAppStatus(DEFAULT_COLOR, TINY | CENTER, "Mapping failed !");
        newAppStatus(DEFAULT_COLOR, TINY | CENTER, "%08X", ret);
        updateUI();
        goto final;
    }
    ret = analyseHomeMenu();
    svcUnmapProcessMemory(hProcess, 0x0f000000, MAX_MAP_SIZE);
    svcCloseHandle(hProcess);
    ntrConfig->HomeMenuVersion = SYSTEM_VERSION(1, 0, 0);
    return (ret);
final:    
    return (RESULT_ERROR);
}