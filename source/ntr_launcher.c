#include "main.h"
#include "config.h"

extern ntrConfig_t		*ntrConfig;
extern bootNtrConfig_t	*bnConfig;
extern u8				*tmpBuffer;
extern bool              g_exit;
extern char				*g_primary_error;
extern char				*g_secondary_error;
extern char				*g_third_error;

int         isNTRAlreadyLaunched(void)
{
    Result  ret;
    Handle  processHandle;

    ret = svcOpenProcess(&processHandle, 0xf);
    if (ret) goto error;
    ret = copyRemoteMemory(CURRENT_PROCESS_HANDLE, (u32)tmpBuffer, processHandle, 0x06000000, 0x1000);
    svcCloseHandle(processHandle);
    if (!ret)
    {
        g_exit = true;
        goto error;
    }
    return (0);
error:
    return (RESULT_ERROR);
}

Result		bnPatchAccessCheck(void)
{
	Result	ret;
	u8		smPatchBuf[] = { 0x02, 0x00, 0xA0, 0xE3, 0x1E, 0xFF, 0x2F, 0xE1 };
	u32		fsPatchValue = 0x47702001;

	svcBackdoor(backdoorHandler);
	// do a dma copy to get the finish state value on current console
	ret = copyRemoteMemory(CURRENT_PROCESS_HANDLE, (u32)tmpBuffer, CURRENT_PROCESS_HANDLE, (u32)tmpBuffer + 0x10, 0x10);
	check_sec(ret, REMOTECOPY_FAILURE);
	ret = patchRemoteProcess(bnConfig->SMPid, bnConfig->SMPatchAddr, smPatchBuf, 8);
	check_sec(ret, SMPATCH_FAILURE);
	ret = patchRemoteProcess(bnConfig->FSPid, bnConfig->FSPatchAddr, (u8 *)&fsPatchValue, 4);
	check_sec(ret, FSPATCH_FAILURE);
	return (0);
error:
	return (RESULT_ERROR);
}

u32 loadNTRBin(void)
{
    u32                 size;
    u32                 alignedSize;
    u8                  *mem;
    FILE                *ntr;
    u32                 ret;
    char                path[0x100];

    static const char   *ntrVersionStrings[3] =
    {
        "ntr_3_2.bin",
        "ntr_3_3.bin",
        "ntr_3_4.bin"
    };

    if (bnConfig->versionToLaunch == V32)
        strJoin(path, "/", "ntr.bin");
    else
        strJoin(path, bnConfig->config->binariesPath + 5, ntrVersionStrings[bnConfig->versionToLaunch]);

    if (bnConfig->versionToLaunch == V34)
    {
        strJoin(ntrConfig->path, bnConfig->config->binariesPath + 5, ntrVersionStrings[bnConfig->versionToLaunch]);
    #if EXTENDEDMODE
        ntrConfig->memorymode = 3;
    #else
        ntrConfig->memorymode = 0;
    #endif
    }
    
    // Get size
    ntr = fopen(path, "rb");
    check_prim(!ntr, FILEOPEN_FAILURE);
    fseek(ntr, 0, SEEK_END);
    size = ftell(ntr);
    fseek(ntr, 0, SEEK_SET);
    alignedSize = rtAlignToPageSize(size);
    ntrConfig->arm11BinSize = alignedSize;

    // Allocate memory
    mem = (u8 *)linearMemAlign(alignedSize * 2, 0x1000);
    check_sec(!mem, LINEARMEMALIGN_FAILURE);
    ntrConfig->arm11BinStart = ((u32)mem + alignedSize);
    ret = rtCheckRemoteMemoryRegionSafeForWrite(getCurrentProcessHandle(), (u32)mem, alignedSize * 2);
    check_prim(ret, PROTECTMEMORY_FAILURE);

    // Read to memory
    memset(mem, 0, alignedSize * 2);
    u8 *temp = (u8 *)calloc(1, alignedSize);
    fread(temp, size, 1, ntr);
    fclose(ntr);
    svcFlushProcessDataCache(getCurrentProcessHandle(), temp, size);

    copyRemoteMemory(getCurrentProcessHandle(), (u32)mem, getCurrentProcessHandle(), (u32)temp, size);
    copyRemoteMemory(getCurrentProcessHandle(), (u32)mem + alignedSize, getCurrentProcessHandle(), (u32)temp, size);
    free(temp);
    return ((u32)mem);
error:
    return (RESULT_ERROR);
}

