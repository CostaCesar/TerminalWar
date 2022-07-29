// #include <stdio.h>
// #include <stdlib.h>
// #include <stdbool.h>
// #include <math.h>
// #include <Windows.h>
#include <time.h>
#include "include/map.h"
#include "include/combat.h"
#include "include/UI.h"

#define MAP_DELAY (2 * 1000)
#define TURNS 30
#define VERSION 0.575f

// Game Variables
B_Map battleMap;
B_Side Side_A, Side_B;
B_endStats Status_A = {0}, Status_B = {0};
B_Unit *unit_Table = NULL;
int unit_TableSize = 0;

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
    side->units[pos].ID = pos * 2 + side->ID;
    return;
}

// Map<->Unit functions
Map_Unit set_MapUnit(B_Unit *source)
{
    Map_Unit choke;
    choke.X = source->position.X;
    choke.Y = source->position.Y;
    choke.ID = source->ID;
    choke.name = source->name;
    return choke;
}

Map_Unit def_Unit(B_Unit *unit, B_Map *map, bool ignoreSpawn)
{
    Map_Unit out = {-1, -1, NO_UNIT, "\0"};
    if(ignoreSpawn == true && unit->position.X > NO_UNIT && unit->position.Y > NO_UNIT)
    {
        out = set_MapUnit(unit);
        return out;
    }
    for (int i = 0; i < map->height; i++)
        for (int j = 0; j < map->width; j++)
        {
            if ((map->tiles[i][j].isSpawnA == true && unit->ID % 2 == 0) || (map->tiles[i][j].isSpawnB == true && unit->ID % 2 == 1))
            {
                if (map->tiles[i][j].unit.ID == NO_UNIT)
                {
                    unit->position.X = j;
                    unit->position.Y = i;
                    out = set_MapUnit(unit);
                    return out;
                }
            }
        }
    return out;
}

int get_UnitIndex(B_Side *side, int ID)
{
    for (int i = 0; i < side->size; i++)
    {
        if (side->units[i].ID == ID)
        {
            return i;
        }
    }
    printf("ERROR: get_UnitIndex >> 1");
    return FUNCTION_FAIL;
}

void update_Unit(B_Unit *unit, Map_Unit source)
{
    unit->position.X = source.X;
    unit->position.Y = source.Y;
    return;
}

// Main functions (screens & logic)
void show_gUnit(B_Unit *unit)
{
    char msg[70];
    char buff[25];

    print_Line(NULL);
    print_Line(" ");
    snprintf(msg, sizeof(msg), "Unit %d", unit->ID);
    print_Line(msg);
    snprintf(msg, sizeof(msg), "%s [%s]", unit->name, unit->faction);
    print_Line(msg);
    snprintf(msg, sizeof(msg), "Level %d", unit->level);
    print_Line(msg);
    snprintf(msg, sizeof(msg), "Men: %d | %d", unit->men, unit->men_Max);
    print_Line(msg);
    snprintf(msg, sizeof(msg), "Morale: %.2f", unit->morale);
    print_Line(msg);
    print_Line("Melee Stats");
    snprintf(msg, sizeof(msg), "%d OFS X %d DFS", unit->attack_MeleeP, unit->defend_MeleeP);
    print_Line(msg);
    print_Line("Ranged Stats");
    snprintf(msg, sizeof(msg), "%d OFS X %d DFS", unit->attack_RangeP, unit->defend_RangeP);
    print_Line(msg);
    snprintf(msg, sizeof(msg), "Level %d", unit->level);
    print_Line(msg);
    snprintf(msg, sizeof(msg), "Build Power: %d", unit->build_Cap);
    print_Line(msg);

    if (unit->range > 0)
    {
        snprintf(msg, sizeof(msg), "Range: %d tiles| Ammo: %d left", unit->range, unit->ammo);
        print_Line(msg);
    }
    print_Line(" ");
    
    msg[0] = '\0';
    for (int i = 0; i < unit->buffs_S; i++)
    {
        snprintf(buff, sizeof(buff), "[+] %s ", get_UnitBuff(unit->buffs[i]));
        strcat(msg, buff);
        if (i > 5)
        {
            print_Line(msg);
            msg[0] = '\0';
        }
    }
    if (msg[0] != '\0')print_Line(msg);

    print_Line(" ");
    if (unit->inCombat == true)
        print_Line("<!> Engaged In Combat");
    if (unit->isRetreating == true)
        print_Line("<!> Retreating");
    print_Line(NULL);
}

