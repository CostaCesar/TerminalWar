#pragma once

#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "UI.h"
#include "unit.h"

typedef struct S_Result
{
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

B_Result combat_Unit(B_Unit *attacker, B_endStats *attackStats,
                     B_Unit *defender, B_endStats *defendStats,
                     int heightDif, short int *fortLevel)
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
    if(unit_HasBuff(attacker, Charge_Buff) == true)
        attacker_Power += DAMAGE_SMALL;
    
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
        print_Message(msg, false);   
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
        gap = 1 + ((float) ( 1.0 / pow(MORALE_EXTREMNESS, (double) combat_Result)) 
                * (defender->men / attacker->morale));
        attacker->morale -= gap, attacker->men -= (gap / 100 * attacker->men_Max);

        // Defender takes damage
        gap = 1 + (pow(MORALE_EXTREMNESS, (double) combat_Result)
                * (1 / (attacker->men / defender->morale)));
        defender->morale -= gap, defender->men -= (gap / 100 * defender->men_Max);
        
        if(defender->morale <= 0 ||  defender->men <= 0)
        {
            defender->men = (short int) (rand() % defender->men);
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
        gap = 1 + ((float) ( 1.0 / pow(MORALE_EXTREMNESS, (double) -combat_Result))
                * (attacker->men / defender->morale));
        defender->morale -= gap, defender->men -= (gap / 100 * defender->men_Max);

        // Attacker takes damage
        gap = 1 + (pow(MORALE_EXTREMNESS, (double) -combat_Result)
                * (1 / (attacker->men / defender->morale)));

        attacker->morale -= gap, attacker->men -= (gap / 100 * attacker->men_Max);
        
        if(attacker->morale <= 0 || attacker->men <= 0)
        {
            attacker->men = (short int) (short int) (rand() % attacker->men);
            attacker->retreating = true;
            attacker->engaged = false;
            attacker->morale = 1.0f;
        }

        result.winner = defender;
        result.looser = attacker;
    }
    else return combat_Unit(attacker, attackStats, defender, defendStats, heightDif, fortLevel);
    
    // Stats
    attackStats->killed += D_Buffer - defender->men;
    attackStats->loss += A_Buffer - attacker->men;
    defendStats->killed += A_Buffer - attacker->men;
    defendStats->loss += D_Buffer - defender->men;

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

bool show_Combat_Result(B_Result *units)
{
    bool out = false;
    if(units->isDraw)
    {
        printf("Despite valiant efforts, neither side came out on top. Both units now run from the battle! \n");
    }
    else if (units->winner->retreating == false && units->looser->retreating == false)
    {
        printf("%s is gaining the upper hand! \n", units->winner->name);
        printf("%s is falling apart! \n", units->looser->name);        
    } 
    else
    {
        printf("%s has won today: For the glory of the %s! \n", units->winner->name, units->winner->faction);
        printf("%s was doomed today: A shamefull day for the %s! \n", units->looser->name, units->looser->faction);  
        out = true;   
    }
    return out;
}


// Centralization of the combat process
void do_Combat(B_Unit* attacker, B_endStats* attackStats, B_Unit* defender, B_endStats* defendStats, int heigthDif, short int *fortLevel)
{
    // Unit cannot be already engaged at combat
    if(attacker->engaged == true)
    {
        /* printf("\n");
        printf("#===============================================# \n");
        printf("| We must disengage the enemy before attacking! | \n");
        printf("#===============================================# \n"); */
        print_Message("We must disengage the enemy before attacking!", true);    
        // attacker->engaged = false;
        return;
    }
    // Trivia
    attacker->attacked = true;
    attacker->engaged = true;
    defender->engaged = true;

    B_Result Result = combat_Unit(attacker, attackStats, defender, defendStats, heigthDif, fortLevel);
    bool LevelUP = show_Combat_Result(&Result);
    // Level up
    if(LevelUP == true)
        Result.winner->level++; 
    // Whoever wins gains the innitiative (can disengage/engage)
    Result.winner->engaged = false;
    
    return;
}

bool check_UnitMove(B_Unit *unit, int moves)
{
    
    if(moves >= unit->moves)
        return false;
    if(unit->retreating == true)
        return false;
    if(unit->attacked == true && unit_HasBuff(unit, Hit_And_Move) == false)
        return false;
    
    if(unit->engaged == false)
        return true;
    else if(unit->engaged == true && moves + 1 < unit->moves)
    {
        unit->engaged = false;
        moves++;
        print_Message("Disengaging!", true);
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
        /* printf("\n");
        printf("#============================================# \n");
        printf("| The units are too far away from eachother! | \n");
        printf("#============================================# \n"); */
        print_Message("The units are too far away from eachother!", true);   
        return FUNCTION_FAIL;
    }

    // Testing if attacker has ammo
    if(attacker->ammo <= 0)
    {
        /* printf("\n");
        printf("#============================================# \n");
        printf("| This has no projectiles. They can't fire!  | \n");
        printf("#============================================# \n"); */
        print_Message("This has no projectiles. They can't fire!", true);
        return FUNCTION_FAIL;       
    }

    // Testing if attacker is routing
    if(attacker->retreating == true)
    {
        /* printf("\n");
        printf("#============================================# \n");
        printf("| This unit will not fire, they're routing!  | \n");
        printf("#============================================# \n"); */     
        print_Message("This unit will not fire, they're routing!", true);
        return FUNCTION_FAIL;       
    }
    return FUNCTION_SUCESS;
}

int do_Combat_Ranged(B_Unit* attacker, B_endStats* attackerStats,
                    B_Unit* defender, B_endStats* defenderStats,
                    int heightDif, T_Veget vegetat, T_Terrain terrain, short int *fortLevel)
{
    char msg[51];
    // Testing 
    if(check_Ranged(attacker, defender) == FUNCTION_FAIL)
        return FUNCTION_FAIL;

    // If attaker != cavalary OR charriot, engage
    // if(attacker->type < Lg_Cavalary || attacker->type > Hv_Charriot)
    //     attacker->inCombat = true;
    // Defender must be engaged
    // defender->inCombat = true;

    // Signaling attack
    system("cls");
    print_Line(NULL);
    print_Line(" ");
    snprintf(msg, sizeof(msg), "%s [%s] at %hdX %2hdY", attacker->name, attacker->faction, attacker->position.X, attacker->position.Y);
    print_Line(msg);
    print_Line("Is firing a volley into");
    snprintf(msg, sizeof(msg), "%s [%s] at %hdX %2hdY", defender->name, defender->faction, defender->position.X, defender->position.Y);
    print_Line(msg);
    print_Line(" ");
    print_Line(NULL);
    Sleep(TIME_STRATEGY);

    // Trivia
    attacker->attacked = true;

    // Doing damage
    attacker->ammo--;
    float volley = attacker->attack_RangeP * (attacker->morale / 100) + (rand() % 10);
    float protec = defender->defend_RangeP * (defender->morale / 100) + (rand() % 10);

    if(unit_HasBuff(defender, Desert_Advtg) == true && terrain == Sand)
        protec += DAMAGE_SMALL;
    else if(unit_HasBuff(defender, Amphibious) == true && terrain == Water)
        protec += DAMAGE_SMALL;
    if(unit_HasBuff(defender, Forest_Advtg) == true && vegetat > Sparse)
        protec += DAMAGE_SMALL;
    if(unit_HasBuff(defender, Height_Advtg) && heightDif < 0)
        protec += DAMAGE_SMALL;
    if(unit_HasBuff(defender, Shield_Wall) && defender->engaged == false)
        protec += DAMAGE_SMALL;


    float damage = volley - protec + get_BonusByHeight(heightDif) - get_BonusByVeget(vegetat)- ((float) *fortLevel / 2)
        + get_BonusByClass(attacker->type, defender->type, true);
    if(damage < 0)
        damage = 0.5f;
    else if(damage > RESULT_CAP)
        damage = RESULT_CAP;
    // float damage = attacker->men * ((((attacker->morale / 10) + attacker->level) * add_BonusDamage_ByHeightDif(heightDif) / add_BonusDamage_ByVeget(vegetat)));
    
    int men_Buffer = defender->men;
    defender->morale -= (float) pow(MORALE_LOOSE, (double) damage) * MORALE_EXTREMNESS + MORALE_EXTREMNESS;

    // If morale <= 0, retreat
    if(defender->morale <= 1)
    {
        defender->men = (short int) (1.0f / (float) MORALE_EXTREMNESS) * defender->men_Max;
        defender->retreating = true;
        defender->engaged = false;
        defender->morale = 1.0f;
    }
    else
    {
        defender->men -= (short int) (pow(MORALE_LOOSE, (double) damage)
        * (attacker->men / defender->morale)) * MORALE_EXTREMNESS + MORALE_EXTREMNESS;
        if(defender->men < 0)
        {
            defender->men = (short int) (1.0f / (float) MORALE_EXTREMNESS) * defender->men_Max;
            defender->morale = 0.0f;
            defender->retreating = true;
            defender->engaged = false;
        }
    }
    defenderStats->loss += men_Buffer - defender->men;
    attackerStats->killed += men_Buffer - defender->men;

    // Showing results
    B_Result res = {attacker, defender, false};
    bool LevelUP = show_Combat_Result(&res);
    if(LevelUP == true)
        attacker->level++;

    return FUNCTION_SUCESS;
}
