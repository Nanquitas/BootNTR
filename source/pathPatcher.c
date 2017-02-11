#include "main.h"
#include "config.h"

extern ntrConfig_t		*ntrConfig;
extern bootNtrConfig_t  *bnConfig;
extern char				*g_primary_error;
extern char				*g_secondary_error;
extern char				*g_third_error;

#define RELOC_COUNT     9
#define BASE            0x100100

enum
{
    BINARY = 0,
    PLUGIN,
    DEBUG,
    KERNEL,
    FS,
    PM,
    SM,
    HOMEMENU,
    ARM
};

static const char  *originalPath[RELOC_COUNT] =
{
        "/ntr.bin",
        "/plugin/%s",
        "/debug.flag",
        "/axiwram.dmp",
        "/pid0.dmp",
        "/pid2.dmp",
        "/pid3.dmp",
        "/pidf.dmp",
        "/arm11.bin"
};

static const char *fixedName[RELOC_COUNT] =
{
    "",
    "%s",
    "",
    "Kernel.dmp",
    "FS.dmp",
    "PM.dmp",
    "SM.dmp",
    "HomeMenu.dmp",
    "arm11.dmp"
};

static char fixedPath[RELOC_COUNT][0x100] = { 0 };

static const char *ntrVersionStrings[4] =
{
    "ntr_3_2.bin",
    "ntr_3_3.bin",
    "ntr.o3ds.bin",
    "ntr.n3ds.bin"
};

static const char *outNtrVersionStrings[3] =
{
    "ntr_3_2.bin",
    "ntr_3_3.bin",
    "ntr_3_4.bin",
};

static void patchBinary(u8 *mem, int size)
{
    int     i;
    int     expand;
    char    *str;
    u32     offset;
    u32     *patchMe;
    u32     patchOffset;
    u32     *patchRel;
    u32     strlength;
    
    expand = 0;
    for (i = 0; i < RELOC_COUNT; i++)
    {
        str = (char *)originalPath[i];
        strlength = strlen(str);
        offset = memfind(mem, size, str, strlength);
        if (offset == 0)
        {
            newAppTop(DEFAULT_COLOR, TINY, "Not found \"%s\".", str);
            continue;
        }

        // Clear string data
        memset(&mem[offset], 0, strlength);

        // Rebase pointer relative to NTRs base.
        offset += BASE;
        patchMe = (u32 *)memfind(mem, size, (u8 *)&offset, 4); // Find xref
        if (patchMe == 0)
        {
            newAppTop(DEFAULT_COLOR, TINY, "Pointer for \"%s\"", str);
            newAppTop(DEFAULT_COLOR, TINY, "is missing!Aborting.\n");
            break;
        }

        // New offset is end + patch count * buffer
        patchOffset = size + (0x100 * expand);
        strcpy((void *)&mem[patchOffset], fixedPath[i]);

        expand += 1;

        patchRel = (u32 *)(&mem[(u32)patchMe]);
        // Rebase new pointer
        *patchRel = patchOffset + BASE;           
    }
}

Result  loadAndPatch(version_t version)
{
    FILE    *ntr;
    int     size;
    int     newSize;
    char    *binPath;
    char    *plgPath;
    char    inPath[0x100];
    char    outPath[0x100];
    u8      *mem;    
    bool    isNew3DS = false;

    APT_CheckNew3DS(&isNew3DS);

    binPath = bnConfig->config->binariesPath;
    plgPath = bnConfig->config->pluginPath; 
    if (version == V34 && isNew3DS)
        strJoin(inPath, "romfs:/", ntrVersionStrings[version + 1]);
    else
        strJoin(inPath, "romfs:/", ntrVersionStrings[version]);        
    

    if (!strncmp("sdmc:", binPath, 5)) binPath += 5;
    if (!strncmp("sdmc:", plgPath, 5)) plgPath += 5;

    if (version != V32)
    {
        strJoin(outPath, binPath, outNtrVersionStrings[version]);
        strJoin(fixedPath[BINARY], binPath, ntrVersionStrings[version]);
        strJoin(fixedPath[PLUGIN], plgPath, fixedName[PLUGIN]);
        strJoin(fixedPath[DEBUG], binPath, ntrVersionStrings[version]);
        strJoin(fixedPath[KERNEL], binPath, fixedName[KERNEL]);
        strJoin(fixedPath[FS], binPath, fixedName[FS]);
        strJoin(fixedPath[PM], binPath, fixedName[PM]);
        strJoin(fixedPath[SM], binPath, fixedName[SM]);
        strJoin(fixedPath[HOMEMENU], binPath, fixedName[HOMEMENU]);
        strJoin(fixedPath[ARM], binPath, fixedName[ARM]);
    }
    else
    {
        strcpy(outPath, originalPath[BINARY]);
    }
    ntr = fopen(inPath, "rb");
    if (!ntr) goto error;
    fseek(ntr, 0, SEEK_END);
    size = ftell(ntr);
    rewind(ntr);
    newSize = (version == V32) ? size : size + (RELOC_COUNT * 0x100);
    mem = (u8 *)malloc(newSize);
    if (!mem) goto error;
    memset(mem, 0, newSize);
    fread(mem, size, 1, ntr);
    fclose(ntr);
    svcFlushProcessDataCache(CURRENT_PROCESS_HANDLE, mem, newSize);
    if (version != V32)
        patchBinary(mem, size);
    ntr = fopen(outPath, "wb");
    if (!ntr) goto error;
    fwrite(mem, newSize, 1, ntr);
    fclose(ntr);
    free(mem);
    return(0);
error:
    return(RESULT_ERROR);
}