int dealloc_ToMenu()
{
    for (int i = 0; i < battleMap.height; i++)
        free(battleMap.tiles[i]);
    free(battleMap.tiles);
    free(Side_A.units);
    free(Side_B.units);
    Side_A.size = 0, Side_B.size = 0;
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
        print_Message("Can't find scenarios folder!", true);
        return FUNCTION_FAIL;    
    }
    for(int i = 0; i < nScen; i++)
    {
        if(i == nScen - 1) strcat(file, fd.cFileName);
        FindNextFile(handle, &fd);
    }
    FindClose(handle);

    // Reading File
    char word[STRING_NAME*3] = {0}, scen_Name[STRING_NAME] = {0}, scen_Map[STRING_NAME] = {0};
    int cMap = 0, strBuff = 0, unit_N = NO_UNIT, lvlFree = 0;
    FILE *scen = fopen(file, "r");
    if(!scen)
    {
        print_Message("Can't open file!", true);
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
            print_Message("Invalid Scenario Title!", true);
            output = FUNCTION_FAIL; break;
        }
        strcpy(scen_Name, scen_Name+strBuff);

        // Traverse the file in search of chosen map
        while(cMap < playMap)
        {
            if(fgets(word, sizeof(word), scen) == NULL) // Not in file
            {
                print_Message("Can't find map!", true);
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
            print_Message("The map can't be read!", true);
            output = FUNCTION_FAIL; break; 
        }
        if(getFile_Map(word+strBuff, &battleMap) == FUNCTION_FAIL){ output = FUNCTION_FAIL; break; }
        lvlFree++;
        // Get Map Name (for player)
        strBuff = strlen("mapTitle: ");
        fgets(scen_Map, sizeof(scen_Map), scen);
        scen_Map[strlen(scen_Map)-1] = '\0';
        if(strlen(scen_Map) < strBuff)
        {
            print_Message("Can't load map title!", true);
            output = FUNCTION_FAIL; break; 
        }
        strcpy(scen_Map, scen_Map+strBuff);
        // Get Unit Table
        strBuff = strlen("table: ");
        fgets(word, sizeof(word), scen);
        word[strlen(word)-1] = '\0';
        if(strlen(word) < strBuff)
        {
            print_Message("Can't load the units!", true);
            output = FUNCTION_FAIL; break; 
        }
        unit_Table = getFile_Unit(word+strBuff, &unit_TableSize);
        if(!unit_Table){ output = FUNCTION_FAIL; break; }
        lvlFree++;
        // Get side attacking
        strBuff = strlen("attacker: ");
        fgets(word, sizeof(word), scen);
        word[strlen(word)-1] = '\0';
        if(strlen(word) < strBuff)
        {
            print_Message("Can't read the side attacking!", true);
            output = FUNCTION_FAIL; break; 
        }
        strBuff = strlen(word);
        if(word[strBuff-1] == 'A' || word[strBuff-1] == 'a')
            Side_A.attacker = true, Side_B.attacker = false;
        else if(word[strBuff-1] == 'B' || word[strBuff-1] == 'b')
            Side_A.attacker = false, Side_B.attacker = true;
        else
        {
            print_Message("Can't load the side attacking!", true);
            output = FUNCTION_FAIL; break; 
        }

        // Get side_A stats
        // Name
        strBuff = strlen("sideA: ");
        fgets(word, sizeof(word), scen);
        word[strlen(word)-1] = '\0';
        if(strlen(word) < strBuff)
        {
            print_Message("Can't load Side_A name!", true);
            output = FUNCTION_FAIL; break; 
        }
        strcpy(Side_A.name, word+strBuff);
        // How to place units
        strBuff = strlen("placement: ");
        fgets(word, sizeof(word), scen);
        word[strlen(word)-1] = '\0';
        if(strlen(word) < strBuff)
        {
            print_Message("Can't read Side_A placement!", true);
            output = FUNCTION_FAIL; break; 
        }
        if(word[strBuff] == 'c' || word[strBuff] == 'C') Side_A.canPlace = true;
        else if(word[strBuff] == 'f' || word[strBuff] == 'F') Side_A.canPlace = false;
        else
        {
            print_Message("Can't load Side_A placement!", true);
            output = FUNCTION_FAIL; break; 
        }
        // Get units in map
        // Quantity
        strBuff = strlen("units: ");
        fgets(word, strBuff, scen); // Ignore this part of parameter
        fscanf(scen, "%c", &word[0]);
        if(word[0] != ' ')
        {
            print_Message("Can't read Side_A placement!", true);
            output = FUNCTION_FAIL; break;  
        }
        fscanf(scen, "%d", &unit_N);
        if(unit_N < 1)
        {
            print_Message("Can't load Side_A placement!", true);
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
        lvlFree++;

        // Side_B
        // Name
        strBuff = strlen("sideB: ");
        fgets(word, sizeof(word), scen);
        word[strlen(word)-1] = '\0';
        if(strlen(word) < strBuff)
        {
            print_Message("Can't load Side_B name!", true);
            output = FUNCTION_FAIL; break; 
        }
        strcpy(Side_B.name, word+strBuff);
        // How to place units
        strBuff = strlen("placement: ");
        fgets(word, sizeof(word), scen);
        word[strlen(word)-1] = '\0';
        if(strlen(word) < strBuff)
        {
            print_Message("Can't read Side_B placement!", true);
            output = FUNCTION_FAIL; break; 
        }
        if(word[strBuff] == 'c' || word[strBuff] == 'C') Side_B.canPlace = true;
        else if(word[strBuff] == 'f' || word[strBuff] == 'F') Side_B.canPlace = false;
        else
        {
            print_Message("Can't load Side_B placement!", true);
            output = FUNCTION_FAIL; break; 
        }
        // Get units in map
        // Quantity
        strBuff = strlen("units: ");
        fgets(word, strBuff, scen); // Ignore this part of parameter
        fscanf(scen, "%c", &word[0]);
        if(word[0] != ' ')
        {
            print_Message("Can't read Side_B placement!", true);
            output = FUNCTION_FAIL; break;  
        }
        fscanf(scen, "%d", &unit_N);
        if(unit_N < 1)
        {
            print_Message("Can't load Side_B placement!", true);
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
        lvlFree++;
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
                print_Line(word);
            } while (!feof(scen));
            print_Line(" ");
            fillSpace_ToBottom(3);
            print_Line("[ARROW KEY] Change map | [ENTER] Select Map | [ESC] Return To Menu");
            print_Line(" ");
            print_Line(NULL);
        }
        break;
    }

    // Treating free levels
    if(lvlFree != 4)
    {
        if(lvlFree > 0) // Map
        {
            for(int i = 0; i < battleMap.height; i++)
                free(battleMap.tiles[i]);
            free(battleMap.tiles);
        }
        if(lvlFree > 1) // Side_B Units
            free(Side_B.units);
        if(lvlFree > 2) // Side_A Units
            free(Side_A.units);
        if(lvlFree > 3) // Unit Table
            free(unit_Table);
    }

    // Get number of maps in scenario
    if(output != FUNCTION_FAIL)
    {
        rewind(scen);
        while(1)
        {
            fgets(word, sizeof(word), scen);
            if(feof(scen)) break;
            if(word[0] == '$')
                output++;
        }
    }
    fclose(scen);

    return output;
}

