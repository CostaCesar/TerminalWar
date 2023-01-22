// #include <stdio.h>
// #include <stdlib.h>
// #include <stdbool.h>
// #include <math.h>
// #include <Windows.h>
#include <time.h>
#include "include/map.h"
#include "include/combat.h"
#include "include/UI.h"
#include "include/AI.h"

#define MAP_DELAY (2 * 1000)
#define TURNS 100
#define VERSION 1.75f

// Game Variables
B_Map battleMap;
B_Side Side_A, Side_B;
B_Unit *unit_Table = NULL;
int unit_TableSize = 0;
int screenHalf = 0;
char unit_Desc[STRING_FILE];
char nowPlaying[STRING_NAME] = {0};

// File->Game functions
B_Unit *getFile_Unit(char *path, int *size)
{
    char rPath[STRING_FILE+STRING_NAME] = "units/";
    strcat(rPath, path);
    FILE *file = fopen(rPath, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "ERROR: could not open file <%s>! \n", path);
        return NULL;
    }

    fread(size, sizeof(int), 1, file);
    B_Unit *output = calloc(*size, sizeof(B_Unit));
    fread(output, sizeof(B_Unit), *size, file);
    fread(unit_Desc, sizeof(char), STRING_FILE, file);

    fclose(file);
    return output;
}

int getFile_Map(char *fPath, B_Map *map)
{
    char rPath[STRING_FILE+STRING_NAME] = "maps/";
    strcat(rPath, fPath);

    FILE *file = fopen(rPath, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "ERROR: File could not be opened! \n");
        return FUNCTION_FAIL;
    }

    fread(&map->width, sizeof(int), 1, file);
    fread(&map->height, sizeof(int), 1, file);
    fread(&map->climate, sizeof(T_Clime), 1, file);
    fread(&map->time, sizeof(T_Time), 1, file);
    fread(&map->name, sizeof(char), STRING_NAME, file);

    map->tiles = (B_Tile **)calloc(map->height, sizeof(B_Tile *));
    if (map->tiles == NULL)
    {
        free(map->tiles);
        map == NULL;
        fprintf(stderr, "ERROR: Not enough memory! \n");
        return FUNCTION_FAIL;
    }
    for (int i = 0; i < map->height; i++)
    {
        map->tiles[i] = (B_Tile *)calloc(map->width, sizeof(B_Tile));
        fread(map->tiles[i], sizeof(B_Tile), map->width, file);
    }

    fclose(file);
    return FUNCTION_SUCESS;
}

void set_fUnitTable(B_Unit *table, int index, B_Side *side)
{
    int pos = side->size;
    side->units = alloc_Unit(side->units, &side->size);

    side->units[pos] = table[index];
    side->units[pos].faction = side->name;
    side->units[pos].Game_ID = pos * 2 + side->ID;
    return;
}

// Map<->Unit functions
void def_Unit(B_Unit *unit, B_Map *map, bool ignoreSpawn)
{
    if(ignoreSpawn == true && unit->position.X > NO_UNIT && unit->position.Y > NO_UNIT)
        return;
    for (int i = 0; i < map->height; i++)
        for (int j = 0; j < map->width; j++)
        {
            if ((map->tiles[i][j].spawn > 0 && unit->Game_ID % 2 == 0) || (map->tiles[i][j].spawn < 0 && unit->Game_ID % 2 == 1))
            {
                if (map->tiles[i][j].unit == NULL)
                {
                    unit->position.X = j;
                    unit->position.Y = i;
                    return;
                }
            }
        }
    return;
}

// Main functions (screens & logic)
int deallocAll()
{
    for (int i = 0; i < battleMap.height; i++)
    {
        if(battleMap.tiles[i])
            free(battleMap.tiles[i]);
    }
    if(battleMap.tiles) free(battleMap.tiles);
    if(Side_A.units) free(Side_A.units);
    if(Side_B.units) free(Side_B.units);
    if(unit_Table) free(unit_Table);
    Side_A.size = 0, Side_B.size = 0, Side_A.isAI = false, Side_B.isAI = true, unit_TableSize = 0;
    return FUNCTION_SUCESS;
}

// Centralization of the combat process
void meleeCombat(B_Side* attackerSide, B_Unit *attacker, B_Side *defenderSide, B_Unit* defender)
{
    // Show
    update_Map(attacker->position.X, attacker->position.Y, "XXX");
    Sleep(TIME_MAP);
    update_Map(defender->position.X, defender->position.Y, "OOO");
    Sleep(TIME_MAP);
    // Glow
    char msg[STRING_NAME];
    snprintf(msg, sizeof(msg), "%s [%s] at %hdX %2hdY", attacker->name, attacker->faction, attacker->position.X, attacker->position.Y);
    print_Message(msg, screenHalf, 1, true, false, false);
    print_Message("Is engaging in melee combat with", screenHalf, 2, false, false, false);
    snprintf(msg, sizeof(msg), "%s [%s] at %hdX %2hdY", defender->name, defender->faction, defender->position.X, defender->position.Y);
    print_Message(msg, screenHalf, 3, false, false, true);
  
    // Go
    system("cls");
    // Unit cannot be already engaged at combat
    if(attacker->engaged == true)
    {
        // printf("#===============================================# \n");
        // printf("| We must disengage the enemy before attacking! | \n");
        // printf("#===============================================# \n"); */
        print_Message("We must disengage the enemy before attacking!", screenHalf, 1, true, false, true);
        return;
    }
    
    // Obtaining data
    int heightDif = get_HeightDif(&battleMap, attacker->position, defender->position);
    short int* fortLvl = &battleMap.tiles[defender->position.Y][defender->position.X].fortLevel;
    T_Terrain terrain = battleMap.tiles[defender->position.Y][defender->position.X].terrain;
    T_Veget vegetation = battleMap.tiles[defender->position.Y][defender->position.X].vegetation;
    
    // Trivia
    attacker->goal = (B_Pos){-1, -1};
    defender->goal = (B_Pos){-1, -1};
    attacker->attacked = true;
    attacker->engaged = true;
    defender->engaged = true;

    // Combat
    B_Result result = execute_MeleeCombat(attacker, defender, heightDif, fortLvl, terrain, vegetation);
    attackerSide->stats.killed += result.defenderLoss;
    defenderSide->stats.killed += result.attackerLoss;
    defenderSide->stats.loss += result.defenderLoss;
    attackerSide->stats.loss += result.attackerLoss;

    // Level up
    if(show_Combat(&result) == true)
        result.winner->level++; 
    
    // Cleanup
    B_Pos target = attacker->position;
    if(defender->men <= 0)
    {
        target = defender->position;
        delete_Unit(defenderSide->units, &defenderSide->size, get_UnitIndex(defenderSide, defender->Game_ID));
        battleMap.tiles[target.Y][target.X].unit = NULL;
    }
    if(attacker->men <= 0)
    {
        delete_Unit(attackerSide->units, &attackerSide->size, get_UnitIndex(attackerSide, attacker->Game_ID));
        battleMap.tiles[target.Y][target.X].unit = NULL;
    }
    else // Whoever wins gains the innitiative (can disengage/engage)
    { result.winner->engaged = false; }

    return;
}

