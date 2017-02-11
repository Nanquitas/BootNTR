#include "main.h"
#include "json/json.h"
#include "graphics.h"
#include "drawableObject.h"
#include "button.h"
#include "config.h"
#include <time.h>

extern bootNtrConfig_t *bnConfig;
static updateData_t     *updateData;
static window_t         *updaterWindow;
static button_t         *okButton;
static sprite_t         *buttonBackground;
static char             *changelog = NULL;
static bool             userOk = false;

static void userApproval(u32 arg)
{
    userOk = (bool)arg;
}

static void print(const char *str, ...)
{
    char        buffer[0x100];
    va_list     vlist;

    va_start(vlist, str);
    vsnprintf(buffer, 0x100, str, vlist);
    va_end(vlist);
    newAppTop(COLOR_BLANK, SKINNY, buffer);
}

static void initUpdater(void)
{
    sprite_t    *background;
    sprite_t    *title;
    sprite_t    *sprite;

    newSpriteFromPNG(&background, "romfs:/sprites/menuBackground.png");
    newSpriteFromPNG(&title, "romfs:/sprites/textSprites/updaterTitle.png");
    setSpritePos(background, 43.0f, 20.0f);
    setSpritePos(title, 150.0f, 12.0f);

    newSpriteFromPNG(&buttonBackground, "romfs:/sprites/largeButtonBackground.png");
    newSpriteFromPNG(&sprite, "romfs:/sprites/textSprites/ok.png");
    okButton = newButton(48.0f, 88.0f, userApproval, 1, buttonBackground, sprite);

    updaterWindow = newWindow(background, title, NULL);
    addTopObject(updaterWindow);
    addBottomObject(okButton);
    updateUI();
    appInfoHideBackground();
}

static void exitUpdater(void)
{
    clearTop(0);
    clearTopScreen();
    clearBottomScreen();
    deleteSprite(updaterWindow->title);
    deleteSprite(updaterWindow->background);
    free(updaterWindow);
    destroyButton(okButton);
    deleteSprite(buttonBackground);
    appInfoShowBackground();
}

static Result openUpdateUrl(u32 *handle)
{
    Result res;

    httpcContext *context = (httpcContext *)calloc(1, sizeof(httpcContext));
    if (context != NULL)
    {
        if (R_SUCCEEDED(res = httpcOpenContext(context, HTTPC_METHOD_GET, updateData->url, 1)))
        {
            if (R_SUCCEEDED(res = httpcSetSSLOpt(context, SSLCOPT_DisableVerify))
                && R_SUCCEEDED(res = httpcBeginRequest(context))
                && R_SUCCEEDED(res = httpcGetResponseStatusCode(context, &updateData->responseCode)))
            {
                if (updateData->responseCode == 200)
                    *handle = (u32)context;
                else if (updateData->responseCode == 301 || updateData->responseCode == 302 || updateData->responseCode == 303)
                {
                    if (R_SUCCEEDED(res = httpcGetResponseHeader(context, "Location", updateData->url, URL_MAX)))
                    {
                        httpcCloseContext(context);
                        free(context);
                        return (openUpdateUrl(handle));
                    }
                }
                else
                    res = RESULT_ERROR;
            }
            else
            {
                res = RESULT_ERROR;
            }
            if (res == RESULT_ERROR)
                httpcCloseContext(context);
        }
        if (res == RESULT_ERROR)
            free(context);
    }
    else
        res = RESULT_ERROR;
    return (res);
}

static Result closeUpdateUrl(u32 handle)
{
    return (httpcCloseContext((httpcContext *)handle));
}

static Result getUpdateSize(u32 handle, u32 *size)
{
    Result  res;

    res = httpcGetDownloadSizeState((httpcContext *)handle, NULL, size);
    return (res);
}

static Result downloadUpdate(u32 handle, u32 *bytesRead, void *buffer, u32 size)
{
    Result res;
    
    res = httpcDownloadData((httpcContext *)handle, buffer, size, bytesRead);
    return (res != HTTPC_RESULTCODE_DOWNLOADPENDING ? res : 0);
}

