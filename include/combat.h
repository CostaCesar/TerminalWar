#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "UI.h"
#include "unit.h"

typedef struct S_Result
{
    B_Unit winner;
    B_Unit looser;
    bool isDraw;
} B_Result;

short int A_Loss = 0, B_Loss = 0;

float add_BonusDamage_ByVeget(int tileVeget)
{
    return (tileVeget / 2.0f + 1);
}

int add_BonusDamage_ByHeightDif(double height)
{
    if(height == 0)
        return 0;   
    else if (height < 0)
        return ((int) pow(2, (height * -1)) * -1);
    else return (int) pow(2, height);
}

int add_BonusDamage_ByClass(T_Class attacker_Type, T_Class defender_Type, bool fromRange)
{
    int output = 0;
    // Bonus againts siege (if not siege itself, excluding Medium Siege)
    if((defender_Type >= Lg_Siege && defender_Type <= Hv_Siege) && (attacker_Type != Lg_Siege && attacker_Type != Hv_Siege))
    {
        output += UNIT_DAMAGE_BONUS_LARGE;
        return output;
    }
    
    // Looking for attacker bonuses
    switch (attacker_Type)
    {
    case Lg_Infantry:
        // Light Invantry
        // Good X Heavier Infantry
        if(defender_Type == Md_Infantry || defender_Type == Hv_Infantry)
            output += UNIT_DAMAGE_BONUS_MEDIUM;
        // Somewaht good X Light_Charriots
        else if (defender_Type == Lg_Charriot)
            output += UNIT_DAMAGE_BONUS_SMALL;
        break;
    case Md_Infantry:
        // Medium Infantry
        // Very Good X Other Infantry
        if (defender_Type == Lg_Infantry || defender_Type == Hv_Infantry)
            output += UNIT_DAMAGE_BONUS_LARGE;
        break;
    case Hv_Infantry:
        // Heavy Infantry
        // Very Good X Cavalary
        if (defender_Type >= Lg_Cavalary && defender_Type <= Hv_Cavalary)
            output += UNIT_DAMAGE_BONUS_LARGE;
        // Good X Charriots
        else if (defender_Type == Lg_Charriot || defender_Type == Hv_Charriot)
            output += UNIT_DAMAGE_BONUS_MEDIUM;
        break;
    case Lg_Cavalary:
        // Light Cavalry
        // Good X Charriots
        if(defender_Type == Lg_Charriot || defender_Type == Hv_Charriot)
            output += UNIT_DAMAGE_BONUS_MEDIUM;
        break;
    case Md_Cavalary:
        // Medium Cavalary
        // Good X Cavalaries
        if(defender_Type >= Lg_Cavalary && defender_Type <= Hv_Cavalary)
            output += UNIT_DAMAGE_BONUS_MEDIUM;
        break;
    case Hv_Cavalary:
        // Heavy Cavalary
        // Very Good X Light and Medium Infantary
        if(defender_Type == Lg_Infantry || defender_Type == Md_Infantry)
            output += UNIT_DAMAGE_BONUS_LARGE;
        // Somewhat good X Meele Charriots
        else if (defender_Type == Hv_Charriot)
            output += UNIT_DAMAGE_BONUS_SMALL;
        break;
    case Lg_Charriot:
        // Light Charriot
        // Very good X Medium Cavalary and Medium Infantry
        if (defender_Type == Md_Cavalary || defender_Type == Md_Infantry)
            output += UNIT_DAMAGE_BONUS_LARGE;
        // Somewhat good X Heavy Cavalary and Heavy Infantry
        else if(defender_Type == Hv_Cavalary || defender_Type == Hv_Infantry)
            output += UNIT_DAMAGE_BONUS_SMALL;
        break;
    case Hv_Charriot:
        // Heavy Charriot
        // Very good X Light and Medium Cavalary
        if(defender_Type == Lg_Cavalary || defender_Type == Md_Cavalary)
            output += UNIT_DAMAGE_BONUS_LARGE;
        // Good X Heavy infantry
        else if (defender_Type == Hv_Infantry)
            output += UNIT_DAMAGE_BONUS_MEDIUM;
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
            output -= UNIT_DAMAGE_BONUS_MEDIUM;
        // Somewaht good X Light_Charriots
        else if (attacker_Type == Lg_Charriot)
            output -= UNIT_DAMAGE_BONUS_SMALL;
        break;
    case Md_Infantry:
        // Medium Infantry
        // Very Good X Other Infantry
        if (attacker_Type == Lg_Infantry || attacker_Type == Hv_Infantry)
            output -= UNIT_DAMAGE_BONUS_LARGE;
        break;
    case Hv_Infantry:
        // Heavy Infantry
        // Very Good X Cavalary
        if (attacker_Type >= Lg_Cavalary && attacker_Type <= Hv_Cavalary)
            output -= UNIT_DAMAGE_BONUS_LARGE;
        // Good X Charriots
        else if (attacker_Type == Lg_Charriot || attacker_Type == Hv_Charriot)
            output -= UNIT_DAMAGE_BONUS_MEDIUM;
        break;
    case Lg_Cavalary:
        // Light Cavalry
        // Good X Charriots
        if(attacker_Type == Lg_Charriot || attacker_Type == Hv_Charriot)
            output -= UNIT_DAMAGE_BONUS_MEDIUM;
        break;
    case Md_Cavalary:
        // Medium Cavalary
        // Good X Cavalaries
        if(attacker_Type >= Lg_Cavalary && attacker_Type <= Hv_Cavalary)
            output -= UNIT_DAMAGE_BONUS_MEDIUM;
        break;
    case Hv_Cavalary:
        // Heavy Cavalary
        // Very Good X Light and Medium Infantary
        if(attacker_Type == Lg_Infantry || attacker_Type == Md_Infantry)
            output -= UNIT_DAMAGE_BONUS_LARGE;
        // Somewhat good X Meele Charriots
        else if (attacker_Type == Hv_Charriot)
            output -= UNIT_DAMAGE_BONUS_SMALL;
        break;
    case Lg_Charriot:
        // Light Charriot
        // Very good X Medium Cavalary and Medium Infantry
        if (attacker_Type == Md_Cavalary || attacker_Type == Md_Infantry)
            output -= UNIT_DAMAGE_BONUS_LARGE;
        // Somewhat good X Heavy Cavalary and Heavy Infantry
        else if(attacker_Type == Hv_Cavalary || attacker_Type == Hv_Infantry)
            output -= UNIT_DAMAGE_BONUS_SMALL;
        break;
    case Hv_Charriot:
        // Heavy Charriot
        // Very good X Light and Medium Cavalary
        if(attacker_Type == Lg_Cavalary || attacker_Type == Md_Cavalary)
            output -= UNIT_DAMAGE_BONUS_LARGE;
        // Good X Heavy infantry
        else if (attacker_Type == Hv_Infantry)
            output -= UNIT_DAMAGE_BONUS_MEDIUM;
        break;
    // What if defender has no type?   
    default:
        fprintf(stderr, "No type defined!");
        break;
    }
    return output;
}