int rangedCombat(B_Side* attackerSide, B_Unit *attacker, B_Side *defenderSide, B_Pos target)
{
    char msg[STRING_NAME];
    // Testing 
    if(check_MapAttack(&battleMap, target, attacker->Game_ID) == FUNCTION_FAIL)
        return FUNCTION_FAIL;
    B_Unit *defender = &defenderSide->units[get_UnitIndex(defenderSide, battleMap.tiles[target.Y][target.X].unit->Game_ID)];
    if(check_Ranged(attacker, defender) == FUNCTION_FAIL)
        return FUNCTION_FAIL; 
    
    // Show
    update_Map(attacker->position.X, attacker->position.Y, "XXX");
    Sleep(TIME_MAP);
    update_Map(target.X, target.Y, "OOO");
    Sleep(TIME_MAP);
    
    // Signaling
    snprintf(msg, sizeof(msg), "%s [%s] at %hdX %2hdY", attacker->name, attacker->faction, attacker->position.X, attacker->position.Y);
    print_Message(msg, screenHalf, 1, true, false, false);
    print_Message("Is firing a volley into", screenHalf, 2, false, false, false);
    snprintf(msg, sizeof(msg), "%s [%s] at %hdX %2hdY", defender->name, defender->faction, defender->position.X, defender->position.Y);
    print_Message(msg, screenHalf, 3, false, false, true);

    // Go
    system("cls");
    int tVeget = getTile_Vegetat(&battleMap, target), tHeight = get_HeightDif(&battleMap, attacker->position, target);
    int tTerrain = getTile_Terrain(&battleMap, target);
    short int *tFort = &battleMap.tiles[target.Y][target.X].fortLevel;
    B_Result res = execute_RangedCombat(attacker, defender, tHeight, tVeget, tTerrain, tFort);
    attackerSide->stats.killed += res.defenderLoss;
    defenderSide->stats.loss += res.defenderLoss;

    // Trivia
    attacker->goal = (B_Pos) {-1, -1};
    defender->goal = (B_Pos) {-1, -1};
    attacker->chaseID = NULL, defender->chaseID = NULL;
    target = defender->position;
    
    // Showing results
    if(show_Combat(&res) == true)
        attacker->level++;
    
    // Done
    show_gUnit(attacker);
    show_gUnit(defender);
    printf(">> Press ENTER to continue ");
    while (get_KeyPress(false) != KEY_ENTER) continue;
    system("cls");
    
    // Cleanup
    if(defender->men == 0)
    {
        delete_Unit(defenderSide->units, &defenderSide->size, get_UnitIndex(defenderSide, defender->Game_ID));
        battleMap.tiles[target.Y][target.X].unit = NULL;
    }
        
    return FUNCTION_SUCESS;
}

