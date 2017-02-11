#include "config.h"
#include "button.h"
#include "graphics.h"
#include "drawableObject.h"

extern bootNtrConfig_t  *bnConfig;
static const char       *rootPath = "sdmc:/";
static const char       *hblPath = "sdmc:/3ds/ntr/";
static bool             pathError = false;
static char             *p_globalPath;
static char             *p_pluginPath;

static text_t           *pluginPathText;
static text_t           *binPathText;
static sprite_t         *pressBackSprite;
static sprite_t         *desiredChoiceSprite;
static sprite_t         *buttonBackground;

static button_t         *binariesPathButton;
static button_t         *pluginsPathButton;
static button_t         *createCustomButton;
static button_t         *useDefaultButton;
static button_t         *useSecondButton;
static button_t         *saveSettingButton;
static button_t         *okButton;

static sprite_t         *mainWindow;
static sprite_t         *customWindow;
static sprite_t         *defaultWindow;
static sprite_t         *secondWindow;
static sprite_t         *warningWindow;
static window_t         *window;

static u32              g_status;

enum
{
   e_MAIN = BIT(0),
   e_EXIT = BIT(1),
   e_DEFAULT = BIT(2),
   e_SECOND = BIT(3),
   e_CUSTOM = BIT(4),
   e_PLUGIN = BIT(5),
   e_BINARIES = BIT(6)
};

void    changeStatus(u32 window)
{
    switch (window)
    {
    case 1:
        g_status |= e_DEFAULT;
        break;
    case 2:
        g_status |= e_SECOND;
        break;
    case 3:
        g_status |= e_CUSTOM;
        break;
    case 4:
        g_status |= e_PLUGIN;
        break;
    case 5:
        g_status |= e_BINARIES;
        break;
    case 6:
        g_status |= e_EXIT;
        break;
    default:
        break;
    }
}

void    initSettingsMenu(void)
{
    sprite_t    *sprite;
    clearTopScreen();
    clearBottomScreen();
    newSpriteFromPNG(&pressBackSprite, "romfs:/sprites/textSprites/pressBBack.png");
    newSpriteFromPNG(&desiredChoiceSprite, "romfs:/sprites/textSprites/touchDesiredChoice.png");
    setSpritePos(desiredChoiceSprite, 34.0f, 6.0f);
    setSpritePos(pressBackSprite, 180.0f, 216.0f);    
    changeBottomFooter(pressBackSprite);
    changeBottomHeader(desiredChoiceSprite);

    newSpriteFromPNG(&buttonBackground, "romfs:/sprites/largeButtonBackground.png");

    newSpriteFromPNG(&sprite, "romfs:/sprites/textSprites/binariesPath.png");
    binariesPathButton = newButton(48.0f, 34.0f, changeStatus, 5, buttonBackground, sprite);

    newSpriteFromPNG(&sprite, "romfs:/sprites/textSprites/pluginsPath.png");
    pluginsPathButton = newButton(48.0f, 88.0f, changeStatus, 4, buttonBackground, sprite);

    newSpriteFromPNG(&sprite, "romfs:/sprites/textSprites/saveSettings.png");
    saveSettingButton = newButton(48.0f, 88.0f, changeStatus, 6, buttonBackground, sprite);

    newSpriteFromPNG(&sprite, "romfs:/sprites/textSprites/useDefault.png");
    useDefaultButton = newButton(48.0f, 34.0f, changeStatus, 1, buttonBackground, sprite);

    newSpriteFromPNG(&sprite, "romfs:/sprites/textSprites/useSettings2.png");
    useSecondButton = newButton(48.0f, 88.0f, changeStatus, 2, buttonBackground, sprite);

    newSpriteFromPNG(&sprite, "romfs:/sprites/textSprites/createCustom.png");
    createCustomButton = newButton(48.0f, 142.0f, changeStatus, 3, buttonBackground, sprite);

    newSpriteFromPNG(&sprite, "romfs:/sprites/textSprites/ok.png");
    okButton = newButton(48.0f, 88.0f, changeStatus, 6, buttonBackground, sprite);

    addBottomObject(binariesPathButton);
    addBottomObject(pluginsPathButton);
    addBottomObject(saveSettingButton);
    addBottomObject(useDefaultButton);
    addBottomObject(useSecondButton);
    addBottomObject(createCustomButton);
    addBottomObject(okButton);

    newSpriteFromPNG(&mainWindow, "romfs:/sprites/textSprites/mainSettingsWindow.png");
    newSpriteFromPNG(&customWindow, "romfs:/sprites/textSprites/customSettingsWindow.png");
    newSpriteFromPNG(&defaultWindow, "romfs:/sprites/textSprites/defaultSettingsWindow.png");
    newSpriteFromPNG(&secondWindow, "romfs:/sprites/textSprites/secondSettingsWindow.png");
    newSpriteFromPNG(&warningWindow, "romfs:/sprites/textSprites/pluginFolderWarning.png");

    setSpritePos(mainWindow, 46.0f, 65.0f);
    setSpritePos(customWindow, 46.0f, 65.0f);
    setSpritePos(defaultWindow, 46.0f, 65.0f);
    setSpritePos(secondWindow, 46.0f, 65.0f);
    setSpritePos(warningWindow, 46.0f, 65.0f);

    newSpriteFromPNG(&sprite, "romfs:/sprites/menuBackground.png");
    setSpritePos(sprite, 43.0f, 20.0f);
    window = newWindow(sprite, sprite, NULL);

    newSpriteFromPNG(&sprite, "romfs:/sprites/textSprites/pathSettingsTitle.png");
    setSpritePos(sprite, 111.0f, 35.0f);
    changeWindowTitle(window, sprite);
    addTopObject(window);
    pluginPathText = newText(NULL, 158, 123, 0.45f, 0.5f, COLOR_BLANK);
    binPathText = newText(NULL, 158, 109, 0.45f, 0.5f, COLOR_BLANK);
    addTopObject(pluginPathText);
    addTopObject(binPathText);
}