int placementMenu(B_Map *map, B_Side *Side)
{
    int mode = MODE_UNITS, Index = -1, out = 0, i = 0;
    short int dUnits = 0;
    do
    {
        Index = -1;
        B_Pos unitPos = {-1, -1};
        Map_Unit unit;
        system("cls");
        toggle_Cursor(false);

        printf(">> ");
        switch (mode)
        {
        case MODE_HEIGHT:
            printf("Heigth Map");
            break;
        case MODE_TERRAIN:
            printf("Terrain Map");
            break;
        case MODE_VEGETAT:
            printf("Vegetation Map");
            break;
        case MODE_UNITS:
            printf("Units Map");
            break;
        }
        printf("\n");

        show_Map(map, mode, false);
        print_Line(NULL);
        print_Line(" ");
        print_Line("[T] Change Map Type  | [A] Place A Unit");
        print_Line("[Esc] Exit To Menu   | [M] Replace Unit");
        print_Line("[Enter] Start Battle | [R] Remove Unit");
        print_Line(" ");
        print_Line(NULL);

        out = get_KeyPress(true);
        switch (out)
        {
        case 't':
            mode++;
            if (mode > MODE_UNITS)
                mode = MODE_HEIGHT;
            continue;
        case 'a':
            printf(">> Placeable units: \n");
            for (i = 0; i < Side->size; i++)
                if (Side->units[i].position.X < 0 && Side->units[i].position.Y < 0)
                    show_gUnit(&Side->units[i]);
            if (i < 0)
            {
                print_Message("No units in the reserve!", true);
                continue;
            }

            toggle_Cursor(true);
            printf(">> ID of the chosen unit: ");
            scanf(" %d", &out);
            toggle_Cursor(false);

            Index = get_UnitIndex(Side, out);
            if (Index > -1 && (Side->units[Index].position.X < 0 && Side->units[Index].position.Y < 0))
            {
                toggle_Cursor(true);
                printf(">> Cordinates of the unit <X Y> inside |*1| area: ");
                scanf(" %hd %hd", &Side->units[Index].position.X, &Side->units[Index].position.Y);
                toggle_Cursor(false);

                unit = set_MapUnit(&Side->units[Index]);
                out = put_Unit_OnMap(map, &unit, false);
                if (out == FUNCTION_FAIL)
                {
                    print_Message("Invalid coordinates!", true);
                    Side->units[Index].position.X = -1, Side->units[Index].position.Y = -1;
                }
                else
                {
                    print_Message("Unit placed sucessfully!", true);
                    dUnits++;
                }
            }
            else
                print_Message("This unit ID is not valid!", true);
            continue;
        case 'm':
            for (i = 0; i < Side->size; i++)
                if (Side->units[i].position.X >= 0 && Side->units[i].position.Y >= 0)
                {
                    printf(">> At %dX - %dY \n", Side->units[i].position.X, Side->units[i].position.Y);
                    show_Unit(Side->units[i]);
                }
            if (i < 0)
            {
                print_Message("No units in the field!", true);
                continue;
            }
            toggle_Cursor(true);
            printf(">> ID of the chosen unit: ");
            scanf("%d", &out);
            toggle_Cursor(false);

            Index = get_UnitIndex(Side, out);
            if (Index > -1 && (Side->units[Index].position.X > -1 && Side->units[Index].position.Y > -1))
            {
                unitPos.X = Side->units[Index].position.X, unitPos.Y = Side->units[Index].position.Y;
                toggle_Cursor(true);
                printf(">> Cordinates of the unit <X Y> inside |*1| area: ");
                scanf("%hd %hd", &Side->units[Index].position.X, &Side->units[Index].position.Y);
                toggle_Cursor(false);

                unit = set_MapUnit(&Side->units[Index]);
                out = put_Unit_OnMap(map, &unit, false);
                if (out == FUNCTION_FAIL)
                {
                    Side->units[Index].position.X = unitPos.X, Side->units[Index].position.Y = unitPos.Y;
                    print_Message("Invalid coordinates!", true);
                }
                else
                {
                    map->tiles[unitPos.Y][unitPos.X].unit.ID = NO_UNIT;
                    print_Message("Unit moved sucessfully!", true);
                }
            }
            else
                print_Message("This unit ID is not valid!", true);
            continue;
        case 'r':
            for (i = 0; i < Side->size; i++)
                if (Side->units[i].position.X >= 0 && Side->units[i].position.Y >= 0)
                {
                    printf(">> At %dX - %dY \n", Side->units[i].position.X, Side->units[i].position.Y);
                    show_Unit(Side->units[i]);
                }
            if (i < 0)
            {
                print_Message("No units in the field!", true);
                continue;
            }

            toggle_Cursor(true);
            printf(">> ID of the chosen unit: ");
            scanf("%d", &out);
            toggle_Cursor(false);

            Index = get_UnitIndex(Side, out);
            if (Index > -1 && (Side->units[Index].position.X > -1 && Side->units[Index].position.Y > -1))
            {
                unitPos.X = Side->units[Index].position.X, unitPos.Y = Side->units[Index].position.Y;
                map->tiles[unitPos.Y][unitPos.X].unit.ID = NO_UNIT;
                Side->units[Index].position.X = -1, Side->units[Index].position.Y = -1;
                print_Message("Unit removed sucessfully!", true);
                dUnits--;
            }
            else
                print_Message("This unit ID is not valid!", true);
            continue;
        case KEY_ESCAPE:
            return FUNCTION_FAIL;
        case KEY_ENTER:
            // Cheking if any units are deployed at all
            if (dUnits < 1)
            {
                print_Message("You must deploy at least one unit!", true);
                continue;
            }

            // Cheking for undeployed units
            if (dUnits < Side->size)
            {
                print_Message("There are undeployed units! Proceed? [Y/N]", false);
                out = get_KeyPress(true);
                if (out != 'y')
                    continue;
            }
            return FUNCTION_SUCESS;
        default:
            print_Message("Invalid action!", true);
            continue;
        }
    } while (1);
}