static Result startInstall(u32 *handle)
{
    return (AM_StartCiaInstall(MEDIATYPE_SD, handle));
}

static Result cancelInstall(u32 handle)
{
    return (AM_CancelCIAInstall(handle));
}

static Result endInstall(u32 handle)
{
    return (AM_FinishCiaInstall(handle));
}

static Result installUpdate(void)
{
    char    *buffer;
    u32     size;
    u32     downloadHandle;
    u32     installHandle;
    u32     res;
    u32     bytesRead;
    u32     bytesWritten;

    size = downloadHandle = installHandle = res = bytesRead = bytesWritten = 0;
    print("Status: Getting infos");
    res = openUpdateUrl(&downloadHandle);
    res = getUpdateSize(downloadHandle, &size);
    buffer = (char *)malloc(size);
    if (!buffer)
    {
        res = RESULT_ERROR;
        goto error;
    }
    res = startInstall(&installHandle);
    removeAppTop();
    print("Status: Downloading");
    res = downloadUpdate(downloadHandle, &bytesRead, buffer, size);
    removeAppTop();
    print("Status: Installing");
    res |= FSFILE_Write(installHandle, &bytesWritten, 0, buffer, size, 0);
    if (res & RESULT_ERROR)
    {
        print("An error occurred ! Abort.");
        cancelInstall(installHandle);
    }   
    else
    {
        removeAppTop();
        newAppTop(COLOR_LIMEGREEN, SKINNY, "Status: Finished");
        endInstall(installHandle);
    }        
    closeUpdateUrl(downloadHandle);
    free(buffer);
error:
    return (res & RESULT_ERROR ? RESULT_ERROR : 0);
}

static void printChangelog(void)
{
    char    *tmp;
    int     size;
    int     i;
    int     j;

    if (!changelog) goto exit;
    size = strlen(changelog);
    if (size < 1) goto exit;
    tmp = (char *)malloc(size);
    if (!tmp) goto exit;
    i = j = 0;
    while (*(changelog + i))
    {
        if (*(changelog + i) != '\r')
        {
            *(tmp + j++) = *(changelog + i);
        }
        i++;
    }
    *(tmp + j) = '\0';
    newAppTop(COLOR_BLANK, SKINNY | NEWLINE, "Changelog:");
    newAppTop(COLOR_SILVER, 0, tmp);
    newAppTop(COLOR_BLANK, 0, "");
    free(tmp);
exit:
    return;
}

static Result parseResponseData(const char *jsonText, u32 size, bool *hasUpdate)
{
    int         i;
    int         j;
    char        *url;
    char        versionString[16];
    Result      res;
    json_value  *json;
    json_value  *name;
    json_value  *assets;
    json_value  *assetName;
    json_value  *assetUrl;
    json_value  *val;
    json_value  *subVal;
    
    url = NULL;
    json = name = assets = assetName = assetUrl = val = subVal = NULL;
    json = json_parse(jsonText, size);
    if (json != NULL)
    {
        if (json->type == json_object)
        {
            for (i = 0; i < json->u.object.length; i++)
            {
                val = json->u.object.values[i].value;
                if (strncmp(json->u.object.values[i].name, "name", json->u.object.values[i].name_length) == 0
                    && val->type == json_string)
                    name = val;
                else if (strncmp(json->u.object.values[i].name, "assets", json->u.object.values[i].name_length) == 0
                         && val->type == json_array)
                    assets = val;
                else if (strncmp(json->u.object.values[i].name, "body", json->u.object.values[i].name_length) == 0
                         && val->type == json_string)
                    changelog = val->u.string.ptr;
            }
            if (name != NULL && assets != NULL)
            {   
                if (!APP_VERSION_REVISION)
                    snprintf(versionString, sizeof(versionString), "%d.%d", APP_VERSION_MAJOR, APP_VERSION_MINOR);
                else
                    snprintf(versionString, sizeof(versionString), "%d.%d.%d", APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_REVISION);
                if (strncmp(name->u.string.ptr, versionString, name->u.string.length) > 0)
                {            
                    for (i = 0; i < assets->u.array.length; i++)
                    {
                        val = assets->u.array.values[i];
                        if (val->type == json_object)
                        {                  
                            for (j = 0; j < val->u.object.length; j++)
                            {
                                subVal = val->u.object.values[j].value;
                                if (strncmp(val->u.object.values[j].name, "name", val->u.object.values[j].name_length) == 0
                                    && subVal->type == json_string)
                                    assetName = subVal;
                                else if (strncmp(val->u.object.values[j].name, "browser_download_url", val->u.object.values[j].name_length) == 0
                                    && subVal->type == json_string)
                                    assetUrl = subVal;
                            }
                            if (assetName != NULL && assetUrl != NULL)
                            {
                                if (strncmp(assetName->u.string.ptr, CIA_VERSION, assetName->u.string.length) == 0)
                                {
                                    url = assetUrl->u.string.ptr;
                                    break;
                                }
                            }
                        }
                    }
                    if (url != NULL)
                    {
                        strncpy(updateData->url, url, URL_MAX);
                        *hasUpdate = true;
                        removeAppTop();
                        newAppTop(COLOR_SALMON, SKINNY, "New update available: %s -> %s", versionString, name->u.string.ptr);
                        printChangelog();
                    }
                    else
                        res = RESULT_ERROR;
                }
            }
            else
                res = RESULT_ERROR;
        }
        else
            res = RESULT_ERROR;
    }
    else
        res = RESULT_ERROR;
    return (res);
}