int load_Scenery(int nScen, int playMap)
{
    char file[STRING_NAME+STRING_FILE] = "scenarios/";
    int output = 0;
    
    // Getting file name
    WIN32_FIND_DATA fd;
    HANDLE handle = FindFirstFile("scenarios/*.txt", &fd);
    if(handle == INVALID_HANDLE_VALUE)
    {
        print_Message("Can't find scenarios folder!", 0, 1, true, false, true);
        return FUNCTION_FAIL;    
    }
    for(int i = 0; i < nScen; i++)
    {
        if(i == nScen - 1) strcat(file, fd.cFileName);
        FindNextFile(handle, &fd);
    }
    FindClose(handle);

    // Reading File
    char word[STRING_DESC] = {0}, scen_Name[STRING_NAME] = {0}, scen_Map[STRING_NAME] = {0};
    int cMap = 0, strBuff = 0, unit_N = NO_UNIT;
    FILE *scen = fopen(file, "r");
    if(!scen)
    {
        print_Message("Can't open file!", 0, 1, true, false, true);
        return FUNCTION_FAIL;
    }
    while(!feof(scen))
    {
        // Get Scenario Name
        strBuff = strlen("scenario: ");
        fgets(scen_Name, sizeof(scen_Name), scen);
        scen_Name[strlen(scen_Name)-1] = '\0';
        if(strlen(scen_Name) < strBuff)
        {
            print_Message("Invalid Scenario Title!", 0, 1, true, false, true);
            output = FUNCTION_FAIL; break;
        }
        strcpy(scen_Name, scen_Name+strBuff);

        // Traverse the file in search of chosen map
        while(cMap < playMap)
        {
            if(fgets(word, sizeof(word), scen) == NULL) // Not in file
            {
                print_Message("Can't find map!", 0, 1, true, false, true);
                output = FUNCTION_FAIL; break;
            }
            else if(word[0] == '$') cMap++;              // Increment map counter
        }

        // Get Map
        strBuff = strlen("map: ");
        fgets(word, sizeof(word), scen);
        word[strlen(word)-1] = '\0';
        if(strlen(word) < strBuff)
        {
            print_Message("The map can't be read!", 0, 1, true, false, true);
            output = FUNCTION_FAIL; break; 
        }
        if(getFile_Map(word+strBuff, &battleMap) == FUNCTION_FAIL){ output = FUNCTION_FAIL; break; }
        // Get Map Name (for player)
        strBuff = strlen("mapTitle: ");
        fgets(scen_Map, sizeof(scen_Map), scen);
        scen_Map[strlen(scen_Map)-1] = '\0';
        if(strlen(scen_Map) < strBuff)
        {
            print_Message("Can't load map title!", 0, 1, true, false, true);
            output = FUNCTION_FAIL; break; 
        }
        strcpy(scen_Map, scen_Map+strBuff);
        // Try to get Music
        strBuff = strlen("music: ");
        fgets(word, sizeof(word), scen);
        word[strlen(word)-1] = '\0';
        if(strlen(word) < strBuff)
        {
            print_Message("Can't load the units!", 0, 1, true, false, true);
            output = FUNCTION_FAIL; break; 
        }
        if(word[4] == 'c' && word[0] == 'm')    // Music detected
        {
            strcpy(nowPlaying, word+7);
            fgets(word, sizeof(word), scen);
            word[strlen(word)-1] = '\0';
            if(strlen(word) < strBuff)
            {
                print_Message("Can't load the units!", 0, 1, true, false, true);
                output = FUNCTION_FAIL; break; 
            }
        }
        // Get Unit Table
        unit_Table = getFile_Unit(word+strBuff, &unit_TableSize);
        if(!unit_Table){ output = FUNCTION_FAIL; break; } 
        
        // Get side attacking
        strBuff = strlen("attacker: ");
        fgets(word, sizeof(word), scen);
        word[strlen(word)-1] = '\0';
        if(strlen(word) < strBuff)
        {
            print_Message("Can't read the side attacking!", 0, 1, true, false, true);
            output = FUNCTION_FAIL; break; 
        }
        strBuff = strlen(word);
        if(word[strBuff-1] == 'A' || word[strBuff-1] == 'a')
            Side_A.attacker = true, Side_B.attacker = false;
        else if(word[strBuff-1] == 'B' || word[strBuff-1] == 'b')
            Side_A.attacker = false, Side_B.attacker = true;
        else
        {
            print_Message("Can't load the side attacking!", 0, 1, true, false, true);
            output = FUNCTION_FAIL; break; 
        }

        // Get side_A stats
        // Name
        strBuff = strlen("sideA: ");
        fgets(word, sizeof(word), scen);
        word[strlen(word)-1] = '\0';
        if(strlen(word) < strBuff)
        {
            print_Message("Can't load Side_A name!", 0, 1, true, false, true);
            output = FUNCTION_FAIL; break; 
        }
        strcpy(Side_A.name, word+strBuff);
        // How to place units
        strBuff = strlen("placement: ");
        fgets(word, sizeof(word), scen);
        word[strlen(word)-1] = '\0';
        if(strlen(word) < strBuff)
        {
            print_Message("Can't read Side_A placement!", 0, 1, true, false, true);
            output = FUNCTION_FAIL; break; 
        }
        if(word[strBuff] == 'c' || word[strBuff] == 'C') Side_A.canPlace = true;
        else if(word[strBuff] == 'f' || word[strBuff] == 'F') Side_A.canPlace = false;
        else
        {
            print_Message("Can't load Side_A placement!", 0, 1, true, false, true);
            output = FUNCTION_FAIL; break; 
        }
        // Get units in map
        // Quantity
        strBuff = strlen("units: ");
        fgets(word, strBuff, scen); // Ignore this part of parameter
        fscanf(scen, "%c", &word[0]);
        if(word[0] != ' ')
        {
            print_Message("Can't read Side_A placement!", 0, 1, true, false, true);
            output = FUNCTION_FAIL; break;  
        }
        fscanf(scen, "%d", &unit_N);
        if(unit_N < 1)
        {
            print_Message("Can't load Side_A placement!", 0, 1, true, false, true);
            output = FUNCTION_FAIL; break; 
        }
        // Unit
        if(Side_A.canPlace == true)
        {
            for(int i = 0; i < unit_N; i++)
            {
                int index = NO_UNIT;
                fscanf(scen, "%d", &index);
                set_fUnitTable(unit_Table, index, &Side_A);
                fgets(word, sizeof(word), scen);    // Get rid of anything until \n
            }
        }
        else
        {
            for(int i = 0; i < unit_N; i++)
            {
                int unit_I = NO_UNIT, unit_X = NO_UNIT, unit_Y = NO_UNIT;
                fscanf(scen, "%d %d %d", &unit_I, &unit_X, &unit_Y);
                set_fUnitTable(unit_Table, unit_I, &Side_A);
                Side_A.units[i].position.X = unit_X, Side_A.units[i].position.Y = unit_Y;
                fgets(word, sizeof(word), scen);    // Get rid of anything until \n
            }            
        }
        

        // Side_B
        // Name
        strBuff = strlen("sideB: ");
        fgets(word, sizeof(word), scen);
        word[strlen(word)-1] = '\0';
        if(strlen(word) < strBuff)
        {
            print_Message("Can't load Side_B name!", 0, 1, true, false, true);
            output = FUNCTION_FAIL; break; 
        }
        strcpy(Side_B.name, word+strBuff);
        // How to place units
        strBuff = strlen("placement: ");
        fgets(word, sizeof(word), scen);
        word[strlen(word)-1] = '\0';
        if(strlen(word) < strBuff)
        {
            print_Message("Can't read Side_B placement!", 0, 1, true, false, true);
            output = FUNCTION_FAIL; break; 
        }
        if(word[strBuff] == 'c' || word[strBuff] == 'C') Side_B.canPlace = true;
        else if(word[strBuff] == 'f' || word[strBuff] == 'F') Side_B.canPlace = false;
        else
        {
            print_Message("Can't load Side_B placement!", 0, 1, true, false, true);
            output = FUNCTION_FAIL; break; 
        }
        // Get units in map
        // Quantity
        strBuff = strlen("units: ");
        fgets(word, strBuff, scen); // Ignore this part of parameter
        fscanf(scen, "%c", &word[0]);
        if(word[0] != ' ')
        {
            print_Message("Can't read Side_B placement!", 0, 1, true, false, true);
            output = FUNCTION_FAIL; break;  
        }
        fscanf(scen, "%d", &unit_N);
        if(unit_N < 1)
        {
            print_Message("Can't load Side_B placement!", 0, 1, true, false, true);
            output = FUNCTION_FAIL; break; 
        }
        // Unit
        if(Side_B.canPlace == true)
        {
            for(int i = 0; i < unit_N; i++)
            {
                int index = NO_UNIT;
                fscanf(scen, "%d", &index);
                set_fUnitTable(unit_Table, index, &Side_B);
                fgets(word, sizeof(word), scen);    // Get rid of anything until \n
            }
        }
        else
        {
            for(int i = 0; i < unit_N; i++)
            {
                int unit_I = NO_UNIT, unit_X = NO_UNIT, unit_Y = NO_UNIT;
                fscanf(scen, "%d %d %d", &unit_I, &unit_X, &unit_Y);
                set_fUnitTable(unit_Table, unit_I, &Side_B);
                Side_B.units[i].position.X = unit_X, Side_B.units[i].position.Y = unit_Y;
                fgets(word, sizeof(word), scen);    // Get rid of anything until \n
            }            
        }
        
        // Printing results
        screen_TopMap(scen_Name, scen_Map);
        // Map Description
        strBuff = strlen("mapDesc: ");
        fgets(word, strBuff+1, scen);
        if(word[0] != '$') 
        {
            do
            {
                fgets(word, sizeof(word), scen);
                if(word[0] == '$') break;
                word[strlen(word)-1] = '\0';
                print_Line(word, 0);
            } while (!feof(scen));
            print_Line(" ", 0);
            fillSpace_ToBottom(3);
            print_Line("[ARROW KEY] Change map | [ENTER] Select Map | [ESC] Return To Menu", 0);
            print_Line(" ", 0);
            print_Line(NULL, 0);
        }
        break;
    }

    // Treating free levels
    if(output == FUNCTION_FAIL)
        deallocAll();
    else
    {   // Get number of maps in scenario
        rewind(scen);
        while(1)
        {
            fgets(word, sizeof(word), scen);
            if(word[0] == '$')
                output++;
            if(feof(scen)) break;
        }
    }
    fclose(scen);

    reset_Cursor();
    return output;
}

