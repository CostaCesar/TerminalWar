// #include <stdbool.h>
// #include <stdio.h>
#include "UI.h"
#include <math.h>
#include <stdlib.h>
#include "const.h"
typedef enum E_Direction
{
    North,
    Northeast,
    East,
    Southeast,
    South,
    Southwest,
    West,
    Northwest
} T_Direc;

typedef enum E_Climate
{
    Sunny,
    Sparse_Clouds,
    Many_Clouds,
    Cloudy,
    Rain,
    Storm
} T_Clime;

typedef enum E_Time
{
    Dawn,
    Morning,
    Noon,
    Afternoon,
    Twilight,
    Night
} T_Time;

typedef struct sMap_Node
{
    float lGoal;
    float gGoal;
    bool isVisited;
    bool conectP[Northwest + 1];
    short int conectS[Northwest + 1];
    int parent_XY[2];

} Map_Node;

typedef struct sMap_Area
{
    short int aPosX;
    short int aPosY;
    short int bPosX;
    short int bPosY;
} Map_Area;

typedef struct sMap_Unit
{
    short int X;
    short int Y;
    int ID; 
    char* name;
} Map_Unit;

typedef struct S_Tile
{
    short int elevation;
    short int fortLevel;
    bool isSpawnA;
    bool isSpawnB;
    Map_Unit unit;
    T_Terrain terrain;
    T_Veget vegetation;
    Map_Node node;
} B_Tile;

typedef struct S_Map
{
    char name[STRING_NAME];
    int width;
    int height;
    T_Clime climate;
    T_Time time;
    B_Tile **tiles;
} B_Map;

float calcDistance(B_Tile A, B_Tile B)
{
    return sqrtf((A.unit.X - B.unit.X) * (A.unit.X - B.unit.X) + (A.unit.Y - B.unit.Y) * (A.unit.Y - B.unit.Y));
}

char *tTerrain_toStr(T_Terrain source)
{
    switch (source)
    {
    case Grass:
        return "Grass";
    case Sand:
        return "Sand";
    case Rock:
        return "Rock";
    case Mud:
        return "Mud";
    case Water:
        return "Water";
    }
}

char *tVeget_toStr(T_Veget source)
{
    switch(source)
    {
        case None:
            return "No vegetation";
        case Field:
            return "Fields";
        case Sparse:
            return "Sparse Trees";
        case Grove:
            return "Groove";
        case Forest:
            return "Forest";
        case Jungle:
            return "Jungle";
    }
}

int cmp(const void *a, const void *b)
{
    const B_Tile *a_Float = *((B_Tile **) a);
    const B_Tile *b_Float = *((B_Tile **) b);
    if(a_Float->node.gGoal > b_Float->node.gGoal)
        return 1;
    else if (a_Float->node.gGoal < b_Float->node.gGoal)
        return -1;
    else
        return 0;
}

void tNum_ToDirec(char* key)
{
    int num = (*key) - 48;
    switch(num) //Clockwise, starintg from north
    {
    case 8:
        num = 0;
        break;
    case 9:
        num = 1;
        break;
    case 6:
        num = 2;
        break;
    case 3:
        num = 3;
        break;
    case 2:
        num = 4;
        break;
    case 1:
        num = 5;
        break;
    case 4:
        num = 6;
        break;
    case 7:
        num = 7;
        break;
    default:
        break;
    }
    *key = num;
}

int getDirection(B_Tile *current, B_Tile *next)
{
    int xDif = current->unit.X - next->unit.X;
    int yDif = current->unit.Y - next->unit.Y;

    if(xDif == 0 && yDif > 0)
        return North;
    else if(xDif < 0 && yDif > 0)
        return Northeast;
    else if(xDif < 0 && yDif == 0)
        return East;
    else if(xDif < 0 && yDif < 0)
        return Southeast;
    else if(xDif == 0 && yDif < 0)
        return South;
    else if(xDif > 0 && yDif < 0)
        return Southwest;
    else if(xDif > 0 && yDif == 0)
        return West;
    else if(xDif > 0 && yDif > 0)
        return Northwest;

    return -1;
}

