#include "main.h"
#include "draw.h"
#include "config.h"
#include "button.h"
#include <time.h>

extern bootNtrConfig_t *bnConfig;
u8				  *tmpBuffer;
char			  *g_primary_error = NULL;
char			  *g_secondary_error = NULL;
char			  *g_third_error = NULL;
bool			  g_exit = false;

int main(void)
{
    u32         keys;
    u32         kernelVersion;
    int         ret;

	gfxInitDefault();
	drawInit();
	romfsInit();
    ptmSysmInit();

    initUI();
    hidScanInput();
    keys = (hidKeysDown() | hidKeysHeld());
    if (keys & KEY_SELECT)
        resetConfig();
    configInit();

    // If keys == X or if config say we should check an update
    if (keys & KEY_X || bnConfig->checkForUpdate)
    {
        // Check if the 3DS is connected
        acInit();

        u32 wifiStatus;
        ACU_GetWifiStatus(&wifiStatus);
        if (wifiStatus)
        {
            amInit();
            httpcInit(0);
            if (launchUpdater())
            {
                newAppStatus(DEFAULT_COLOR, CENTER | BOLD | NEWLINE, "Updated !");
                goto waitForExit;
            }            

        }
    }


    kernelVersion = osGetKernelVersion();
    initMainMenu();
    waitAllKeysReleased();
    ret = mainMenu();
    if (ret == 2) goto waitForExit;
    if (!g_exit)
    {
        ret = bnBootNTR();
        if (!ret)
        {
            newAppStatus(DEFAULT_COLOR, CENTER | BOLD | NEWLINE, "Success !");

        #if EXTENDEDMODE
            
            newAppStatus(DEFAULT_COLOR, CENTER | TINY | SKINNY | NEWLINE, "Press Home to launch");
            newAppStatus(DEFAULT_COLOR, CENTER | TINY | SKINNY, "your game.");
            updateUI();

            g_exit = true;
            while (aptMainLoop())
            {
                updateUI();
            }
            goto exit;

        #else

            newAppStatus(DEFAULT_COLOR, CENTER | TINY | SKINNY | NEWLINE, "Returning to home");
            newAppStatus(DEFAULT_COLOR, CENTER | TINY | SKINNY, "menu ...");
            g_exit = true;
            updateUI();
            svcSleepThread(100000);
            goto exit;
        #endif

        }        
    }
	if (g_exit || ret)
	{
        newAppStatus(DEFAULT_COLOR, CENTER | BOLD | NEWLINE, "Load failed !");
        if (!g_third_error) newAppStatus(DEFAULT_COLOR, CENTER | BOLD, "\uE00A");
        if (g_primary_error != NULL)
        {
            if (g_primary_error == UNKNOWN_FIRM)
            {
                newAppStatus(DEFAULT_COLOR, TINY | CENTER, "#Firmware unknown");
                newAppStatus(DEFAULT_COLOR, TINY | CENTER, "#Detected firm: %d.%d.%d", \
                    GET_VERSION_MAJOR(kernelVersion), \
                    GET_VERSION_MINOR(kernelVersion), \
                    GET_VERSION_REVISION(kernelVersion));
            }
            else
                newAppStatus(DEFAULT_COLOR, TINY | CENTER, "#%s", g_primary_error);
        }            
        if (g_secondary_error != NULL)
            newAppStatus(DEFAULT_COLOR, TINY | CENTER, "#%s", g_secondary_error);
        if (g_third_error != NULL)
            newAppStatus(DEFAULT_COLOR, TINY | CENTER, "#%s", g_third_error);
waitForExit:
        newAppStatus(DEFAULT_COLOR, CENTER | TINY | SKINNY | NEWLINE, "Press any key to");
        if (g_exit)
            newAppStatus(DEFAULT_COLOR, CENTER | TINY | SKINNY, "return to HomeMenu");
        else
            newAppStatus(DEFAULT_COLOR, CENTER | TINY | SKINNY, "reboot");
        while (aptMainLoop())
        {
            updateUI();
            hidScanInput();
            keys = hidKeysDown();
            if (keys)
                break;
        }
	}
exit:
    configExit();
    exitMainMenu();
    exitUI();
    acExit();
    amExit();
    httpcExit();
	romfsExit();
	drawExit();
	gfxExit();
    if (!g_exit)
        PTMSYSM_RebootAsync(0);
    ptmSysmExit();
	return (0);
}