int placementMenu(B_Map *map, B_Side *Side, int *mode)
{
    system("cls");
    int Index = -1, out = 0, i = 0;
    short int dUnits = 0;
    char msg[STRING_NAME];
    do
    {
        Index = -1;
        B_Pos unitPos = {-1, -1};
        reset_Cursor();
        toggle_Cursor(false);
        snprintf(msg, sizeof(msg), "%s [%s]", map->name, get_MapMode(*mode));

        print_Line(NULL, 0);
        print_Line(" ", 0);
        print_Line(msg, 0);
        print_Line("[T] Change Map Type  | [A] Place A Unit", 0);
        print_Line("[Esc] Exit To Menu   | [M] Replace Unit", 0);
        print_Line("[Enter] Start Battle | [R] Remove Unit", 0);
        print_Line(" ", 0);
        print_Line(NULL, 0);
        if(*mode != Map_Graphic)
        {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
            BACKGROUND_GREEN | BACKGROUND_RED | FOREGROUND_RED | BACKGROUND_INTENSITY);
        }
        show_Map(map, *mode, false);
        
        reset_Cursor();
        out = get_KeyPress(true);
        switch (out)
        {
        case 't':
            (*mode)++;
            if (*mode > Map_Spawns)
                *mode = Map_Height;
            continue;
        case 'a':
            if (dUnits == Side->size)
            {
                print_Message("All the units have been deployed!", 0, 1, true, false, true);
                continue;
            }
            clear_afterMap(map->height);
            printf(">> Placeable units: \n");
            for (i = 0; i < Side->size; i++)
                if (Side->units[i].position.X < 0 && Side->units[i].position.Y < 0)
                    show_gUnit(&Side->units[i]);

            printf(">> ID of the chosen unit: ");
            toggle_Cursor(true);
            scanf(" %d", &out);
            toggle_Cursor(false);

            Index = get_UnitIndex(Side, out);
            reset_Cursor();
            if (Index > -1 && (Side->units[Index].position.X < 0 && Side->units[Index].position.Y < 0))
            {
                print_Message(">> Cordinates of the unit <X Y> inside |*1| area: ", 0, 1, true, true, false);
                toggle_Cursor(true);
                scanf(" %hd %hd", &Side->units[Index].position.X, &Side->units[Index].position.Y);
                toggle_Cursor(false);

                out = put_Unit_OnMap(map, &Side->units[Index], false);
                if (out == FUNCTION_FAIL)
                {
                    print_Message("Invalid coordinates!", 0, 3, false, false, true);
                    Side->units[Index].position.X = -1, Side->units[Index].position.Y = -1;
                }
                else
                {
                    print_Message("Unit placed sucessfully!", 0, 3, false, false, true);
                    dUnits++;
                }
            }
            else
                print_Message("This unit ID is not valid!", 0, 1, true, false, true);
            clear_afterMap(map->height);
            continue;
        case 'm':
            if (dUnits < 1)
            {
                print_Message("No units deployed to move", 0, 1, true, false, true);
                continue;
            }
            clear_afterMap(map->height);
            for (i = 0; i < Side->size; i++)
            {
                if (Side->units[i].position.X >= 0 && Side->units[i].position.Y >= 0)
                {
                    printf(">> At %dX - %dY \n", Side->units[i].position.X, Side->units[i].position.Y);
                    show_gUnit(&Side->units[i]);
                }
            }
            printf(">> ID of the chosen unit: ");
            toggle_Cursor(true);
            scanf("%d", &out);
            toggle_Cursor(false);

            Index = get_UnitIndex(Side, out);
            if (Index > -1 && (Side->units[Index].position.X > -1 && Side->units[Index].position.Y > -1))
            {
                unitPos.X = Side->units[Index].position.X, unitPos.Y = Side->units[Index].position.Y;
                print_Message(">> Cordinates of the unit <X Y> inside |*1| area: ", 0, 1, true, true, false);
                toggle_Cursor(true);
                scanf("%hd %hd", &Side->units[Index].position.X, &Side->units[Index].position.Y);
                toggle_Cursor(false);

                out = put_Unit_OnMap(map, &Side->units[Index], false);
                if (out == FUNCTION_FAIL)
                {
                    Side->units[Index].position.X = unitPos.X, Side->units[Index].position.Y = unitPos.Y;
                    print_Message("Invalid coordinates!", 0, 3, false, false, true);
                }
                else
                {
                    map->tiles[unitPos.Y][unitPos.X].unit = NULL;
                    print_Message("Unit moved sucessfully!", 0, 3, false, false, true);
                }
            }
            else
                print_Message("This unit ID is not valid!", 0, 3, true, false, true);
            clear_afterMap(map->height);
            continue;
        case 'r':
            if (dUnits < 1)
            {
                print_Message("No units deployed to remove", 0, 1, true, false, true);
                continue;
            }
            clear_afterMap(map->height);
            for (i = 0; i < Side->size; i++)
            {
                if (Side->units[i].position.X >= 0 && Side->units[i].position.Y >= 0)
                {
                    printf(">> At %dX - %dY \n", Side->units[i].position.X, Side->units[i].position.Y);
                    show_gUnit(&Side->units[i]);
                }
            }

            printf(">> ID of the chosen unit: ");
            toggle_Cursor(true);
            scanf("%d", &out);
            toggle_Cursor(false);

            Index = get_UnitIndex(Side, out);
            if (Index > -1 && (Side->units[Index].position.X > -1 && Side->units[Index].position.Y > -1))
            {
                unitPos.X = Side->units[Index].position.X, unitPos.Y = Side->units[Index].position.Y;
                map->tiles[unitPos.Y][unitPos.X].unit = NULL;
                Side->units[Index].position.X = -1, Side->units[Index].position.Y = -1;
                print_Message("Unit removed sucessfully!", 0, 1, true, false, true);
                dUnits--;
            }
            else
                print_Message("This unit ID is not valid!", 0, 1, true, false, true);
            clear_afterMap(map->height);
            continue;
        case KEY_ESCAPE:
            return FUNCTION_FAIL;
        case KEY_ENTER:
            // Cheking if any units are deployed at all
            if (dUnits < 1)
            {
                print_Message("You must deploy at least one unit!", 0, 1, true, false, true);
                continue;
            }

            // Cheking for undeployed units
            if (dUnits < Side->size)
            {
                print_Message("There are undeployed units! Proceed? [Y/N]", 0, 1, true, false, false);
                out = get_KeyPress(true);
                if (out != 'y')
                    continue;
            }
            return FUNCTION_SUCESS;
        case -32:
            (void) get_KeyPress(false);
        default:
            print_Message("Invalid action!", 0, 1, true, false, true);
            fflush(stdin);
            continue;
        }
    } while (1);
}

