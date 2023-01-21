#include <string.h>
#include <Windows.h>
#include "include/unit.h"

#define VERSION 1.500f

#define SUCESS 0
#define CONTINUE 1
#define DEFAULT_FAIL 2
#define UNKNOW_PATH 3
#define OUT_OF_MEMORY -1
#define NULL_POINTER -2

void handleBuffs(B_Unit *unit)
{
    int temp = 0;
    bool skip = false;

    for(int i = 0; i < BUFFS_MAX; i++)
        unit->buffs[i] = Buff_NONE;
    
    for(int i = 0; i < unit->buffs_S; i++)
    {
        for(int j = 0; j < BUFFS_IMPLEMENTED; j++)
        {
            for(int k = 0; k < unit->buffs_S; k++)
            {
                if(j == unit->buffs[k])
                {
                    printf("<X> ");
                    skip = true;
                    break;
                }
            } 
            if(!skip)
                printf("< > ");
            skip = false;
            printf("[%d] %s \n", j, get_UnitBuff((Unit_Buff) j));
        }

        printf("> Adicionar habilidade: ");
        scanf("%d", &temp);
        if(temp < 0 || temp > BUFFS_IMPLEMENTED)
        {
            printf("Identificador invalido! \n");
            i--;
            continue;
        }

        for(int j = 0; j < unit->buffs_S; j++)
        {
            if(unit->buffs[j] == temp)
            {
                printf("Esta habilidade ja foi adicionada! \n");
                skip = true;
                break;
            }
        }
        if(!skip)
        {
            unit->buffs[i] = temp;
            printf("Habilidade adicionada com sucesso! \n");
        }
        else
        {
            i--;
            continue;
        }
    }
    return;
}

B_Unit* edit_List(int* out, B_Unit *list, int *size)
{
    char chr = 'X';
    short int pos = 0;
    
    getchar();
    system("cls");
    
    // Testing for null pointer
    if(list == NULL)
    {
        *out = NULL_POINTER;
        return NULL;
    }
    do
    {
        if(*size > 0)
        {
            printf("Unidades definidas nesta lista: \n");
            for(int i = 0; i < *size; i++)
            {
                printf(">%02d> %s \n", i, list[i].name);
                list[i].Table_ID = i;
            }
        }
        printf("[C] Criar nova unidade \n");
        printf("[E] Editar uma unidade \n");
        printf("[V] Ver unidade \n");
        printf("[R] Remover uma unidade \n");
        printf("[F] Finalizar \n");
        
        printf("> ");
        scanf("%c", &chr);
        if(chr >= 'A' && chr <= 'Z')
            chr += 32;
        switch (chr)
        {
        case 'c':
            pos = *size;
            list = alloc_Unit(list, size);
            getchar();
            break;
        case 'e':
            printf("Indice da unidade: ");
            scanf("%d%c", &pos, &chr);
            if(pos > *size || pos < 0)
                continue;
            break;
        case 'v':
            printf("Indice da unidade: ");
            scanf("%d%c", &pos, &chr);
            if(pos < *size && pos >= 0)
                show_Unit(list[pos]);
            continue; 
        case 'r':
            printf("Indice da unidade: ");
            scanf("%d%c", &pos, &chr);
            if(pos > *size || pos < 0)
                continue;
            else delete_Unit(list, size, pos);
                continue;
            break;
        case 'f':
            *out = CONTINUE;
            system("cls");
            return list;
        default:
            continue;
        }
    
        printf("> Nome da unidade: ");
        fgets(list[pos].name, STRING_NAME, stdin);
        list[pos].name[strlen(list[pos].name) - 1] = '\0';
        printf("[%02d] Infantaria Leve \n",   Lg_Infantry);
        printf("[%02d] Infantaria Media \n",  Md_Infantry);
        printf("[%02d] Infantaria Pesada \n", Hv_Infantry);
        printf("[%02d] Cavalaria Leve \n",    Lg_Cavalary);
        printf("[%02d] Cavalaria Media \n",   Md_Cavalary);
        printf("[%02d] Cavalaria Pesada \n",  Hv_Cavalary);
        printf("[%02d] Biga Leve \n",         Lg_Charriot);
        printf("[%02d] Biga Pesada \n",       Hv_Charriot);
        printf("[%02d] Artilharia Leve \n",   Lg_Siege);
        printf("[%02d] Artilharia Media \n",  Md_Siege);
        printf("[%02d] Artilharia Pesada \n", Hv_Siege);
        printf("> Tipo da unidade: ");
        scanf("%d%c", &list[pos].type, &chr);
        
        printf("> Movimentos por turno: ");
        scanf("%hd%c", &list[pos].moves, &chr);
        
        printf("> Homens na unidade: ");
        scanf("%hd%c", &list[pos].men_Max, &chr);
        list[pos].men = list[pos].men_Max;

        printf("> Poder corpo-a-corpo ofensivo: ");
        scanf("%hd%c", &list[pos].attack_MeleeP, &chr);
        printf("> Poder corpo-a-corpo defensivo: ");
        scanf("%hd%c", &list[pos].defend_MeleeP, &chr);
        printf("> Poder a distancia defensivo: ");
        scanf("%hd%c", &list[pos].defend_RangeP, &chr);
        list[pos].attack_RangeP = -1;

        printf("> Moral da unidade [MAXIMO = %.3f]: ", UNIT_MAX_MORALE);
        scanf("%f%c", &list[pos].morale, &chr);
        if(list[pos].morale > UNIT_MAX_MORALE)
            list[pos].morale = UNIT_MAX_MORALE;
        
        printf("> Capacidade de construcao da unidade: ");
        scanf("%hd%c", &list[pos].build_Cap, &chr);

        printf("> Nivel padrao da unidade [MAXIMO = %d]: ", UNIT_MAX_LEVEL);
        scanf("%hd%c", &list[pos].level, &chr);
        if(list[pos].level > UNIT_MAX_LEVEL)
            list[pos].level = UNIT_MAX_LEVEL;

        printf("> Quantidade de habilidades [MAXIMO = %d]: ", BUFFS_MAX);
        scanf("%hd%c", &list[pos].buffs_S, &chr);
        if(list[pos].buffs_S > BUFFS_MAX)
            list[pos].buffs_S = BUFFS_MAX;
        else if(list[pos].buffs_S < 0)
            list[pos].buffs_S = 0;
        handleBuffs(&list[pos]);

        printf("> Alcance [quadrado] da unidade: ");
        scanf("%hd%c", &list[pos].range, &chr);
        if(list[pos].range > 0)
        {
            printf("> Municao padrao da unidade: ");
            scanf("%hd%c", &list[pos].ammo, &chr);     
            printf("> Poder a distancia ofensivo: ");
            scanf("%hd%c", &list[pos].attack_RangeP, &chr);      
        }

        printf(">> Feito! << \n\n");

        // Defaults
        list[pos].faction = NULL;
        list[pos].Game_ID = -1;
        list[pos].retreating = false;
        list[pos].engaged = false;
        list[pos].position.X = -1;
        list[pos].position.Y = -1;
        list[pos].goal.X = -1;
        list[pos].goal.Y = -1;
        list[pos].path = NULL;
        list[pos].chaseID = NULL;

    } while (1);
}