static bool checkUpdate(void)
{
    bool            hasUpdate = false;
    u32             size = 0;
    u32             responseCode = 0;
    u32             bytesRead;
    Result          res = 0;
    httpcContext    context;
    char            userAgent[128];
    char            *jsonText;

    if (R_SUCCEEDED(res = httpcOpenContext(&context, HTTPC_METHOD_GET, "https://api.github.com/repos/Nanquitas/BootNTR/releases/latest", 1)))
    {        
        snprintf(userAgent, sizeof(userAgent), "Mozilla/5.0 (Nintendo 3DS; Mobile; rv:10.0)");
        if (R_SUCCEEDED(res = httpcSetSSLOpt(&context, SSLCOPT_DisableVerify))
            && R_SUCCEEDED(res = httpcAddRequestHeaderField(&context, "User-Agent", userAgent))
            && R_SUCCEEDED(res = httpcBeginRequest(&context))
            && R_SUCCEEDED(res = httpcGetResponseStatusCode(&context, &responseCode)))
        {
            if (responseCode == 200)
            {
                if (R_SUCCEEDED(res = httpcGetDownloadSizeState(&context, NULL, &size)))
                {
                    jsonText = (char*)calloc(sizeof(char), size);
                    if (jsonText != NULL)
                    {
                        bytesRead = 0;
                        if (R_SUCCEEDED(res = httpcDownloadData(&context, (u8*)jsonText, size, &bytesRead)))
                        {
                            res = parseResponseData(jsonText, size, &hasUpdate);
                        }
                        free(jsonText);
                    }
                    else
                        res = RESULT_ERROR;
                }
            }
            else
                res = RESULT_ERROR;
        }
        httpcCloseContext(&context);
    }
    if (hasUpdate)
    {
       res = installUpdate();
       if (!res)
        return (true);
    }
    return (false);
}

bool launchUpdater(void)
{
    bool update;
    update = false;
    updateData = (updateData_t *)calloc(1, sizeof(updateData_t));
    if (!updateData) goto error;
#if EXTENDEDMODE
    bnConfig->config->lastUpdateTime3 = time(NULL);
#else
    bnConfig->config->lastUpdateTime = time(NULL);
#endif
    initUpdater();
    newAppTop(COLOR_BLANK, NEWLINE, "");
    newAppTop(COLOR_BLANK, 0, "Checking for update");
    update = checkUpdate();
    free(updateData);
    if (update)
    {
        okButton->show(okButton);
        while (!userOk)
        {
            updateUI();
            if ((hidKeysDown() | hidKeysHeld()) & KEY_B)
                break;
        }
    }
    exitUpdater();
error:
    return (update);
}