Result		bnLoadAndExecuteNTR(void)
{
	u32		outAddr;
	u32		*bootArgs;

    outAddr = loadNTRBin();
    if (outAddr == RESULT_ERROR) 
	{
	        goto error;
	}
	bootArgs = (u32 *)(outAddr + 4);
	bootArgs[0] = 0;
	bootArgs[1] = 0xb00d;
	bootArgs[2] = (u32)ntrConfig;
	((funcType)(outAddr))();
	return (0);
error:
	return (RESULT_ERROR);
}
void        showDbg(char *str)
{
     newAppTop(DEFAULT_COLOR, TINY | SKINNY, str);
}

Result		bnBootNTR(void)
{
	Result  ret;
    u8      *linearAddress;

    linearAddress = NULL;
    
    // Set firm params
    check_prim(bnInitParamsByFirmware(), UNKNOWN_FIRM);
	
    // Alloc temp buffer
    linearAddress = (u8 *)linearMemAlign(TMPBUFFER_SIZE, 0x1000);
    tmpBuffer = linearAddress;
	check_prim(!tmpBuffer, LINEARMEMALIGN_FAILURE);
    rtCheckRemoteMemoryRegionSafeForWrite(getCurrentProcessHandle(), (u32)tmpBuffer, TMPBUFFER_SIZE);
    ret = isNTRAlreadyLaunched();
    check_prim(ret, NTR_ALREADY_LAUNCHED);
    // Patch services
    ret = bnPatchAccessCheck();
    check_third(ret, ACCESSPATCH_FAILURE);
    
    // Init home menu params
    check_sec(bnInitParamsByHomeMenu(), UNKNOWN_HOMEMENU);
    
    // Free temp buffer
    linearFree(linearAddress);
#if DEBUG
    ntrConfig->ShowDbgFunc = (u32)showDbg;
#endif
    // Load NTR
	ret = bnLoadAndExecuteNTR();
	check_third(ret, LOAD_FAILED);
	return (ret);
error:
    if (linearAddress) linearFree(linearAddress);
	return (RESULT_ERROR);
}

static char *dumpString[] =
{
    "Kernel",
    "FS Process",
    "PM Process",
    "SM Process",
    "HomeMenu Process"
};

void        printDumpLog(char *str)
{
    static int  remove = 0;
    static int  changeMode = 0;
    static int  mode = 0;
    static char address[9] = "DFFF0000";
    //Mode:
    // 0 = Kernel Shared Memeory
    // 1 = FS
    // 2 = PM
    // 3 = SM
    // 4 = HomeMenu

    if (strncmp(str, "dump finished", 13) == 0)
    {
        changeMode = 1;
        str += 5;
    }
    if (changeMode || strncmp(str, "addr:", 5) == 0)
    {
        if (mode == 0)
        {
            strncpyFromTail(address, str, 8);
            if (*address == '0')
            {
                remove = 0;
                mode += 1;
            }
        }
        if (remove) removeAppTop();
        remove = 1;
        newAppTop(DEFAULT_COLOR, TINY | SKINNY, "%s\uE019%s", dumpString[mode], str);
        mode += changeMode;
        remove = !changeMode;
        changeMode = 0;
    }
    else
    {
        if (strncmp(str, "current", 7) && strncmp(str, "firmware", 8))
            newAppTop(DEFAULT_COLOR, TINY | SKINNY, str);
    }
}

void        launchNTRDumpMode(void)
{
    u32		isNew3DS = 0;
    char    buffer[0x20];

    APT_CheckNew3DS((bool *)&isNew3DS);

    ntrConfig->firmVersion = 0;
    ntrConfig->HomeMenuVersion = 0;
    ntrConfig->isNew3DS = isNew3DS;
    ntrConfig->PMPid = 2;
    ntrConfig->HomeMenuPid = 0xf;
    bnConfig->versionToLaunch = V33;
    ntrConfig->ShowDbgFunc = (u32)printDumpLog;
    copyRemoteMemory(CURRENT_PROCESS_HANDLE, (u32)buffer, CURRENT_PROCESS_HANDLE, (u32)buffer + 0x10, 0x10);
    bnLoadAndExecuteNTR();
}