char *get_MapSprite(B_Tile* tile, int mode)
{
    static char mapStat[4] = {0};
    int colorOut = 0;
    B_tileData tileData;
    if(tile->unit.ID != NO_UNIT)
    {
        strcpy(mapStat, tile->unit.name);
        return mapStat;
    }
    switch (mode)
    {
    case MODE_HEIGHT:
        snprintf(mapStat, sizeof(mapStat), "%3d", tile->elevation);
        break;
    case MODE_UNITS:
        if(tile->isSpawnA)
            snprintf(mapStat, sizeof(mapStat), "<A>");
        else if(tile->isSpawnB)
            snprintf(mapStat, sizeof(mapStat), "<B>");
        else snprintf(mapStat, sizeof(mapStat), "   ");
        break;
    case MODE_VEGETAT:
        snprintf(mapStat, sizeof(mapStat), "%3d", tile->vegetation);
        break;
    case MODE_TERRAIN:
        snprintf(mapStat, sizeof(mapStat), "%3d", tile->terrain);
        break;
    case MODE_GRAPHIC:
        tileData.height = &tile->elevation, tileData.veget = (int *) &tile->vegetation;
        tileData.terrain = (int *) &tile->terrain, tileData.unit = tile->unit.name;
        tileData.spawn = 0;
        colorOut = get_MapSprite_Graphic(&tileData, mapStat);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorOut);
    default:
        break;
    }
    return mapStat;
}

void show_Map(B_Map *source, int mode, bool skipBanner)
{
    B_tileData *tiles = (B_tileData*) malloc(source->height * source->width * sizeof(B_tileData));
    char tileAux[4] = {0};
    int cNames = 1;
    
    for(int i = 0; i < source->height; i++)
    {
        for(int j = 0; j < source->width; j++)
        {
            switch (mode)
            {
            case MODE_HEIGHT:
                tiles[i * source->width + j].height = (short int *) &source->tiles[i][j].elevation;
                tiles[i * source->width + j].veget = NULL;
                tiles[i * source->width + j].terrain = NULL;
                tiles[i * source->width + j].spawn = NO_UNIT;
                break;
            case MODE_UNITS:
                if(source->tiles[i][j].isSpawnA == true)
                    tiles[i * source->width + j].spawn = 1;
                else if(source->tiles[i][j].isSpawnB == true)
                    tiles[i * source->width + j].spawn = 2;
                else tiles[i * source->width + j].spawn = 0;
                break;
            case MODE_TERRAIN:
                tiles[i * source->width + j].terrain = (int *) &source->tiles[i][j].terrain;
                tiles[i * source->width + j].spawn  = NO_UNIT;
                tiles[i * source->width + j].veget = NULL;
                tiles[i * source->width + j].height = NULL;
                break;
            case MODE_GRAPHIC:
                if(source->tiles[i][j].unit.ID == NO_UNIT)
                {
                    tiles[i * source->width + j].height = (short int *) &source->tiles[i][j].elevation;
                    tiles[i * source->width + j].veget = (int *) &source->tiles[i][j].vegetation;
                    tiles[i * source->width + j].terrain = (int *) &source->tiles[i][j].terrain;
                    tiles[i * source->width + j].unit = NULL;
                }
                else
                    tiles[i * source->width + j].unit = source->tiles[i][j].unit.name;
                break;
            case MODE_VEGETAT:
                tiles[i * source->width + j].veget = (int *) &source->tiles[i][j].vegetation;
                tiles[i * source->width + j].terrain = NULL;
                tiles[i * source->width + j].spawn = NO_UNIT;
                tiles[i * source->width + j].height = NULL;
                break;
            }
            if(source->tiles[i][j].unit.ID != NO_UNIT) // Override tile with unit name 
                tiles[i * source->width + j].unit = source->tiles[i][j].unit.name;
            else tiles[i * source->width + j].unit = NULL;
        }
    }
    COORD pos = {0, 8};
    if(skipBanner == true) SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
    
    if(mode == MODE_GRAPHIC)
        print_MapGraphic(source->height, source->width, tiles);
    else print_MapStats(source->height, source->width, tiles);
    free(tiles);
    if(skipBanner == true)
        reset_Cursor();
    return;
}

int get_AbsHeigthDif(B_Map* map, Map_Unit aPoint, Map_Unit bPoint)
{
    int height_A = map->tiles[aPoint.Y][aPoint.X].elevation;
    int height_B = map->tiles[bPoint.Y][bPoint.X].elevation;

    if(height_A > height_B)
    {
        return (height_A - height_B);
    }
    else
    {
        return (height_B - height_A);
    }
}

