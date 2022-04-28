/*****************************************
		NanoShell Operating System
	      (C) 2022 iProgramInCpp

      Icon listing module headerfile
******************************************/
#ifndef _ICON_H
#define _ICON_H

#include <main.h>
#include <video.h>

//WORK: Add in your icons here in this enum:
enum 
{
	ICON_NULL,
	ICON_CABINET,
	ICON_CHIP,
	ICON_CHIP_SQ,
	ICON_COMPUTER,
	ICON_COMPUTER_SHUTDOWN,
	ICON_DESKTOP,
	ICON_DRAW,
	ICON_EARTH,
	ICON_ERROR,
	ICON_EXECUTE_FILE,
	ICON_FILE,
	ICON_FILES,
	ICON_FOLDER,
	ICON_FOLDER_BLANK,
	ICON_FOLDER_MOVE,
	ICON_FOLDER_PARENT,
	ICON_FOLDER16_CLOSED,
	ICON_FOLDER16_OPEN,
	ICON_GLOBE,
	ICON_GO,
	ICON_HAND,
	ICON_HELP,
	ICON_INFO,
	ICON_KEYBOARD,
	ICON_KEYBOARD2,
	ICON_LAPTOP,
	ICON_NOTES,
	ICON_PAINT,
	ICON_SERIAL,
	ICON_STOP,
	ICON_TEXT_FILE,
	ICON_WARNING,
	ICON_NANOSHELL_LETTERS,
	ICON_NANOSHELL_LETTERS16,
	ICON_NANOSHELL,
	ICON_NANOSHELL16,
	ICON_BOMB,
	ICON_BOMB_SPIKEY,
	ICON_FILE16,
	ICON_TEXT_FILE16,
	ICON_EXECUTE_FILE16,
	ICON_FOLDER_PARENT16,
	//icons V1.1
	ICON_FOLDER_SETTINGS,
	ICON_CABINET16,
	ICON_COMPUTER16,
	ICON_COMMAND,
	ICON_COMMAND16,
	ICON_ERROR16,
	//icons V1.2
	ICON_LOCK,
	ICON_DIRECTIONS,
	ICON_CERTIFICATE,
	ICON_FILE_WRITE,
	ICON_SCRAP_FILE,
	ICON_SCRAP_FILE16,
	ICON_RESMON,
	ICON_BILLBOARD,
	ICON_FILE_CSCRIPT,
	ICON_FILE_CSCRIPT16,
	ICON_FILE_CLICK,
	ICON_KEYS,
	ICON_RESTRICTED,
	ICON_HOME,
	ICON_HOME16,
	ICON_ADAPTER,
	ICON_CLOCK,
	ICON_CLOCK16,
	//icons V1.3
	ICON_APPLICATION,
	ICON_APPLICATION16,
	ICON_TASKBAR,
	ICON_APP_DEMO,
	ICON_COMPUTER_FLAT,
	ICON_CALCULATOR,
	ICON_CALCULATOR16,
	ICON_DESKTOP2,
	ICON_MOUSE,
    //Icons V1.31
	ICON_AMBULANCE,
	//icons V1.32
	ICON_FONTS,
	ICON_FONTS16,
	//icons V1.33
	ICON_RESMON16,
	ICON_NOTES16,
	ICON_FILE_NANO,
	//icons V1.34
	ICON_CLOCK_EMPTY,//Special case which draws more stuff
	//icons V1.35
	ICON_RUN,
	ICON_RUN16,
	//icons V1.4
	ICON_DEVTOOL,
	ICON_DEVTOOL_FILE,
	ICON_HEX_EDIT,
	ICON_CHAIN,
	ICON_CHAIN16,
	ICON_DEVTOOL16,
	ICON_TODO,
	ICON_FOLDER_DOCU,
	ICON_DLGEDIT,
	ICON_DESK_SETTINGS,
	ICON_SHUTDOWN,
	ICON_NOTEPAD,
	ICON_FILE_MKDOWN,
	ICON_FILE_MKDOWN16,
	ICON_COMPUTER_PANIC,
	ICON_EXPERIMENT,
	ICON_GRAPH,
	ICON_CABINET_COMBINE,
	ICON_REMOTE,
	ICON_CABINET_OLD,
	//icons V1.5
	ICON_DEVICE_CHAR,
	ICON_DEVICE_BLOCK,
	ICON_HARD_DRIVE,
	ICON_HARD_DRIVE_MOUNT,
	ICON_WINDOW,
	ICON_WINDOW_SNAP,
	ICON_WINDOW_OVERLAP,
	ICON_SWEEP_SMILE,
	ICON_SWEEP_CLICK,
	ICON_SWEEP_DEAD,
	ICON_SWEEP_CARET,
	ICON_DLGEDIT16,
	ICON_BOMB_SPIKEY16,
	ICON_MAGNIFY,
	ICON_MAGNIFY16,
	ICON_TAR_ARCHIVE,
	ICON_SYSMON,
	ICON_SYSMON16,
	ICON_COMPUTER_SHUTDOWN16,
	ICON_EXIT,
	ICON_KEY,
	ICON_KEYB_REP_SPEED,
	ICON_KEYB_REP_DELAY,
	ICON_MONITOR,
	//icons V1.6
	ICON_FILE_INI,
	ICON_WMENU,
	ICON_WMENU16,
	ICON_FILE_IMAGE,
	ICON_FILE_IMAGE16,
	ICON_FILE_LOG,
	ICON_STICKY_NOTES,
	ICON_STICKY_NOTES16,
	ICON_NOTE_YELLOW,
	ICON_NOTE_BLUE,
	ICON_NOTE_GREEN,
	ICON_NOTE_WHITE,
	ICON_FOLDER_OPEN,
	//icons V1.61
	ICON_EXPERIMENT2,
	ICON_FLOPPY, ICON_ACTION_SAVE = ICON_FLOPPY,
	ICON_ACTION_SAVE16,
	ICON_ACTION_OPEN,
	ICON_ACTION_OPEN16,
	ICON_PLUS,
	ICON_COUNT
};

typedef int IconType;

/**
 * Gets an icon image.
 */
Image* GetIconImage(IconType type, int sz);

/**
 * Renders an icon to the screen.
 */
void RenderIcon(IconType type, int x, int y);
void RenderIconOutline(IconType type, int x, int y, uint32_t outline);

/**
 * Renders an icon to the screen, forcing it to be a certain size.
 */
void RenderIconForceSize(IconType type, int x, int y, int size);
void RenderIconForceSizeOutline(IconType type, int x, int y, int size, uint32_t color);

#endif//_ICON_H