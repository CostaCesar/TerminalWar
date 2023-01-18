#pragma once

#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "UI.h"
#include "unit.h"

typedef struct S_Result
{
    short int attackerLoss;
    short int defenderLoss;
    
    B_Unit *winner;
    B_Unit *looser;

    bool isDraw;
} B_Result;

int get_BonusByHeight(double height)
{
    if(height == 0)
        return 0;   
    else if (height < 0)
        return ((int) pow(2, (height * -1)) * -1);
    else return (int) pow(2, height);
}

double handle_Advantages(B_Unit *unit, T_Terrain terrain, T_Veget vegetation, int height)
{
    double value = 0.0;
    if((unit_HasBuff(unit, Desert_Advtg) && terrain == Sand)
    || (unit_HasBuff(unit, Amphibious) && terrain == Water)
    || (unit_HasBuff(unit, Snow_Advtg) && terrain == Snow))
        value += DAMAGE_SMALL;
    
    if(unit_HasBuff(unit, Forest_Advtg) && vegetation > Sparse)
        value += DAMAGE_SMALL;
    
    if(unit_HasBuff(unit, Height_Block) && height < 0)
        value += get_BonusByHeight(-height);
    return value;
}

double get_BonusByVeget(int value)
{ return ((double) value / 2.0); }

int get_UnitIndex(B_Side *side, int ID)
{
    for (int i = 0; i < side->size; i++)
    {
        if (side->units[i].Game_ID == ID)
            return i;
    }
    printf("ERROR: get_UnitIndex >> 1");
    return FUNCTION_FAIL;
}