B_Result combat_Unit(B_Unit unit_Attacker, B_Unit unit_Defender, int heightDif, short int *fortLevel)
{
    B_Result result;
    short int A_Buffer = 0, D_Buffer = 0;
    result.isDraw = false;

    unit_Attacker.inCombat = true, unit_Defender.inCombat = true;
    float attacker_Power = unit_Attacker.attack_MeleeP * (unit_Attacker.morale / 100) + (rand() % 10) + unit_Attacker.level;
    // float attacker_Power = ((unit_Attacker.morale / 10) + unit_Attacker.level + add_BonusDamage_ByHeightDif(heightDif)) * unit_Attacker.men;
    float defender_Power = unit_Defender.defend_MeleeP * (unit_Defender.morale / 100) + (rand() % 10) + unit_Defender.level;
    // float defender_Power = ((unit_Defender.morale / 10) + unit_Defender.level) * unit_Defender.men;
    if(attacker_Power < 0)
        attacker_Power = 0;
    if(defender_Power < 0)
        defender_Power = 0;
    
    float combat_Result = attacker_Power - defender_Power + add_BonusDamage_ByHeightDif(heightDif) - ((float) *fortLevel / 2)
        + add_BonusDamage_ByClass(unit_Attacker.type, unit_Defender.type, false);

    if(combat_Result > *fortLevel && *fortLevel >= 1)
    {
        (*fortLevel)--;
        char msg[46];
        snprintf(msg, sizeof(msg), "Fort level at %hdX and %hdY decreased to %hd!", unit_Defender.position.X, unit_Defender.position.Y, *fortLevel);
        print_Message(msg, false);   
    }

    // Calculating battle results
    if(combat_Result > RESULT_CAP)
        combat_Result = RESULT_CAP;
    else if (combat_Result < (RESULT_CAP * -1))
        combat_Result = RESULT_CAP * -1;

    // Returnig winner or definig draw
    if(combat_Result > 0)
    {
        unit_Attacker.morale -= (float) (OFFSET / pow(MORALE_WIN, (double) combat_Result)) / 2;

        // Remaining men is defined by (RNG % Fraction of men) / (remaing moral + gear of the unit)
        A_Buffer = unit_Attacker.men;
        unit_Attacker.men -= (short int) (OFFSET / pow(MORALE_WIN, (double) combat_Result))
        * (unit_Defender.men / unit_Attacker.morale);
        A_Loss = A_Buffer - unit_Attacker.men;

        // Looser losses more morale and troops
        // If losser's men is bellow 0, capitulate. If losser's morale <= 0, capitulate.
        D_Buffer = unit_Defender.men;
        unit_Defender.morale -= (float) (pow(MORALE_LOOSE, (double) combat_Result) * OFFSET + OFFSET) / 4;
        if(unit_Defender.morale <= 1 )
        {
            unit_Defender.men = (short int) ((1.0f / (float) OFFSET) * unit_Defender.men_Max);
            unit_Defender.isRetreating = true;
            unit_Defender.inCombat = false;
            unit_Defender.morale = 1.0f;
        }
        else 
        {
            unit_Defender.men -= (short int) pow(MORALE_LOOSE, (double) combat_Result)
            * (A_Buffer / unit_Defender.morale) * OFFSET + OFFSET;
            if(unit_Defender.men < 0)
            {
                unit_Defender.men = (short int) ((1.0f / (float) OFFSET) * unit_Defender.men_Max);
                unit_Defender.morale = 0.0f;
                unit_Defender.isRetreating = true;
                unit_Defender.inCombat = false;
            }
        }
        B_Loss = D_Buffer - unit_Defender.men;

        result.winner = unit_Attacker;
        result.looser = unit_Defender;
    }
    else if (combat_Result < 0)
    {  
        combat_Result = (float) fabs((double) combat_Result);
        unit_Defender.morale -= (float) (OFFSET / pow(MORALE_WIN, (double) combat_Result)) / 2;

        // Remaining men is Defined by (RNG % Fraction of men) / (remaing moral + gear of the unit)
        D_Buffer = unit_Defender.men;
        unit_Defender.men -= (short int) (OFFSET / pow(MORALE_WIN, (double) combat_Result))
        * (unit_Attacker.men / unit_Defender.morale);
        B_Loss = D_Buffer - unit_Defender.men;
        
        // Looser losses more morale and troops
        // If losser's men is bellow 0, capitulate. If losser's morale <= 0, capitulate.
        A_Buffer = unit_Attacker.men;
        unit_Attacker.morale -= (float) (pow(MORALE_LOOSE, (double) combat_Result) * OFFSET + OFFSET) / 4;
        if(unit_Attacker.morale <= 1)
        {
            unit_Attacker.men = (short int) ((1.0f / (float) OFFSET) * unit_Attacker.men_Max);
            unit_Attacker.isRetreating = true;
            unit_Attacker.inCombat = false;
            unit_Attacker.morale = 1.0f;
        }
        else 
        {
            unit_Attacker.men -= (short int) pow(MORALE_LOOSE, (double) combat_Result)
            * (A_Buffer / unit_Attacker.morale) * OFFSET + OFFSET;
            if(unit_Attacker.men < 0)
            {
                unit_Attacker.men = (short int) ((1.0f / (float) OFFSET) * unit_Attacker.men_Max);
                unit_Attacker.morale = 0.0f;
                unit_Attacker.isRetreating = true;
                unit_Attacker.inCombat = false;
            }
        }
        A_Loss = A_Buffer - unit_Attacker.men;

        result.winner = unit_Defender;
        result.looser = unit_Attacker;
    }
    else
    {
        // Draw: Both loose same amount of morale; RNG varies men losses
        unit_Attacker.morale -= combat_Result;
        unit_Attacker.men -= (combat_Result * unit_Defender.men) / unit_Attacker.men;
        /*unit_Attacker.men -= (short int) (rand() % (unit_Attacker.men / MAX_FRACTION_LOSS))
        / (int) (unit_Attacker.gear + (unit_Attacker.morale / MORALE_WIN_CONSTANT)); */
        unit_Defender.morale -= combat_Result;
        unit_Defender.men -= (combat_Result * unit_Attacker.men) / unit_Defender.men;
        /*unit_Defender.men -= (short int) (rand() % (unit_Defender.men / MAX_FRACTION_LOSS))
        / (int) (unit_Defender.gear + (unit_Defender.morale / MORALE_WIN_CONSTANT)); */
        result.winner = unit_Attacker;
        result.looser = unit_Defender;
        result.isDraw = true;
    }
    return result;
}

