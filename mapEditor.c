#include "include/map.h"
#include <Windows.h>
#include <string.h>

#define SUCESS 0
#define DEFAULT_FAIL 1
#define UNKNOW_PATH 2
#define OUT_OF_MEMORY -1
#define NULL_POINTER -2

#define VERSION 1.500f
#define DEFAULT_MAP 5

int map_Save(B_Map *map, char *fPath)
{
    getchar();
    printf("Nome para o arquivo <qualquer.map>: ");
    fgets(fPath, STRING_NAME, stdin);
    fPath[strlen(fPath) - 1] = '\0';

    FILE* file = fopen(fPath, "wb");
    if(file == NULL)
    {
        fPath = "NONE";
        fprintf(stderr, "ERROR: File could not be opened! \n");
        return UNKNOW_PATH;
    }

    fwrite(&map->width, sizeof(int), 1, file);
    fwrite(&map->height, sizeof(int), 1, file);
    fwrite(&map->climate, sizeof(T_Clime), 1, file);
    fwrite(&map->time, sizeof(T_Time), 1, file);
    fwrite(map->name, sizeof(char), STRING_NAME, file);
    
    for(int i = 0; i < map->height; i++)
        fwrite(map->tiles[i], sizeof(B_Tile), map->width, file);

    fclose(file);

    printf(">> Arquivo escrito com sucesso! <<\n");
    return SUCESS;
}

int map_Load(B_Map *map, char *fPath)
{
    int fHeight_Old = map->height;
    
    getchar();
    printf("Nome para o arquivo <qualquer.map>: ");
    fgets(fPath, STRING_NAME, stdin);
    fPath[strlen(fPath) - 1] = '\0';

    FILE* file = fopen(fPath, "rb");
    if(file == NULL)
    {
        fprintf(stderr, "ERROR: File could not be opened! \n");
        return NULL_POINTER;
    }

    fread(&map->width, sizeof(int), 1, file);
    fread(&map->height, sizeof(int), 1, file);
    fread(&map->climate, sizeof(T_Clime), 1, file);
    fread(&map->time, sizeof(T_Time), 1, file);
    fread(map->name, sizeof(char), STRING_NAME, file);
    
    map->tiles = (B_Tile **) realloc(map->tiles, map->height * sizeof(B_Tile *));
    if(map->tiles == NULL)
    {
        fprintf(stderr, "ERROR: Not enough memory! \n");
        return OUT_OF_MEMORY;
    }
    for(int i = 0; i < fHeight_Old; i++)
        free(map->tiles[i]);
    for(int i = 0; i < map->height; i++)
    {
        map->tiles[i] = (B_Tile*) calloc(map->width, sizeof(B_Tile));
        fread(map->tiles[i], sizeof(B_Tile), map->width, file);
    }
    
    fclose(file);
    printf(">> Arquivo aberto com sucesso! <<\n");
    return SUCESS;
}

void map_Connections(B_Map* map)
{
    // Getting conections
    for(int i = 0; i < map->height; i++)
    {
        for(int j = 0; j < map->width; j++)
        {
            for(int k = 0; k <= Northwest; k++)
            {
                if(((k > West || k < East) && i < 1) || ((k < West && k > East) && i >= map->height - 1)
                ||  (k > South && j < 1) || ((k > North && k < South) && j >= map->width - 1))
                {
                    map->tiles[i][j].node.conectP[k] = false;
                    map->tiles[i][j].node.conectS[k] = -1;
                    continue;
                }

                B_Pos pos_Current = {j, i};
                B_Tile *temp = get_AdjTile(map, pos_Current, k);
                B_Pos pos_Adj = get_AdjTile_Pos(map, pos_Current, k);
                if((get_AbsHeigthDif(map, pos_Current, pos_Adj) > HEIGHT_DIF) || (temp->elevation < -1 && temp->terrain == Water))
                {
                    map->tiles[i][j].node.conectP[k] = false;
                    map->tiles[i][j].node.conectS[k] = -1;
                }
                else
                {
                    map->tiles[i][j].node.conectP[k] = true;
                    map->tiles[i][j].node.conectS[k] = temp->terrain;  
                }
            }
        }
    }
    return;
}