int get_HeightDif(B_Map* map, Map_Unit aPoint, Map_Unit bPoint)
{
    int height_A = map->tiles[aPoint.Y][aPoint.Y].elevation;
    int height_B = map->tiles[bPoint.Y][bPoint.X].elevation;
    return (height_A - height_B);
}

int getTile_Vegetat(B_Map* map, Map_Unit unit)
{
    int X = unit.X;
    int Y = unit.Y;
    return map->tiles[Y][X].vegetation;
}

int getTile_Terrain(B_Map* map, Map_Unit* unit)
{
    int X = unit->X;
    int Y = unit->Y;
    return map->tiles[Y][X].terrain;
}

int move_Unit(B_Map *source_Map, Map_Unit *unitPos, T_Direc direction)
{
    Map_Unit destPos = {0, 0, unitPos->ID, unitPos->name};
    // Checking if coordiantes are valid
    if((unitPos->X < 0 || unitPos->X >= source_Map->width) || (unitPos->Y  < 0 || unitPos->Y  >= source_Map->height))
    {
        print_Message("Coordinates out of boundaries!", true);       
        return FUNCTION_FAIL;
    }
    
    // Checking if coordinates represent a real unit
    if(source_Map->tiles[unitPos->Y][unitPos->X].unit.ID == NO_UNIT)
    {
        char msg[24];
        snprintf(msg, sizeof(msg), "Nothing in %3dX %3dY", unitPos->X, unitPos->Y);
        print_Message(msg, true);
        return FUNCTION_FAIL;
    }

    // Moving
    switch (direction)
    {
    case North:
        destPos.X = unitPos->X;
        destPos.Y = unitPos->Y - 1;
        break;
    case Northeast:
        destPos.X = unitPos->X + 1;
        destPos.Y = unitPos->Y - 1;
        break;
    case East:
        destPos.X = unitPos->X + 1;
        destPos.Y = unitPos->Y;
        break;
    case Southeast:
        destPos.X = unitPos->X + 1;
        destPos.Y = unitPos->Y + 1;
        break;
    case South:
        destPos.X = unitPos->X;
        destPos.Y = unitPos->Y + 1;
        break;
    case Southwest:
        destPos.X = unitPos->X - 1;
        destPos.Y = unitPos->Y + 1;
        break;
    case West:
        destPos.X = unitPos->X - 1;
        destPos.Y = unitPos->Y;
        break;
    case Northwest:
        destPos.X = unitPos->X - 1;
        destPos.Y = unitPos->Y - 1;
        break;
    }
    
    // Checking if new coordiantes are valid
    if((destPos.X >= 0 || destPos.X < source_Map->width) || (destPos.Y >= 0 || destPos.Y < source_Map->height))
    {
        // Checking for terrain-related movement
        if(source_Map->tiles[destPos.Y][destPos.X].elevation < -1 && source_Map->tiles[destPos.Y][destPos.X].terrain == Water)
        {
            print_Message("The water is too deep to cross!", true);
            return FUNCTION_FAIL;           
        }
        if(abs(source_Map->tiles[unitPos->Y][unitPos->X].elevation - source_Map->tiles[destPos.Y][destPos.X].elevation) > HEIGHT_DIF)
        {
            print_Message("The terrain is too step to go!", true);
            return FUNCTION_FAIL;
        }

        // Cheking for units (if friend, then if foe)
        if (source_Map->tiles[destPos.Y][destPos.X].unit.ID != NO_UNIT && (source_Map->tiles[destPos.Y][destPos.X].unit.ID % 2) == (source_Map->tiles[unitPos->Y][unitPos->X].unit.ID % 2))
        {
            print_Message("Units can't go over eachother!", true);
            return FUNCTION_FAIL;             
        }
        else if (source_Map->tiles[destPos.Y][destPos.X].unit.ID != NO_UNIT && (source_Map->tiles[destPos.Y][destPos.X].unit.ID % 2) != (source_Map->tiles[unitPos->Y][unitPos->X].unit.ID % 2))
            return OUT_COMBAT;
        
        short int mCost = 0;//source_Map->tiles[destPos.Y][destPos.X].elevation - source_Map->tiles[unitPos->Y][unitPos->X].elevation;
        mCost += source_Map->tiles[destPos.Y][destPos.X].terrain;
        source_Map->tiles[destPos.Y][destPos.X].unit = destPos;
        source_Map->tiles[unitPos->Y][unitPos->X].unit.ID = NO_UNIT;
        *unitPos = destPos;

        return mCost;
    }
    print_Message("Destination out of boundaries!", true);
    return FUNCTION_FAIL;
}