int get_BonusByClass(T_Class attacker_Type, T_Class defender_Type, bool fromRange)
{
    int output = 0;
    // Bonus againts siege (if not siege itself, excluding Medium Siege)
    if((defender_Type >= Lg_Siege && defender_Type <= Hv_Siege) && (attacker_Type != Lg_Siege && attacker_Type != Hv_Siege))
    {
        output += DAMAGE_LARGE;
        return output;
    }
    
    // Looking for attacker bonuses
    switch (attacker_Type)
    {
    case Lg_Infantry:
        // Light Invantry
        // Good X Heavier Infantry
        if(defender_Type == Md_Infantry || defender_Type == Hv_Infantry)
            output += DAMAGE_MEDIUM;
        // Somewaht good X Light_Charriots
        else if (defender_Type == Lg_Charriot)
            output += DAMAGE_SMALL;
        break;
    case Md_Infantry:
        // Medium Infantry
        // Very Good X Other Infantry
        if (defender_Type == Lg_Infantry || defender_Type == Hv_Infantry)
            output += DAMAGE_LARGE;
        break;
    case Hv_Infantry:
        // Heavy Infantry
        // Very Good X Cavalary
        if (defender_Type >= Lg_Cavalary && defender_Type <= Hv_Cavalary)
            output += DAMAGE_LARGE;
        // Good X Charriots
        else if (defender_Type == Lg_Charriot || defender_Type == Hv_Charriot)
            output += DAMAGE_MEDIUM;
        break;
    case Lg_Cavalary:
        // Light Cavalry
        // Good X Charriots
        if(defender_Type == Lg_Charriot || defender_Type == Hv_Charriot)
            output += DAMAGE_MEDIUM;
        break;
    case Md_Cavalary:
        // Medium Cavalary
        // Good X Cavalaries
        if(defender_Type >= Lg_Cavalary && defender_Type <= Hv_Cavalary)
            output += DAMAGE_MEDIUM;
        break;
    case Hv_Cavalary:
        // Heavy Cavalary
        // Very Good X Light and Medium Infantary
        if(defender_Type == Lg_Infantry || defender_Type == Md_Infantry)
            output += DAMAGE_LARGE;
        // Somewhat good X Meele Charriots
        else if (defender_Type == Hv_Charriot)
            output += DAMAGE_SMALL;
        break;
    case Lg_Charriot:
        // Light Charriot
        // Very good X Medium Cavalary and Medium Infantry
        if (defender_Type == Md_Cavalary || defender_Type == Md_Infantry)
            output += DAMAGE_LARGE;
        // Somewhat good X Heavy Cavalary and Heavy Infantry
        else if(defender_Type == Hv_Cavalary || defender_Type == Hv_Infantry)
            output += DAMAGE_SMALL;
        break;
    case Hv_Charriot:
        // Heavy Charriot
        // Very good X Light and Medium Cavalary
        if(defender_Type == Lg_Cavalary || defender_Type == Md_Cavalary)
            output += DAMAGE_LARGE;
        // Good X Heavy infantry
        else if (defender_Type == Hv_Infantry)
            output += DAMAGE_MEDIUM;
        break;
    case Lg_Siege:
        // Light Siege
        // Good X Infantry
        // IMPLEMENT
        break;
    case Md_Siege:
        // Light Siege
        // Somewhat Good X Other siege
        // IMPLEMENT
        break;
    case Hv_Siege:
        // Light Siege
        // Somewhat Good X ??
        // IMPLEMENT
        break;
    // What if attacker has no type?   
    default:
        fprintf(stderr, "No type defined!");
        break;
    }

    // If ranged attack, ignore defender bonus
    if (fromRange)
        return output;
    
    // Looking for defender Bonuses
    switch (defender_Type)
    {
    case Lg_Infantry:
        // Light Invantry
        // Good X Heavier Infantry
        if(attacker_Type == Md_Infantry || attacker_Type == Hv_Infantry)
            output -= DAMAGE_MEDIUM;
        // Somewaht good X Light_Charriots
        else if (attacker_Type == Lg_Charriot)
            output -= DAMAGE_SMALL;
        break;
    case Md_Infantry:
        // Medium Infantry
        // Very Good X Other Infantry
        if (attacker_Type == Lg_Infantry || attacker_Type == Hv_Infantry)
            output -= DAMAGE_LARGE;
        break;
    case Hv_Infantry:
        // Heavy Infantry
        // Very Good X Cavalary
        if (attacker_Type >= Lg_Cavalary && attacker_Type <= Hv_Cavalary)
            output -= DAMAGE_LARGE;
        // Good X Charriots
        else if (attacker_Type == Lg_Charriot || attacker_Type == Hv_Charriot)
            output -= DAMAGE_MEDIUM;
        break;
    case Lg_Cavalary:
        // Light Cavalry
        // Good X Charriots
        if(attacker_Type == Lg_Charriot || attacker_Type == Hv_Charriot)
            output -= DAMAGE_MEDIUM;
        break;
    case Md_Cavalary:
        // Medium Cavalary
        // Good X Cavalaries
        if(attacker_Type >= Lg_Cavalary && attacker_Type <= Hv_Cavalary)
            output -= DAMAGE_MEDIUM;
        break;
    case Hv_Cavalary:
        // Heavy Cavalary
        // Very Good X Light and Medium Infantary
        if(attacker_Type == Lg_Infantry || attacker_Type == Md_Infantry)
            output -= DAMAGE_LARGE;
        // Somewhat good X Meele Charriots
        else if (attacker_Type == Hv_Charriot)
            output -= DAMAGE_SMALL;
        break;
    case Lg_Charriot:
        // Light Charriot
        // Very good X Medium Cavalary and Medium Infantry
        if (attacker_Type == Md_Cavalary || attacker_Type == Md_Infantry)
            output -= DAMAGE_LARGE;
        // Somewhat good X Heavy Cavalary and Heavy Infantry
        else if(attacker_Type == Hv_Cavalary || attacker_Type == Hv_Infantry)
            output -= DAMAGE_SMALL;
        break;
    case Hv_Charriot:
        // Heavy Charriot
        // Very good X Light and Medium Cavalary
        if(attacker_Type == Lg_Cavalary || attacker_Type == Md_Cavalary)
            output -= DAMAGE_LARGE;
        // Good X Heavy infantry
        else if (attacker_Type == Hv_Infantry)
            output -= DAMAGE_MEDIUM;
        break;
    // What if defender has no type?   
    default:
        fprintf(stderr, "No type defined!");
        break;
    }
    return output;
}

float get_UnitPowerGap(B_Unit *attacker, B_Unit *defender, int HeightDiff, int atck_Fort, bool ranged)
{
    float attacker_Power = attacker->attack_MeleeP * (attacker->morale / 100) + attacker->level;
    float defender_Power = defender->defend_MeleeP * (defender->morale / 100) + defender->level;
    
    if(attacker_Power < 0)
        attacker_Power = 0;
    if(defender_Power < 0)
        defender_Power = 0;

    attacker_Power += HeightDiff;
    attacker_Power += (atck_Fort / 2.0f);
    return attacker_Power - defender_Power + get_BonusByClass(attacker->type, defender->type, ranged);
}

