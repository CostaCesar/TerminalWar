#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "const.h"

typedef enum E_buffs
{
    No_Buff = -1,
    Shield_Wall,
    Forest_Advtg,
    Desert_Advtg,
    Height_Advtg,
    Amphibious,
    Rally,
    Ambusher,
    Pike_Formation,
    Charge_Buff
} Unit_Buff;

typedef enum E_class
{
    Lg_Infantry,    // Ranged Infantry
    Md_Infantry,    // Sword Infantry
    Hv_Infantry,    // Pole-arm Infantry
    Lg_Cavalary,    // Ranged Cavalary
    Md_Cavalary,    // Sword Cavalary
    Hv_Cavalary,    // Pole-arm Cavalary
    Lg_Charriot,    // Ranged Charriot
    Hv_Charriot,    // Melee Charriot
    Lg_Siege,       // Arrow Artillery
    Md_Siege,       // Projectile Artillery
    Hv_Siege        // Siege (ag. walls & forts) Artilery
} T_Class;

typedef struct S_Position
{
    short int X;
    short int Y;
} B_Pos;

typedef struct S_Unit
{
    char name[STRING_NAME];
    T_Class type;
    char *faction;
    int ID;
    
    short int moves;
    short int men;
    short int men_Max;

    short int attack_MeleeP;
    short int defend_MeleeP;
    short int attack_RangeP;
    short int defend_RangeP;

    float morale;
    short int level;
    short int build_Cap;
    
    short int ammo;
    short int range;  
    short int buffs_S;
    Unit_Buff buffs[BUFFS_IMPLEMENTED];

    bool isRetreating;
    bool inCombat;
    
    B_Pos position;
    B_Pos goal;
    int *path;              // Actualy a T_Direc, see "map.h"
    int *chaseID;
} B_Unit;

typedef struct S_Side
{
    B_Unit *units;
    char name[STRING_NAME];
    int size;
    int ID;
    B_Pos cCoords;
    bool canPlace;
    bool isAI;
    bool attacker;
} B_Side;

// Alloc & Dealloc of Units

B_Unit* alloc_Unit(B_Unit* Unit, int* size)
{
    Unit = (B_Unit*) realloc(Unit, ++*size * sizeof(B_Unit));
    return Unit;
}

B_Unit* dealloc_Unit(B_Unit* Unit, int* size)
{
    Unit = (B_Unit*) realloc(Unit, --*size * sizeof(B_Unit));
    return Unit;
}

void remove_Unit(B_Unit* Unit, int size, int posit)
{
    for(int i = posit; i <= (size - 1); i++)
    {
        Unit[i] = Unit[i + 1];
    }
}
// Centralization of the Unit removal process

void delete_Unit(B_Unit* Unit, int* size, int posit)
{
    printf("Unit %d has vanished forever! \n", Unit[posit].ID);
    remove_Unit(Unit, *size, posit);
    Unit = dealloc_Unit(Unit, size);
}

char *get_UnitType(T_Class unit)
{
    char *type;
    switch (unit)
    {
        case Lg_Infantry:
            type = "Light Infantary";
            break;
        case Md_Infantry:
            type = "Medium Infantary";
            break;
        case Hv_Infantry:
            type = "Heavy Infantary";
            break;
        case Lg_Cavalary:
            type = "Light Cavalary";
            break;
        case Md_Cavalary:
            type = "Medium Cavalary";
            break;
        case Hv_Cavalary:
            type = "Heavy Cavalary";
            break;
        case Lg_Charriot:
            type = "Light Charriot";
            break;
        case Hv_Charriot:
            type = "Heavy Charriot";
            break;
        case Lg_Siege:
            type = "Portable Artillery";
            break;
        case Md_Siege:
            type = "Field Artillery";
            break;
        case Hv_Siege:
            type = "Siege Artillery";
            break;
        default:
            type = "UNKNOWN";
            break;
    }
    return type;
}

char *get_UnitBuff(Unit_Buff buff)
{
    char *type;
    switch (buff)
    {
        case Shield_Wall:
            type = "Shield Wall";
            break;
        case Forest_Advtg:
            type = "Forest Native";
            break;
        case Desert_Advtg:
            type = "Desert Lover";
            break;
        case Height_Advtg:
            type = "Higher Ground";
            break;
        case Amphibious:
            type = "Amphibious";
            break;
        case Rally:
            type = "Rally Up";
            break;
        case Ambusher:
            type = "Ambush Expert";
            break;
        case Pike_Formation:
            type = "Pike Wall";
            break;
        case Charge_Buff:
            type = "Charger";
            break;
        default:
            type = "UNKNOWN";
            break;
    }
    return type;   
}

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
        snprintf(msg, sizeof(msg), "Range: %d tiles | Ammo: %d left", unit->range, unit->ammo);
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

    if(unit->chaseID != NULL)
    {
        snprintf(buff, sizeof(buff), "Chasing unit %d", *(unit->chaseID));
        print_Line(buff);
    }
    if(unit->goal.X != NO_UNIT && unit->goal.Y != NO_UNIT)
    {
        snprintf(buff, sizeof(buff), "Going to %dX %dY", unit->goal.X, unit->goal.Y);
        print_Line(buff);
    }

    print_Line(" ");
    if (unit->inCombat == true)
        print_Line("<!> Engaged In Combat");
    if (unit->isRetreating == true)
        print_Line("<!> Retreating");
    print_Line(NULL);
}

void show_Unit(B_Unit Unit)
{
    char* type = get_UnitType(Unit.type);
    printf("======================================================\n");
    if(Unit.ID > -1)
    printf("| Unit %04d \n", Unit.ID);
    printf("| %s", Unit.name);
    if(Unit.faction != NULL)
        printf(" (%s) ", Unit.faction);
    printf("\n");
    printf("| Level %d %s \n", Unit.level, type);
    printf("| Men: %d / %d \n", Unit.men, Unit.men_Max);
    printf("| Morale %.2f \n", Unit.morale);
    printf("| Melee: %d Attack and %d Defense \n", Unit.attack_MeleeP, Unit.defend_MeleeP);
    printf("| Ranged Defense: %d \n", Unit.defend_RangeP);
    printf("| Build Power: %d \n", Unit.build_Cap);
    if(Unit.range > 0){
    printf("| Ranged Attack: %d \n", Unit.attack_RangeP);
    printf("| Range: %d tiles [square] \n", Unit.range);
    printf("| Ammo: %d \n", Unit.ammo);
    }
    printf("| Special powers: ");
    for(int i = 0; i < Unit.buffs_S; i++)
        printf("%s, ", get_UnitBuff(Unit.buffs[i]));
    printf("\n");
    if(Unit.isRetreating){
    printf("| <!> Retreating \n");
    }
    if(Unit.inCombat){
    printf("| <!> Engaged \n");
    }
    printf("======================================================\n");
}