int do_Turn(B_Side *player, B_Side *opponent, B_Map *battleMap, int unitA_I, int turn);

int main(/*int argc, char** argv*/)
{
    // Randomizing seed
    srand(time(NULL));

    // Setting up terminal
    SetConsoleTitle("Total Terminal War");
    toggle_Cursor(false);

    int nMaps = 0, cScen = 0, cMap = 0, out = 0;
    extern short int A_Loss, B_Loss;
    extern short int xHiLi, yHiLi;
    A_Loss = 0, B_Loss = 0;
    // unit_Table = getFile_Unit("units/new2.bin", &unit_TableSize);

    // Side_A
    Side_A.size = 0, Side_A.ID = 0, Side_A.units = NULL, Side_A.isAI = false;
    // Side_B
    Side_B.size = 0, Side_B.ID = 1, Side_B.units = NULL, Side_B.isAI = true;

    // Playing music
    PlaySound("sound/Menu.wav", NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);

startMenu:
    cMap = 0;
    do
    {
        switch (screen_Menu(VERSION))
        {
        case 'i':
            cScen = screen_Scenery();
            do
            {   // Scroll through maps
                Side_A.units = (B_Unit *)malloc(sizeof(B_Unit));
                Side_B.units = (B_Unit *)malloc(sizeof(B_Unit));
                nMaps = load_Scenery(cScen, cMap);
                if(nMaps < 1) // Error
                {
                    free(Side_A.units), free(Side_B.units);
                    goto startMenu;
                }
                cMap = screen_MapInput(cMap, nMaps);
                if(cMap == FUNCTION_SUCESS) break;
                else if(cMap == FUNCTION_FAIL)
                {
                    dealloc_ToMenu();
                    goto startMenu;
                }
                else dealloc_ToMenu();
            } while (1);
            
            // out = getFile_Map("maps/Catalon.map", &battleMap);
            // if (out != FUNCTION_SUCESS)
            //     return FUNCTION_FAIL;

            // Continue
            break;
        case 's':
            free(unit_Table);
            return 0;
        default:
            continue;
        }
        break;
    } while (1);

    xHiLi = NO_UNIT, yHiLi = NO_UNIT;
    Status_A.name = Side_A.name, Status_B.name = Side_B.name;

    // Setting up some units
    // set_fUnitTable(unit_Table, 0, &Side_A);
    // set_fUnitTable(unit_Table, 4, &Side_A);
    // set_fUnitTable(unit_Table, 0, &Side_A);
    // set_fUnitTable(unit_Table, 1, &Side_B);
    // set_fUnitTable(unit_Table, 1, &Side_B);
    // set_fUnitTable(unit_Table, 2, &Side_B);

    // Placing AI on Map
    Map_Unit unitB, unitA;
    if(Side_B.isAI == true)
    {
        for (int i = 0; i < Side_B.size; i++)
        {
            unitB = def_Unit(&Side_B.units[i], &battleMap, !Side_B.canPlace);
            put_Unit_OnMap(&battleMap, &unitB, !Side_B.canPlace);
        }
    }
    // Placing Player on map
    if (placementMenu(&battleMap, &Side_A) == FUNCTION_FAIL)
    {
        (void)dealloc_ToMenu();
        goto startMenu;
    }

    // Getting deployed troops
    for (int i = 0; i < Side_A.size; i++)
        if(Side_A.units[i].position.X != NO_UNIT && Side_A.units[i].position.Y != NO_UNIT)
            Status_A.deployed += Side_A.units[i].men;
    for (int i = 0; i < Side_B.size; i++)
        if(Side_A.units[i].position.X != NO_UNIT && Side_A.units[i].position.Y != NO_UNIT)
            Status_B.deployed += Side_B.units[i].men;

    // Game Starts
    int mode = MODE_HEIGHT;
    PlaySound("sound/Game1.wav", NULL, SND_ASYNC | SND_LOOP | SND_FILENAME);
    system("cls");
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
        if(do_Turn(&Side_A, &Side_B, &battleMap, unitA_I, i) == FUNCTION_FAIL)
            break;
        unitA_I++;

        // Get next unit
        for(int index = 0; index < Side_B.size; index++)
        {
            if(unitB_I >= Side_B.size)
                unitB_I = 0;
            if(Side_B.units[unitB_I].position.X == NO_UNIT || Side_B.units[unitB_I].position.Y == NO_UNIT)
                unitB_I++;
            else break;
        } // Side_B turn
        if (do_Turn(&Side_B, &Side_A, &battleMap, unitB_I, i) == FUNCTION_FAIL)
            break;
        unitB_I++;
        
        // Cheking for retreat
        for (int j = 0; j < Side_A.size && j < Side_B.size; j++)
        {
            bool retreated = true;
            char msg[STRING_NAME];
            if (Side_A.units[j].isRetreating == true)
            {
                unitA = set_MapUnit(&Side_A.units[j]);
                snprintf(msg, sizeof(msg), "%3d", battleMap.tiles[unitA.Y][unitA.X].elevation);
                update_Map(unitA.X, unitA.Y, msg);
                for (int moves = 0; moves < Side_A.units[j].moves; moves++)
                {
                    retreated = unit_Retreat(&unitA, &battleMap);
                    update_Unit(&Side_A.units[j], unitA);
                    if (retreated == false)
                    {
                        battleMap.tiles[unitA.Y][unitA.X].unit.ID = NO_UNIT;
                        delete_Unit(Side_A.units, &Side_A.size, j);
                        break;
                    }
                    update_Map(unitA.X, unitA.Y, unitA.name);
                }
            }
            if (Side_B.units[j].isRetreating == true)
            {
                unitB = set_MapUnit(&Side_B.units[j]);
                snprintf(msg, sizeof(msg), "%3d", battleMap.tiles[unitB.Y][unitB.X].elevation);
                update_Map(unitB.X, unitB.Y, msg);
                for (int moves = 0; moves < Side_B.units[j].moves; moves++)
                {
                    clear_afterMap(battleMap.height);
                    retreated = unit_Retreat(&unitB, &battleMap);
                    update_Unit(&Side_B.units[j], unitB);
                    if (retreated == false)
                    {
                        battleMap.tiles[unitB.Y][unitB.X].unit.ID = NO_UNIT;
                        delete_Unit(Side_B.units, &Side_B.size, j);
                        break;
                    }
                    update_Map(unitB.X, unitB.Y, unitB.name);
                }
            }
        }
        // Checking for victory
        if (Side_A.size == 0)
        {
            screen_Victory(Status_B, Status_A);
            while (get_KeyPress(false) != KEY_ENTER)
                continue;
            break;
        }
        else if (Side_B.size == 0)
        {
            screen_Victory(Status_A, Status_B);
            while (get_KeyPress(false) != KEY_ENTER)
                continue;
            break;
        }
    }

    // Freeing
    (void)dealloc_ToMenu();
    // Playing music
    PlaySound("sound/Menu.wav", NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
    goto startMenu;

    return 0;
}