B_Result execute_MeleeCombat(B_Unit *attacker, B_Unit *defender, int heightDif, short int *fortLevel, T_Terrain terrain, T_Veget vegetation)
{
    B_Result result = {0};
    float gap = 0.0;
    short int A_Buffer = 0, D_Buffer = 0;

    float attacker_Power = attacker->attack_MeleeP * (attacker->morale / 100) + (rand() % 10) + attacker->level;
    float defender_Power = defender->defend_MeleeP * (defender->morale / 100) + (rand() % 10) + defender->level;
    
    // Limits
    if(attacker_Power < 0)
        attacker_Power = 0;
    if(defender_Power < 0)
        defender_Power = 0;

    // Buffs
    if(unit_HasBuff(attacker, Charge_Buff) == true && attacker->men == attacker->men_Max)
        attacker_Power += DAMAGE_LARGE;
    attacker_Power += handle_Advantages(attacker, terrain, vegetation, heightDif);
    defender_Power += handle_Advantages(defender, terrain, vegetation, heightDif);
    
    // Calculating battle results
    float combat_Result = attacker_Power - defender_Power
                        + get_BonusByHeight(heightDif)
                        - ((float) *fortLevel / 2.0)
                        + get_BonusByClass(attacker->type, defender->type, false);
    if(combat_Result > *fortLevel && *fortLevel > 0)
    {
        (*fortLevel)--;
        char msg[46];
        snprintf(msg, sizeof(msg), "Fort level at %hdX and %hdY decreased to %hd!", defender->position.X, defender->position.Y, *fortLevel);
        print_Message(msg, get_ScreenWidth(), 1, true, false, false);   
    }

    // Limits
    if(combat_Result > RESULT_CAP)
        combat_Result = RESULT_CAP;
    else if (combat_Result < (RESULT_CAP * -1))
        combat_Result = RESULT_CAP * -1;
    A_Buffer = attacker->men, D_Buffer = defender->men;

    // Returnig winner or definig draw
    if(combat_Result > 0)
    {
        // Attacker takes damage
        gap = 1.0 + ((float) ( 1.0 / pow(MORALE_EXTREMNESS, (double) combat_Result)) 
                * (defender->men / attacker->morale));
        attacker->morale -= gap, attacker->men -= (gap / 100 * attacker->men_Max);

        // Defender takes damage
        gap = 1.0 + (pow(MORALE_EXTREMNESS, (double) combat_Result)
                * (attacker->men / defender->morale));
        defender->morale -= gap, defender->men -= (gap / 100 * defender->men_Max);
        
        if(defender->morale <= 0 ||  defender->men <= 0)
        {
            if(defender->men < 0)
                defender->men = (short int) (rand() % defender->men);
            else defender->men = 0;
            defender->retreating = true;
            defender->engaged = false;
            defender->morale = 0.0f;
        }

        result.winner = attacker;
        result.looser = defender;
    }
    else if (combat_Result < 0)
    {  
        // Defender takes damage
        gap = 1.0 + ((float) ( 1.0 / pow(MORALE_EXTREMNESS, (double) -combat_Result))
                * (attacker->men / defender->morale));
        defender->morale -= gap, defender->men -= (gap / 100 * defender->men_Max);

        // Attacker takes damage
        gap = 1.0 + ((float) (pow(MORALE_EXTREMNESS, (double) -combat_Result)
                * (defender->men / attacker->morale)));

        attacker->morale -= gap, attacker->men -= (gap / 100 * attacker->men_Max);
        
        if(attacker->morale <= 0 || attacker->men <= 0)
        {
            if(attacker->men < 0)
                attacker->men = (short int) (rand() % attacker->men);
            else attacker->men = 0;
            attacker->retreating = true;
            attacker->engaged = false;
            attacker->morale = 0.0f;
        }

        result.winner = defender;
        result.looser = attacker;
    }
    else return execute_MeleeCombat(attacker, defender, heightDif, fortLevel, terrain, vegetation);
    
    // Stats
    result.attackerLoss = A_Buffer - attacker->men;
    result.defenderLoss = D_Buffer - defender->men;

    if(attacker->retreating == true && defender->retreating == true)
        result.isDraw = true;
    return result;
}

void tryHeal_Unit(B_Side* Side_A, B_Side* Side_B)
{
    // Cheking for units in side A
    for(int i = 0; i < Side_A->size; i++)
    {
        if(Side_A->units[i].retreating == true && Side_A->units[i].engaged == false)
        {
            Side_A->units[i].morale += (rand() % 10);
            // Can't regain > 100 points
            if(Side_A->units[i].morale > 100.0f)
            {
                Side_A->units[i].morale = 100.0f;  
                Side_A->units[i].retreating = false;        
            }
        }
    }

    // Cheking for units in side B
    for(int i = 0; i < Side_B->size; i++)
    {
        if(Side_B->units[i].retreating == true && Side_B->units[i].engaged == false)
        {
            Side_B->units[i].morale += (rand() % 10);
            // Can't regain > 100 points
            if(Side_B->units[i].morale > 100.0f)
            {
                Side_B->units[i].morale = 100.0f;    
                Side_B->units[i].retreating = false;
            }
        }
    }
}