int map_Defaults(B_Map* map)
{
    map->climate = 0;
    map->time = 0;
    strcpy(map->name, "Sem Nome");

    // Definig map tiles
    for(int i = 0; i < map->height; i++)
    {
        for(int j = 0; j < map->width; j++)
        {
            if((i == 0 || i == (map->height - 1)) || (j == 0 || j == (map->width - 1))) // Marking water borders
            {
                map->tiles[i][j].elevation = -2;
                map->tiles[i][j].terrain = Water;
                map->tiles[i][j].vegetation = None;
            }
            else // Flat land in middle
            {
                map->tiles[i][j].elevation = 1;
                map->tiles[i][j].terrain = Grass;
                map->tiles[i][j].vegetation = Field;
            }
            
            // Starting tiles
            map->tiles[i][j].spawn = 0;
            map->tiles[i][j].fortLevel = 0;
            map->tiles[i][j].unit = NULL;
            map->tiles[i][j].pos.X = j, map->tiles[i][j].pos.Y = i; 
        }
    }
    return SUCESS;
}

int map_Edit(B_Map* map)
{
    char chr = 'X';
    Map_Area fArea = {-1, -1, -1, -1};
    int fAux = 0, fMode = MODE_HEIGHT;
    bool keepVegetation = false, keepTerrain = false;

    B_Tile fTile = {0};

    do
    {   
        char mElevat = 'X', mFortif = 'X';
        system("cls");
        show_Map(map, fMode, false);
        printf(">> %s << \n", map->name);
        printf("<Area> \n");
        printf("Ponto 1: %dX - %dY \n", fArea.pointA.X, fArea.pointA.Y);
        printf("Ponto 2: %dX - %dY \n", fArea.pointB.X, fArea.pointB.Y);

        switch (fMode)
        {
        case 0:
            printf(">> Mapa de altura \n");
            break;
        case 1:
            printf(">> Mapa de terreno \n");
            break; 
        case 2:
            printf(">> Mapa de vegetacao \n");
            break;
        case 3:
            printf(">> Mapa de unidades \n");
            break;
        }

        printf("[M] Marcar area \n");
        printf("[L] Ladrilhos \n");
        printf("[P] Pontos de partida\n");
        printf("[T] Trocar tipo de mapa \n");
        printf("[S] Sair para o menu \n\n");

        printf(">=> ");
        scanf("%c", &chr);
        getchar();
        if(chr >= 'A' && chr <= 'Z')
            chr += 32;
        switch (chr)
        {
        case 'm':
            printf("Digite as coordenadas do primeiro ponto <X Y>: ");
            scanf("%d %d", &fArea.pointA.X, &fArea.pointA.Y);
            printf("Digite as coordenadas do segundo ponto <X Y>: ");
            scanf("%d %d", &fArea.pointB.X, &fArea.pointB.Y);

            if(fArea.pointA.X < 0 || fArea.pointA.X > map->width || fArea.pointA.Y < 0 || fArea.pointA.Y > map->height
            || fArea.pointB.X < 0 || fArea.pointB.X > map->width || fArea.pointB.Y < 0 || fArea.pointB.Y > map->height)
            {
                fArea.pointA.X = 0;
                fArea.pointA.Y = 0;
                fArea.pointB.X = (map->width - 1);
                fArea.pointB.Y = (map->height - 1); 
                
                printf(">> Valor dos pontos fora de limites! <<\n");
                printf(">> Marcando mapa inteiro... << \n");
            }
           
            // Sorting so fArea.pointA.X && fArea.pointA.Y are the lowest values
            if(fArea.pointA.X > fArea.pointB.X)
            {
                fAux = fArea.pointA.X;
                fArea.pointA.X = fArea.pointB.X;
                fArea.pointB.X = fAux;
            }
            if(fArea.pointA.Y > fArea.pointB.Y)
            {
                fAux = fArea.pointA.Y;
                fArea.pointA.Y = fArea.pointB.Y;
                fArea.pointB.Y = fAux;
            }
            printf(">> Area selecionada! <<\n");
            printf("Aperte ENTER \n");
            getchar();
            break;


        case 'l':
            // Failsafe
            if(fArea.pointA.X < 0 || fArea.pointA.Y < 0 || fArea.pointB.X < 0 || fArea.pointB.Y < 0)
            {
                printf(">> Area invalida! <<\n");
                printf("Aperte ENTER \n");
                getchar();
                break;
            }

            printf("Modos: \n [Q] para manter\n [W] para adicionar valor ao atual \n");
            printf("Selecione uma altura para a area selecionada <NumeroModo>: ");
            scanf("%d%c", &fTile.elevation, &mElevat);
            if(mElevat >= 'A' && mElevat <= 'Z')
                mElevat += 32;

            printf("Modos: \n [Q] para manter\n [W] para adicionar valor ao atual \n");
            printf("Selecione um nivel de entrincheiramento para a area selecionada <NumeroModo>: ");
            scanf("%d%c", &fTile.fortLevel, &mFortif);
            if(mFortif >= 'A' && mFortif <= 'Z')
                mFortif += 32;
            
            printf("[00] Grass  > Grama \n");
            printf("[01] Sand   > Areia \n");
            printf("[02] Rock   > Pedra \n");
            printf("[03] Mud    > Lama \n");
            printf("[04] Water  > Agua \n");
            printf("Novo terreno para a area selecionada <negativo para manter>: ");
            scanf("%d", &fAux);
            if(fAux < 0)
                keepTerrain = true;
            else fTile.terrain = fAux;

            printf("[00] None   > Nada \n");
            printf("[01] Field  > Campo \n");
            printf("[02] Sparse > Savana \n");
            printf("[03] Grove  > Bosque \n");
            printf("[04] Forest > Floresta \n");
            printf("[05] Jungle > Selva \n");
            printf("Nova vegetacao para a area selecionada <negativo para manter>: ");
            scanf("%d", &fAux);
            if(fAux < 0)
                keepVegetation = true;
            else fTile.vegetation = fAux;
            
            // Editing map
            for(int i = fArea.pointA.Y; i <= fArea.pointB.Y; i++)
            {
                for(int j = fArea.pointA.X; j <= fArea.pointB.X; j++)
                {
                    // Elevation
                    if(mElevat == 'q')
                    { /* Nothing */ }
                    else if (mElevat == 'w')
                        map->tiles[i][j].elevation += fTile.elevation;
                    else
                        map->tiles[i][j].elevation = fTile.elevation;
                    
                    // Fortification
                    if(mFortif == 'q')
                    { /* Nothing */ }
                    else if (mFortif == 'w')
                        map->tiles[i][j].fortLevel += fTile.fortLevel;
                    else
                        map->tiles[i][j].fortLevel = fTile.fortLevel;     
                                  
                    // Terrain
                    if(keepTerrain == false)
                        map->tiles[i][j].terrain = fTile.terrain;

                    // Vegetation
                    if(keepVegetation == false)
                        map->tiles[i][j].vegetation = fTile.vegetation;
                }
            }
            keepTerrain = false;
            keepVegetation = false;

            printf(">> Area alterada! << \n");
            printf("Aperte ENTER \n");
            getchar();
            break;
        case 'p':
            // Failsafe
            if(fArea.pointA.X < 0 || fArea.pointA.Y < 0 || fArea.pointB.X < 0 || fArea.pointB.Y < 0)
            {
                printf(">> Area invalida! <<\n");
                printf("Aperte ENTER \n");
                getchar();
                break;
            }

            printf("Essa area deve sinalizar que? \n");
            printf("[1] Colocar como partida do Lado A\n");
            printf("[2] Colocar como partida do Lado B \n");
            printf("[3] Remover como partida do Lado A\n");
            printf("[4] Remover como partida do Lado B \n");
            printf("[5] Reiniciar todo o Lado A \n");
            printf("[6] Reiniciar todo o Lado B \n");
            printf("[0] Cancelar \n");

            printf(">=> ");
            scanf("%d", &fAux);
            switch (fAux)
            {
            case 1:
                for(int i = fArea.pointA.Y; i <= fArea.pointB.Y; i++)
                    for(int j = fArea.pointA.X; j <= fArea.pointB.X; j++)
                        map->tiles[i][j].spawn = 1;
                break;
            case 2:
                for(int i = fArea.pointA.Y; i <= fArea.pointB.Y; i++)
                    for(int j = fArea.pointA.X; j <= fArea.pointB.X; j++)
                        map->tiles[i][j].spawn= -1;
                break;
            case 3:
                for(int i = fArea.pointA.Y; i <= fArea.pointB.Y; i++)
                    for(int j = fArea.pointA.X; j <= fArea.pointB.X; j++)
                        if(map->tiles[i][j].spawn > 0)
                            map->tiles[i][j].spawn = 0;
            case 4:
                break;
                for(int i = fArea.pointA.Y; i <= fArea.pointB.Y; i++)
                    for(int j = fArea.pointA.X; j <= fArea.pointB.X; j++)
                        if(map->tiles[i][j].spawn < 0)
                        map->tiles[i][j].spawn = 0;
                break;
            case 5:
                for(int i = 0; i < map->height; i++)
                    for(int j = 0; j < map->width; j++)
                        if(map->tiles[i][j].spawn > 0)
                            map->tiles[i][j].spawn = 0;
                break;
            case 6:
                for(int i = 0; i < map->height; i++)
                    for(int j = 0; j < map->width; j++)
                        if(map->tiles[i][j].spawn < 0)
                        map->tiles[i][j].spawn = 0;     
                break;         
            case 0:
                printf(">> Acao abortada! << \n");
                printf("Aperte ENTER \n");
                getchar();
                continue;
            default:
                printf(">> Comando invalido! << \n");
                printf("Aperte ENTER \n");
                getchar();
                break;
            } 
            
            printf(">> Area configurada! << \n");
            printf("Aperte ENTER \n");
            getchar();
            break;
        case 't':
            fMode++;
            if(fMode > MODE_UNITS)
                fMode = MODE_HEIGHT;
            break;
        case 's':
            return SUCESS;
        default:
            break;
        }
    } while(1);
}

