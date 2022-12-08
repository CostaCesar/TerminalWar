#pragma once
// common
#define STRING_NAME (50 + 1)
#define STRING_FILE (20 + 1)
#define STRING_DESC (200 + 1)
#define FUNCTION_FAIL -2
#define FUNCTION_SUCESS -1
#define TIME_MAP 1500
#define TIME_STRATEGY 2000

// Keys
#define KEY_ESCAPE 27
#define KEY_ENTER 13

// combat.h
#define DAMAGE_SMALL 1
#define DAMAGE_MEDIUM 3
#define DAMAGE_LARGE 5

#define RESULT_CAP 7.0
#define OFFSET 20.0
#define MORALE_WIN 1.35
#define MORALE_RANGED 1.4
#define MORALE_LOOSE 1.20


// unit.h
#define BUFFS_IMPLEMENTED 10
#define UNIT_MAX_BUFFS 10
#define UNIT_MAX_LEVEL 5
#define UNIT_MAX_MORALE 120.0f

// UI.h
#define MAP_OFFSET_X 6
#define MAP_OFFSET_Y 10

// map.h
#define BACKUP_MAP_COLUMNS 10
#define BACKUP_MAP_ROWS 10
#define HEIGHT_DIF 2
#define MAX_FORT_LEVEL 10
typedef enum E_Terrain
{
    Grass,
    Sand,
    Rock,
    Mud,
    Water,
    Snow
} T_Terrain;

typedef enum E_Vegetation
{
    None,
    Field,
    Sparse,
    Grove,
    Forest,
    Jungle
} T_Veget;

#define NO_UNIT -1
#define OUT_COMBAT -3
#define MODE_HEIGHT 0
#define MODE_TERRAIN 1
#define MODE_VEGETAT 2
#define MODE_GRAPHIC 3
#define MODE_UNITS 4