int handleMove(B_Map *map, B_Unit *unit, int *moves, B_Side *player, B_Side *opponent, int mode)
{
    int result = 0;
    B_Pos pos_A = unit->position, pos_B, pos_Screen = unit->position;
    char msg[STRING_NAME];
    for (int steps = 0; *moves < unit->moves; steps++)
    {
        if (compPos(unit->position, unit->goal) == true || result == OUT_COMBAT)
        {
            // unit->goal.X = NO_UNIT, unit->goal.Y = NO_UNIT;
            update_Map(pos_A.X, pos_A.Y, get_MapSprite(&map->tiles[pos_A.Y][pos_A.X], mode));
            (*moves)--; return FUNCTION_SUCESS;
        }
        result = move_Unit(map, unit, unit->path[steps]);
        pos_A = unit->position;
        update_Map(pos_Screen.X, pos_Screen.Y, get_MapSprite(&map->tiles[pos_Screen.Y][pos_Screen.X], mode));
        pos_Screen.X = pos_A.X,pos_Screen.Y = pos_A.Y;
        if (result > -1)
            (*moves) += result+1;
        else if(result == OUT_COMBAT && unit->attacked == true)
        {
            print_Message("This unit can't attack until next turn!", screenHalf, 1, true, false, true);
            (*moves)--; break;
        }
        else if (result == OUT_COMBAT)
        {
            pos_B = get_AdjTile_Pos(map, pos_A, unit->path[steps]);
            int target_I = get_UnitIndex(opponent, map->tiles[pos_B.Y][pos_B.X].unit->Game_ID);
            meleeCombat(player, unit, opponent, &opponent->units[target_I]);
            // Done
            show_gUnit(unit);
            show_gUnit(&opponent->units[target_I]);
            printf(">> Press ENTER to continue ");
            while (get_KeyPress(false) != KEY_ENTER) continue;
            system("cls");
            if(opponent->units[target_I].men == 0)
            {
                delete_Unit(opponent->units, &opponent->size, target_I);
                map->tiles[pos_B.Y][pos_B.X].unit = NULL;
            }
            show_Map(map, mode, true);
            (*moves)= unit->moves;
            break;
        }
        else if (result == FUNCTION_FAIL)
        {(*moves)--; break; }
    }
    update_Map(pos_A.X, pos_A.Y, get_MapSprite(&map->tiles[pos_A.Y][pos_A.X], mode));
    return result;
}