int main(int argc, char *argv)
{
    char chr = 'X';
    char file_Name[STRING_NAME] = "NONE";
    int mHeight_Old = DEFAULT_MAP, output = DEFAULT_FAIL;

    B_Map map;
    map.height = DEFAULT_MAP;
    map.width = DEFAULT_MAP;
    map.tiles = (B_Tile **) calloc(map.height, sizeof(B_Tile *));
    for(int i = 0; i < map.height; i++)
        map.tiles[i] = (B_Tile *) calloc(map.width, sizeof(B_Tile));
    map_Defaults(&map);
    PlaySound("sound/Editor.wav", NULL, SND_ASYNC | SND_LOOP | SND_FILENAME);

    do
    {
        printf("Total Terminal War: Editor de mapa \n");
        printf("V. %.3f \n", VERSION);
        printf("Arquivo de mapa aberto: %s \n", file_Name);
        printf("[D] Definir mapa \n");
        printf("[E] Editar mapa\n");
        printf("[P] Propiedades do mapa \n");
        printf("[S] Salvar mapa \n");
        printf("[C] Carregar mapa \n");
        printf("[F] Fechar o editor \n\n");
        
        printf(">=> ");
        scanf("%c", &chr);
        if(chr >= 'A' && chr <= 'Z')
            chr += 32;

        switch (chr)
        {
        case 'd':
            printf("Nova dimensao do mapa <X Y>: ");
            scanf("%d %d", &map.width, &map.height);
            if(map.width < 1 || map.height < 1)
            {
                map.width = DEFAULT_MAP;
                map.height = DEFAULT_MAP;
                printf(">> Parametros invalidos! << \n");
                getchar();
                continue;
            }
            
            // Freeing old rows & realocing columns & callocing rows & setting old height
            for(int i = 0; i < mHeight_Old; i++)
                free(map.tiles[i]);
            map.tiles = (B_Tile **) realloc(map.tiles, map.height * sizeof(B_Tile *));
            for(int i = 0; i < map.height; i++)
                map.tiles[i] = (B_Tile *) calloc(map.width, sizeof(B_Tile));
            mHeight_Old = map.height;
            map_Defaults(&map);
            
            printf(">> Mapa redimensionado com sucesso! << \n");
            getchar();
            continue;
        case 'e':
            getchar();
            map_Edit(&map);
            system("cls");
            continue;
        case 'p':
            getchar();
            printf("Nome para o mapa: ");
            fgets(map.name, STRING_NAME, stdin);
            map.name[strlen(map.name) - 1] = '\0';
            
            printf("[00] Sunny > Ensolarado \n");
            printf("[01] Sparse Clouds > Algumas nuvens \n");
            printf("[02] Many Clouds > Muitas nuvens \n");
            printf("[03] Cloudy > Nublado \n");
            printf("[04] Rain > Chuva \n");
            printf("[05] Storm > Tempestade \n");
            printf("Novo clima para o mapa: ");
            scanf("%d%c", &map.climate, &chr);

            printf("[00] Dawn > Penumbra \n");
            printf("[01] Morning > Manha \n");
            printf("[02] Noon > Meio-dia \n");
            printf("[03] Afternoon > Tarde\n");
            printf("[04] Twilight > Crepusculo\n");
            printf("[05] Night > Noite\n");
            printf("Novo tempo para o mapa: ");
            scanf("%d%c", &map.time, &chr);

            printf("\n");
            continue;
        case 's':
            // Generating paths
            map_Connections(&map);
            output = map_Save(&map, file_Name);
            break;
        case 'c':
            output = map_Load(&map, file_Name);
            break;
        case 'f':
            for(int i = 0; i < map.height; i++)
                free(map.tiles[i]);
            free(map.tiles);
            return SUCESS;
        default:
            continue;
        }
    } while (1);
    
    return NO_UNIT;
}