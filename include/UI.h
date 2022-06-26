#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <conio.h>
#include <Windows.h>
#include <math.h>

typedef struct P_endStats
{
    char *name;
    int deployed;
    int loss;
    int killed;
} B_endStats;

int get_ScreenWidth()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi); 
    return (int) csbi.srWindow.Right - csbi.srWindow.Left + 1;         
    // tSize_Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;   
}
int get_ScreenHeight()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi); 
    return (int) csbi.srWindow.Bottom - csbi.srWindow.Top + 1;  
}

void toggle_Cursor(bool cursor)
{
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = cursor;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

void print_Message(char *message, bool doWait)
{
    int screenWidth = get_ScreenWidth();
    int msg_len = strlen(message);
    if(msg_len % 2 == 1)
        msg_len++;
    
    printf("#");
    for(int i = 0; i < screenWidth - 2; i++)
        printf("-");
    printf("#\n");

    printf("|");
        for(int i = 0; i < screenWidth - 2; i++)
        printf(" ");
    printf("|\n");

    printf("|");
        for(int i = 0; i < (screenWidth - msg_len) / 2 - 1; i++)
            printf(" ");
        printf("%s", message);
        for(int i = 0; i < (screenWidth - msg_len + 1) / 2 - 1; i++)
            printf(" ");
    printf("|\n");

        printf("|");
        for(int i = 0; i < screenWidth - 2; i++)
        printf(" ");
    printf("|\n");

    printf("#");
    for(int i = 0; i < screenWidth - 2; i++)
        printf("-");
    printf("#\n");

    if(doWait)
        Sleep(2000);

    return;     
}

char get_KeyPress(bool fLowerCase)
{
    char out = (char) getch();
    if(out >= 'A' && out <= 'Z' && fLowerCase == true)
        out += 32;
    return out;
}

void info_Upper(char* mapName, int turns, char *side, bool isPlayer, char *unitName, int Id, short int X, short int Y, short int moves)
{
    int screenWidth = get_ScreenWidth();   
    
    // Upper Line
    for(int i = 0; i < screenWidth - 1; i++)
    {
        printf("=");
    }
    if(screenWidth % 2 == 0)
        printf("=");
    printf("\n");
    
    // Map name
    printf("||");
    for(int i = 0; i < (screenWidth / 2) - (int) (strlen(mapName) / 2) - 2; i++)
    {
        printf(" ");
    }
    printf("%s", mapName);
    for(int i = 0; i < (screenWidth / 2) - (int) (strlen(mapName) / 2) - 2; i++)
    {
        printf(" ");
    }
    printf("||\n");

    // Turn
    printf("||");
    for(int i = 0; i < (screenWidth / 2) - 5; i++)
    {
        printf(" ");
    }
    printf("Turn %-7d", turns);
    for(int i = 0; i < (screenWidth / 2) - 11; i++)
    {
        printf(" ");
    }
    printf("||\n");

    // Side
    printf("||");
    for(int i = 0; i < (screenWidth / 2) - (int) (strlen(side) / 2) - 8; i++)
    {
        printf(" ");
    }
    if(isPlayer)
        printf("(You) ");
    else
        printf("      ");
    printf("%s", side);
    for(int i = 0; i < (screenWidth / 2) - (int) (strlen(side) / 2) - 2; i++)
    {
        printf(" ");
    }
    printf("||\n");

    // Unit name
    int j = 0;
    printf("||");
    for(j = 0; j < (screenWidth / 2) - 11; j++)
    {
        printf(" ");
    }
    printf("[%04d] <||> %s", Id, unitName);
    for(j = 0; j < (screenWidth / 2) - (int) (strlen(unitName) + 5); j++)
    {
        printf(" ");
    }
    printf("||\n");

    // Unit position
    printf("||");
    for(int i = 0; i < (screenWidth / 2) - 10; i++)
    {
        printf(" ");
    }
    printf("%3dX  <||>%3dY", X, Y);
    for(int i = 0; i < (screenWidth / 2) - 8; i++)
    {
        printf(" ");
    }
    printf("||\n");

    // Moves left
     printf("||");
    for(int i = 0; i < (screenWidth / 2) - 9; i++)
    {
        printf(" ");
    }
    printf("%3d Moves Left", moves);
    for(int i = 0; i < (screenWidth / 2) - 9; i++)
    {
        printf(" ");
    }
    printf("||\n");

    // Lower Line
    for(int i = 0; i < screenWidth - 1; i++)
    {
        printf("=");
    }
    if(screenWidth % 2 == 0)
        printf("=");

    return;
}

void info_Bottom()
{
    printf("===============================================\n");
    printf("| [NumPad] Move Unit | [F] Fire At Enemy Unit |\n");
    printf("| [Esc] Exit To Menu | [A] Set Tile As Target |\n");
    printf("| [W] View Unit Wiki | [S] Set Unit As Target |\n");
    printf("| [Q] Change To Unit | [D] Current Unit Stats |\n");
    printf("| [E] Build Trenches | [Enter] Skip Your Turn |\n");
    printf("===============================================\n"); 
}

int screen_Menu(float version)
{
    int sWidth = get_ScreenWidth();
    
    system("cls");
    for(int i = 0; i < sWidth / 2 - 14; i++)
        printf("-");
    printf("#==========================#");
    for(int i = 0; i < sWidth / 2 - 14; i++)
        printf("-");
    printf("\n");
    
    for(int i = 0; i < sWidth / 2 - 14; i++)
        printf("-");
    printf("|                          |");
    for(int i = 0; i < sWidth / 2 - 14; i++)
        printf("-");
    printf("\n");
    
    for(int i = 0; i < sWidth / 2 - 14; i++)
        printf("-");
    printf("|    Total Terminal War    |");
    for(int i = 0; i < sWidth / 2 - 14; i++)
        printf("-");
    printf("\n");
    
    for(int i = 0; i < sWidth / 2 - 14; i++)
        printf("-");
    printf("|        V. %.2f           |", version);
    for(int i = 0; i < sWidth / 2 - 14; i++)
        printf("-");
    printf("\n");
    
    for(int i = 0; i < sWidth / 2 - 14; i++)
        printf("-");
    printf("|                          |");
    for(int i = 0; i < sWidth / 2 - 14; i++)
        printf("-");
    printf("\n");
    
    for(int i = 0; i < sWidth / 2 - 14; i++)
        printf("-");
    printf("#==========================#");
    for(int i = 0; i < sWidth / 2 - 14; i++)
        printf("-");
    printf("\n");
    
    for(int i = 0; i < sWidth / 2 - 14; i++)
        printf(" ");
    printf("    [I]> Iniciar um mapa    ");
    for(int i = 0; i < sWidth / 2 - 14; i++)
        printf(" ");
    printf("\n");
    
    for(int i = 0; i < sWidth / 2 - 14; i++)
        printf(" ");
    printf("     [C]> Carregar mapa     ");
    for(int i = 0; i < sWidth / 2 - 14; i++)
        printf(" ");
    printf("\n");

    for(int i = 0; i < sWidth / 2 - 14; i++)
        printf(" ");
    printf("     [S]> Encerrar jogo     ");
    for(int i = 0; i < sWidth / 2 - 14; i++)
        printf(" ");
    printf("\n");
    ShowCursor(FALSE);

    // print_Message("Hello there!", true);

    /*
    printf("--------------------#==========================#-------------------- \n");
    printf("--------------------|                          |-------------------- \n");
    printf("--------------------|    Total Terminal War    |-------------------- \n");
    printf("--------------------|        V. %.2f           |-------------------- \n", version);
    printf("--------------------|                          |-------------------- \n");
    printf("--------------------#==========================#-------------------- \n");
    printf("                        [I]> Iniciar um mapa                         \n");
    printf("                         [C]> Carregar mapa                          \n");
    printf("                         [S]> Encerrar jogo                          \n"); */

    return get_KeyPress(true);
}

void screen_Victory(B_endStats winner, B_endStats looser)
{
    system("cls");
    int sWidth = get_ScreenWidth();
    int sHeight = get_ScreenHeight();
    
    // printf("           Winner X Looser\n");
    // printf("Deployed:  %6d   %6d\n", winner.deployed, looser.deployed);
    // printf("Killed:    %6d   %6d\n", winner.killed, looser.killed);
    // printf("Lost:      %6d   %6d\n", winner.loss, looser.loss);
    // printf("Remaining: %6d   %6d\n", winner.deployed - winner.loss, looser.deployed - looser.loss);

    printf("#");
    for(int i = 0; i < sWidth - 2; i++)
        printf("-");
    printf("#\n");

    printf("|");
    for(int i = 0; i < sWidth - 2; i++)
        printf(" ");
    printf("|\n");

    printf("|");
    for(int i = 0; i < (sWidth / 2) - floorf((strlen(winner.name) / 2.0f)) - 11; i++)
        printf(" ");
    printf("%s has won the battle!", winner.name);
    for(int i = 0; i < (sWidth / 2) - ceilf((strlen(winner.name) / 2.0f)) - 11; i++)
        printf(" ");
    printf("|\n");

    for(int i = 0; i < ceilf((sHeight - 13) / 2.0f); i++)
    {
        printf("|");
        for(int j = 0; j < sWidth - 2; j++)
            printf(" ");
        printf("|\n");
    }
    
    int aux = 0;
    printf("|");
    for(int i = 0; i < (sWidth / 4.0f) - (strlen(winner.name) / 2.0f) - 1; i++)
    { printf(" "); aux++; }
    printf("%s", winner.name);
    for(int i = 0; i < (sWidth / 2.0f) - aux; i++)
        printf(" ");
    for(int i = 0; i < (sWidth / 4.0f) - (strlen(looser.name) / 2.0f); i++)
        printf(" ");
    printf("%s", looser.name);
    for(int i = 0; i < (sWidth / 4.0f) - (strlen(looser.name) / 2.0f) - 8; i++)
        printf(" ");
    printf("|\n");

    printf("> Press ENTER to continue...\n");
    getchar();
    return;
}