bool show_Combat(B_Result *units)
{
    bool out = false;
    // Normalizing men (just in case)
    if(units->winner->men < 0)
        units->winner->men = 0;
    if(units->looser->men < 0)
        units->looser->men = 0;

    if(units->isDraw == true)
    {
        printf("Despite valiant efforts, neither side came out on top. Both units now run from the battle! \n");
    }
    else if (units->winner->retreating == false && units->looser->retreating == false)
    {
        printf("%s[%s] is gaining the upper hand! \n", units->winner->name, units->winner->faction);
        printf("%s[%s] is falling apart! \n", units->looser->name, units->looser->faction);        
    } 
    else
    {
        printf("%s has won today: For the glory of the %s! \n", units->winner->name, units->winner->faction);
        printf("%s was doomed today: A shamefull day for the %s! \n", units->looser->name, units->looser->faction);  
        out = true;   
    }
    return out;
}

bool check_UnitMove(B_Unit *unit, int *moves)
{
    
    if((*moves) >= unit->moves)
        return false;
    if(unit->attacked == true && unit_HasBuff(unit, Hit_And_Move) == false)
        return false;
    
    if(unit->engaged == false)
        return true;
    else if(unit->engaged == true && (*moves) + 1 < unit->moves)
    {
        unit->engaged = false;
        (*moves)++;
        print_Message("Disengaging!", get_HalfWidth(), 1, true, false, true);
        return true;
    }
    else return false;
}

int check_Ranged(B_Unit *attacker, B_Unit *defender)
{
    char msg[STRING_NAME];
    if(defender->position.X > attacker->position.X + attacker->range
    || defender->position.X < attacker->position.X - attacker->range
    || defender->position.Y > attacker->position.Y + attacker->range
    || defender->position.X < attacker->position.X - attacker->range)
    {
        print_Message("The units are too far away from eachother!", get_HalfWidth(), 1, true, false, true);   
        return FUNCTION_FAIL;
    }

    // Testing if attacker has ammo
    if(attacker->ammo <= 0)
    {
        print_Message("This has no projectiles. They can't fire!", get_HalfWidth(), 1, true, false, true);   
        return FUNCTION_FAIL;       
    }

    return FUNCTION_SUCESS;
}

B_Result execute_RangedCombat(B_Unit* attacker, B_Unit* defender, int heightDif, T_Veget vegetat, T_Terrain terrain, short int *fortLevel)
{
    // Trivia
    B_Result result = {0};
    result.winner = attacker, result.looser = defender;
    attacker->attacked = true;
    attacker->ammo--;

    // Doing damage
    float volley = attacker->attack_RangeP * (attacker->morale / 100) + (rand() % 10) + attacker->level;
    float protec = defender->defend_RangeP * (defender->morale / 100) + (rand() % 10) + defender->level;

    // Buffs
    protec += handle_Advantages(defender, terrain, vegetat, heightDif);
    if(unit_HasBuff(defender, Shield_Wall) && defender->engaged == false)
        protec += DAMAGE_SMALL;
    // Negating amphibious buff
    if(unit_HasBuff(defender, Amphibious) == true && terrain == Water)
        protec -= DAMAGE_SMALL;

    float damage = volley - protec + get_BonusByHeight(heightDif) - get_BonusByVeget(vegetat)- ((float) *fortLevel / 2)
        + get_BonusByClass(attacker->type, defender->type, true);
    if(damage > RESULT_CAP)
        damage = RESULT_CAP;
    
    int men_Buffer = defender->men;
    float gap = 1.0 + (pow(MORALE_EXTREMNESS, (double) damage)
                * (attacker->men / defender->morale) / 3.0);
    defender->morale -= gap, defender->men -= (gap / 100 * defender->men_Max);
    
    if(defender->morale <= 0 ||  defender->men <= 0)
    {
        defender->men = (short int) (rand() % defender->men);
        defender->retreating = true;
        defender->engaged = false;
        defender->morale = 0.0f;
    }
    
    result.defenderLoss = men_Buffer - defender->men;
    return result;
}
