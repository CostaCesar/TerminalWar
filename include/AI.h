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

    if(*nFoes == 0)
    {
        free(targets);
        targets = NULL;
    }
    return targets;
}

int* get_BestMatchup(B_Unit *unit, B_Unit *foes, int nFoes, B_Map *map)
{
    int bestAdvtg = -100, posibAdvtg = -100, index = NO_UNIT;
    B_Pos foe_Pos;
    // Checking if units exists
    if((unit && nFoes < 1) || (unit->position.X < 0 || unit->position.Y < 0))
        return NULL;

    for(int i = 0; i < nFoes; i++)
    {
        if(foes[i].position.X < 0 || foes[i].position.Y < 0)
            continue;

        T_Direc *path = autoMove(map, 
            &map->tiles[unit->position.Y][unit->position.X],
            &map->tiles[foes[i].position.Y][foes[i].position.X]);
        if(path == NULL)
            continue;
        free(path);

        posibAdvtg = get_BonusByClass(unit->type, foes[i].type, false);
        // + get_BonusByHeight(map->tiles[unit->position.Y][unit->position.X].elevation -
        // map->tiles[foes[i].position.Y][foes[i].position.X].elevation)
        // + get_BonusByVeget(map->tiles[unit->position.Y][unit->position.X].vegetation);
        if(posibAdvtg > bestAdvtg)
            index = i, bestAdvtg = posibAdvtg;
        else if(posibAdvtg = bestAdvtg)
        {
            B_Pos cDiff = { abs(foes[i].position.X - unit->position.X),
                            abs(foes[i].position.Y - unit->position.Y) };
            B_Pos pDiff = { abs(foes[index].position.X - unit->position.X),
                            abs(foes[index].position.Y - unit->position.Y) };
            int cDist = cDiff.X > cDiff.Y ? cDiff.X : cDiff.Y; 
            int pDist = pDiff.X > pDiff.Y ? pDiff.X : pDiff.Y; 
            
            index = pDist < cDist ? i : index;              
        }
    }
    return &foes[index].ID;
}

int get_ClosestEnemyMove(B_Side *enemies, B_Pos from)
{
    int closest = 0x7fffffff, test = 0;;
    for(int i = 0; i < enemies->size; i++)
    {
        if(enemies->units[i].position.X < 0 || enemies->units[i].position.Y < 0)
            continue;
        B_Pos diff = { abs(enemies->units[i].position.X - from.X), abs(enemies->units[i].position.Y - from.Y) };
        test = diff.X > diff.Y ? diff.X : diff.Y;
        if(test < closest)
            closest = test;
    }
    return closest;
}

int get_ClosestEnemyIndex(B_Side *enemies, B_Pos from)
{
    int closest = 0, test = 0, buffer = 0x7fffffff;
    for(int i = 0; i < enemies->size; i++)
    {
        if(enemies->units[i].position.X < 0 || enemies->units[i].position.Y < 0)
            continue;
        B_Pos diff = { abs(enemies->units[i].position.X - from.X), abs(enemies->units[i].position.Y - from.Y) };
        test = diff.X > diff.Y ? diff.X : diff.Y;
        if(test < buffer)
            buffer = test, closest = i;
    }
    return closest;
}

B_Pos get_BestTileRanged(B_Map *map, B_Unit *unit, B_Unit *foe)
{
    B_Pos start = { foe->position.X - unit->range < 0 ? 0 : foe->position.X - unit->range,
                    foe->position.Y - unit->range < 0 ? 0 : foe->position.Y - unit->range};
    B_Pos finsh = { foe->position.X + unit->range > map->width ? map->width : foe->position.X + unit->range,
                    foe->position.Y + unit->range > map->height ? map->height : foe->position.Y + unit->range};
    B_Pos best;
    int bestDmg = -100, possibDmg = 0, bestDist = -100, possibDist = 0;
    for(int i = start.Y; i <= finsh.Y; i++)
    {
        for(int j = start.X; j < finsh.X; j++)
        {
            if(i > start.Y && i < finsh.Y && j > start.X && j < finsh.X)
                continue;
            if(abs(j - unit->position.X) < abs(i - unit->position.Y))
                possibDist = j;
            else possibDist = i;
            
            possibDmg = get_AbsHeigthDif(map, (B_Pos){i,j}, foe->position);
            if(possibDist + possibDmg < bestDist + bestDmg)
            {
                bestDist = possibDist, bestDmg = bestDmg;
                best.X = j, best.Y = i;
            }
        }
    }
    return best;
}

