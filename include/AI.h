#include "const.h"
#include "unit.h"
#include "map.h"
#include "combat.h"

typedef enum E_Response
{
    AI_Wait = 0,
    AI_GoTo = 1,
    AI_Engage = 2,
    AI_Fire = 3,
    AI_Chase = 4,
    AI_Retreat = 5,
    AI_Fortify = 6
} T_Response;

typedef enum E_Level
{
    AI_Easy = 1,
    AI_Medium = 2,
    AI_Hard = 3,
    AI_Passive = 0
} T_Level;

B_Unit **get_UnitsInRange(B_Unit *unit, B_Map* map, int* nFoes, int distance)
{
    B_Pos minCoord, maxCoord;
    B_Unit **targets = (B_Unit**) malloc(sizeof(B_Unit*));
    *nFoes = 0;

    minCoord.X = (unit->position.X - distance > 0) ? unit->position.X - distance : 0;
    minCoord.Y = (unit->position.Y - distance > 0) ? unit->position.Y - distance : 0;
    maxCoord.X = (unit->position.X + distance < map->width) ? unit->position.X + distance : map->width - 1;
    maxCoord.Y = (unit->position.Y + distance < map->height) ? unit->position.Y + distance : map->height - 1;

    for(int i = minCoord.Y; i < maxCoord.Y; i++)
    {
        for(int j = minCoord.X; j < maxCoord.X; j++)
        {
            if(map->tiles[i][j].unit != NULL)
            {
                if(map->tiles[i][j].unit->ID % 2 == unit->ID % 2) // Same team
                    continue;
                targets[*nFoes] = map->tiles[i][j].unit;
                (*nFoes)++;
                targets = (B_Unit **) realloc(targets, ((*nFoes)+1) * sizeof(B_Unit*));
                for(int k = (*nFoes)-1; k > 0; k--)
                {
                    if(calcDistance(targets[k]->position, unit->position) < calcDistance(targets[k-1]->position, unit->position))
                        targets[k] = targets[k-1], targets[k-1] = map->tiles[i][j].unit;
                    else break;
                }
            }
        }
    }

    if(nFoes == 0)
        free(targets);
    return targets;
}

int get_BestMatchup(B_Unit *unit, B_Unit *foes, int nFoes, B_Map *map)
{
    int bestAdvtg = -100, posibAdvtg = -100, index = NO_UNIT;
    B_Pos foe_Pos;
    bool ranged = false;
    // Checking if units exists
    if((unit && nFoes < 1) || (unit->position.X < 0 || unit->position.Y < 0))
        return FUNCTION_FAIL;

    for(int i = 0; i < nFoes; i++)
    {
        if(foes[i].position.X < 0 || foes[i].position.Y < 0)
            continue;

        if(autoMove(map, 
            &map->tiles[unit->position.Y][unit->position.X],
            &map->tiles[foes[i].position.Y][foes[i].position.X]) == NULL)
            continue;

        posibAdvtg = get_BonusByClass(unit->type, foes[i].type, false);
        // + get_BonusByHeight(map->tiles[unit->position.Y][unit->position.X].elevation -
        // map->tiles[foes[i].position.Y][foes[i].position.X].elevation)
        // + get_BonusByVeget(map->tiles[unit->position.Y][unit->position.X].vegetation);
        if(posibAdvtg > bestAdvtg)
        {
            index = foes[i].ID, bestAdvtg = posibAdvtg;
            if (unit->ammo > 0 && unit->range > 0 && unit->attack_RangeP > foes[i].defend_RangeP)
                ranged = true;
            else ranged = false;
        }
    }
    return index;
}

int get_ClosestEnemyMove(B_Side *enemies, B_Pos from)
{
    int closest = __INT_MAX__, test = 0;;
    for(int i = 0; i < enemies->size; i++)
    {
        test = (enemies->units[i].position.X > enemies->units[i].position.Y) ?
                enemies->units[i].position.X : enemies->units[i].position.Y;
        if(test < closest)
            closest = test;
    }
    return closest;
}

int get_ClosestEnemyID(B_Side *enemies, B_Pos from)
{
    int closest = 0, test = 0, buffer = __INT_MAX__;
    for(int i = 0; i < enemies->size; i++)
    {
        test = (enemies->units[i].position.X > enemies->units[i].position.Y) ?
                enemies->units[i].position.X : enemies->units[i].position.Y;
        if(test < buffer)
            buffer = test, closest = i;
    }
    return closest;
}

T_Response AI_Process(B_Map *map, B_Side *ours, B_Side *they, B_Unit *current, T_Level lvl)
{
    if(get_ClosestEnemyMove(they, current->position) < current->moves);
        return AI_Retreat;

    if(current->range > 0 && current->ammo > 0)
    {
        B_Unit **inRange = get_UnitsInRange(current, map, they->size, current->range);
        if(inRange != NULL)
        {
            int rangeID = get_ClosestEnemyID(*inRange, current->position);
            free(inRange);
        }
    }
    return AI_Wait;
}