void tryHeal_Unit(B_Side* Side_A, B_Side* Side_B)
{
    // Cheking for units in side A
    for(int i = 0; i < Side_A->size; i++)
    {
        if(Side_A->units[i].isRetreating == true && Side_A->units[i].inCombat == false)
        {
            Side_A->units[i].morale += (rand() % 10);
            // Can't regain > 100 points
            if(Side_A->units[i].morale > 100.0f)
            {
                Side_A->units[i].morale = 100.0f;  
                Side_A->units[i].isRetreating = false;        
            }
        }
    }

    // Cheking for units in side B
    for(int i = 0; i < Side_B->size; i++)
    {
        if(Side_B->units[i].isRetreating == true && Side_B->units[i].inCombat == false)
        {
            Side_B->units[i].morale += (rand() % 10);
            // Can't regain > 100 points
            if(Side_B->units[i].morale > 100.0f)
            {
                Side_B->units[i].morale = 100.0f;    
                Side_B->units[i].isRetreating = false;
            }
        }
    }
}

bool show_Combat_Result(B_Result units)
{
    bool out = false;
    if(units.isDraw)
    {
        printf("Both units were equal. The fight resulted in retreat and devastating losses for both sides! \n");
    }
    else if (units.winner.isRetreating == false && units.looser.isRetreating == false)
    {
        printf("%s is gaining the upper hand! \n", units.winner.name);
        printf("%s is falling apart! \n", units.looser.name);        
    } 
    else
    {
        printf("%s has won today: For the glory of the %s! \n", units.winner.name, units.winner.faction);
        printf("%s was doomed today: A shamefull day for the %s! \n", units.looser.name, units.looser.faction);  
        out = true;   
    }

    show_Unit(units.winner);
    show_Unit(units.looser);
    printf(">> Press ENTER to continue ");
    return out;
}


