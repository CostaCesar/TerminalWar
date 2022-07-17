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

// File->Game functions
B_Unit *getFile_Unit(char *path, int *size)
{
    FILE *file = fopen(path, "rb");
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
    FILE *file = fopen(fPath, "rb");
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
        map = NULL;
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

Map_Unit def_Unit(B_Unit *unit, B_Map *map)
{
    Map_Unit out = {-1, -1, NO_UNIT, "\0"};
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
        printf("===========================================\n");
        printf("| [T] Change Map Type  | [A] Place A Unit |\n");
        printf("| [Esc] Exit To Menu   | [M] Replace Unit |\n");
        printf("| [Enter] Start Battle | [R] Remove Unit  |\n");
        printf("===========================================\n");

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
                printf(">> Cordinates of the unit <X Y> inside |*1| area: ");
                scanf(" %hd %hd", &Side->units[Index].position.X, &Side->units[Index].position.Y);

                unit = set_MapUnit(&Side->units[Index]);
                out = put_Unit_OnMap(map, &unit);
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
                printf(">> Cordinates of the unit <X Y> inside |*1| area: ");
                scanf("%hd %hd", &Side->units[Index].position.X, &Side->units[Index].position.Y);

                unit = set_MapUnit(&Side->units[Index]);
                out = put_Unit_OnMap(map, &unit);
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

int main(/*int argc, char** argv*/)
{
    // Randomizing seed
    srand(time(NULL));

    // Setting up terminal
    SetConsoleTitle("Total Terminal War");
    toggle_Cursor(false);

    int unit_TableSize = 0, out = 0;
    extern short int A_Loss, B_Loss;
    extern short int xHiLi, yHiLi;
    A_Loss = 0, B_Loss = 0;
    B_Unit *unit_Table = getFile_Unit("units/new2.bin", &unit_TableSize);

    // Side_A
    B_endStats Status_A = {0};
    Side_A.size = 0, Side_A.ID = 0, Side_A.units = NULL;

    // Side_B
    Side_B.size = 0, Side_B.ID = 1, Side_B.units = NULL;
    B_endStats Status_B = {0};

    // Playing music
    PlaySound("sound/Menu.wav", NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);

startMenu:
    do
    {
        switch (screen_Menu(VERSION))
        {
        case 'i':
            // Setting up map & units
            out = getFile_Map("maps/Catalon.map", &battleMap);
            if (out != FUNCTION_SUCESS)
                return FUNCTION_FAIL;

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
    Side_A.units = (B_Unit *)malloc(sizeof(B_Unit));
    strcpy(Side_A.name, "Greeks");
    Side_B.units = (B_Unit *)malloc(sizeof(B_Unit));
    strcpy(Side_B.name, "Gauls");
    Status_A.name = Side_A.name, Status_B.name = Side_B.name;

    // Setting up some units
    set_fUnitTable(unit_Table, 0, &Side_A);
    set_fUnitTable(unit_Table, 4, &Side_A);
    set_fUnitTable(unit_Table, 0, &Side_A);
    set_fUnitTable(unit_Table, 1, &Side_B);
    set_fUnitTable(unit_Table, 1, &Side_B);
    set_fUnitTable(unit_Table, 2, &Side_B);

    // Placing AI on Map
    Map_Unit unitB, unitA;
    for (int i = 0; i < Side_B.size; i++)
    {
        unitB = def_Unit(&Side_B.units[i], &battleMap);
        put_Unit_OnMap(&battleMap, &unitB);
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
    PlaySound("sound/Game1.wav", NULL, SND_ASYNC | SND_LOOP | SND_FILENAME);
    system("cls");
    show_Map(&battleMap, MODE_HEIGHT, true);
    for (int i = 0; i < TURNS; i++)
    {
        int unitA_I = 1, unitB_I = 0, moves = 0;
        B_Tile *position;
        char action = '\0';
        unitA = set_MapUnit(&Side_A.units[unitA_I]);

        for (moves = 0; moves < Side_A.units[unitA_I].moves; moves++) // Side_A turn
        {
            int FRes = false, xGoal = Side_A.units[unitA_I].goal.X, yGoal = Side_A.units[unitA_I].goal.Y, xTarget = -1, yTarget = -1;
            char msg[31];
            xHiLi = unitA.X, yHiLi = unitA.Y;
            info_Upper(battleMap.name, i, Side_A.name, true, unitA.name, unitA.ID, unitA.X, unitA.Y, Side_A.units[unitA_I].moves - moves);
            // show_Map(&battleMap, MODE_HEIGHT);
            if (moves < Side_A.units[unitA_I].moves && Side_A.units[unitA_I].isRetreating == false && Side_A.units[unitA_I].inCombat == false)
            {
                toggle_Cursor(false);
                while(1)
                {
                    if(_kbhit())
                    {
                        action = _getch();
                        break;
                    }
                    char msg[3] = {219, 219, 219};
                    update_Map(battleMap.height, unitA.X, unitA.Y, msg);
                    Sleep(500);
                    toggle_Cursor(false);
                    if(_kbhit())
                    {
                        action = _getch();
                        break;
                    }
                    update_Map(battleMap.height, unitA.X, unitA.Y, unitA.name);
                    Sleep(500);
                }
                COORD pos = {0, 9+battleMap.height*2};
                if (action >= '0' && action <= '9') // Move to a adjacent tile
                {
                    tNum_ToDirec(&action);
                    position = get_AdjTile(&battleMap, unitA, action, false);
                    xGoal = position->unit.X, yGoal = position->unit.Y;
                }
                else if (action == 'a') // Move to a location
                {
                    clear_afterMap(battleMap.height);
                    toggle_Cursor(true);
                    printf(">> Goal coordinates <X Y> \n");
                    printf(" >=> ");
                    scanf("%hd %hd", &Side_A.units[unitA_I].goal.X, &Side_A.units[unitA_I].goal.Y);
                    xGoal = Side_A.units[unitA_I].goal.X, yGoal = Side_A.units[unitA_I].goal.Y;
                    toggle_Cursor(false);
                    print_Message("Moving to tile!", true);
                }
                else if (action == 's') // Intercept a unit
                {
                    clear_afterMap(battleMap.height);
                    toggle_Cursor(true);
                    printf(">> Target unit coordinates <X Y> \n");
                    printf(" >=> ");
                    scanf("%hd %hd", &Side_A.units[unitA_I].goal.X, &Side_A.units[unitA_I].goal.Y);
                    xGoal = Side_A.units[unitA_I].goal.X, yGoal = Side_A.units[unitA_I].goal.Y;
                    toggle_Cursor(false);
                    if (xGoal >= 0 && xGoal < battleMap.width && yGoal >= 0 && yGoal < battleMap.height)
                    {
                        unitB = battleMap.tiles[yGoal][xGoal].unit;
                        if (unitB.ID != NO_UNIT)
                        {
                            Side_A.units[unitA_I].chaseID = &unitB.ID;
                            // unitB_I = get_UnitIndex(&Side_B, unitB.ID);
                            // xChase = &Side_B.units[unitB_I].position.X, yChase = &Side_B.units[unitB_I].position.Y,
                            print_Message("Chasing Target!", true);
                        }
                        else
                        {
                            print_Message("Target Not Found!", true);
                            xGoal = -1, yGoal = -1, Side_A.units[unitA_I].goal.X = -1, Side_A.units[unitA_I].goal.Y = -1;
                            moves--;
                        }
                    }
                    else
                    {
                        print_Message("Invalid Coordinates! ", true);
                        xGoal = -1, yGoal = -1, Side_A.units[unitA_I].goal.X = -1, Side_A.units[unitA_I].goal.Y = -1;
                        moves--;
                    }
                }
                else if (action == 'f')
                {
                    if (Side_A.units[unitA_I].range < 1)
                    {
                        print_Message("This unit can't do ranged attacks!", true);
                        moves--;
                        continue;
                    }
                    clear_afterMap(battleMap.height);
                    toggle_Cursor(true);
                    printf(">> Target coodiantes <X Y> \n");
                    printf(" >=> ");
                    scanf("%d %d", &xTarget, &yTarget);
                    toggle_Cursor(false);
                    if (xTarget < 0 || xTarget >= battleMap.width || yTarget < 0 || yTarget >= battleMap.height)
                    {
                        print_Message("Coordinates out of boundaries!", true);
                        moves--;
                    }
                    else if(battleMap.tiles[yTarget][xTarget].unit.ID == NO_UNIT)
                    {
                        print_Message("There's nothing to attack here!", true);
                        moves--;
                    }
                    else
                    {
                        // Show
                        update_Map(battleMap.height, unitA.X, unitA.Y, "XXX");
                        Sleep(TIME_MAP);
                        position = get_AdjTile(&battleMap, unitA, Side_A.units[unitA_I].path[0], false);
                        update_Map(battleMap.height, unitB.X, unitB.Y, "OOO");
                        Sleep(TIME_MAP);
                        // Go
                        unitB = battleMap.tiles[yTarget][xTarget].unit;
                        unitB_I = get_UnitIndex(&Side_B, unitB.ID);
                        int tVeget = getTile_Vegetat(&battleMap, unitB), tHeight = get_HeightDif(&battleMap, unitA, unitB);
                        short int *tFort = &battleMap.tiles[unitB.Y][unitB.X].fortLevel;
                        FRes = do_Combat_Ranged(&Side_A.units[unitA_I], &Side_B.units[unitB_I], tHeight, tVeget, tFort);
                        // Done
                        if(FRes == FUNCTION_SUCESS)
                        {
                            show_gUnit(&Side_A.units[unitA_I]);
                            show_gUnit(&Side_B.units[unitB_I]);
                            printf(">> Press ENTER to continue ");
                            Status_A.loss += A_Loss, Status_B.loss += B_Loss;
                            while (get_KeyPress(false) != KEY_ENTER) continue;
                            system("cls");
                            show_Map(&battleMap, MODE_HEIGHT, true);
                        }
                        break;
                    }
                    continue;
                }
                else if (action == 'd')
                {
                    reset_Cursor();
                    system("cls");
                    show_gUnit(&Side_A.units[unitA_I]);
                    printf(">> Press ENTER to continue \n");
                    while (get_KeyPress(false) != KEY_ENTER) continue;
                    system("cls");
                    show_Map(&battleMap, MODE_HEIGHT, true);
                    moves--;
                }
                else if (action == 'e')
                {
                    clear_afterMap(battleMap.height);
                    if (Side_A.units[unitA_I].build_Cap > 0)
                    {
                        out = inc_FortLevel(&battleMap, Side_A.units[unitA_I].build_Cap, unitA);
                        if (out == FUNCTION_FAIL)
                            moves--;
                    }
                    else
                    {
                        print_Message("This unit cannot build fortifications!", true);
                        moves--;
                    }
                }
                else if (action == 'g')
                {
                    clear_afterMap(battleMap.height);
                    toggle_Cursor(true);
                    printf(">> Target coodiantes <X Y> \n");
                    printf(" >=> ");
                    scanf("%d %d", &xTarget, &yTarget);
                    getchar();
                    toggle_Cursor(false);
                    if (xTarget >= 0 && xTarget < battleMap.width && yTarget >= 0 && yTarget < battleMap.height)
                    {
                        print_Line(NULL);
                        print_Line(" ");
                        snprintf(msg, sizeof(msg), "Terrain: %s", tTerrain_toStr(battleMap.tiles[yTarget][xTarget].terrain));
                        print_Line(msg);
                        snprintf(msg, sizeof(msg), "Vegetation: %s", tVeget_toStr(battleMap.tiles[yTarget][xTarget].vegetation));
                        print_Line(msg);
                        snprintf(msg, sizeof(msg), "Elevation: %d", battleMap.tiles[yTarget][xTarget].elevation);
                        print_Line(msg);
                        snprintf(msg, sizeof(msg), "Fortification: %d", battleMap.tiles[yTarget][xTarget].fortLevel);
                        print_Line(msg);
                        snprintf(msg, sizeof(msg), "Unit in here: %d", battleMap.tiles[yTarget][xTarget].unit.ID);
                        print_Line(msg);
                        print_Line(" ");
                        print_Line(NULL);
                        printf(">> Press ENTER to continue \n");
                        while (get_KeyPress(false) != KEY_ENTER) continue;
                        system("cls");
                        show_Map(&battleMap, MODE_HEIGHT, true);
                        moves--;
                    }
                    else
                    {
                        print_Message("Coordinates out of boundaries!", true);
                        moves--;
                    }
                }
                else if (action == KEY_ESCAPE)
                {
                    (void)dealloc_ToMenu();
                    PlaySound("sound/Menu.wav", NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
                    goto startMenu;
                }
                else if (action == KEY_ENTER) // Next Turn
                {
                }
                else
                {
                    clear_afterMap(battleMap.height);
                    print_Message("Invalid action!", true);
                    moves--;
                }
                if (xGoal > -1 && yGoal > -1 && xGoal < battleMap.width && yGoal < battleMap.height)
                {
                    // Change to coordinates of unitB if unitA is chasing
                    if (Side_A.units[unitA_I].chaseID != NULL)
                    {
                        unitB_I = get_UnitIndex(&Side_B, *Side_A.units[unitA_I].chaseID);
                        xGoal = Side_B.units[unitB_I].position.X, yGoal = Side_B.units[unitB_I].position.Y;
                        Side_A.units[unitA_I].goal.X = xGoal, Side_A.units[unitA_I].goal.Y = yGoal;
                    }
                    Side_A.units[unitA_I].path = (int *)autoMove(&battleMap, &battleMap.tiles[unitA.Y][unitA.X], &battleMap.tiles[yGoal][xGoal]);
                    if (Side_A.units[unitA_I].path != NULL)
                    {
                        for (int steps = 0; moves < Side_A.units[unitA_I].moves; steps++)
                        {
                            if ((unitA.X == xGoal && unitA.Y == yGoal) || FRes == OUT_COMBAT)
                                break;
                            FRes = move_Unit(&battleMap, &unitA, Side_A.units[unitA_I].path[steps]);
                            if (FRes == OUT_COMBAT)
                            {
                                // Show
                                update_Map(battleMap.height, unitA.X, unitA.Y, "XXX");
                                Sleep(TIME_MAP);
                                position = get_AdjTile(&battleMap, unitA, Side_A.units[unitA_I].path[0], false);
                                update_Map(battleMap.height, position->unit.X, position->unit.Y, "OOO");
                                Sleep(TIME_MAP);
                                // Glow
                                system("cls");   
                                snprintf(msg, sizeof(msg), "Trying engagement at %3dX %3dY", position->unit.X, position->unit.Y);
                                print_Message(msg, true);
                                // Go
                                unitB_I = get_UnitIndex(&Side_B, position->unit.ID);
                                do_Combat(&Side_A.units[unitA_I], &Side_B.units[unitB_I], get_HeightDif(&battleMap, unitA, unitB), &battleMap.tiles[unitB.Y][unitB.X].fortLevel);
                                // Done
                                show_gUnit(&Side_A.units[unitA_I]);
                                show_gUnit(&Side_B.units[unitB_I]);
                                Status_A.loss += A_Loss, Status_B.loss += B_Loss;
                                printf(">> Press ENTER to continue ");
                                while (get_KeyPress(false) != KEY_ENTER) continue;
                                system("cls");
                                show_Map(&battleMap, MODE_HEIGHT, true);
                            }
                            else if (FRes == FUNCTION_FAIL)
                                moves--;
                            else if (FRes > -1)
                            {
                                snprintf(msg, sizeof(msg), "%3d", battleMap.tiles[unitB.Y][unitB.X].elevation);
                                update_Map(battleMap.height, Side_A.units[unitA_I].position.X,  Side_A.units[unitA_I].position.Y, msg);
                                moves += FRes;
                            }
                        }
                        free(Side_A.units[unitA_I].path);
                    }
                    else
                    {
                        clear_afterMap(battleMap.height);
                        print_Message("Can't move in this direction!", true);
                        moves--;
                    }
                }
                update_Map(battleMap.height, unitA.X, unitA.Y, unitA.name);
                if ((unitA.X == xGoal && unitA.Y == yGoal) || FRes == OUT_COMBAT)
                {
                    Side_A.units[unitA_I].chaseID = NULL;
                    Side_A.units[unitA_I].goal.X = -1, Side_A.units[unitA_I].goal.Y = -1;
                    if(position == NULL)
                    {
                        clear_afterMap(battleMap.height);
                        print_Message("Auto move disabled!", true);
                    }
                }
                position = NULL;
                update_Unit(&Side_A.units[unitA_I], unitA);
                toggle_Cursor(false);
            }
            else
                Side_A.units[unitA_I].inCombat = false; 
        }

        unitB = set_MapUnit(&Side_B.units[unitB_I]);
        xHiLi = NO_UNIT, yHiLi = NO_UNIT;
        for (moves = 0; moves < Side_B.units[unitB_I].moves; moves++) // Side_B turn
        {
            // show_Map(&battleMap, MODE_HEIGHT);
            char msg[31];
            int FRes = false;
            // clear_afterMap(battleMap.height);
            info_Upper(battleMap.name, i, Side_B.name, false, unitB.name, unitB.ID, unitB.X, unitB.Y, Side_B.units[unitB_I].moves - moves);
            update_Map(battleMap.height, unitB.X, unitB.Y, unitB.name);
            Sleep(TIME_STRATEGY);
            COORD pos = {0, 9+battleMap.height*2};

            if (moves < Side_B.units[unitB_I].moves && Side_B.units[unitB_I].isRetreating == false && Side_B.units[unitB_I].inCombat == false)
            {
                if (unitB.X > 10)
                {
                    snprintf(msg, sizeof(msg), "%3d", battleMap.tiles[unitB.Y][unitB.X].elevation);
                    update_Map(battleMap.height, unitB.X, unitB.Y, msg);
                    FRes = move_Unit(&battleMap, &unitB, West);
                }
                else
                {
                    clear_afterMap(battleMap.height);
                    inc_FortLevel(&battleMap, Side_B.units[unitB_I].build_Cap, unitB);
                }
                if (FRes == OUT_COMBAT)
                {
                    // Show
                    fflush(stdin);
                    update_Map(battleMap.height, unitB.X, unitB.Y, "XXX");
                    Sleep(TIME_MAP);
                    position = get_AdjTile(&battleMap, unitB, West, false);
                    update_Map(battleMap.height, position->unit.X, position->unit.Y, "OOO");
                    Sleep(TIME_MAP);
                    // Glow
                    system("cls");   
                    snprintf(msg, sizeof(msg), "Trying engagement at %3dX %3dY", position->unit.X, position->unit.Y);
                    print_Message(msg, true);
                    // Go
                    unitA_I = get_UnitIndex(&Side_A, position->unit.ID);
                    do_Combat(&Side_B.units[unitB_I], &Side_A.units[unitA_I], get_HeightDif(&battleMap, unitB, unitA), &battleMap.tiles[unitA.Y][unitA.X].fortLevel);
                    Status_B.loss += A_Loss, Status_A.loss += B_Loss;
                    show_gUnit(&Side_A.units[unitA_I]);
                    show_gUnit(&Side_B.units[unitB_I]);
                    printf(">> Press ENTER to continue ");
                    while (get_KeyPress(false) != KEY_ENTER) continue;
                    system("cls");
                }
                else if (FRes > -1)
                    moves += FRes;
                update_Unit(&Side_B.units[unitB_I], unitB);
                update_Map(battleMap.height, unitB.X, unitB.Y, unitB.name);
            }
            else
                Side_B.units[unitB_I].inCombat = false;
        }
        info_Upper(battleMap.name, i, Side_B.name, false, unitB.name, unitB.ID, unitB.X, unitB.Y, 0);
        // show_Map(&battleMap, MODE_HEIGHT, true);
        update_Map(battleMap.height, unitB.X, unitB.Y, unitB.name);
        Sleep(TIME_STRATEGY);

        // Cheking for retreat
        for (int j = 0; j < Side_A.size && j < Side_B.size; j++)
        {
            bool retreated = true;
            if (Side_A.units[j].isRetreating == true)
            {
                unitA = set_MapUnit(&Side_A.units[j]);
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
                }
            }
            if (Side_B.units[j].isRetreating == true)
            {
                unitB = set_MapUnit(&Side_B.units[j]);
                for (int moves = 0; moves < Side_B.units[j].moves; moves++)
                {
                    retreated = unit_Retreat(&unitB, &battleMap);
                    update_Unit(&Side_B.units[j], unitB);
                    if (retreated == false)
                    {
                        battleMap.tiles[unitB.Y][unitB.X].unit.ID = NO_UNIT;
                        delete_Unit(Side_B.units, &Side_B.size, j);
                        break;
                    }
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