void    exitSettingsMenu(void)
{
    clearBottomScreen();
    clearTopScreen();
    destroyButton(binariesPathButton);
    destroyButton(pluginsPathButton);
    destroyButton(createCustomButton);
    destroyButton(useDefaultButton);
    destroyButton(useSecondButton);
    destroyButton(saveSettingButton);
    deleteSprite(buttonBackground);
    deleteSprite(pressBackSprite);
    deleteSprite(desiredChoiceSprite);
    deleteSprite(window->title);
    deleteSprite(window->background);
    deleteSprite(mainWindow);
    deleteSprite(customWindow);
    deleteSprite(defaultWindow);
    deleteSprite(secondWindow);
    deleteSprite(warningWindow);
    free(window);
    free(pluginPathText);
    free(binPathText);
}

static void showWarning(void)
{
    waitAllKeysReleased();
    changeWindowContent(window, warningWindow);
    okButton->show(okButton);
    changeBottomFooter(NULL);
    g_status = 0;
    while (!g_status)
        updateUI();
}


static void customSettings(void)
{
    u32             keys;
    bool            ret;
    u32             status;
    char            pluginPath[0x100] = "sdmc:/plugin";
    char            globalPath[0x100] = "sdmc:/";
    static char     *hintTextGlobal = "Enter the path for all NTR Files";
    static char     *hintTextPlugin = "Enter the path for the plugin folder";

    changeWindowContent(window, customWindow);
    useDefaultButton->hide(useDefaultButton);
    useSecondButton->hide(useSecondButton);
    createCustomButton->hide(createCustomButton);
    binariesPathButton->show(binariesPathButton);
    pluginsPathButton->show(pluginsPathButton);
    moveButton(saveSettingButton, 48.0f, 142.0f);
    saveSettingButton->show(saveSettingButton);
    changeBottomFooter(pressBackSprite);
    showText(pluginPathText);
    showText(binPathText);
    pluginPathText->str = pluginPath;
    binPathText->str = globalPath;
    strcpy(p_globalPath, rootPath);
    strJoin(p_pluginPath, rootPath, "plugin/");
again:
    waitAllKeysReleased();
    status = g_status = 0;
    while (!status)
    {
        status = g_status;
        updateUI();
        hidScanInput();
        keys = hidKeysDown() | hidKeysHeld();
        if (status & e_BINARIES)
        {
            ret = inputPathKeyboard(globalPath, hintTextGlobal, globalPath, 0x100);
            if (ret)
                strcpy(p_globalPath, globalPath);
            goto again;
        }
        else if (status & e_PLUGIN)
        {
            ret = inputPathKeyboard(pluginPath, hintTextPlugin, pluginPath, 0x100);
            if (ret)
                strcpy(p_pluginPath, pluginPath);
            goto again;
        }
        else if (status & e_EXIT)
        {
            if (checkPath())
            {
                saveSettingButton->hide(saveSettingButton);
                binariesPathButton->hide(binariesPathButton);
                pluginsPathButton->hide(pluginsPathButton);
                hideText(pluginPathText);
                hideText(binPathText);
                if (strncmp("sdmc:/plugin", p_pluginPath, 12))
                    showWarning();
                return;
            }                
            goto again;
        }
        else if (keys & KEY_B)
        {
            pathError = false;
            saveSettingButton->hide(saveSettingButton);
            moveButton(saveSettingButton, 48.0f, 88.0f);
            binariesPathButton->hide(binariesPathButton);
            pluginsPathButton->hide(pluginsPathButton);
            g_status |= e_MAIN;
            hideText(pluginPathText);
            hideText(binPathText);
            return;
        }
    }
}

