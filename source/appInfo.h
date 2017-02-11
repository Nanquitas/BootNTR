#ifndef APPINFO_H
#define APPINFO_H

#include "draw.h"

#define MAX_ENTRIES     10
#define BUFFER_SIZE     100
#define DEFAULT_COLOR   0xFFF8AE2D
#define RED             0xFFFF0000
#define GREEN           0xFF00FF00


typedef enum
{
    BOLD = BIT(0),
    SKINNY = BIT(1),
    TINY = BIT(2),
    SMALL = BIT(3),
    MEDIUM = BIT(4),
    BIG = BIT(5),
    CENTER = BIT(6),
    RIGHT_ALIGN = BIT(7),
    NEWLINE = BIT(8),
    SCROLL = BIT(9)
}               appInfoEntryFlags;

typedef struct  appInfoEntry_s
{
    u32         color;
    u32         flags;
    char        buffer[BUFFER_SIZE];
}               appInfoEntry_t;

typedef struct  appInfoObject_s
{
    u32         entryCount;
    u32         maxEntryCount;
    u32         *entryList;
    cursor_t    cursor;
    float       boundX;
    float       boundY;
    sprite_t    *sprite;
    float       spritePosX;
    float       spritePosY;

}               appInfoObject_t;

appInfoObject_t     *newAppInfoObject(sprite_t *sprite, u32 maxEntryCount, u32 posX, u32 posY);
void                deleteAppInfoObject(appInfoObject_t *object);
void                appInfoSetTextBoundaries(appInfoObject_t *object, float posX, float posY);
void                appInfoSetSpritePosition(appInfoObject_t *object, float posX, float posY);
void                newAppInfoEntry(appInfoObject_t *object, u32 color, u32 flags, char *text, ...);
void                removeAppInfoEntry(appInfoObject_t *object);
void                clearAppInfo(appInfoObject_t *object, bool updateScreen);
void                drawAppInfo(appInfoObject_t *object);
void                appInfoDisableAutoUpdate(void);
void                appInfoEnableAutoUpdate(void);
void                appInfoHideBackground(void);
void                appInfoShowBackground(void);
/*
** graphic.h
*/
int    updateUI(void);
#endif