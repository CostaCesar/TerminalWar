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

    for(int i = minCoord.Y; i <= maxCoord.Y; i++)
    {
        for(int j = minCoord.X; j <= maxCoord.X; j++)
        {
            if(map->tiles[i][j].unit != NULL)
            {
                if(map->tiles[i][j].unit->Game_ID % 2 == unit->Game_ID % 2) // Same team
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
            // B_Pos cDiff = { abs(foes[i].position.X - unit->position.X),
            //                 abs(foes[i].position.Y - unit->position.Y) };
            // B_Pos pDiff = { abs(foes[index].position.X - unit->position.X),
            //                 abs(foes[index].position.Y - unit->position.Y) };
            int cDist = calcMoves(foes[i].position, unit->position); //cDiff.X > cDiff.Y ? cDiff.X : cDiff.Y; 
            int pDist = calcMoves(foes[index].position, unit->position); // pDiff.X > pDiff.Y ? pDiff.X : pDiff.Y; 
            
            index = pDist < cDist ? i : index;              
        }
    }
    return &foes[index].Game_ID;
}

int get_ClosestEnemyMove(B_Side *enemies, B_Pos from)
{
    int closest = 0x7fffffff, test = 0;;
    for(int i = 0; i < enemies->size; i++)
    {
        if(enemies->units[i].position.X < 0 || enemies->units[i].position.Y < 0)
            continue;
        // B_Pos diff = { abs(enemies->units[i].position.X - from.X), abs(enemies->units[i].position.Y - from.Y) };
        test = calcMoves(enemies->units[i].position, from); //diff.X > diff.Y ? diff.X : diff.Y;
        if(test < closest)
            closest = test;
    }
    return closest;
}

B_Unit *get_ClosestUnit(B_Side *side, B_Pos from)
{
    int closest = 0x7fffffff, test = 0;
    B_Unit *out = NULL;
    for(int i = 0; i < side->size; i++)
    {
        if(side->units[i].position.X < 0 || side->units[i].position.Y < 0)
            continue;
        if(compPos(from, side->units[i].position) == true)
            continue;
        test = calcMoves(side->units[i].position, from);
        if(test < closest)
            closest = test, out = &side->units[i];
    }
    return out;
}

B_Pos get_BestTileRanged(B_Map *map, B_Unit *unit, B_Unit *foe)
{
    B_Pos start = { foe->position.X - unit->range < 0 ? 0 : foe->position.X - unit->range,
                    foe->position.Y - unit->range < 0 ? 0 : foe->position.Y - unit->range};
    B_Pos finsh = { foe->position.X + unit->range > map->width ? map->width : foe->position.X + unit->range,
                    foe->position.Y + unit->range > map->height ? map->height : foe->position.Y + unit->range};
    B_Pos best = {-1, -1};
    int bestDmg = 0, possibDmg = 0, bestMoves = 0x7fffffff, possibMoves = 0;
    for(int i = start.Y; i <= finsh.Y; i++)
    {
        for(int j = start.X; j < finsh.X; j++)
        {
            // MUDAR para -1
            if(abs(i - foe->position.Y) <= foe->moves -1 && abs(j - foe->position.X) <= foe->moves -1)
                continue;
            
            // comp.X = abs(j - unit->position.X), comp.Y = abs(i - unit->position.Y);
            // if(comp.X > comp.Y)
            //     possibMoves = comp.X;
            // else possibMoves = comp.Y;
            possibMoves = calcMoves(unit->position, (B_Pos){j, i});
            
            possibDmg = get_HeightDif(map, (B_Pos){j,i}, foe->position);
            if((possibMoves - possibDmg < bestMoves - bestDmg) || (possibDmg == bestDmg && possibMoves < bestMoves))
            {
                bestMoves = possibMoves, bestDmg = possibDmg;
                best.X = j, best.Y = i;
            }
            else if (possibDmg == bestDmg && possibMoves == bestMoves)
            {
                if(calcDistance((B_Pos){j, i}, unit->position) < calcDistance(best, unit->position))
                {
                    bestMoves = possibMoves, bestDmg = possibDmg;
                    best.X = j, best.Y = i;
                }
            }
        }
    }
    return best;
}

B_Pos get_BestTileMatch(B_Map *map, B_Pos from, B_Unit* unit)
{
    int best = -100, bestI = -1;
    double possib = -1;
    
    B_Tile* test[8];
    B_Tile* foe_Tile = &map->tiles[from.Y][from.X];
    B_Unit* foe = map->tiles[from.Y][from.X].unit;
    
    for(int i = 0; i <= Northwest; i++)
    {
        test[i] = get_AdjTile(map, from, i);
        if(test[i] == NULL) continue;

        int heightDif = get_HeightDif(map, test[i]->pos, from);
        possib = get_BonusByHeight(heightDif);
        
        // When we defend
        possib += handle_Advantages(unit, test[i]->terrain, test[i]->vegetation, heightDif);
        // When we attack
        possib += handle_Advantages(unit, foe_Tile->terrain, foe_Tile->vegetation, -heightDif);
        // When they attack
        possib -= handle_Advantages(foe, test[i]->terrain, test[i]->vegetation, heightDif);
        // When they defend
        possib -= handle_Advantages(foe, foe_Tile->terrain, foe_Tile->vegetation, -heightDif);
        
        if(possib > best)
            best = possib, bestI = i;
        else if(possib == best)
        {
            B_Pos unitPos = unit->position;
            int cDist = get_MovesToTile(map, &map->tiles[unitPos.Y][unitPos.X], test[bestI]); 
            int pDist = get_MovesToTile(map, &map->tiles[unitPos.Y][unitPos.X], test[i]);
            
            bestI = pDist <= cDist ? i : bestI;             
        }
    }
    return test[bestI]->pos;
}