int put_Unit_OnMap(B_Map *map, Map_Unit *unit, int ignoreSpawn)
{
    short int X = unit->X;
    short int Y = unit->Y;
    
    // Checking if coordinates are valid
    if((X < 0 || X >= map->width) || (Y < 0 || Y >= map->height))
    {
        fprintf(stderr, "Unit outside of map!");
        return FUNCTION_FAIL;
    }

    // Cheking if it's spawn tiles
    if(((!map->tiles[Y][X].isSpawnA && unit->ID % 2 == 0) || (!map->tiles[Y][X].isSpawnB && unit->ID % 2 == 1)) && ignoreSpawn == 0)
    {
        fprintf(stderr, "Not in spawn coordinates!");
        return FUNCTION_FAIL;    
    }

    // Cheking if tile is free
    if(map->tiles[Y][X].unit.ID != NO_UNIT)
    {
        fprintf(stderr, "Tile already used!");
        return FUNCTION_FAIL;    
    }
    
    map->tiles[Y][X].unit = *unit;
    return FUNCTION_SUCESS;
}

int inc_FortLevel(B_Map *map, short int amount, Map_Unit pos)
{
    map->tiles[pos.Y][pos.X].fortLevel += amount;
    if(map->tiles[pos.Y][pos.X].fortLevel > MAX_FORT_LEVEL)
    {
        map->tiles[pos.Y][pos.X].fortLevel = MAX_FORT_LEVEL;
        char msg[28];
        snprintf(msg, sizeof(msg), "Max Fort Level Reached [%2d]", MAX_FORT_LEVEL);
        print_Message(msg, true);
        return FUNCTION_FAIL;
    }
    else
    {
        char msg[29];
        snprintf(msg, sizeof(msg), "Fort Upgraded from %2d to %2d!", map->tiles[pos.Y][pos.X].fortLevel - amount, map->tiles[pos.Y][pos.X].fortLevel);
        print_Message(msg, true);       
        return FUNCTION_SUCESS;
    }
}

B_Tile* get_AdjTile(B_Map* map, Map_Unit unit, T_Direc direction, bool isAI)
{
    Map_Unit destPos = {0, 0, unit.ID, unit.name};
    
    // Testing for valid unit
    if(unit.ID == NO_UNIT && isAI == false)
    {
        printf("ERROR: getRLTV_UnitId >> 1!");
    }

    switch (direction)
    {
    case North:
        destPos.X = unit.X;
        destPos.Y = unit.Y - 1;
        break;
    case Northeast:
        destPos.X = unit.X + 1;
        destPos.Y = unit.Y - 1;
        break;
    case East:
        destPos.X = unit.X + 1;
        destPos.Y = unit.Y;
        break;
    case Southeast:
        destPos.X = unit.X + 1;
        destPos.Y = unit.Y + 1;
        break;
    case South:
        destPos.X = unit.X;
        destPos.Y = unit.Y + 1;
        break;
    case Southwest:
        destPos.X = unit.X - 1;
        destPos.Y = unit.Y + 1;
        break;
    case West:
        destPos.X = unit.X - 1;
        destPos.Y = unit.Y;
        break;
    case Northwest:
        destPos.X = unit.X - 1;
        destPos.Y = unit.Y - 1;
        break;
    }

    if((destPos.X >= 0 || destPos.X < BACKUP_MAP_COLUMNS) || (destPos.Y >= 0 || destPos.Y < BACKUP_MAP_ROWS))
        return &map->tiles[destPos.Y][destPos.X];
    else
    {
        printf("ERROR: getRLTV_UnitId >> 2!");
        return NULL;
    }
}

