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
#define TURNS 30
#define VERSION 1.75f

// Game Variables
B_Map battleMap;
B_Side Side_A, Side_B;
B_Unit *unit_Table = NULL;
int unit_TableSize = 0;
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
    char word[STRING_DESC] = {0}, scen_Name[STRING_NAME] = {0}, scen_Map[STRING_NAME] = {0};
    int cMap = 0, strBuff = 0, unit_N = NO_UNIT;
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
        // Try to get Music
        strBuff = strlen("music: ");
        fgets(word, sizeof(word), scen);
        word[strlen(word)-1] = '\0';
        if(strlen(word) < strBuff)
        {
            print_Message("Can't load the units!", true);
            output = FUNCTION_FAIL; break; 
        }
        if(word[4] == 'c' && word[0] == 'm')    // Music detected
        {
            strcpy(nowPlaying, word+7);
            fgets(word, sizeof(word), scen);
            word[strlen(word)-1] = '\0';
            if(strlen(word) < strBuff)
            {
                print_Message("Can't load the units!", true);
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

    return output;
}

int placementMenu(B_Map *map, B_Side *Side, int *mode)
{
    system("cls");
    int Index = -1, out = 0, i = 0;
    short int dUnits = 0;
    do
    {
        Index = -1;
        B_Pos unitPos = {-1, -1};
        reset_Cursor();
        toggle_Cursor(false);

        printf(">> ");
        switch (*mode)
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
        case MODE_GRAPHIC:
            printf("Battle Map");
            break;
        }
        printf("\n");

        show_Map(map, *mode, false);
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
            (*mode)++;
            if (*mode > MODE_UNITS)
                *mode = MODE_HEIGHT;
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

                out = put_Unit_OnMap(map, &Side->units[Index], false);
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
                    show_gUnit(&Side->units[i]);
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

                out = put_Unit_OnMap(map, &Side->units[Index], false);
                if (out == FUNCTION_FAIL)
                {
                    Side->units[Index].position.X = unitPos.X, Side->units[Index].position.Y = unitPos.Y;
                    print_Message("Invalid coordinates!", true);
                }
                else
                {
                    map->tiles[unitPos.Y][unitPos.X].unit = NULL;
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
                    show_gUnit(&Side->units[i]);
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
                map->tiles[unitPos.Y][unitPos.X].unit = NULL;
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
        else if (result == OUT_COMBAT)
        {
            // Show
            update_Map(pos_A.X, pos_A.Y, "XXX");
            Sleep(TIME_MAP);
            pos_B = get_AdjTile_Pos(map, pos_A, unit->path[steps]);
            update_Map(pos_B.X, pos_B.Y, "OOO");
            Sleep(TIME_MAP);
            // Glow
            system("cls");   
            snprintf(msg, sizeof(msg), "Trying engagement at %3dX %3dY", pos_B.X, pos_B.Y);
            print_Message(msg, true);
            // Go
            int target_I = get_UnitIndex(opponent, map->tiles[pos_B.Y][pos_B.X].unit->Game_ID);
            do_Combat(unit, &player->stats,
                      &opponent->units[target_I], &opponent->stats,
                      get_HeightDif(map, pos_A, pos_B),
                      &map->tiles[pos_B.Y][pos_B.X].fortLevel);
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
    if(pos_A.Y == NO_UNIT || pos_A.X == NO_UNIT)
        return FUNCTION_FAIL;
    if(player->isAI == false) // Player Zone
    {
        for (int moves = 0; moves < player->units[cUnit_I].moves; moves++)
        {
            pos_A = player->units[cUnit_I].position;
            info_Upper (battleMap->name, turn, player->name, true, player->units[cUnit_I].name,
                        player->units[cUnit_I].Game_ID, pos_A.X, pos_A.Y, player->units[cUnit_I].moves - moves);

            if(check_UnitMove(&player->units[cUnit_I], moves) == false)
            {
                clear_afterMap(battleMap->height);
                continue;
            }

            if (player->units[cUnit_I].retreating == true)
            {
                Sleep(TIME_STRATEGY);
                // Algo gráfico pra representar o retreat
                // IMPLEMENTAR
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
                printf(">> Goal coordinates <X Y> \n");
                printf(" >=> ");
                scanf("%hd %hd", &goal.X, &goal.Y);
                toggle_Cursor(false);
                if(goal.X < 0 || goal.Y < 0 || goal.X >= battleMap->width || goal.Y >= battleMap->height)
                {
                    print_Message("Invalid Coords", true);
                    moves--; continue;
                }
                else if(!autoMove(battleMap, &battleMap->tiles[pos_A.Y][pos_A.Y], &battleMap->tiles[goal.Y][goal.X]))
                {
                    snprintf(msg, sizeof(msg), "Can't find a path to %dX %dY", goal.X, goal.Y);
                    print_Message(msg, true);
                    moves--; continue;
                }
                else
                {
                    player->units[cUnit_I].goal = goal;
                    print_Message("Moving to tile!", true);
                }
            }
            else if (action == 't') // Change Map Mode
            {
                (*mode)++;
                if(*mode > MODE_UNITS) *mode = MODE_HEIGHT;
                show_Map(battleMap, *mode, true);
                moves--; continue;
            }
            else if (action == 's') // Intercept a unit
            {
                clear_afterMap(battleMap->height);
                toggle_Cursor(true);
                printf(">> Target unit coordinates <X Y> \n");
                printf(" >=> ");
                scanf("%hd %hd", &goal.X, &goal.Y);
                toggle_Cursor(false);
                if(goal.X < 0 || goal.Y < 0 || goal.X >= battleMap->width || goal.Y >= battleMap->height)
                {
                    print_Message("Invalid Coords", true);
                    moves--; continue;
                }
                else if(battleMap->tiles[goal.Y][goal.X].unit == NULL)
                {
                    print_Message("Target Not Found!", true);
                    moves--; continue;
                }
                else if(!autoMove(battleMap, &battleMap->tiles[pos_A.Y][pos_A.Y], &battleMap->tiles[goal.Y][goal.X]))
                {
                    snprintf(msg, sizeof(msg), "Can't find a path to %dX %dY", goal.X, goal.Y);
                    print_Message(msg, true);
                    moves--; continue;
                }
                else
                {
                    target_I = get_UnitIndex(opponent, battleMap->tiles[goal.Y][goal.X].unit->Game_ID);
                    player->units[cUnit_I].chaseID = &opponent->units[target_I].Game_ID;
                    player->units[cUnit_I].goal = goal;
                    print_Message("Moving to intercept the unit!", true);
                }
            }
            else if(action == 'f') // Fire at unit
            {
                if (player->units[cUnit_I].range < 1)
                {
                    print_Message("This unit can't do ranged attacks!", true);
                    moves--; continue;
                }
                
                clear_afterMap(battleMap->height);
                toggle_Cursor(true);
                printf(">> Target coodiantes <X Y> \n");
                printf(" >=> ");
                scanf("%hd %hd", &target.X, &target.Y);
                toggle_Cursor(false);
                if((check_MapAttack(battleMap, target, player->units[cUnit_I].Game_ID) == FUNCTION_FAIL) || 
                    check_Ranged(&player->units[cUnit_I], battleMap->tiles[target.Y][target.X].unit) == FUNCTION_FAIL)
                { moves--; continue; }
                
                // Show
                pos_B = battleMap->tiles[target.Y][target.X].unit->position;
                update_Map(pos_A.X, pos_A.Y, "XXX");
                Sleep(TIME_MAP);
                update_Map(pos_B.X, pos_B.Y, "OOO");
                Sleep(TIME_MAP);
                clear_afterMap(battleMap->height);
                // Go
                target_I = get_UnitIndex(opponent, battleMap->tiles[target.Y][target.X].unit->Game_ID);
                int tVeget = getTile_Vegetat(battleMap, pos_B), tHeight = get_HeightDif(battleMap, pos_A, pos_B);
                int tTerrain = getTile_Terrain(battleMap, pos_B);
                short int *tFort = &battleMap->tiles[pos_B.Y][pos_B.X].fortLevel;
                // Done
                if(do_Combat_Ranged(&player->units[cUnit_I], &player->stats,
                                    &opponent->units[target_I], &opponent->stats,
                                    tHeight, tVeget, tTerrain, tFort) == FUNCTION_SUCESS)
                {
                    show_gUnit(&player->units[cUnit_I]);
                    show_gUnit(&opponent->units[target_I]);
                    printf(">> Press ENTER to continue ");
                    while (get_KeyPress(false) != KEY_ENTER) continue;
                    system("cls");
                    if(opponent->units[target_I].men == 0)
                    {
                        delete_Unit(opponent->units, &opponent->size, target_I);
                        battleMap->tiles[pos_B.Y][pos_B.X].unit = NULL;
                    }
                    show_Map(battleMap, *mode, true);
                }
                else moves--;
                update_Map(pos_A.X, pos_A.Y, player->units[cUnit_I].name);
                update_Map(pos_B.X, pos_B.Y, get_MapSprite(&battleMap->tiles[pos_B.Y][pos_B.X], *mode));
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
                clear_afterMap(battleMap->height);
                if (player->units[cUnit_I].build_Cap > 0)
                {
                    if (inc_FortLevel(battleMap, player->units[cUnit_I].build_Cap, pos_A) == FUNCTION_FAIL)
                        moves--;
                    else moves = player->units[cUnit_I].moves;
                }
                else
                {
                    print_Message("This unit cannot build fortifications!", true);
                    moves--; continue;
                }
                clear_afterMap(battleMap->height);
            }
            else if (action == 'g') // See tile stats
            {
                clear_afterMap(battleMap->height);
                toggle_Cursor(true);
                printf(">> Target coodiantes <X Y> \n");
                printf(" >=> ");
                scanf("%hd %hd", &target.X, &target.Y);
                getchar();
                toggle_Cursor(false);
                if (target.X >= 0 && target.X < battleMap->width && target.Y >= 0 && target.Y < battleMap->height)
                {
                    print_Line(NULL);
                    print_Line(" ");
                    snprintf(msg, sizeof(msg), "Terrain: %s", tTerrain_toStr(battleMap->tiles[target.Y][target.X].terrain));
                    print_Line(msg);
                    snprintf(msg, sizeof(msg), "Vegetation: %s", tVeget_toStr(battleMap->tiles[target.Y][target.X].vegetation));
                    print_Line(msg);
                    snprintf(msg, sizeof(msg), "Elevation: %d", battleMap->tiles[target.Y][target.X].elevation);
                    print_Line(msg);
                    snprintf(msg, sizeof(msg), "Fortification: %d", battleMap->tiles[target.Y][target.X].fortLevel);
                    print_Line(msg);
                    if(battleMap->tiles[target.Y][target.X].unit)
                    {
                        snprintf(msg, sizeof(msg), "Unit in here: %d (%s)", battleMap->tiles[target.Y][target.X].unit->Game_ID, battleMap->tiles[target.Y][target.X].unit->name);
                        print_Line(msg);
                    }
                    print_Line(" ");
                    print_Line(NULL);
                    printf(">> Press ENTER to continue \n");
                    while (get_KeyPress(false) != KEY_ENTER) continue;
                    system("cls");
                    show_Map(battleMap, *mode, true);
                    moves--; continue;
                }
                else
                {
                    print_Message("Coordinates out of boundaries!", true);
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
                print_Message("Invalid action!", true);
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
                    print_Message("No path to this tile!", true);
                    moves--; continue;
                }

                int res = handleMove(battleMap, &player->units[cUnit_I], &moves, player, opponent, *mode);
                free(player->units[cUnit_I].path);
                goal = player->units[cUnit_I].goal;
                if (moves < player->units[cUnit_I].moves-1 || res == OUT_COMBAT)
                {
                    player->units[cUnit_I].chaseID = NULL;
                    player->units[cUnit_I].goal.X = -1, player->units[cUnit_I].goal.Y = -1;
                    if(action == KEY_ENTER)
                    {
                        clear_afterMap(battleMap->height);
                        print_Message("Auto move disabled!", true);
                    }
                }
                toggle_Cursor(false);
            }
        }
        update_Map(pos_A.X, pos_A.Y, get_MapSprite(&battleMap->tiles[pos_A.Y][pos_A.X], *mode));
    }
    else // AI Zone
    {
        B_Tile *position;
        for (int moves = 0; moves < player->units[cUnit_I].moves; moves++)
        {
            // show_Map(&battleMap, MODE_HEIGHT);
            int FRes = false;
            info_Upper(battleMap->name, turn, player->name, false, player->units[cUnit_I].name, player->units[cUnit_I].Game_ID, pos_A.X, pos_A.Y, NO_UNIT);
            update_Map(pos_A.X, pos_A.Y, player->units[cUnit_I].name);
            Sleep(TIME_STRATEGY);
            pos_Screen.Y = MAP_OFFSET_Y+battleMap->height*2;

            if(check_UnitMove(&player->units[cUnit_I], moves) == false)
            {
                clear_afterMap(battleMap->height);
                continue;
            }
            
            pos_Screen.X = pos_A.X, pos_Screen.Y = pos_A.Y;
            switch(AI_Process(battleMap, player, opponent, &player->units[cUnit_I], AI_Easy))
            {
                case AI_Fire:
                    pos_B = player->units[cUnit_I].goal;
                    // Show
                    update_Map(pos_A.X, pos_A.Y, "XXX");
                    Sleep(TIME_MAP);
                    update_Map(pos_B.X, pos_B.Y, "OOO");
                    Sleep(TIME_MAP);
                    clear_afterMap(battleMap->height);
                    // Go
                    target_I = get_UnitIndex(opponent, battleMap->tiles[pos_B.Y][pos_B.X].unit->Game_ID);
                    int tVeget = getTile_Vegetat(battleMap, pos_B), tHeight = get_HeightDif(battleMap, pos_A, pos_B);
                    int tTerrain = getTile_Terrain(battleMap, pos_B);
                    short int *tFort = &battleMap->tiles[pos_B.Y][pos_B.X].fortLevel;
                    FRes = do_Combat_Ranged(&player->units[cUnit_I], &player->stats,
                                            &opponent->units[target_I], &opponent->stats,
                                            tHeight, tVeget, tTerrain, tFort);
                    // Done
                    if(FRes == FUNCTION_SUCESS)
                    {
                        show_gUnit(&player->units[cUnit_I]);
                        show_gUnit(&opponent->units[target_I]);
                        printf(">> Press ENTER to continue ");
                        while (get_KeyPress(false) != KEY_ENTER) continue;
                        system("cls");
                        if(opponent->units[target_I].men == 0)
                        {
                            delete_Unit(opponent->units, &opponent->size, target_I);
                            battleMap->tiles[pos_B.Y][pos_B.X].unit = NULL;
                        }
                        show_Map(battleMap, *mode, true);
                    }
                    else update_Map(pos_B.X, pos_B.Y, get_MapSprite(&battleMap->tiles[pos_B.Y][pos_B.X], *mode));
                    moves = player->units[cUnit_I].moves;
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
                        print_Message("No path to this tile!", true);
                        moves--; continue;
                    }

                    (void) handleMove(battleMap, &player->units[cUnit_I], &moves, player, opponent, *mode);
                    free(player->units[cUnit_I].path);
                    break;
                default:
                    FRes = move_Unit(battleMap, &player->units[cUnit_I], Northeast);
                    pos_A = player->units[cUnit_I].position;
                    break;
            }
            clear_afterMap(battleMap->height);
            pos_A = player->units[cUnit_I].position;
            update_Map(pos_Screen.X, pos_Screen.Y, get_MapSprite(&battleMap->tiles[pos_Screen.Y][pos_Screen.X], *mode));
        }
        info_Upper(battleMap->name, turn, Side_B.name, false, player->units[cUnit_I].name, player->units[cUnit_I].Game_ID, pos_A.X, pos_A.Y, NO_UNIT);
        update_Map(pos_A.X, pos_A.Y, get_MapSprite(&battleMap->tiles[pos_A.Y][pos_A.X], *mode));
        Sleep(TIME_STRATEGY);
    }
    return FUNCTION_SUCESS;
}

int main(/*int argc, char** argv*/)
{
    // Randomizing seed
    srand(time(NULL));

    // Setting up terminal
    SetConsoleTitle("Total Terminal War");
    toggle_Cursor(false);

    int nMaps = 0, cScen = 0, cMap = 0, out = 0, mode = MODE_UNITS;
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
    mode = MODE_UNITS;
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
    
    mode = MODE_GRAPHIC;
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
        
        // Checking for victory
        // Use fielded troops to get victory
        // IMPLEMENT
        fflush(stdin);
        if(Side_B.size == 0)
        {
            screen_Victory(Side_A.stats, Side_B.stats);
            while (get_KeyPress(false) != KEY_ENTER)
                continue;
            break;
        }

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
        // Use fielded troops to get victory
        // IMPLEMENT
        int remainingA, remainingB;
        fflush(stdin);
        for(remainingA = Side_A.size-1; remainingA >= 0; remainingA--)
        {
            if(compPos(Side_A.units[remainingA].position, (B_Pos){-1, -1}) == false)
                break;
            else if(remainingA == 0)
            { remainingA = -1; break; }
        }
        for(remainingB = Side_B.size-1; remainingB >= 0; remainingB--)
        {
            if(compPos(Side_B.units[remainingB].position, (B_Pos){-1, -1}) == false)
                break;
            else if(remainingB == 0)
            { remainingB = -1; break; }
        }
        if(remainingA < 0)
        { screen_Victory(Side_B.stats, Side_A.stats); break; }
        else if(remainingB < 0)
        { screen_Victory(Side_A.stats, Side_B.stats); break; }
        
        // Cheking for retreat
        for (int j = 0; j < Side_A.size && j < Side_B.size; j++)
        {
            bool retreated = true;
            char *msg;
            if (Side_A.units[j].retreating == true)
            { 
                for (int moves = 0; moves < Side_A.units[j].moves; moves++)
                {
                    retreated = unit_Retreat(&Side_A.units[j], &battleMap, mode);
                    if (retreated == false)
                    {
                        battleMap.tiles[Side_A.units[j].position.Y][Side_A.units[j].position.X].unit = NULL;
                        delete_Unit(Side_A.units, &Side_A.size, j);
                        break;
                    }
                }
                update_Map(Side_A.units[j].position.X, Side_A.units[j].position.Y, Side_A.units[j].name);
            }
            if (Side_B.units[j].retreating == true)
            {
                msg = get_MapSprite(&battleMap.tiles[Side_B.units[j].position.Y][Side_B.units[j].position.X], mode);
                for (int moves = 0; moves < Side_B.units[j].moves; moves++)
                {
                    retreated = unit_Retreat(&Side_B.units[j], &battleMap, mode);
                    if (retreated == false)
                    {
                        battleMap.tiles[Side_B.units[j].position.Y][Side_B.units[j].position.X].unit = NULL;
                        delete_Unit(Side_B.units, &Side_B.size, j);
                        break;
                    }
                    update_Map(Side_B.units[j].position.X, Side_B.units[j].position.Y, Side_B.units[j].name);
                }
            }
        }
    }

    // Freeing to Menu
    (void)deallocAll();
    PlaySound("sound/Menu.wav", NULL, SND_ASYNC | SND_FILENAME | SND_LOOP); 
    goto startMenu;

    return 0;
}