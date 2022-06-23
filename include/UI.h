#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

int get_ScreenWidtdh();
int get_ScreenHeight();
void toggle_Cursor(bool cursor);

#ifdef _WIN32
    #include <conio.h>
    #include <Windows.h>
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
    #define CLEAR "cls"
#else
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <sys/types.h>
    #include <termios.h>
    #include <unistd.h>
    #include <stdlib.h>

    int kbhit()
    {
        struct termios argin, argout;
        unsigned char ch = 0;
        tcgetattr(0,&argin);
        argout = argin;
        argout.c_lflag &= ~(ICANON);
        argout.c_iflag &= ~(ICRNL);
        argout.c_oflag &= ~(OPOST);
        argout.c_cc[VMIN] = 1;
        argout.c_cc[VTIME] = 0;
        tcsetattr(0,TCSADRAIN,&argout);
        read(0, &ch, 1);
        tcsetattr(0,TCSADRAIN,&argin);
        return ch;
    }
    int get_ScreenWidth()
    {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return w.ws_col;
    }
    int get_ScreenHeight()
    {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return w.ws_row;
    }
    void toggle_Cursor(bool cursor)
    {
        if(cursor == true)
            printf("\e[?25l");
        else
            printf("\e[?25h");
        return;
    }
    #define getch() kbhit();
    #define Sleep(time) sleep(time);
    #define CLEAR "clear"
#endif

typedef struct P_endStats
{
    int deployed;
    int loss;
    int killed;
} B_endStats;


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
    
    system("clear");
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
    toggle_Cursor(false);
    // ShowCursor(FALSE);

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
    
    printf("           Winner X Looser\n");
    printf("Deployed:  %6d   %6d\n", winner.deployed, looser.deployed);
    printf("Killed:    %6d   %6d\n", winner.killed, looser.killed);
    printf("Lost:      %6d   %6d\n", winner.loss, looser.loss);
    printf("Remaining: %6d   %6d\n", winner.deployed - winner.loss, looser.deployed - looser.loss);

    // printf("#");
    // for(int i = 0; i < sWidth - 2; i++)
    //     printf("-");
    // printf("#\n");
// 
    // printf("|");
    // for(int i = 0; i < sWidth - 2; i++)
    //     printf(" ");
    // printf("|\n");
// 
    // printf("|");
    // for(int i = 0; i < sWidth - 2; i++)
    //     printf(" ");
    // printf("|\n");

    printf("> Press ENTER to continue...\n");
    getchar();
    return;
}