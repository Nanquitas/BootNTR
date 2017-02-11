#ifndef CONFIG_H
#define CONFIG_H

#include "main.h"
#include <time.h>   

#ifndef SYSTEM_VERSION
#define SYSTEM_VERSION(major, minor, revision) \
    (((major)<<24)|((minor)<<16)|((revision)<<8))
#endif

#define CURRENT_CONFIG_VERSION  SYSTEM_VERSION(1, 0, 4)

#define SECONDS_IN_WEEK     604800
#define SECONDS_IN_DAY      86400
#define SECONDS_IN_HOUR     3600
#define SECONDS_IN_MINUTE   60

enum
{
    LV32 = BIT(0),
    LV33 = BIT(1),
    LV34 = BIT(2),
    CUSTOM_PLUGIN_PATH = BIT(3)
}configFlags;

typedef struct  config_s
{
    u32         version;
    u32         flags;
    char        binariesPath[0x100];
    char        pluginPath[0x100];
    time_t      lastUpdateTime;
    time_t      lastUpdateTime3;

}               config_t;

typedef struct  ntrConfig_s
{
    u32         bootNTRVersion;
    u32         isNew3DS;
    u32         firmVersion;

    u32         IoBasePad;
    u32         IoBaseLcd;
    u32         IoBasePdc;
    u32         PMSvcRunAddr;
    u32         PMPid;
    u32         HomeMenuPid;

    u32         HomeMenuVersion;
    u32         HomeMenuInjectAddr; // FlushDataCache Function
    u32         HomeFSReadAddr;
    u32         HomeFSUHandleAddr;
    u32         HomeCardUpdateInitAddr;
    u32         HomeAptStartAppletAddr;

    u32         KProcessHandleDataOffset;
    u32         KProcessPIDOffset;
    u32         KProcessCodesetOffset;
    u32         ControlMemoryPatchAddr1;
    u32         ControlMemoryPatchAddr2;
    u32         KernelFreeSpaceAddr_Optional;
    u32         KMMUHaxAddr;
    u32         KMMUHaxSize;
    u32         InterProcessDmaFinishState;
    u32         fsUserHandle;
    u32         arm11BinStart;
    u32         arm11BinSize;
    u32         ShowDbgFunc;
    u32         memorymode;
    char        path[0x100];
}               ntrConfig_t;

typedef struct  bootNtrConfig_s
{
    u32         FSPatchAddr;
    u32         SMPatchAddr;
    u32         SvcPatchAddr;
    u32         FSPid;
    u32         SMPid;
    u32         requireKernelHax;
    version_t   versionToLaunch;
    config_t    *config;
    bool        checkForUpdate;
}               bootNtrConfig_t;

void    configInit(void);
void    resetConfig(void);
void    configExit(void);
bool    checkPath(void);

void    firstLaunch(void);

#endif