bool unit_Retreat(Map_Unit* unit, B_Map* map)
{
    int hasMoved = false;
    T_Direc direction;

    // What direction it should move
    if (unit->X < (map->width / 2))
        direction = West;
    else if(unit->X > map->width / 2)
        direction = East;
    else if(unit->Y < map->height / 2)
        direction = North;
    else if(unit->Y > map->height / 2)
        direction = South;
    else
        direction = rand() % Northwest; // Random direction

    // Trying to move foward, then diagonals
    B_Tile* adj = get_AdjTile(map, *unit, direction, true);
    if((adj) && adj->unit.ID == NO_UNIT)
        hasMoved = move_Unit(map, unit, direction);
    if (hasMoved == FUNCTION_FAIL)
    {
        adj = get_AdjTile(map, *unit, direction -1, true);
        if((adj) && adj->unit.ID == NO_UNIT)
            hasMoved = move_Unit(map, unit, direction -1);
        if (hasMoved == FUNCTION_FAIL)
        {
            adj = get_AdjTile(map, *unit, direction +1, true);
            if((adj) && adj->unit.ID == NO_UNIT)
                hasMoved = move_Unit(map, unit, direction +1);
            if (hasMoved == FUNCTION_FAIL)
                return false;
        }
    }
    return true;
}

B_Map create_Map(T_Clime climate, T_Time time)
{
    B_Map creation;
    creation.tiles = (B_Tile **) calloc(BACKUP_MAP_ROWS, sizeof(B_Tile *));
    for(int i = 0; i < BACKUP_MAP_ROWS; i++)
        creation.tiles[i] = (B_Tile *) calloc(BACKUP_MAP_COLUMNS, sizeof(B_Tile));
    creation.climate = climate;
    creation.time = time;
    creation.width = BACKUP_MAP_COLUMNS;
    creation.height = BACKUP_MAP_ROWS;
    
    // Definig map tiles
    for(int i = 0; i < BACKUP_MAP_ROWS; i++)
    {
        for(int j = 0; j < BACKUP_MAP_COLUMNS; j++)
        {
            // Marking water borders
            if((i == 0 || i == (BACKUP_MAP_ROWS - 1)) || (j == 0 || j == (BACKUP_MAP_COLUMNS - 1)))
            {
                creation.tiles[i][j].elevation = -2;
                creation.tiles[i][j].terrain = Water;
                creation.tiles[i][j].vegetation = None;
            }
            else if ((j == 5 && i > 0 && i < BACKUP_MAP_ROWS - 2) || (i == 7 && j > 0 && j <= 5 && j != 2))
            {
                creation.tiles[i][j].elevation = 5;
                creation.tiles[i][j].terrain = Rock;
                creation.tiles[i][j].vegetation = None;          
            }
            else if((j > 0 && j < 4 && i == 5))
            {
                creation.tiles[i][j].elevation = -2;
                creation.tiles[i][j].terrain = Water;
                creation.tiles[i][j].vegetation = None;
            }
            else
            {
                creation.tiles[i][j].elevation = 1;
                creation.tiles[i][j].terrain = Grass;
                creation.tiles[i][j].vegetation = Field;
            }
            
            creation.tiles[i][j].fortLevel = 0;
            creation.tiles[i][j].unit.ID = NO_UNIT;
            creation.tiles[i][j].unit.X = j;
            creation.tiles[i][j].unit.Y = i;
        }
    }


    // Getting conections
    for(int i = 0; i < BACKUP_MAP_ROWS; i++)
    {
        for(int j = 0; j < BACKUP_MAP_COLUMNS; j++)
        {
            for(int k = 0; k <= Northwest; k++)
            {
                if(((k > West || k < East) && i < 1) || ((k < West && k > East) && i >= BACKUP_MAP_ROWS - 1)
                ||  (k > South && j < 1) || ((k > North && k < South) && j >= BACKUP_MAP_COLUMNS - 1))
                    continue;

                B_Tile *temp = get_AdjTile(&creation, creation.tiles[i][j].unit, k, true);
                if((get_AbsHeigthDif(&creation, creation.tiles[i][j].unit, temp->unit) > HEIGHT_DIF) || (temp->elevation < -1 && temp->terrain == Water))
                {
                    creation.tiles[i][j].node.conectP[k] = false;
                    creation.tiles[i][j].node.conectS[k] = -1;
                }
                else
                {
                    creation.tiles[i][j].node.conectP[k] = true;
                    creation.tiles[i][j].node.conectS[k] = temp->terrain;  
                }
            }
        }
    }

    return creation;
}