// Centralization of the combat process

void do_Combat(B_Unit* attacker, B_Unit* defender, int heigthDif, short int *fortLevel)
{
    // Unit cannot be already engaged at combat
    if(attacker->inCombat == true)
    {
        /* printf("\n");
        printf("#===============================================# \n");
        printf("| We must disengage the enemy before attacking! | \n");
        printf("#===============================================# \n"); */
        print_Message("We must disengage the enemy before attacking!", true);    
        attacker->inCombat = false;
        return;
    }

    B_Result Result = combat_Unit(*attacker, *defender, heigthDif, fortLevel);
    bool LevelUP = show_Combat_Result(Result);
    getchar();
    // Level up
    if(LevelUP == true)
        Result.winner.level++; 
    // Whoever wins gains the innitiative (can disengage/engage)
    Result.winner.inCombat = false;
    
    if(attacker->ID == Result.winner.ID)
    {
        *attacker = Result.winner;
        *defender = Result.looser;        
    }
    else
    {
        *defender = Result.winner;
        *attacker = Result.looser;               
    }
}

int do_Combat_Ranged(B_Unit* attacker, B_Unit* defender, int heightDif, int vegetat, short int *fortLevel)
{
    // Testing if in range of attack
    if(defender->position.X > attacker->position.X + attacker->range || defender->position.Y > attacker->position.Y + attacker->range)
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
    if(attacker->isRetreating == true)
    {
        /* printf("\n");
        printf("#============================================# \n");
        printf("| This unit will not fire, they're routing!  | \n");
        printf("#============================================# \n"); */     
        print_Message("This unit will not fire, they're routing!", true);
        return FUNCTION_FAIL;       
    }


    // If attaker != cavalary OR charriot, engage
    if(attacker->type < Lg_Cavalary || attacker->type > Hv_Charriot)
        attacker->inCombat = true;

    // Signaling attack
    printf("\n");
    printf("#================================# \n");
    printf("| %14s from %-10s | \n", attacker->name, attacker->faction);
    printf("| Firing from %2hdX %2hdY at %2hdX %2hdY | \n",attacker->position.X, attacker->position.Y, defender->position.X, defender->position.Y);
    printf("| %14s from %-10s | \n", defender->name, defender->faction);
    printf("#================================# \n");

    // Doing damage
    attacker->ammo--;
    float volley = attacker->attack_RangeP * (attacker->morale / 100) + (rand() % 10);
    float protec = defender->defend_RangeP * (defender->morale / 100) + (rand() % 10);
    float damage = volley - protec + add_BonusDamage_ByHeightDif(heightDif) - add_BonusDamage_ByVeget(vegetat) - ((float) *fortLevel / 2)
        + add_BonusDamage_ByClass(attacker->type, defender->type, true);
    if(damage < 0)
        damage = 1.0f;
    else if(damage > RESULT_CAP)
        damage = RESULT_CAP;
    // float damage = attacker->men * ((((attacker->morale / 10) + attacker->level) * add_BonusDamage_ByHeightDif(heightDif) / add_BonusDamage_ByVeget(vegetat)));
    
    defender->morale -= (float) pow(MORALE_LOOSE, (double) damage) * OFFSET + OFFSET;

    // If morale <= 0, retreat
    if(defender->morale <= 1)
    {
        defender->men = (short int) (1.0f / (float) OFFSET) * defender->men_Max;
        defender->isRetreating = true;
        defender->inCombat = false;
        defender->morale = 1.0f;
    }
    else
    {
        defender->men -= (short int) (pow(MORALE_LOOSE, (double) damage)
        * (attacker->men / defender->morale)) * OFFSET + OFFSET;
        if(defender->men < 0)
        {
            defender->men = (short int) (1.0f / (float) OFFSET) * defender->men_Max;
            defender->morale = 0.0f;
            defender->isRetreating = true;
            defender->inCombat = false;
        }
    }

    // Showing results
    B_Result res = {*attacker, *defender, false};
    bool LevelUP = show_Combat_Result(res);
    getchar();
    if(LevelUP == true)
        attacker->level++;

    return FUNCTION_SUCESS;
}
