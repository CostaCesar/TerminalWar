#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <conio.h>
#include <Windows.h>
#include <math.h>
#include "const.h"

bool firstLine = true;
short int xHiLi = NO_UNIT, yHiLi = NO_UNIT;
typedef struct P_endStats
{
    char *name;
    int deployed;
    int loss;
    int killed;
} B_endStats;

void reset_Cursor()
{
    COORD pos = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

int get_Digits(int num)
{
    int i;
    for(i = 1; num > 9; i++)
        num /= 10;
    return i;
}

int get_ScreenWidth()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi); 
    return (int) csbi.srWindow.Right - csbi.srWindow.Left + 1;            
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

void clear_afterMap(short int mHeight)
{
    int screenWidth = get_ScreenWidth();
    COORD pos = {0, 9+mHeight*2};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
    printf("                                                                                                                                                                        ");
    printf("                                                                                                                                                                        ");
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
    return;   
}

void print_Line(char *message)
{
    int screenWidth = get_ScreenWidth();

    if(!message)
    {
        if(firstLine == true)
        {
            putchar(201);
            for(int i = 0; i < screenWidth - 2; i++)
                putchar(205);
            putchar(187);
        }
        else
        {
            putchar(200);
            for(int i = 0; i < screenWidth - 2; i++)
                putchar(205);
            putchar(188); 
        }
        firstLine = !firstLine;
    }
    else if(message[0] == ' ')
    {
        putchar(186);
        for(int i = 0; i < screenWidth - 2; i++)
             printf(" ");
        putchar(186);
    }
    else
    {
        int msg_len = strlen(message);
        putchar(186);
        for(int i = 0; i < floorf(screenWidth / 2.0f) - floorf(msg_len / 2.0f) - 1; i++)
             printf(" ");
         printf("%s", message);
         for(int i = 0; i < ceilf(screenWidth / 2.0f) - ceilf(msg_len / 2.0f) - 1; i++)
             printf(" ");
        putchar(186);
        printf("\n");
    }
    return;
}

void fillSpace_ToBottom(int offset)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi); 
    int sHeight = get_ScreenHeight();
    for(int i = csbi.dwCursorPosition.Y; i < sHeight - offset - 1; i++)
        print_Line(" ");
    return;
}

void print_LineOffset(char *message, int offset, bool doBorders)
{
    int screenWidth = get_ScreenWidth();
    if(!message)
    {
        if(firstLine == true)
        {
            putchar(201);
            for(int i = 0; i < screenWidth - 2; i++)
                putchar(205);
            putchar(187);
        }
        else
        {
            putchar(200);
            for(int i = 0; i < screenWidth - 2; i++)
                putchar(205);
            putchar(188); 
        }
        firstLine = !firstLine;
    }
    else
    {
        int msg_len = strlen(message);
        if(doBorders == true)
        {
            putchar(186);
            for(int i = 0; i < offset - 1; i++)
                printf(" ");
            printf("%s", message);
            for(int i = 0; i < screenWidth - offset - msg_len - 1; i++)
                printf(" ");
            putchar(186);
        }
        else
        {
            for(int i = 0; i < offset; i++)
                printf(" ");
            printf("%s", message);
            for(int i = 0; i < screenWidth - offset - msg_len; i++)
                printf(" ");
        }
        printf("\n");
    }
    return;
}