static void defaultSettings(void)
{
    u32 keys;

    waitAllKeysReleased();
    changeWindowContent(window, defaultWindow);
    useDefaultButton->hide(useDefaultButton);
    useSecondButton->hide(useSecondButton);
    createCustomButton->hide(createCustomButton);
    saveSettingButton->show(saveSettingButton);
    changeBottomFooter(pressBackSprite);
    strcpy(p_globalPath, hblPath);
    strJoin(p_pluginPath, rootPath, "plugin/");
    g_status = 0;
    while (!g_status)
    {
        updateUI();
        hidScanInput();
        keys = hidKeysDown() | hidKeysHeld();
        if (keys & KEY_B)
        {
            saveSettingButton->hide(saveSettingButton);
            g_status |= e_MAIN;
            break;
        }
    }
}

void secondSettings(void)
{
    u32 keys;

    waitAllKeysReleased();
    changeWindowContent(window, secondWindow);
    useDefaultButton->hide(useDefaultButton);
    useSecondButton->hide(useSecondButton);
    createCustomButton->hide(createCustomButton);
    saveSettingButton->show(saveSettingButton);
    changeBottomFooter(pressBackSprite);

    strcpy(p_globalPath, hblPath);
    strJoin(p_pluginPath, hblPath, "plugin/");
    g_status = 0;
    while (!g_status)
    {
        updateUI();
        hidScanInput();
        keys = hidKeysDown() | hidKeysHeld();
        if (keys & KEY_B)
        {
            g_status |= e_MAIN;
            break;
        }
    }
    saveSettingButton->hide(saveSettingButton);
    if (!(g_status & e_MAIN))
        showWarning();
}

static void    setFiles(void)
{
    int     ret;
    if (!fileExists(bnConfig->config->binariesPath + 5))
    {
        newAppTop(COLOR_BLANK, SKINNY, "%s, doesn't exist", bnConfig->config->binariesPath);
        newAppTop(COLOR_BLANK, SKINNY, "Creating directory:");
        updateUI();
        ret = createDir(bnConfig->config->binariesPath + 5);
        removeAppTop();
        if (ret)
            newAppTop(COLOR_SALMON, SKINNY, "Creating directory: Failed");
        else
            newAppTop(COLOR_LIMEGREEN, SKINNY, "Creating directory: Success");
        updateUI();
    }        
    if (!fileExists(bnConfig->config->pluginPath + 5))
    {
        newAppTop(COLOR_BLANK, SKINNY, "%s, doesn't exist", bnConfig->config->pluginPath);
        newAppTop(COLOR_BLANK, SKINNY, "Creating directory:");
        updateUI();
        ret = createDir(bnConfig->config->pluginPath + 5);
        removeAppTop();
        if (ret)
            newAppTop(COLOR_SALMON, SKINNY, "Creating directory: Failed");
        else
            newAppTop(COLOR_LIMEGREEN, SKINNY, "Creating directory: Success");
        updateUI();
    }
        
    newAppTop(COLOR_BLANK, NEWLINE | SKINNY, "Setting up files...");

    newAppTop(COLOR_BLANK, SKINNY, "Setting up 3.2...");
    updateUI();
    ret = loadAndPatch(V32);
    removeAppTop();
    if (ret)
        newAppTop(COLOR_SALMON, SKINNY, "Setting up 3.2... Error.");
    else
        newAppTop(COLOR_LIMEGREEN, SKINNY, "Setting up 3.2... Done.");
    updateUI();

    newAppTop(COLOR_BLANK, SKINNY, "Setting up 3.3...");
    updateUI();
    ret = loadAndPatch(V33);
    removeAppTop();
    if (ret)
        newAppTop(COLOR_SALMON, SKINNY, "Setting up 3.3... Error.");
    else
        newAppTop(COLOR_LIMEGREEN, SKINNY, "Setting up 3.3... Done.");
    updateUI();

    newAppTop(COLOR_BLANK, SKINNY, "Setting up 3.4...");
    updateUI();
    ret = loadAndPatch(V34);
    removeAppTop();
    if (ret)
        newAppTop(COLOR_SALMON, SKINNY, "Setting up 3.4... Error.");
    else
        newAppTop(COLOR_LIMEGREEN, SKINNY, "Setting up 3.4... Done.");
    updateUI();

    newAppTop(COLOR_LIMEGREEN, 0, "Finished");
    updateUI();
    clearTop(1);
    wait(3);
}

void firstLaunch(void)
{ 
    u32         status;

    updateUI();
    initSettingsMenu();
    appInfoDisableAutoUpdate();
    p_globalPath = bnConfig->config->binariesPath;
    p_pluginPath = bnConfig->config->pluginPath;
again:
    waitAllKeysReleased();
    status = g_status = 0;
    changeWindowContent(window, mainWindow);
    useDefaultButton->show(useDefaultButton);
    useSecondButton->show(useSecondButton);
    createCustomButton->show(createCustomButton);
    changeBottomFooter(NULL);
    updateUI();
    while (!(status & e_EXIT))
    {
        updateUI();
        if (status & e_MAIN) goto again;
        else if (status & e_DEFAULT) defaultSettings();
        else if (status & e_SECOND) secondSettings();
        else if (status & e_CUSTOM) customSettings();
        status = g_status;
    }
    setFiles();
    exitSettingsMenu();
    appInfoEnableAutoUpdate();
    updateUI();
}