int save_List(B_Unit *list, int size, char *name, char *desc)
{
    getchar();
    printf("Nome para o arquivo a ser salvo [qualquer.bin]: ");
    fgets(name, STRING_FILE, stdin);
    name[strlen(name) - 1] = '\0';

    system("cls");
    FILE *file = fopen(name, "wb");
    if(file == NULL)
    {
        printf("ERROR: Could not save file \n");
        name = "NONE";
        return UNKNOW_PATH;
    }

    fwrite(&size, sizeof(int), 1, file);
    fwrite(list, sizeof(B_Unit), size, file);
    fwrite(desc, sizeof(char), STRING_FILE, file);
    fclose(file);
    
    printf(">> Arquivo salvo com sucesso! << \n");
    return CONTINUE;
}

B_Unit* load_List(int *out, B_Unit *list, int *size, char *name, char *desc)
{
    getchar();
    printf("Nome para o arquivo a ser aberto [qualquer.bin]: ");
    fgets(name, STRING_FILE, stdin);
    name[strlen(name) - 1] = '\0';

    system("cls");
    FILE *file = fopen(name, "rb");
    if(file == NULL)
    {
        printf("ERROR: Could not load file \n");
        name = "NONE";
        *out = UNKNOW_PATH;
        return NULL;
    }

    fread(size, sizeof(int), 1, file);
    list = (B_Unit *) calloc(*size, sizeof(B_Unit));
    if(list == NULL)
    {
        *out = NULL_POINTER;
        return NULL;
    }
    fread(list, sizeof(B_Unit), *size, file);
    fread(desc, sizeof(char), STRING_FILE, file);
    fclose(file);
    
    printf(">> Arquivo aberto com sucesso! << \n");
    // File correction
    for(int i = 0; i < *size; i++)
    {
        if(list[i].range <= 0) list[i].attack_RangeP = -1;
        list[i].attacked = false;
        list[i].chaseID = NULL;
    }

    *out = CONTINUE;
    return list;
}

int main(int argc, char **argv)
{
    char chr = 'X';
    char fTable[STRING_FILE] = "NONE";
    char fDescp[STRING_FILE] = "NONE";
    int list_Size = 1, output = CONTINUE;
    B_Unit *list = (B_Unit *) calloc(list_Size, sizeof(B_Unit));
    list_Size = 0;

    PlaySound("sound/Editor.wav", NULL, SND_ASYNC | SND_LOOP | SND_FILENAME);
    do
    {
        printf("Total Terminal War: Editor de Unidade \n");
        printf("V. %.3f \n", VERSION);
        printf("Lista de unidades em '%s' \n", fTable);
        printf("Descricao das unidades em '%s' \n", fDescp);
        printf("[E] Editar lista de unidades \n");
        printf("[M] Mudar arquivo de descricao \n");
        printf("[S] Salvar lista de unidades \n");
        printf("[C] Carregar lista de unidades \n");
        printf("[F] Fechar o editor \n");
        
        printf(">=> ");
        scanf("%c", &chr);
        if(chr >= 'A' && chr <= 'Z')
            chr += 32;
        switch (chr)
        {
        case 'e':
            list = edit_List(&output, list, &list_Size);
            getchar();
            break;
        case 'm':
            getchar();
            printf("Novo caminho para o arquivo <../qualquer.txt>: ");
            fgets(fDescp, STRING_FILE, stdin);
            fDescp[strlen(fDescp) - 1] = '\0';      

            system("cls");    
            printf(">> Caminho de descricao alterado! << \n\n");
            break;
        case 'c':
            list = load_List(&output, list, &list_Size, fTable, fDescp);
            break;
        case 's':
            output = save_List(list, list_Size, fTable, fDescp);
            break;
        case 'f':
            output = SUCESS;
            break;
        default:
            continue;
        }

        if(output == CONTINUE)
            continue;
        else
        {
            free(list);
            return output;
        }

    } while (1);
}