void print_Message(char *message, bool doWait)
{
    print_Line(NULL);
    print_Line(" ");
    print_Line(message);
    print_Line(" ");
    print_Line(NULL);

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

void print_Map(short int mHeight, short int mWidth, int* data, char** words)
{
    // Top Line
    bool mapEdge = false, edge = false;
    int words_I = 0;
    putchar(218);
    for(short int i = 0; i < mWidth; i++)
    {
        printf("%c%c%c", 196, 196, 196);
        if(edge == false) putchar(194);
        else putchar (191);

        if(i == mWidth-2) edge = true;
    }
    putchar('\n');

    // Tiles
    for(short int i = 0; i < mHeight; i++)
    {
        edge = false;
        putchar(179);
        for(short int j = 0; j < mWidth; j++)
        {
            //if(i == yHiLi && j == xHiLi)
            //    printf("%c%c%c", 219, 219, 219);
            if(data[i * mWidth + j] == MAP_MODE_A)
                printf("<A>");
            else if(data[i * mWidth + j] == MAP_MODE_B)
                printf("<B>");
            else if(data[i * mWidth + j] == MAP_MODE_CHAR) // NÃO RODA
                printf("%.3s", words[words_I++]);
            else if(data[i * mWidth + j] == MAP_MODE_NULL)
                printf("   ");
            else if(data[i * mWidth + j] < 0)
                printf("%c%c%c", 126, 126, 126);
            else printf("%3d", data[i * mWidth + j]);
            putchar(179);
        }
        putchar('\n');
        if(mapEdge == false)
        {
            putchar(195);
            for(short int j = 0; j < mWidth; j++)
            {
                printf("%c%c%c", 196, 196, 196);
                if(edge == false) putchar(197);
                else putchar(180);

                if(j == mWidth - 2) edge = true;
            }
            putchar('\n');
        }
        if(i == mHeight - 2) mapEdge = true;
    }

    // Bottom Line
    edge = false;
    putchar(192);
    for(short int i = 0; i < mWidth; i++)
    {
        printf("%c%c%c", 196, 196, 196);
        if(edge == false) putchar(193);
        else putchar (217);

        if(i == mWidth-2) edge = true;
    }
    putchar('\n');
    return;
}

void update_Map(short int xUpdate, short int yUpdate, char *data)
{
    if(!data)
        return;
    HANDLE consoleInfo = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {1+xUpdate*4, 9+yUpdate*2};
    int hScreen = get_ScreenHeight();
    SetConsoleCursorPosition(consoleInfo, pos);
    printf("%.3s", data);
    if(pos.Y < hScreen)         // Top of terminal
        pos.X = 0, pos.Y = 0;
    else pos.Y -= (hScreen/2);  // Current Location - some offset
    SetConsoleCursorPosition(consoleInfo, pos);
    return;
}

void info_Upper(char* mapName, int turns, char *side, bool isPlayer, char *unitName, int Id, short int X, short int Y, short int moves)
{
    float screenWidth = (float) get_ScreenWidth(); 
    int msg_len = 0, aux1 = 0, aux2 = 0;
    reset_Cursor();
    switch ((int) screenWidth % 4)
    {
    case 3:
        aux1++;
        break;
    case 2:
        aux2++;
        break;
    }
    screenWidth /= 2.0f;  

    // Upper Line
    putchar(201);
    for(int i = 0; i < ceilf(screenWidth) - 2; i++)
        putchar(205);
    putchar(203);
    for(int i = 0; i < floorf(screenWidth) - 1; i++)
        putchar(205);
    putchar(187);
    printf("\n");
    
    // Map name
    putchar(186);
    for(int i = 0; i < ceilf(screenWidth / 2.0f) - floorf(strlen(mapName) / 2.0f) - 1; i++)
        printf(" ");
    printf("%s", mapName);
    for(int i = 0; i < aux1 + floorf(screenWidth / 2.0f) - ceilf(strlen(mapName) / 2.0f) - 1; i++)
        printf(" ");
    putchar(186);
    // -> Comands
    msg_len = strlen("[NumPad] Move Unit | [F] Fire At Enemy Unit");
    for(int i = 0; i < floorf(screenWidth / 2.0f) - floorf(msg_len / 2.0f) - 1; i++)
        printf(" ");
    printf("[NumPad] Move Unit | [F] Fire At Enemy Unit");
    for(int i = 0; i < aux1 + aux2 + floorf(screenWidth / 2.0f) - floor(msg_len / 2.0f) - 1; i++)
        printf(" ");
    putchar(186);
    printf("\n");

    // Turn
    msg_len = strlen("Turn ") + get_Digits(turns);
    putchar(186);
    for(int i = 0; i < ceilf(screenWidth / 2.0f) - floorf(msg_len / 2.0f) - 1; i++)
        printf(" ");
    printf("Turn %d", turns);
    for(int i = 0; i < aux1 + floorf(screenWidth / 2.0f) - ceilf(msg_len / 2.0f) - 1; i++)
        printf(" ");
    putchar(186);
    // -> Comands
    msg_len = strlen("[Esc] Exit To Menu | [A] Set Tile As Target");
    for(int i = 0; i < floorf(screenWidth / 2.0f) - floorf(msg_len / 2.0f) - 1; i++)
        printf(" ");
    printf("[Esc] Exit To Menu | [A] Set Tile As Target");
    for(int i = 0; i < aux1 + aux2 + floorf(screenWidth / 2.0f) - floorf(msg_len  / 2.0f) - 1; i++)
        printf(" ");
    putchar(186);
    printf("\n");

    // Side
    putchar(186);
    for(int i = 0; i < ceilf(screenWidth / 2.0f) - floorf(strlen(side) / 2.0f) - 7; i++)
        printf(" ");
    if(isPlayer)
        printf("(You) ");
    else
        printf("      ");
    printf("%s", side);
    for(int i = 0; i < aux1 + floorf(screenWidth / 2.0f) - ceilf(strlen(side) / 2.0f) - 1; i++)
        printf(" ");
    putchar(186);
    // -> Comands
    msg_len = strlen("[W] View Unit Wiki | [S] Set Unit As Target");
    for(int i = 0; i < floorf(screenWidth / 2.0f) - floorf(msg_len / 2.0f) - 1; i++)
        printf(" ");
    printf("[W] View Unit Wiki | [S] Set Unit As Target");
    for(int i = 0; i < aux1 + aux2 + floorf(screenWidth / 2.0f) - floorf(msg_len  / 2.0f) - 1; i++)
        printf(" ");
    putchar(186);
    printf("\n");

    // Unit name
    int j = 0;
    putchar(186);
    for(j = 0; j < ceilf(screenWidth / 2.0f) - 10; j++)
        printf(" ");
    printf("[%04d] <||> %s", Id, unitName);
    for(j = 0; j < aux1 + floorf(screenWidth / 2.0f) - strlen(unitName) - 4; j++)
        printf(" ");
    putchar(186);
    // -> Comands
    msg_len = strlen("[G] Get Tile Stats | [D] Current Unit Stats");
    for(int i = 0; i < floorf(screenWidth / 2.0f) - floorf(msg_len / 2.0f) - 1; i++)
        printf(" ");
    printf("[G] Get Tile Stats | [D] Current Unit Stats");
    for(int i = 0; i < aux1 + aux2 + floorf(screenWidth / 2.0f) - floorf(msg_len  / 2.0f) - 1; i++)
        printf(" ");
    putchar(186);
    printf("\n");

    // Unit position
    putchar(186);
    for(int i = 0; i < ceilf(screenWidth / 2.0f) - 9; i++)
        printf(" ");
    printf("%3dX  <||>%3dY", X, Y);
    for(int i = 0; i < aux1 + floorf(screenWidth / 2.0f) - 7; i++)
        printf(" ");
    putchar(186);
    // -> Comands
    msg_len = strlen("[T] Shift Map Mode | [Q] Get The Unit Stats");
    for(int i = 0; i < floorf(screenWidth / 2.0f) - floorf(msg_len / 2.0f) - 1; i++)
        printf(" ");
    printf("[T] Shift Map Mode | [Q] Get The Unit Stats");
    for(int i = 0; i < aux1 + aux2 + floorf(screenWidth / 2.0f) - floorf(msg_len  / 2.0f) - 1; i++)
        printf(" ");
    putchar(186);
    printf("\n");

    // Moves left
    putchar(186);
    if(moves > -1)
    {
        msg_len = get_Digits(moves) + 11;
        for(int i = 0; i < ceilf(screenWidth / 2.0f) - floorf(msg_len / 2.0f) - 1; i++)
            printf(" ");
        printf("%d Moves Left", moves);
        for(int i = 0; i < aux1 + floorf(screenWidth / 2.0f) - ceilf(msg_len / 2.0f) - 1; i++)
            printf(" ");
    }
    else
    {
        int k = 0;
        if((int) (screenWidth * 2) % 4 == 2) k++; 
        for(k; k < aux2 + ceilf(screenWidth) - 2; k++)
            printf(" ");
    }
    putchar(186);
    // -> Comands
        msg_len = strlen("[E] Build Trenches | [Enter] Skip Your Turn");
        for(int i = 0; i < floorf(screenWidth / 2.0f) - floorf(msg_len / 2.0f) - 1; i++)
            printf(" ");
        printf("[E] Build Trenches | [Enter] Skip Your Turn");
        for(int i = 0; i < aux1 + aux2 + floorf(screenWidth / 2.0f) - floorf(msg_len  / 2.0f) - 1; i++)
            printf(" ");
    putchar(186);
    printf("\n");

    // Lower Line
    putchar(200);
    for(int i = 0; i < ceilf(screenWidth) - 2; i++)
        putchar(205);
    putchar(202);
    for(int i = 0; i < floorf(screenWidth) - 1; i++)
        putchar(205);
    putchar(188);
    // if(screenWidth % 2 == 0)
    //     printf("=");
    putchar('\n');
    return;
}

// void info_Bottom()
// {
//     putchar(201);
//     for(int i = 0; i < 45; i++)
//         putchar(205);
//     putchar(187);
//     printf("\n");
//     printf("%c [NumPad] Move Unit | [F] Fire At Enemy Unit %c\n", 186, 186);
//     printf("%c [Esc] Exit To Menu | [A] Set Tile As Target %c\n", 186, 186);
//     printf("%c [W] View Unit Wiki | [S] Set Unit As Target %c\n", 186, 186);
//     printf("%c [G] Get Tile Stats | [D] Current Unit Stats %c\n", 186, 186);
//     printf("%c [E] Build Trenches | [Enter] Skip Your Turn %c\n", 186, 186);
//     printf("%c [XXX] PLACEHOLDER1 | [XXX] BIG_PLACEHOLDER2 %c");
//     putchar(200);
//     for(int i = 0; i < 45; i++)
//         putchar(205);
//     putchar(188);
//     printf("\n");
// }

int listScen()
{
    system("cls");
    int nScen = 0;
    WIN32_FIND_DATA fd;
    HANDLE handle = FindFirstFile("scenarios/*.txt", &fd);
    if(handle != INVALID_HANDLE_VALUE)
    {
        print_Line(NULL);
        print_Line(" ");
        do
        {
            if(fd.dwFileAttributes && FILE_ATTRIBUTE_DIRECTORY)
            {
                nScen++;
                print_LineOffset(fd.cFileName, 7, true);
            }  
        } while (FindNextFile(handle, &fd));
        print_Line(" ");
        print_Line(NULL);
        FindClose(handle);
    }
    else
        print_Message("Can't load the scenarios folder, get at least one scenario in there!", true);
    return nScen;
}

int screen_Scenery()
{
    int key = 0, nScen = 0;
    COORD pos = {3, 2};
    toggle_Cursor(false);
    nScen = listScen();
    do 
    {
        pos.X = 3;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
        printf(">");
        key = get_KeyPress(false);
        if(key == -32)
        {
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
            printf(" ");
            switch ((int) getch())
            {
            case 72: // Up
                pos.Y--;
                if(pos.Y < 2)
                    pos.Y = nScen+1;
                break;
            case 80: // Down
                pos.Y++;
                if(pos.Y > nScen+1)
                    pos.Y = 2;
                break;
            default:
                break;
            }
        }
        else if(key == KEY_ENTER) break;
    } while(1);
    nScen = pos.Y-1;
    pos.X = 0, pos.Y = nScen+5;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
    return nScen;
}

void screen_TopMap(char *scn_Name, char *map_Name)
{
    char word[STRING_NAME+5] = {0};

    system("cls");
    print_Line(NULL);
    print_Line(" ");
    
    // Scenario Name
    snprintf(word, sizeof(word), "%c", 218);
    for(int i = 1; i < strlen(scn_Name)+3; i++)
        word[i] = 196;
    word[strlen(scn_Name)+3] = 191;
    print_Line(word);
    snprintf(word, sizeof(word), "%c %s %c", 179, scn_Name, 179);
    print_Line(word);
    for(int i = 1; i < strlen(scn_Name)+3; i++)
        word[i] = 196;
    word[0] = 192; word[strlen(scn_Name)+3] = 217;
    print_Line(word);
    
    print_Line(" ");
    snprintf(word, sizeof(word), "%c %s %c", 175, map_Name, 174);
    print_Line(word);
    print_Line(" ");
    return;
}

int screen_MapInput(int cMap, int nMaps)
{
    int key = '\0';
    while(1)
    {
        key = get_KeyPress(false);
        if(key == -32)
        {
            key = getch();
            switch (key)
            {
            case 77: // Up
                cMap++;
                if(cMap >= nMaps) cMap = 0;
                return cMap;
            case 75: // Down
                cMap--;
                if(cMap < 0) cMap = nMaps-1;
                return cMap;
            default:
                break;
            }
        }
        else if(key == KEY_ENTER) return FUNCTION_SUCESS;
        else if(key == KEY_ESCAPE) return FUNCTION_FAIL;
    }
}

int screen_Menu(float version)
{
    int sWidth = get_ScreenWidth();
    bool isOdd = (sWidth % 2) == 1;
    COORD pos = {(sWidth / 2 - 15), 0};
    HANDLE hand = GetStdHandle(STD_OUTPUT_HANDLE);

    system("cls");
    SetConsoleCursorPosition(hand, pos);
    if(isOdd) putchar(218);
    putchar(201);
    for(int i = 0; i < 28; i++)
        putchar(205);
    putchar(187);
    printf("\n");
    pos.Y++;
    
    SetConsoleCursorPosition(hand, pos);
    if(isOdd) putchar(179);
    printf("%c                            %c", 186, 186);;
    printf("\n");
    pos.Y++;
    
    SetConsoleCursorPosition(hand, pos);
    if(isOdd) putchar(179);
    printf("%c     Total Terminal War     %c", 186, 186);
    printf("\n");
    pos.Y++;
    
    SetConsoleCursorPosition(hand, pos);
    if(isOdd) putchar(179);
    printf("%c         V. %.2f            %c", 186, version, 186);
    printf("\n");
    pos.Y++;
    
    SetConsoleCursorPosition(hand, pos);
    if(isOdd) putchar(179);
    printf("%c                            %c", 186, 186);;
    printf("\n");
    pos.Y++;
    
    SetConsoleCursorPosition(hand, pos);
    if(isOdd) putchar(192);
    putchar(200);
    for(int i = 0; i < 28; i++)
        putchar(205);
    putchar(188);
    printf("\n");
    pos.Y++;
    
    for(int i = 0; i < sWidth / 2 - 12; i++)
        printf(" ");
    printf("[ENTER] Iniciar um mapa");
    // for(int i = 0; i < sWidth / 2 - 12; i++)
    //     printf(" ");
    printf("\n");
    
    for(int i = 0; i < sWidth / 2 - 11; i++)
        printf(" ");
    printf(" [S] Sobre o jogo");
    // for(int i = 0; i < sWidth / 2 - 9; i++)
    //     printf(" ");
    printf("\n");

    for(int i = 0; i < sWidth / 2 - 11; i++)
        printf(" ");
    printf("[ESC] Encerrar jogo");
    // for(int i = 0; i < sWidth / 2 - 10; i++)
    //     printf(" ");
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
    int auxHeight = 0;
    
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
        auxHeight++;
        printf("|");
        for(int j = 0; j < sWidth - 2; j++)
            printf(" ");
        printf("|\n");
    }
    
    int aux = 0;
    printf("|");
    for(int i = 0; i < (sWidth / 4.0f) - floorf((strlen(winner.name) / 2.0f)) - 1; i++)
    { printf(" "); aux++; }
    printf("%s", winner.name);
    for(int i = 0; i < (sWidth / 2.0f) - aux - (strlen(winner.name)); i++)
        printf(" ");
    for(int i = 0; i < (sWidth / 4.0f) - (strlen(looser.name) / 2.0f); i++)
        printf(" ");
    printf("%s", looser.name);
    for(int i = 0; i < (sWidth / 4.0f) - ceilf((strlen(looser.name) / 2.0f)) - 2; i++)
        printf(" ");
    printf("|\n");

    printf("|");
    for(int i = 0; i < sWidth - 2; i++)
        printf(" ");
    printf("|\n");

    aux = 0;
    int nLen = get_Digits(winner.deployed);
    printf("|");
    for(int i = 0; i < (sWidth / 4.0f) - floorf(nLen / 2.0f) - 1; i++)
    { printf(" "); aux++; }
    printf("%d", winner.deployed); aux += nLen;
    for(int i = 0; i < (sWidth / 2.0f) - aux - 4; i++)
        printf(" ");
    printf("Deployed"); nLen = get_Digits(looser.deployed);
    for(int i = 0; i < (sWidth / 4.0f) - floorf(nLen / 2.0f) - 4; i++)
        printf(" ");
    printf("%d", looser.deployed);
    for(int i = 0; i < (sWidth / 4.0f) - ceilf(nLen / 2.0f) - 2; i++)
        printf(" ");
    printf("|\n");

    aux = 0, nLen = get_Digits(winner.killed);
    printf("|");
    for(int i = 0; i < (sWidth / 4.0f) - floorf(nLen / 2.0f) - 1; i++)
    { printf(" "); aux++; }
    printf("%d", winner.killed); aux += nLen;
    for(int i = 0; i < (sWidth / 2.0f) - aux - 3; i++)
        printf(" ");
    printf("Killed"); nLen = get_Digits(looser.killed);
    for(int i = 0; i < (sWidth / 4.0f) - floorf(nLen / 2.0f) - 3; i++)
        printf(" ");
    printf("%d", looser.killed);
    for(int i = 0; i < (sWidth / 4.0f) - ceilf(nLen / 2.0f) - 2; i++)
        printf(" ");
    printf("|\n");

    aux = 0, nLen = get_Digits(winner.loss);
    printf("|");
    for(int i = 0; i < (sWidth / 4.0f) - floorf(nLen / 2.0f) - 1; i++)
    { printf(" "); aux++; }
    printf("%d", winner.loss); aux += nLen;
    for(int i = 0; i < (sWidth / 2.0f) - aux - 2; i++)
        printf(" ");
    printf("Lost"); nLen = get_Digits(looser.loss);
    for(int i = 0; i < (sWidth / 4.0f) - floorf(nLen / 2.0f) - 2; i++)
        printf(" ");
    printf("%d", looser.loss);
    for(int i = 0; i < (sWidth / 4.0f) - ceilf(nLen / 2.0f) - 2; i++)
        printf(" ");
    printf("|\n");

    aux = 0, nLen = get_Digits(winner.deployed - winner.loss);
    printf("|");
    for(int i = 0; i < (sWidth / 4.0f) - floorf(nLen / 2.0f) - 1; i++)
    { printf(" "); aux++; }
    printf("%d", winner.deployed - winner.loss); aux += nLen;
    for(int i = 0; i < (sWidth / 2.0f) - aux - 4; i++)
        printf(" ");
    printf("Remaining"); nLen = get_Digits(looser.deployed - looser.loss);
    for(int i = 0; i < (sWidth / 4.0f) - floorf(nLen / 2.0f) - 5; i++)
        printf(" ");
    printf("%d", looser.deployed - looser.loss);
    for(int i = 0; i < (sWidth / 4.0f) - ceilf(nLen / 2.0f) - 2; i++)
        printf(" ");
    printf("|\n");

    for(int i = 0; i < ceilf((sHeight - 13) / 2.0f) - 1; i++)
    {
        printf("|");
        for(int j = 0; j < sWidth - 2; j++)
            printf(" ");
        printf("|\n");
    }

    printf("|");
    for(int i = 0; i < (sWidth / 2.0f) - 14; i++)
        printf(" ");
    printf("Press ENTER to continue..."); 
    for(int i = 0; i < (sWidth / 2.0f) - 14; i++)
        printf(" ");
    printf("|\n");

    printf("|");
    for(int i = 0; i < sWidth - 2; i++)
        printf(" ");
    printf("|\n");

    printf("|");
    for(int i = 0; i < sWidth - 2; i++)
        printf(" ");
    printf("|\n");

    printf("#");
    for(int i = 0; i < sWidth - 2; i++)
        printf("-");
    printf("#\n");

    return;
}