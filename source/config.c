#include "config.h"

static bootNtrConfig_t  g_bnConfig = { 0 };
static ntrConfig_t      g_ntrConfig = { 0 };
bootNtrConfig_t         *bnConfig;
ntrConfig_t             *ntrConfig;

static const char   *configPath = "/Nintendo 3DS/EBNTR/config";
static const char   *configDir = "/Nintendo 3DS/EBNTR/";

bool    checkPath(void)
{
    char    *tmp;

    if (strncmp(bnConfig->config->binariesPath, "sdmc:/", 6)) goto error;
    if (strncmp(bnConfig->config->pluginPath, "sdmc:/", 6)) goto error;
    tmp = bnConfig->config->binariesPath;
    while (*(tmp + 1)) tmp++;
    if (*tmp != '/') *(tmp + 1) = '/';
    tmp = bnConfig->config->pluginPath;
    while (*(tmp + 1)) tmp++;
    if (*tmp != '/') *(tmp + 1) = '/';
    return (true);
error:
    if (*bnConfig->config->binariesPath == '/')
        strInsert(bnConfig->config->binariesPath, "sdmc:", 0);
    else
        strInsert(bnConfig->config->binariesPath, "sdmc:/", 0);
    if (*bnConfig->config->pluginPath == '/')
        strInsert(bnConfig->config->pluginPath, "sdmc:", 0);
    else
        strInsert(bnConfig->config->pluginPath, "sdmc:/", 0);
    return (checkPath());
}

bool    loadConfigFromFile(config_t *config)
{
    FILE        *file;

    file = NULL;
    if (!config) goto error;
    if (!fileExists(configPath)) goto error;
    file = fopen(configPath, "rb");
    if (!file) goto error;
    fread(config, sizeof(config_t), 1, file);
    fclose(file);

    // Check version of the config file
    if (config->version != CURRENT_CONFIG_VERSION)
    {
        memset(config, 0, sizeof(config_t));
        config->version = CURRENT_CONFIG_VERSION;
        bnConfig->checkForUpdate = true;
        goto error;
    }

    return (true);
error:
    return (false);
}

bool    saveConfig(void)
{
    FILE        *file;
    config_t    *config;

    config = g_bnConfig.config;
    if (!fileExists(configDir))
        createDir(configDir);
    if (!fileExists(configDir)) goto error;
    file = fopen(configPath, "wb");
    if (!file) goto error;
    fwrite(config, sizeof(config_t), 1, file);
    fclose(file);
    return (true);
error:
    return (false);
}

void    resetConfig(void)
{
    char        path[0x100];
    config_t    *config = NULL;
    bool        binPath = false;
    u32         keys;
    u32         size;

    do
    {
      updateUI();
      keys = (hidKeysDown() | hidKeysHeld());
    } while (keys & KEY_SELECT);

    if (!fileExists(configPath)) goto exit;
    config = (config_t *)calloc(1, sizeof(config_t));
    if (config)
        binPath = loadConfigFromFile(config);
    remove(configPath);
    rmdir(configDir);
    if (!binPath) goto exit;
    size = strlen(config->binariesPath);
    if (!size) goto exit;
    memset(path, 0, 0x100);
    strJoin(path, config->binariesPath + 5, "ntr_3_2.bin");
    remove(path);
    memset(path, 0, 0x100);
    strJoin(path, config->binariesPath + 5, "ntr_3_3.bin");
    remove(path);
    memset(path, 0, 0x100);
    strJoin(path, config->binariesPath + 5, "ntr_3_4.bin");
    remove(path);
    
exit:
    if (config)
        free(config);
    return;
}

void    configInit(void)
{
    config_t    *config;
    Handle      fsuHandle;

    srvGetServiceHandle(&fsuHandle, "fs:USER");
    FSUSER_Initialize(fsuHandle);
    memset(&g_ntrConfig, 0, sizeof(g_ntrConfig));
    memset(&g_bnConfig, 0, sizeof(g_bnConfig));
    ntrConfig = &g_ntrConfig;
    bnConfig = &g_bnConfig;
    ntrConfig->fsUserHandle = fsuHandle;
    config = (config_t *)calloc(1, sizeof(config_t));
    if (!config) goto error;
    bnConfig->config = config;
    if (!loadConfigFromFile(config))
    {
        firstLaunch();
        if (!saveConfig())
            newAppTop(DEFAULT_COLOR, 0, "A problem occured while saving the settings.");
    #if EXTENDEDMODE
        bnConfig->versionToLaunch = V34;
    #endif
    }
    else
    {

        time_t current = time(NULL);
#if EXTENDEDMODE
        if (current - config->lastUpdateTime3 >= SECONDS_IN_WEEK)
#else
        if (current - config->lastUpdateTime >= SECONDS_IN_WEEK)            
#endif
            bnConfig->checkForUpdate = true;
        else
            bnConfig->checkForUpdate = false;
    }
   // svcCloseHandle(fsuHandle);
#if EXTENDEDMODE
    bnConfig->versionToLaunch = V34;
    config->flags = LV34;
#else
    if (config->flags & LV32) bnConfig->versionToLaunch = V32;
    else if (config->flags & LV33) bnConfig->versionToLaunch = V33;
    else if (config->flags & LV34) bnConfig->versionToLaunch = V34;
#endif
error:
    return;
}

void    configExit(void)
{
    version_t   version;
    config_t    *config;
    u32         flags;

    version = bnConfig->versionToLaunch;
    config = bnConfig->config;
    if (version == V32) flags = LV32;
    else if (version == V33) flags = LV33;
    else if (version == V34) flags = LV34;
    else flags = 0;
    config->flags = flags;
    saveConfig();
    free(config);
}