B_Pos get_BestTileFort(B_Map *map, B_Pos this, B_Pos closest_Foe, B_Pos closest_Ally, B_Unit* us)
{
    B_Pos best = {-1, -1};
    bool hasAlly = closest_Ally.X > -1 && closest_Ally.Y > -1;
    int bestTileStats = -1, bestHeight = -1, bestDist = map->height + map->width;
    int posbTileStats, posbHeight, posbDist;
    for(int i = 0; i < map->height; i++)
    {
        for(int j = 0; j < map->width; j++)
        {
            if(map->tiles[i][j].elevation < bestHeight)
                continue;//map->tiles[unit->position.Y][unit->position.X].elevation)
            else posbHeight = map->tiles[i][j].elevation;
            
            if(get_BonusByVeget(map->tiles[i][j].vegetation)
             + handle_Advantages(us, map->tiles[i][j].terrain, map->tiles[i][j].vegetation, 0) < bestTileStats)
                continue;
            else posbTileStats = get_BonusByVeget(map->tiles[i][j].vegetation)
                                 + handle_Advantages(us, map->tiles[i][j].terrain, map->tiles[i][j].vegetation, 0);
            
            if(hasAlly == false)
                continue;
            else if(bestDist < calcMoves((B_Pos){j, i}, closest_Ally))
                continue;
            else posbDist = calcMoves((B_Pos){j, i}, closest_Ally);

            if (calcMoves(best, this) - bestTileStats - bestHeight - posbDist >
                calcMoves((B_Pos){j, i}, this) - posbHeight - posbTileStats - posbDist)
            {
                bestHeight = posbHeight, bestTileStats = posbTileStats;
                bestDist = posbDist, best.X = j, best.Y = i;
            }
        }
    }
    return best;
}

T_Response AI_Process(B_Map *map, B_Side *ours, B_Side *they, B_Unit *current, T_Level lvl)
{
    B_Pos test;
    B_Unit *closest_Foe = get_ClosestUnit(they, current->position);
    B_Unit *closest_Ally = get_ClosestUnit(ours, current->position);
    B_Tile *current_Tile = &map->tiles[current->position.Y][current->position.X];
    
    // Alvo atual ainda existe?
    if((current->chaseID) && get_UnitIndex(they, *current->chaseID) == FUNCTION_FAIL)
        current->goal.X = NO_UNIT, current->goal.Y = NO_UNIT, current->chaseID = NULL;
    
    // Se muito perto, recuar
    // talvex só ranged ou algo assim
    // IMPLEMENTAR?
    if((get_UnitPowerGap(current, closest_Foe,
                         get_HeightDif(map, current->position, closest_Foe->position),
                         current_Tile->fortLevel, false) <= 0.5
        && calcMoves(closest_Foe->position, current->position) < closest_Foe->moves 
        && closest_Foe->attack_MeleeP > current->defend_MeleeP
        && closest_Foe->defend_RangeP > current->attack_MeleeP)
        ||
        (get_UnitPowerGap(current, closest_Foe,
                         get_HeightDif(map, current->position, closest_Foe->position),
                         current_Tile->fortLevel, true) <= 0.5
        && calcMoves(closest_Foe->position, current->position) < closest_Foe->range 
        && closest_Foe->attack_RangeP > current->defend_RangeP))
        return AI_Retreat;

    // Atacar alvos a distância
    if(current->range > 0 && current->ammo > 0)
    {
        int nFoes = 0;
        B_Unit **inRange = get_UnitsInRange(current, map, &nFoes, current->range);
        if(inRange != NULL && current->attacked == false)
        {
            current->goal = closest_Foe->position; // MUDAR
            free(inRange);
            return AI_Fire;
        }
        else if(current->attacked == true)
        {
            // Corret para longe
        }
    }

    // Buscar posição avantajosa
    if(ours->attacker == false)
    {
        test = get_BestTileFort(map, current->position, closest_Foe->position, closest_Ally->position, current);
        if(test.X != -1 && test.Y != -1)
        {
            current->goal = test;
            if(current->build_Cap < 1)
                return AI_Wait;
            if(current->position.X == test.X && current->position.Y == test.Y)
                return AI_Fortify;
            if(autoMove(map, current_Tile, &map->tiles[test.Y][test.X]) != NULL)
                return AI_GoTo;
            // Posição alvo é uma unidade amiga, como resolver?
            // IMPLEMENTAR
            
            // Se for atacada, unidade não pode fortificar e unidade tem mobilidade...
            // IMPLEMENTAR
        }
    }

    // Ir para posição ranged
    if(closest_Foe->defend_RangeP < current->attack_RangeP)
    {
        current->goal = get_BestTileRanged(map, current, closest_Foe);
        if(current->goal.X != -1 && current->goal.Y != -1)
            return AI_GoTo;
    }

    // Buscar combate melee
    if(current->morale > 70 && current->men > current->men_Max / 3) 
    {
        // Obtendo alvo e melhor angulo de ataque
        current->chaseID = get_BestMatchup(current, they->units, they->size, map);
        current->goal = get_BestTileMatch(map, closest_Foe->position, current);
        
        // Se estiver do lado, engajar
        if(compPos(current->goal, current->position) == true)
        {
            current->goal = closest_Foe->position;
            return AI_Engage;
        }
        else if(current->attacked == true)
        {
            // Corret para longe
        }
        else return AI_GoTo;
    }
    return AI_Wait;
}