int do_Turn(B_Side *player, B_Side *opponent, B_Map *battleMap, int unitA_I, int turn)
{
    static int mode = MODE_HEIGHT;
    B_Tile *position = NULL;   
    int target_I = 0;
    char action = 'x', msg[STRING_NAME];

    // Setting up unit
    if(player->units[unitA_I].position.Y == NO_UNIT || player->units[unitA_I].position.X == NO_UNIT)
        return FUNCTION_FAIL;
    Map_Unit unitA = battleMap->tiles[player->units[unitA_I].position.Y][player->units[unitA_I].position.X].unit, unitB;
    if(player->isAI == false) // Player Zone
    {
        unitA = set_MapUnit(&player->units[unitA_I]);
        for (int moves = 0; moves < player->units[unitA_I].moves; moves++)
        {
            short int xGoal = NO_UNIT, yGoal = NO_UNIT, xTarget = NO_UNIT, yTarget = NO_UNIT;
            int FRes = false;
            char msg[STRING_NAME];
            xHiLi = unitA.X, yHiLi = unitA.Y;
            info_Upper(battleMap->name, turn, player->name, true, unitA.name, unitA.ID, unitA.X, unitA.Y, player->units[unitA_I].moves - moves);
            if (moves < player->units[unitA_I].moves && player->units[unitA_I].isRetreating == false && player->units[unitA_I].inCombat == false)
            {
                toggle_Cursor(false);
                fflush(stdin);
                while(1)
                {
                    if(_kbhit())
                    {
                        action = _getch();
                        break;
                    }
                    char msg[3] = {219, 219, 219};
                    update_Map(unitA.X, unitA.Y, msg);
                    Sleep(500);
                    toggle_Cursor(false);
                    if(_kbhit())
                    {
                        action = _getch();
                        break;
                    }
                    update_Map(unitA.X, unitA.Y, unitA.name);
                    Sleep(500);
                }
                COORD pos = {0, 9+battleMap->height*2};
                if (action >= '0' && action <= '9') // Move to a adjacent tile
                {
                    tNum_ToDirec(&action);
                    position = get_AdjTile(battleMap, unitA, action, false);
                    player->units[unitA_I].goal.X = position->unit.X, player->units[unitA_I].goal.Y = position->unit.Y;
                    player->units[unitA_I].chaseID = NULL;
                }
                else if (action == 'a') // Move to a location
                {
                    clear_afterMap(battleMap->height);
                    toggle_Cursor(true);
                    printf(">> Goal coordinates <X Y> \n");
                    printf(" >=> ");
                    scanf("%hd %hd", &xGoal, &yGoal);
                    toggle_Cursor(false);
                    if(xGoal < 0 || yGoal < 0 || xGoal >= battleMap->width || yGoal >= battleMap->height)
                    {
                        print_Message("Invalid Coords", true);
                        moves--; continue;
                    }
                    else if(!autoMove(battleMap, &battleMap->tiles[unitA.Y][unitA.Y], &battleMap->tiles[yGoal][xGoal]))
                    {
                        snprintf(msg, sizeof(msg), "Can't find a path to %dX %dY", xGoal, yGoal);
                        print_Message(msg, true);
                        moves--; continue;
                    }
                    else
                    {
                        player->units[unitA_I].goal.X = xGoal, player->units[unitA_I].goal.Y = yGoal;
                        print_Message("Moving to tile!", true);
                    }
                }
                else if (action == 's') // Intercept a unit
                {
                    clear_afterMap(battleMap->height);
                    toggle_Cursor(true);
                    printf(">> Target unit coordinates <X Y> \n");
                    printf(" >=> ");
                    scanf("%hd %hd", &xGoal, &yGoal);
                    toggle_Cursor(false);
                    if(xGoal < 0 || yGoal < 0 || xGoal >= battleMap->width || yGoal >= battleMap->height)
                    {
                        print_Message("Invalid Coords", true);
                        moves--; continue;
                    }
                    else if(battleMap->tiles[yGoal][xGoal].unit.ID == NO_UNIT)
                    {
                        print_Message("Target Not Found!", true);
                        moves--; continue;
                    }
                    else if(!autoMove(battleMap, &battleMap->tiles[unitA.Y][unitA.Y], &battleMap->tiles[yGoal][xGoal]))
                    {
                        snprintf(msg, sizeof(msg), "Can't find a path to %dX %dY", xGoal, yGoal);
                        print_Message(msg, true);
                        moves--; continue;
                    }
                    else
                    {
                        target_I = get_UnitIndex(opponent, battleMap->tiles[yGoal][xGoal].unit.ID);
                        player->units[unitA_I].chaseID = &opponent->units[target_I].ID;
                        player->units[unitA_I].goal.X = xGoal, player->units[unitA_I].goal.Y = yGoal;
                        print_Message("Moving to intercept the unit!", true);
                    }
                }
                else if (action == 'f') // Fire at unit
                {
                    if (player->units[unitA_I].range < 1)
                    {
                        print_Message("This unit can't do ranged attacks!", true);
                        moves--; continue;
                    }
                    clear_afterMap(battleMap->height);
                    toggle_Cursor(true);
                    printf(">> Target coodiantes <X Y> \n");
                    printf(" >=> ");
                    scanf("%hd %hd", &xTarget, &yTarget);
                    toggle_Cursor(false);
                    if (xTarget < 0 || xTarget >= battleMap->width || yTarget < 0 || yTarget >= battleMap->height)
                    {
                        print_Message("Coordinates out of boundaries!", true);
                        moves--; continue;
                    }
                    else if(battleMap->tiles[yTarget][xTarget].unit.ID == NO_UNIT)
                    {
                        print_Message("There's nothing to attack here!", true);
                        moves--; continue;
                    }
                    else if(battleMap->tiles[yTarget][xTarget].unit.ID % 2 == unitA.ID % 2)
                    {
                        print_Message("These are our own troops Sir!", true);
                        moves--; continue;
                    }
                    else
                    {
                        unitB = battleMap->tiles[yTarget][xTarget].unit;
                        // Show
                        update_Map(unitA.X, unitA.Y, "XXX");
                        Sleep(TIME_MAP);
                        update_Map(unitB.X, unitB.Y, "OOO");
                        Sleep(TIME_MAP);
                        clear_afterMap(battleMap->height);
                        // Go
                        target_I = get_UnitIndex(opponent, unitB.ID);
                        int tVeget = getTile_Vegetat(battleMap, unitB), tHeight = get_HeightDif(battleMap, unitA, unitB);
                        short int *tFort = &battleMap->tiles[unitB.Y][unitB.X].fortLevel;
                        FRes = do_Combat_Ranged(&player->units[unitA_I], &opponent->units[target_I], tHeight, tVeget, tFort);
                        // Done
                        if(FRes == FUNCTION_SUCESS)
                        {
                            show_gUnit(&player->units[unitA_I]);
                            show_gUnit(&opponent->units[target_I]);
                            printf(">> Press ENTER to continue ");
                            Status_A.loss += A_Loss, Status_B.loss += B_Loss;
                            while (get_KeyPress(false) != KEY_ENTER) continue;
                            system("cls");
                            show_Map(battleMap, mode, true);
                            moves = Side_A.units[unitA_I].moves;
                        }
                        else moves--;
                        update_Map(unitA.X, unitA.Y, unitA.name);
                        update_Map(unitB.X, unitB.Y, unitB.name);
                    }
                    continue;
                }
                else if (action == 'd') // View unit stats
                {
                    reset_Cursor();
                    system("cls");
                    show_gUnit(&player->units[unitA_I]);
                    printf(">> Press ENTER to continue \n");
                    while (get_KeyPress(false) != KEY_ENTER) continue;
                    system("cls");
                    show_Map(battleMap, mode, true);
                    moves--;
                }
                else if (action == 'e') // Fortify
                {
                    clear_afterMap(battleMap->height);
                    if (player->units[unitA_I].build_Cap > 0)
                    {
                        if (inc_FortLevel(battleMap, player->units[unitA_I].build_Cap, unitA) == FUNCTION_FAIL)
                            moves--;
                    }
                    else
                    {
                        print_Message("This unit cannot build fortifications!", true);
                        moves--;
                    }
                }
                else if (action == 'g') // See tile stats
                {
                    clear_afterMap(battleMap->height);
                    toggle_Cursor(true);
                    printf(">> Target coodiantes <X Y> \n");
                    printf(" >=> ");
                    scanf("%hd %hd", &xTarget, &yTarget);
                    getchar();
                    toggle_Cursor(false);
                    if (xTarget >= 0 && xTarget < battleMap->width && yTarget >= 0 && yTarget < battleMap->height)
                    {
                        print_Line(NULL);
                        print_Line(" ");
                        snprintf(msg, sizeof(msg), "Terrain: %s", tTerrain_toStr(battleMap->tiles[yTarget][xTarget].terrain));
                        print_Line(msg);
                        snprintf(msg, sizeof(msg), "Vegetation: %s", tVeget_toStr(battleMap->tiles[yTarget][xTarget].vegetation));
                        print_Line(msg);
                        snprintf(msg, sizeof(msg), "Elevation: %d", battleMap->tiles[yTarget][xTarget].elevation);
                        print_Line(msg);
                        snprintf(msg, sizeof(msg), "Fortification: %d", battleMap->tiles[yTarget][xTarget].fortLevel);
                        print_Line(msg);
                        snprintf(msg, sizeof(msg), "Unit in here: %d", battleMap->tiles[yTarget][xTarget].unit.ID);
                        print_Line(msg);
                        print_Line(" ");
                        print_Line(NULL);
                        printf(">> Press ENTER to continue \n");
                        while (get_KeyPress(false) != KEY_ENTER) continue;
                        system("cls");
                        show_Map(battleMap, mode, true);
                        moves--;
                    }
                    else
                    {
                        print_Message("Coordinates out of boundaries!", true);
                        moves--;
                    }
                }
                else if (action == KEY_ESCAPE) // Return to Menu
                {
                    (void)dealloc_ToMenu();
                    PlaySound("sound/Menu.wav", NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
                    return FUNCTION_FAIL;
                }
                else if (action == KEY_ENTER) // Next Turn
                {
                    if(player->units[unitA_I].goal.X == NO_UNIT || player->units[unitA_I].goal.Y == NO_UNIT)
                        moves = player->units[unitA_I].moves;
                }
                else // Invalid
                {
                    clear_afterMap(battleMap->height);
                    print_Message("Invalid action!", true);
                    moves--; continue;
                }
                xGoal = player->units[unitA_I].goal.X, yGoal = player->units[unitA_I].goal.Y;
                if (xGoal > -1 && yGoal > -1 && xGoal < battleMap->width && yGoal < battleMap->height)
                {
                    pos.X = player->units[unitA_I].position.X, pos.Y = player->units[unitA_I].position.Y;
                    snprintf(msg, sizeof(msg), "%3d", battleMap->tiles[pos.Y][pos.X].elevation);
                    update_Map(player->units[unitA_I].position.X, player->units[unitA_I].position.Y, msg);
                    // Change to coordinates of unitB if unitA is chasing
                    if (player->units[unitA_I].chaseID != NULL)
                    {
                        target_I = get_UnitIndex(opponent, *player->units[unitA_I].chaseID);
                        player->units[unitA_I].goal.X = opponent->units[target_I].position.X, player->units[unitA_I].goal.Y = opponent->units[target_I].position.Y;
                        xGoal = player->units[unitA_I].goal.X, yGoal = player->units[unitA_I].goal.Y;
                    }
                    player->units[unitA_I].path = (int *)autoMove(battleMap, &battleMap->tiles[unitA.Y][unitA.X], &battleMap->tiles[yGoal][xGoal]);
                    if (player->units[unitA_I].path != NULL)
                    {
                        for (int steps = 0; moves < player->units[unitA_I].moves; steps++)
                        {
                            if ((unitA.X == xGoal && unitA.Y == yGoal) || FRes == OUT_COMBAT)
                            {
                                player->units[unitA_I].goal.X = NO_UNIT, player->units[unitA_I].goal.Y = NO_UNIT;
                                moves--; break;
                            }
                            FRes = move_Unit(battleMap, &unitA, player->units[unitA_I].path[steps]);
                            if (FRes == OUT_COMBAT)
                            {
                                // Show
                                update_Map(unitA.X, unitA.Y, "XXX");
                                Sleep(TIME_MAP);
                                position = get_AdjTile(battleMap, unitA, player->units[unitA_I].path[steps], false);
                                update_Map(position->unit.X, position->unit.Y, "OOO");
                                Sleep(TIME_MAP);
                                // Glow
                                system("cls");   
                                snprintf(msg, sizeof(msg), "Trying engagement at %3dX %3dY", position->unit.X, position->unit.Y);
                                print_Message(msg, true);
                                // Go
                                unitB = position->unit;
                                target_I = get_UnitIndex(opponent, position->unit.ID);
                                do_Combat(&player->units[unitA_I], &opponent->units[target_I], get_HeightDif(battleMap, unitA, unitB), &battleMap->tiles[unitB.Y][unitB.X].fortLevel);
                                // Done
                                show_gUnit(&player->units[unitA_I]);
                                show_gUnit(&opponent->units[target_I]);
                                Status_A.loss += A_Loss, Status_B.loss += B_Loss;
                                printf(">> Press ENTER to continue ");
                                while (get_KeyPress(false) != KEY_ENTER) continue;
                                system("cls");
                                show_Map(battleMap, mode, true);
                            }
                            else if (FRes == FUNCTION_FAIL)
                            {
                                moves--; break;
                            }
                            else if (FRes > -1)
                                moves += FRes+1;
                        }
                        free(player->units[unitA_I].path);
                    }
                    else
                    {
                        clear_afterMap(battleMap->height);
                        print_Message("Can't move in this direction!", true);
                        moves--;
                    }
                }
                update_Map(unitA.X, unitA.Y, unitA.name);
                if ((unitA.X == xGoal && unitA.Y == yGoal) || FRes == OUT_COMBAT)
                {
                    player->units[unitA_I].chaseID = NULL;
                    player->units[unitA_I].goal.X = -1, player->units[unitA_I].goal.Y = -1;
                    if(position == NULL)
                    {
                        clear_afterMap(battleMap->height);
                        print_Message("Auto move disabled!", true);
                    }
                }
                position = NULL;
                update_Unit(&player->units[unitA_I], unitA);
                toggle_Cursor(false);
            }
            else
            {
                player->units[unitA_I].inCombat = false;
                snprintf(msg, sizeof(msg), "Disegaging unit from combat!");
                print_Message(msg, true);
            }
        }
    }
    else // AI Zone
    {
        unitA = set_MapUnit(&player->units[unitA_I]);
        B_Tile *position;
        xHiLi = NO_UNIT, yHiLi = NO_UNIT;
        for (int moves = 0; moves < player->units[unitA_I].moves; moves++)
        {
            // show_Map(&battleMap, MODE_HEIGHT);
            int FRes = false;
            info_Upper(battleMap->name, turn, player->name, false, unitA.name, unitA.ID, unitA.X, unitA.Y, NO_UNIT);
            update_Map(unitA.X, unitA.Y, unitA.name);
            Sleep(TIME_STRATEGY);
            COORD pos = {0, 9+battleMap->height*2};

            if (moves < player->units[unitA_I].moves && player->units[unitA_I].isRetreating == false && player->units[unitA_I].inCombat == false)
            {
                if (unitA.Y > 10)
                {
                    snprintf(msg, sizeof(msg), "%3d", battleMap->tiles[unitA.Y][unitA.X].elevation);
                    update_Map(unitA.X, unitA.Y, msg);
                    FRes = move_Unit(battleMap, &unitA, Northeast);
                }
                else
                {
                    clear_afterMap(battleMap->height);
                    inc_FortLevel(battleMap, player->units[unitA_I].build_Cap, unitA);
                }
                if (FRes == OUT_COMBAT)
                {
                    // Show
                    update_Map(unitA.X, unitA.Y, "XXX");
                    Sleep(TIME_MAP);
                    position = get_AdjTile(battleMap, unitA, Northeast, false);
                    update_Map(position->unit.X, position->unit.Y, "OOO");
                    Sleep(TIME_MAP);
                    // Glow
                    system("cls");   
                    unitB = position->unit;
                    snprintf(msg, sizeof(msg), "Trying engagement at %3dX %3dY", unitB.X, unitB.Y);
                    print_Message(msg, true);
                    // Go
                    target_I = get_UnitIndex(opponent, unitB.ID);
                    do_Combat(&player->units[unitA_I], &opponent->units[target_I], get_HeightDif(battleMap, unitA, unitB), &battleMap->tiles[unitA.Y][unitA.X].fortLevel);
                    Status_B.loss += A_Loss, Status_A.loss += B_Loss;
                    show_gUnit(&player->units[unitA_I]);
                    show_gUnit(&opponent->units[target_I]);
                    printf(">> Press ENTER to continue ");
                    while (get_KeyPress(false) != KEY_ENTER) continue;
                    system("cls");
                    show_Map(battleMap, mode, true);
                }
                else if (FRes > -1)
                    moves += FRes+1;
                update_Unit(&player->units[unitA_I], unitA);
                update_Map(unitA.X, unitA.Y, unitA.name);
            }
            else
            {
                player->units[unitA_I].inCombat = false;
                snprintf(msg, sizeof(msg), "Disegaging unit from combat!");
                print_Message(msg, true);
            }
            info_Upper(battleMap->name, turn, Side_B.name, false, unitA.name, unitA.ID, unitA.X, unitA.Y, NO_UNIT);
            update_Map(unitA.X, unitA.Y, unitA.name);
            Sleep(TIME_STRATEGY);
        }
    }
    return FUNCTION_SUCESS;
}