int do_Turn(B_Side *player, B_Side *opponent, B_Map *battleMap, int cUnit_I, int turn, int *mode)
{
    B_Pos pos_A = player->units[cUnit_I].position, pos_B = {-1, -1}, goal = {-1, -1}, target = {-1, -1};
    COORD pos_Screen = {0, 0};
    int target_I = 0;
    char action = 'x', msg[STRING_NAME];

    // Setting up unit
    if(_kbhit() && _getch() == KEY_ESCAPE)
        return FUNCTION_FAIL;
    if(pos_A.Y == NO_UNIT || pos_A.X == NO_UNIT)
        return FUNCTION_FAIL;
    if(player->isAI == false) // Player Zone
    {
        for (int moves = 0; moves < player->units[cUnit_I].moves; moves++)
        {
            pos_A = player->units[cUnit_I].position;
            if(check_UnitMove(&player->units[cUnit_I], &moves) == false)
            {
                clear_afterMap(battleMap->height);
                continue;
            }
            info_Upper (battleMap->name, *mode, turn, player->name, true, player->units[cUnit_I].name,
                        player->units[cUnit_I].Game_ID, pos_A, player->units[cUnit_I].moves - moves);

            if (player->units[cUnit_I].retreating == true)
            {
                toggle_Cursor(false);
                print_Message("Retreating", get_HalfWidth(), 5, false, false, true);
                break;
            }
            
            toggle_Cursor(false);
            fflush(stdin);
            
            while(1)
            {
                if(_kbhit())
                { action = _getch(); break; }
                char highlight[3] = {219, 219, 219};
                update_Map(pos_A.X, pos_A.Y, highlight);
                Sleep(250);
                if(_kbhit())
                { action = _getch(); break; }
                Sleep(250);
                toggle_Cursor(false);
                if(_kbhit())
                { action = _getch(); break; }
                update_Map(pos_A.X, pos_A.Y, player->units[cUnit_I].name);
                Sleep(250);
                if(_kbhit())
                { action = _getch(); break; }
                Sleep(250);
            }
            pos_Screen.X = 0, pos_Screen.Y = MAP_OFFSET_Y + (battleMap->height * 2);
            if (action >= '0' && action <= '9') // Move to a adjacent tile
            {
                tNum_ToDirec(&action);
                pos_B = get_AdjTile_Pos(battleMap, pos_A, action);
                player->units[cUnit_I].goal = pos_B;
                player->units[cUnit_I].chaseID = NULL;
            }
            else if (action == 'a') // Move to a location
            {
                clear_afterMap(battleMap->height);
                toggle_Cursor(true);
                print_Message(">> Goal coordinates <X Y>: ", screenHalf, 1, true, true, false);
                scanf("%hd %hd", &goal.X, &goal.Y);
                toggle_Cursor(false);
                if(goal.X < 0 || goal.Y < 0 || goal.X >= battleMap->width || goal.Y >= battleMap->height)
                {
                    print_Message("Invalid Coords", screenHalf, 3, false, false, true);
                    moves--; continue;
                }
                else if(!autoMove(battleMap, &battleMap->tiles[pos_A.Y][pos_A.Y], &battleMap->tiles[goal.Y][goal.X]))
                {
                    snprintf(msg, sizeof(msg), "Can't find a path to %dX %dY", goal.X, goal.Y);
                    print_Message(msg, screenHalf, 3, false, false, true);
                    moves--; continue;
                }
                else
                {
                    player->units[cUnit_I].goal = goal;
                    print_Message("Moving to tile!", screenHalf, 3, false, false, true);
                }
            }
            else if (action == 't') // Change Map Mode
            {
                (*mode)++;
                if(*mode > Map_Spawns) *mode = Map_Height;
                show_Map(battleMap, *mode, true);
                moves--; continue;
            }
            else if (action == 's') // Intercept a unit
            {
                clear_afterMap(battleMap->height);
                toggle_Cursor(true);
                print_Message(">> Target unit coordinates <X Y>: ", screenHalf, 1, true, true, false);
                scanf("%hd %hd", &goal.X, &goal.Y);
                toggle_Cursor(false);
                if(goal.X < 0 || goal.Y < 0 || goal.X >= battleMap->width || goal.Y >= battleMap->height)
                {
                    print_Message("Invalid Coords", screenHalf, 3, false, false, true);
                    moves--; continue;
                }
                else if(battleMap->tiles[goal.Y][goal.X].unit == NULL)
                {
                    print_Message("Target Not Found!", screenHalf, 3, false, false, true);
                    moves--; continue;
                }
                else if(!autoMove(battleMap, &battleMap->tiles[pos_A.Y][pos_A.Y], &battleMap->tiles[goal.Y][goal.X]))
                {
                    snprintf(msg, sizeof(msg), "Can't find a path to %dX %dY", goal.X, goal.Y);
                    print_Message(msg, screenHalf, 3, false, false, true);
                    moves--; continue;
                }
                else
                {
                    target_I = get_UnitIndex(opponent, battleMap->tiles[goal.Y][goal.X].unit->Game_ID);
                    player->units[cUnit_I].chaseID = &opponent->units[target_I].Game_ID;
                    player->units[cUnit_I].goal = goal;
                    print_Message("Moving to intercept the unit!", screenHalf, 3, false, false, true);
                }
            }
            else if(action == 'f') // Fire at unit
            {
                if (player->units[cUnit_I].attacked == true)
                {
                    print_Message("This unit can't attack until next turn!", screenHalf, 1, true, false, true);
                    moves--; continue;
                }

                
                if (player->units[cUnit_I].range < 1)
                {
                    print_Message("This unit can't do ranged attacks!", screenHalf, 1, true, false, true);
                    moves--; continue;
                }

                // clear_afterMap(battleMap->height);
                toggle_Cursor(true);
                print_Message(">> Target coordinates <X Y>: ", screenHalf, 1, true, true, false);
                scanf("%hd %hd", &target.X, &target.Y);
                toggle_Cursor(false);
                
                if(rangedCombat(player, &player->units[cUnit_I], opponent, target) == FUNCTION_FAIL)
                { moves--; continue; }
                if(opponent->units[target_I].men == 0)
                {
                    delete_Unit(opponent->units, &opponent->size, target_I);
                    battleMap->tiles[pos_B.Y][pos_B.X].unit = NULL;
                }
                
                show_Map(battleMap, *mode, true);
                continue;
            }
            else if (action == 'd') // View unit stats
            {
                reset_Cursor();
                system("cls");
                show_gUnit(&player->units[cUnit_I]);
                printf(">> Press ENTER to continue \n");
                while (get_KeyPress(false) != KEY_ENTER) continue;
                system("cls");
                show_Map(battleMap, *mode, true);
                moves--; continue;
            }
            else if (action == 'w') // View unit wiki
            {
                reset_Cursor();
                system("cls");
                show_gUnit(&unit_Table[player->units[cUnit_I].Table_ID]);
                print_UnitDesc(player->units[cUnit_I].Table_ID, unit_Desc);
                printf(">> Press ENTER to continue \n");
                while (get_KeyPress(false) != KEY_ENTER) continue;
                system("cls");
                show_Map(battleMap, *mode, true);
                moves--; continue;
            }
            else if (action == 'e') // Fortify
            {
                if (inc_FortLevel(battleMap, player->units[cUnit_I].build_Cap, pos_A) == FUNCTION_FAIL)
                { moves--; continue; } 
                else moves = player->units[cUnit_I].moves;
            }
            else if (action == 'g') // See tile stats
            {
                clear_afterMap(battleMap->height);
                toggle_Cursor(true);
                print_Message(">> Target tile coordinates <X Y>: ", screenHalf, 1, true, true, false);
                scanf("%hd %hd", &target.X, &target.Y);
                getchar();
                toggle_Cursor(false);
                if (target.X >= 0 && target.X < battleMap->width && target.Y >= 0 && target.Y < battleMap->height)
                {
                    snprintf(msg, sizeof(msg), "Terrain: %s", tTerrain_toStr(battleMap->tiles[target.Y][target.X].terrain));
                    print_Message(msg, screenHalf, 0, true, false, false);
                    snprintf(msg, sizeof(msg), "Vegetation: %s", tVeget_toStr(battleMap->tiles[target.Y][target.X].vegetation));
                    print_Message(msg, screenHalf, 1, false, false, false);
                    snprintf(msg, sizeof(msg), "Elevation: %d", battleMap->tiles[target.Y][target.X].elevation);
                    print_Message(msg, screenHalf, 2, false, false, false);
                    snprintf(msg, sizeof(msg), "Fortification: %d", battleMap->tiles[target.Y][target.X].fortLevel);
                    print_Message(msg, screenHalf, 3, false, false, false);
                    if(battleMap->tiles[target.Y][target.X].unit)
                    {
                        snprintf(msg, sizeof(msg), "Unit in here: %d (%s)", battleMap->tiles[target.Y][target.X].unit->Game_ID, battleMap->tiles[target.Y][target.X].unit->name);
                        print_Message(msg, screenHalf, 4, false, false, false);
                    }
                    print_Message(">> Press ENTER to continue", screenHalf, 5, false, false, false);
                    while (get_KeyPress(false) != KEY_ENTER) continue;
                    moves--; continue;
                }
                else
                {
                    print_Message("Coordinates out of boundaries!", screenHalf, 3, false, false, true);
                    moves--; continue;
                }
            }
            else if (action == KEY_ESCAPE) // Return to Menu
                return FUNCTION_FAIL;
            else if (action == KEY_ENTER) // Next Turn
            {
                if(player->units[cUnit_I].goal.X == NO_UNIT || player->units[cUnit_I].goal.Y == NO_UNIT)
                    moves = player->units[cUnit_I].moves;
            }
            else // Invalid
            {
                clear_afterMap(battleMap->height);
                print_Message("Invalid action!", screenHalf, 1, true, false, true);
                moves--; continue;
            }
            goal = player->units[cUnit_I].goal;
            if(goal.X > -1 && goal.Y > -1 && goal.X < battleMap->width && goal.Y < battleMap->height)
            {
                pos_A = player->units[cUnit_I].position;
                pos_Screen.X = pos_A.X, pos_Screen.Y = pos_Screen.Y = pos_A.Y;
                if (player->units[cUnit_I].chaseID != NULL) 
                { // Change to coordinates of goal if player unit is chasing
                    target_I = get_UnitIndex(opponent, *player->units[cUnit_I].chaseID);
                    goal = player->units[cUnit_I].goal = opponent->units[target_I].position;
                }
                
                player->units[cUnit_I].path = (int *)
                autoMove(battleMap, &battleMap->tiles[pos_A.Y][pos_A.X], &battleMap->tiles[goal.Y][goal.X]);
                if(player->units[cUnit_I].path == NULL)
                {
                    clear_afterMap(battleMap->height);
                    reset_Cursor();
                    print_Message("No path to this tile!", screenHalf, 1, true, false, true);
                    moves--; continue;
                }

                int res = handleMove(battleMap, &player->units[cUnit_I], &moves, player, opponent, *mode);
                free(player->units[cUnit_I].path);
                goal = player->units[cUnit_I].goal;
                if (moves < player->units[cUnit_I].moves-1 || res == OUT_COMBAT)
                {
                    player->units[cUnit_I].chaseID = NULL;
                    player->units[cUnit_I].goal.X = -1, player->units[cUnit_I].goal.Y = -1;
                    if(action == KEY_ENTER && moves != 0)
                        print_Message("Auto move disabled!", screenHalf, 1, true, false, true);
                }
                toggle_Cursor(false);
            }
        }
        player->units[cUnit_I].attacked = false, player->units[cUnit_I].engaged = false;
        update_Map(pos_A.X, pos_A.Y, get_MapSprite(&battleMap->tiles[pos_A.Y][pos_A.X], *mode));}
    else // AI Zone
    {
        B_Tile *position;
        for (int moves = 0; moves < player->units[cUnit_I].moves; moves++)
        {
            // show_Map(&battleMap, Map_Height);
            int realMoves = player->units[cUnit_I].moves - moves;
            info_Upper(battleMap->name, *mode, turn, player->name, false, player->units[cUnit_I].name,
                       player->units[cUnit_I].Game_ID, pos_A, NO_UNIT);
            if (player->units[cUnit_I].retreating == true)
            {
                toggle_Cursor(false);
                print_Message("Retreating", get_HalfWidth(), 5, false, false, true);
                break;
            }

            update_Map(pos_A.X, pos_A.Y, player->units[cUnit_I].name);
            Sleep(TIME_STRATEGY);
            pos_Screen.Y = MAP_OFFSET_Y+battleMap->height*2;

            if(check_UnitMove(&player->units[cUnit_I], &moves) == false)
            {
                clear_afterMap(battleMap->height);
                continue;
            }
            
            pos_Screen.X = pos_A.X, pos_Screen.Y = pos_A.Y;
            switch(AI_Process(battleMap, player, opponent, &player->units[cUnit_I], AI_Easy, realMoves))
            {
                case AI_Fire:
                    pos_B = player->units[cUnit_I].goal;
                    if(rangedCombat(player, &player->units[cUnit_I], opponent, pos_B) == FUNCTION_SUCESS)
                    {
                        show_Map(battleMap, *mode, true);
                        continue;
                    }
                    moves--;
                    break;
                case AI_GoTo:
                case AI_Engage:
                    pos_A.X = player->units[cUnit_I].position.X, pos_A.Y = player->units[cUnit_I].position.Y;
                    pos_B.X = player->units[cUnit_I].goal.X, pos_B.Y = player->units[cUnit_I].goal.Y;
                    player->units[cUnit_I].path = (int *)
                    autoMove(battleMap, &battleMap->tiles[pos_A.Y][pos_A.X], &battleMap->tiles[pos_B.Y][pos_B.X]);
                    if(player->units[cUnit_I].path == NULL)
                    {
                        clear_afterMap(battleMap->height);
                        print_Message("No path to this tile!", screenHalf, 1, true, false, true);
                        moves--; continue;
                    }
                    (void) handleMove(battleMap, &player->units[cUnit_I], &moves, player, opponent, *mode);
                    free(player->units[cUnit_I].path);
                    break;
                case AI_Fortify:
                    if (inc_FortLevel(battleMap, player->units[cUnit_I].build_Cap, pos_A) == FUNCTION_FAIL)
                    { moves--; continue; } 
                    else moves = player->units[cUnit_I].moves;
                    break;
                default:
                    (void) move_Unit(battleMap, &player->units[cUnit_I], Northeast);
                    pos_A = player->units[cUnit_I].position;
                    break;
            }
            clear_afterMap(battleMap->height);
            pos_A = player->units[cUnit_I].position;
            update_Map(pos_Screen.X, pos_Screen.Y, get_MapSprite(&battleMap->tiles[pos_Screen.Y][pos_Screen.X], *mode));
        }
        player->units[cUnit_I].attacked = false, player->units[cUnit_I].engaged = false;
        info_Upper(battleMap->name, *mode, turn, Side_B.name, false, player->units[cUnit_I].name,
                    player->units[cUnit_I].Game_ID, pos_A, NO_UNIT);
        update_Map(pos_A.X, pos_A.Y, get_MapSprite(&battleMap->tiles[pos_A.Y][pos_A.X], *mode));
        Sleep(TIME_STRATEGY);
    }
    return FUNCTION_SUCESS;
}