B_Pos get_BestTileMatch(B_Map *map, B_Pos from, B_Unit* unit)
{
    int best = -100, possib = -1, bestI = -1;
    B_Tile* test[8];
    for(int i = 0; i <= Northwest; i++)
    {
        test[i] = get_AdjTile(map, from, i);
        if(test[i]->vegetation >= Grove && unit_HasBuff(unit, Forest_Advtg))
            possib = UNIT_DAMAGE_BONUS_SMALL;
        else if(test[i]->terrain == Desert_Advtg && unit_HasBuff(unit, Desert_Advtg))
            possib = UNIT_DAMAGE_BONUS_SMALL;
        else if(test[i]->elevation > map->tiles[from.Y][from.X].elevation && unit_HasBuff(unit, Height_Advtg))
            possib = UNIT_DAMAGE_BONUS_SMALL;

        if(possib > best)
            best = possib, bestI = i;
        else if(possib == best)
        {
            B_Pos cDiff = { abs(test[bestI]->pos.X - unit->position.X),
                            abs(test[bestI]->pos.Y - unit->position.Y) };
            B_Pos pDiff = { abs(test[i]->pos.X - unit->position.X),
                            abs(test[i]->pos.Y - unit->position.Y) };
            int cDist = cDiff.X > cDiff.Y ? cDiff.X : cDiff.Y; 
            int pDist = pDiff.X > pDiff.Y ? pDiff.X : pDiff.Y; 
            
            bestI = pDist < cDist ? i : bestI;             
        }
    }
    return test[bestI]->pos;
}


T_Response AI_Process(B_Map *map, B_Side *ours, B_Side *they, B_Unit *current, T_Level lvl)
{
    // Alvo atual ainda existe?
    if((current->chaseID) && get_UnitIndex(they, *current->chaseID) == FUNCTION_FAIL)
        current->goal.X = NO_UNIT, current->goal.Y = NO_UNIT, current->chaseID = NULL;
    
    if(get_ClosestEnemyMove(they, current->position) < current->moves)
        return AI_Retreat;

    // Atacar alvos a distância
    if(current->range > 0 && current->ammo > 0)
    {
        int nFoes = 0;
        B_Unit **inRange = get_UnitsInRange(current, map, &nFoes, current->range);
        if(inRange != NULL)
        {
            current->goal = they->units[get_ClosestEnemyIndex(they, current->position)].position;
            free(inRange);
            return AI_Fire;
        }
    }

    // Buscar posição avantajosa
    // IMPLEMENTAR

    // Ir para pisação ranged
    if(they->units[get_ClosestEnemyIndex(they, current->position)].defend_RangeP < current->attack_RangeP)
    {
        B_Tile* 
    }

    // Buscar combate melee
    if(current->morale > 70 && current->men > current->men_Max / 3) 
    {
        // Enquanto tiver um alvo, não reprocessar;
        if(current->goal.X != -1 && current->goal.Y != -1)
            return AI_GoTo;
        
        // Se estiver do lado, engajar
        if(current->goal.X == current->position.X && current->goal.Y == current->position.Y)
            return AI_Engage;

        // Obtendo alvo e melhor angulo de ataque
        current->chaseID = get_BestMatchup(current, they->units, they->size, map);
        current->goal = get_BestTileMatch(map, they->units[get_UnitIndex(they, *current->chaseID)].position, current);
        return AI_GoTo;
    }
    return AI_Wait;
}