#include "graphics.h"
#include "drawableObject.h"
#include "button.h"

sprite_t	     *bottomSprite;
sprite_t	     *topSprite;

static sprite_t	        *botStatusSprite;
static sprite_t         *topInfoSprite;
static drawableScreen_t *botScreen;
static drawableScreen_t *topScreen;
appInfoObject_t         *appStatus;
appInfoObject_t         *appTop;

static char      appVersion[20];


void    initUI(void)
{
    backgroundScreen_t *bg;

    newSpriteFromPNG(&topSprite, "romfs:/sprites/topBackground.png");
    newSpriteFromPNG(&bottomSprite, "romfs:/sprites/bottomBackground.png");    
    newSpriteFromPNG(&botStatusSprite, "romfs:/sprites/statusBackground.png");
    newSpriteFromPNG(&topInfoSprite, "romfs:/sprites/topInfoBackground.png");

    setSpritePos(topSprite, 0, 0);
    setSpritePos(bottomSprite, 0, 0);
    
    bg = newBackgroundObject(bottomSprite, NULL, NULL);
    botScreen = newDrawableScreen(bg);
    bg = newBackgroundObject(topSprite, NULL, NULL);
    topScreen = newDrawableScreen(bg);

    setSpritePos(botStatusSprite, 0, 0);
    appStatus = newAppInfoObject(botStatusSprite, 10, 177.0f, 64.0f);
    appInfoSetTextBoundaries(appStatus, 297.0f, 190.0f);

    setSpritePos(topInfoSprite, 50, 20);
    appTop = newAppInfoObject(topInfoSprite, 14, 55.0f, 30.0f);
    appInfoSetTextBoundaries(appTop, 345.0f, 210.0f);

    if (!APP_VERSION_REVISION)
        sprintf(appVersion, "Version: %d.%d", APP_VERSION_MAJOR, APP_VERSION_MINOR);
    else
        sprintf(appVersion, "Version: %d.%d.%d", APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_REVISION);
    updateUI();
}

void    exitUI(void)
{
    drawEndFrame();
    deleteAppInfoObject(appTop);
    deleteAppInfoObject(appStatus);
    deleteSprite(bottomSprite);
    deleteSprite(topSprite);
    deleteSprite(botStatusSprite);
    deleteSprite(topInfoSprite);
}

static inline void drawUITop(void)
{
    setScreen(GFX_TOP);
    
    topScreen->draw(topScreen);

    setTextColor(COLOR_BLANK);
    renderText(1.0f, 1.0f, 0.4f, 0.45f, false, appVersion, NULL);
    drawAppInfo(appTop);
}

static inline void drawUIBottom(void)
{
    setScreen(GFX_BOTTOM);
    
    botScreen->draw(botScreen);
    drawAppInfo(appStatus);
}

int   updateUI(void)
{
    hidScanInput();
    drawUITop();
    drawUIBottom();
    updateScreen();
    return (1);
}

void    addTopObject(void *object)
{
    addObjectToScreen(topScreen, object);
}

void    addBottomObject(void *object)
{
    addObjectToScreen(botScreen, object);
}

void    changeTopFooter(sprite_t *footer)
{
    changeBackgroundFooter(topScreen->background, footer);
}

void    changeTopHeader(sprite_t *header)
{
    changeBackgroundHeader(topScreen->background, header);
}

void    changeBottomFooter(sprite_t *footer)
{
    changeBackgroundFooter(botScreen->background, footer);
}

void    changeBottomHeader(sprite_t *header)
{
    changeBackgroundHeader(botScreen->background, header);
}

void    clearTopScreen(void)
{
    clearObjectListFromScreen(topScreen);
}

void    clearBottomScreen(void)
{
    clearObjectListFromScreen(botScreen);
}