bool check_Victory()
{
    fflush(stdin);
    int leftA, leftB;
    B_Pos invalid = {-1, -1};
    for(leftA = Side_A.size-1; leftA >= 0; leftA--)
    {
        if(compPos(Side_A.units[leftA].position, invalid) == false)
            break;
        else if(leftA == 0)
            leftA--;
    }
    for(leftB = Side_B.size-1; leftB >= 0; leftB--)
    {
        if(compPos(Side_B.units[leftB].position, invalid) == false)
            break;
        else if(leftB == 0)
            leftB--;
    }
    if(leftA < 0)
    { screen_Victory(Side_B.stats, Side_A.stats); return true;}
    else if(leftB < 0)
    { screen_Victory(Side_A.stats, Side_B.stats); return true;}
    return false;
}

bool check_Retreat(int* mode, B_Side* side)
{
    char *msg;
    for (int j = 0; j < side->size; j++)
    {
        B_Pos unitPos = side->units[j].position;
        bool retreated = true;
        if (side->units[j].retreating == true)
        { 
            for (int moves = 0; moves < side->units[j].moves; moves++)
            {
                retreated = unit_Retreat(&side->units[j], &battleMap, *mode);
                unitPos = side->units[j].position;
                if (retreated == false)
                {
                    update_Map(unitPos.X, unitPos.Y, "#*#");
                    Sleep(TIME_STRATEGY);
                    clear_afterMap(battleMap.height);
                    battleMap.tiles[unitPos.Y][unitPos.X].unit = NULL;
                    delete_Unit(side->units, &side->size, j);
                    return true;
                }
            }
            update_Map(unitPos.X, unitPos.Y, get_MapSprite(&battleMap.tiles[unitPos.Y][unitPos.X], *mode));
        }
    }
    return false;
}
int main(/*int argc, char** argv*/)
{
    // Randomizing seed
    srand(time(NULL));

    // Setting up terminal
    SetConsoleTitle("Total Terminal War");
    toggle_Cursor(false);

    int nMaps = 0, cScen = 0, cMap = 0, out = 0, mode = Map_Spawns;
    extern bool muted;

startMenu:
    // Playing music
    jukebox("Menu.wav", SND_ASYNC | SND_FILENAME | SND_LOOP);
    // Side_A
    Side_A.size = 0, Side_A.ID = 0, Side_A.isAI = false;
    // Side_B
    Side_B.size = 0, Side_B.ID = 1, Side_B.isAI = true;
    // Menu 
    cMap = 0, mode = 0;
    do
    {
        switch (screen_Menu(VERSION))
        {
        case 0:
            cScen = screen_Scenery();
            do
            {   // Scroll through maps
                Side_A.units = (B_Unit *)malloc(sizeof(B_Unit));
                Side_B.units = (B_Unit *)malloc(sizeof(B_Unit));
                nMaps = load_Scenery(cScen, cMap);
                if(nMaps < 1) // Error
                    goto startMenu;
                cMap = screen_MapInput(cMap, nMaps);
                if(cMap == FUNCTION_SUCESS) break;
                else deallocAll();
                if(cMap == FUNCTION_FAIL) goto startMenu;
            } while (1);
            break;
        case 1:
            system("cls");
            show_TextFile("tutorial.txt");
            do { /* Nothing */ }
            while(get_KeyPress(false) != KEY_ENTER);
            continue;
        case 2:
            screen_Credits(VERSION);
            continue;
        case 3:
            muted = !muted;
            jukebox("Menu.wav", SND_ASYNC | SND_FILENAME | SND_LOOP);
            continue;
        case 4:
            deallocAll();
            return 0;
        default:
            continue;
        }
        break;
    } while (1);

    Side_A.stats.name = Side_A.name, Side_B.stats.name = Side_B.name;
    Side_A.stats.deployed = 0, Side_A.stats.killed = 0, Side_A.stats.loss = 0;
    Side_B.stats.deployed = 0, Side_B.stats.killed = 0, Side_B.stats.loss = 0;

    // Placing AI on Map
    if(Side_B.isAI == true)
    {
        for (int i = 0; i < Side_B.size; i++)
        {
            def_Unit(&Side_B.units[i], &battleMap, false);
            put_Unit_OnMap(&battleMap, &Side_B.units[i], !Side_B.canPlace);
        }
    }
    // Placing Player on map
    mode = Map_Spawns;
    if (placementMenu(&battleMap, &Side_A, &mode) == FUNCTION_FAIL)
    {
        (void)deallocAll();
        goto startMenu;
    }

    // Getting deployed troops
    for (int i = 0; i < Side_A.size; i++)
        if(Side_A.units[i].position.X != NO_UNIT && Side_A.units[i].position.Y != NO_UNIT)
            Side_A.stats.deployed += Side_A.units[i].men;
    for (int i = 0; i < Side_B.size; i++)
        if(Side_B.units[i].position.X != NO_UNIT && Side_B.units[i].position.Y != NO_UNIT)
            Side_B.stats.deployed += Side_B.units[i].men;
    // Game Starts
    jukebox("Game1.wav", SND_ASYNC | SND_LOOP | SND_FILENAME);
    system("cls");
    
    mode = Map_Graphic;
    screenHalf = (int) ceilf(get_ScreenWidth() / 2.0f);
    show_Map(&battleMap, mode, true);
    
    int unitA_I = 0, unitB_I = 0, target_I = 0;
    char action = '\0';
    for (int i = 0; i < TURNS; i++)
    {
        // Get next unit
        for(int index = 0; index < Side_A.size; index++)
        {
            if(unitA_I >= Side_A.size)
                unitA_I = 0;
            if(Side_A.units[unitA_I].position.X == NO_UNIT || Side_A.units[unitA_I].position.Y == NO_UNIT)
                unitA_I++;
            else break;
        } // Side_A turn
        if(do_Turn(&Side_A, &Side_B, &battleMap, unitA_I, i, &mode) == FUNCTION_FAIL)
            break;
        unitA_I++;
        // Cheking for retreat
        check_Retreat(&mode, &Side_A);
        
        // Checking for victory
        if(check_Victory() == true)
            break;

        // Get next unit
        for(int index = 0; index < Side_B.size; index++)
        {
            if(unitB_I >= Side_B.size)
                unitB_I = 0;
            if(Side_B.units[unitB_I].position.X == NO_UNIT || Side_B.units[unitB_I].position.Y == NO_UNIT)
                unitB_I++;
            else break;
        } // Side_B turn
        if (do_Turn(&Side_B, &Side_A, &battleMap, unitB_I, i, &mode) == FUNCTION_FAIL)
            break;
        unitB_I++;

        // Checking for victory
        check_Retreat(&mode, &Side_B);

        // Checking for victory
        if(check_Victory() == true)
            break;

    }

    // Freeing to Menu
    (void)deallocAll();
    PlaySound("sound/Menu.wav", NULL, SND_ASYNC | SND_FILENAME | SND_LOOP); 
    goto startMenu;

    return 0;
}