T_Direc *autoMove(B_Map* map, B_Tile *startNode, B_Tile *endNode)
{
    int toTest_Size = 1, path_Size = 1;
    T_Direc *path = (T_Direc *) calloc(path_Size, sizeof(T_Direc));
    B_Tile **toTest = (B_Tile **) calloc(toTest_Size, sizeof(B_Tile *));
    
    for(int i = 0; i < map->height; i++)
    {
        for(int j = 0; j < map->width; j++)
        {
            map->tiles[i][j].node.isVisited = false;
            map->tiles[i][j].node.lGoal = INFINITY;
            map->tiles[i][j].node.gGoal = INFINITY;
            map->tiles[i][j].node.parent_XY[0] = -1;
            map->tiles[i][j].node.parent_XY[1] = -1;
        }
    }
    
    B_Tile *cTile = startNode;
    startNode->node.lGoal = 0.0f;
    startNode->node.gGoal = calcDistance(*startNode, *endNode);
    toTest[toTest_Size - 1] = startNode;

    while(toTest_Size > 0 && cTile != endNode)
    {
        // Uncomment to see the AI finding his way :)
        /*
        system("cls");
        show_Map(map, UNITS);
        Sleep(250); */

        qsort(toTest, toTest_Size, sizeof(B_Tile *), cmp);

        while(toTest_Size > 0 && toTest[0]->node.isVisited == true)
        {
            for(int i = 0; i < toTest_Size - 1; i++)
            {
                toTest[i] = toTest[i + 1];
                //printf("%d> %dX %dY > %f lG # %f gG > %d pX %d pY \n", i, toTest[i]->unit.X, toTest[i]->unit.Y,
                //toTest[i]->node.lGoal, toTest[i]->node.gGoal, toTest[i]->node.parent_XY[0], toTest[i]->node.parent_XY[1])
            }
            toTest_Size--;
            toTest = (B_Tile **) realloc(toTest, toTest_Size * sizeof(B_Tile *));
        }

        if(toTest_Size <= 0)
            break;
        cTile = toTest[0];
        cTile->node.isVisited = true;

        for(int i = 0; i <= Northwest; i++)
        {
            B_Tile *nTile = get_AdjTile(map, cTile->unit, i, true);
            if(cTile->node.conectP[i] == true && nTile->node.isVisited == false && (nTile->unit.ID == NO_UNIT || nTile->unit.ID == endNode->unit.ID))
            {
                toTest_Size++;
                toTest = (B_Tile **) realloc(toTest, toTest_Size * sizeof(B_Tile *));
                toTest[toTest_Size - 1] = nTile;

                float pLowerGoal = cTile->node.lGoal + calcDistance(*cTile, *nTile) + cTile->node.conectS[i];
                if(pLowerGoal < nTile->node.lGoal)
                {
                    nTile->node.parent_XY[0] = cTile->unit.X;
                    nTile->node.parent_XY[1] = cTile->unit.Y;
                    nTile->node.lGoal = pLowerGoal;
                    nTile->node.gGoal = nTile->node.lGoal + calcDistance(*nTile, *endNode);
                }
            }
        }
    }

    cTile = endNode;
    while(cTile->node.parent_XY[0] > -1 && cTile->node.parent_XY[1] > -1)
    {
        int parentX = cTile->node.parent_XY[0];
        int parentY = cTile->node.parent_XY[1];
        int prePath = -1;
        B_Tile *nTile = &map->tiles[parentY][parentX];

        prePath = getDirection(cTile, nTile);
        if(prePath > -1)
        {
            path = (T_Direc *) realloc(path, path_Size * sizeof(T_Direc));
            if(prePath < 4)
                prePath += 4;
            else prePath -= 4;
            
            path[path_Size - 1] = (T_Direc) prePath;
            cTile = nTile;
            path_Size++;
        }
        else break;
    }
    path_Size--;
    if(cTile != startNode) // WATCH!
        return NULL;

    for(int i = 0; i < path_Size / 2 + 1; i++)
    {
        T_Direc aux = path[i];
        path[i] = path[path_Size - i - 1];
        path[path_Size - i - 1] = aux;
    }
    if(path_Size < 0)
    {
        // printf("\n");
        // printf("#=======================================# \n");
        // printf("| Path to %3dX %3dY could not be found  | \n");
        // printf("#=======================================# \n");
        char msg[38];
        snprintf(msg, sizeof(msg), "Path to %3dX %3dY could not be found!", endNode->unit.X, endNode->unit.Y);
        print_Message(msg, true);
        
        getchar();
    }

    return path;
} 
