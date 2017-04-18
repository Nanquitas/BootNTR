# TARGET #

TARGET := 3DS
LIBRARY := 0

ifeq ($(TARGET),$(filter $(TARGET),3DS WIIU))
    ifeq ($(strip $(DEVKITPRO)),)
        $(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro")
    endif
endif

# COMMON CONFIGURATION #

FONZD = 0
PABLOMK7 = 0
EXTENDEDMODE = 0
DEBUG = 0

ifeq ($(EXTENDEDMODE), 1)
    ifeq ($(FONZD), 1)
	    NAME := BootNTRSelector-Mode-3-FONZD-Banner
	else
	    NAME := BootNTRSelector-Mode3-PabloMK7-Banner
	endif
else
    ifeq ($(FONZD), 1)
	    NAME := BootNTRSelector-FONZD-Banner
	else
	    NAME := BootNTRSelector-PabloMK7-Banner
	endif
endif

	
BUILD_DIR := build
OUTPUT_DIR := output
INCLUDE_DIRS := $(SOURCE_DIRS) include
SOURCE_DIRS := source source/json

EXTRA_OUTPUT_FILES :=

LIBRARY_DIRS := $(PORTLIBS) $(CTRULIB)	
LIBRARIES := citro3d ctru png z m

VERSION_MAJOR := 2
VERSION_MINOR := 7
VERSION_MICRO := 3



BUILD_FLAGS := -march=armv6k -mtune=mpcore -mfloat-abi=hard
BUILD_FLAGS_CC := -g -Wall -Wno-strict-aliasing -O3 -mword-relocations \
					-fomit-frame-pointer -ffast-math $(ARCH) $(INCLUDE) -DARM11 -D_3DS $(BUILD_FLAGS) \
					-DFONZD_BANNER=${FONZD} -DPABLOMK7_BANNER=${PABLOMK7} \
					-DAPP_VERSION_MAJOR=${VERSION_MAJOR} \
					-DAPP_VERSION_MINOR=${VERSION_MINOR} \
					-DAPP_VERSION_REVISION=${VERSION_MICRO} \
					-DEXTENDEDMODE=${EXTENDEDMODE} \
					-DDEBUGMODE=${DEBUG}

BUILD_FLAGS_CXX := $(COMMON_FLAGS) -std=gnu++11
RUN_FLAGS :=





# 3DS/Wii U CONFIGURATION #

ifeq ($(TARGET),$(filter $(TARGET),3DS WIIU))
	ifeq ($(EXTENDEDMODE), 1)
    	TITLE := Boot NTR Selector Mode 3
    else
    	TITLE := Boot NTR Selector
    endif
    DESCRIPTION := Enhanced NTR CFW Loader
    AUTHOR := Nanquitas
endif

# 3DS CONFIGURATION #

ifeq ($(TARGET),3DS)
    LIBRARY_DIRS += $(DEVKITPRO)/libctru $(DEVKITPRO)/portlibs/armv6k/
    LIBRARIES += citro3d ctru png z m

    PRODUCT_CODE := CTR-P-BNTR
    ifeq ($(EXTENDEDMODE), 1)
    	UNIQUE_ID := 0xEB300
    else
    	UNIQUE_ID := 0xEB000
    endif

    CATEGORY := Application
    USE_ON_SD := true

    MEMORY_TYPE := Application
    ifeq ($(EXTENDEDMODE), 1)
    	SYSTEM_MODE := 80MB
    else
    	SYSTEM_MODE := 64MB
    endif
    SYSTEM_MODE_EXT := Legacy
    CPU_SPEED := 268MHz
    ENABLE_L2_CACHE := true

    ICON_FLAGS := --flags visible,ratingrequired,recordusage --cero 153 --esrb 153 --usk 153 --pegigen 153 --pegiptr 153 --pegibbfc 153 --cob 153 --grb 153 --cgsrr 153

    ROMFS_DIR := romfs
    BANNER_AUDIO := resources/audio.wav
	LOGO := 
    ifeq ($(FONZD), 1)
    	BANNER_IMAGE := resources/FonzD_banner.cgfx
    else
    	BANNER_IMAGE := resources/PabloMK7_banner.cgfx
    endif
    ifeq ($(EXTENDEDMODE), 1)
    	ICON := resources/iconM3.png
    else
    	ICON := resources/icon.png
    endif
endif

# INTERNAL #

include buildtools/make_base

cleanupdater:
	@rm -f $(BUILD_DIR)/3ds-arm/source/updater.d $(BUILD_DIR)/3ds-arm/source/updater.o

re:
	@rm -rf $(BUILD_DIR)
	@echo cleaned build dir

FONZD: cleanupdater 	
	make FONZD=1

PABLOMK7: cleanupdater 
	make PABLOMK7=1

FONZDM3: cleanupdater 
	make FONZD=1 EXTENDEDMODE=1

PABLOMK7M3: cleanupdater 
	make PABLOMK7=1 EXTENDEDMODE=1