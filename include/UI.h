#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <conio.h>
#include <Windows.h>
#include <math.h>
#include "const.h"
#include "unit.h"

bool firstLine = true, muted = false;
typedef struct P_tileData
{
    int *veget;
    int *terrain;
    short int *height;
    char *unit;
    int spawn;
} B_tileData;

void reset_Cursor()
{
    COORD pos = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
    return;
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

int get_HalfWidth()
{
    return (int) ceilf(get_ScreenWidth() / 2.0f);
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
    // int screenWidth = get_ScreenWidth();
    COORD pos = {0, MAP_OFFSET_Y+mHeight*2};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
    printf("%100s", " ");
    printf("%100s", " ");
    printf("%100s", " ");
    printf("%100s", " ");
    printf("%100s", " ");
    printf("%100s", " ");
    printf("%100s", " ");
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
    return;   
}

void print_Line(char *message, int size)
{
    int screenSize = get_ScreenWidth();
    if (size == 0)
        size = screenSize;
    CONSOLE_SCREEN_BUFFER_INFO cursor;
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor);
    bool cursorInMiddle = cursor.dwCursorPosition.X > 0;
    
    if(!message)
    {
        if(firstLine == true)
        {
            if(cursorInMiddle == true)
                putchar(203);
            else putchar(201);
            for(int i = 0; i < size - 2; i++)
                putchar(205);
            if(size != screenSize && !cursorInMiddle)
                putchar(203);
            else putchar(187);
        }
        else
        {
            if(cursorInMiddle == true)
                putchar(202);
            else putchar(200);
            for(int i = 0; i < size - 2; i++)
                putchar(205);
            if(size != screenSize && !cursorInMiddle)
                putchar(202);
            else putchar(188); 
        }
        firstLine = !firstLine;
    }
    else
    {
        putchar(186);
        // GetConsoleScreenBufferInfo(console, &cursor);

        if(message[0] != ' ' || strlen(message) > 1)
        {
            int msg_len = strlen(message);
            for(int i = 0; i < ceilf(size / 2.0f) - floorf(msg_len / 2.0f) - 1; i++)
                putchar(' ');
            printf(message);
            for(int i = 0; i < floorf(size / 2.0f) - ceilf(msg_len / 2.0f) - 1; i++)
                putchar(' ');
            // cursor.dwCursorPosition.X = floorf(screenWidth / 2.0f) - floorf(msg_len / 2.0f);
            // SetConsoleCursorPosition(console, cursor.dwCursorPosition);
            // printf("%s", message);
        }
        else
        {
            for(int i = 0; i < size - 2; i++)
                putchar(' ');
        }
        // cursor.dwCursorPosition.X = screenWidth-1;
        // SetConsoleCursorPosition(console, cursor.dwCursorPosition);
        putchar(186);
    }
    printf("\n");
    return;
}

void print_LineOffset(char *message, int width, int offset)
{
    int screenWidth = get_ScreenWidth();
    CONSOLE_SCREEN_BUFFER_INFO cursor;
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor);
    bool cursorInMiddle = cursor.dwCursorPosition.X > 0;
    if (width == 0)
        width = screenWidth;

    if(!message)
    {
        if(firstLine == true)
        {
            if(cursorInMiddle == true)
                putchar(203);
            else putchar(201);
            for(int i = 0; i < width - 2; i++)
                putchar(205);
            if(width != screenWidth && !cursorInMiddle)
                putchar(203);
            else putchar(187);
        }
        else
        {
            if(cursorInMiddle == true)
                putchar(202);
            else putchar(200);
            for(int i = 0; i < width - 2; i++)
                putchar(205);
            if(width != screenWidth && !cursorInMiddle)
                putchar(202);
            else putchar(188); 
        }
        firstLine = !firstLine;
    }
    else
    {
        int msg_len = strlen(message);
        putchar(186);
        GetConsoleScreenBufferInfo(console, &cursor);
        
        cursor.dwCursorPosition.X = offset;
        SetConsoleCursorPosition(console, cursor.dwCursorPosition);
        printf("%s", message);
        
        cursor.dwCursorPosition.X = width-1;
        SetConsoleCursorPosition(console, cursor.dwCursorPosition);
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
    for(int i = csbi.dwCursorPosition.Y; i < sHeight - offset; i++)
        print_Line(" ", 0);
    return;
}

void print_Message(char *message, int width, int line, bool overwrite, bool offset, bool wait)
{
    // little hack to clear screen
    if(offset == true && line == 1)
        print_Message("  ", width, 0, true, false, false);

    int msg_len = strlen(message);
    int screenWidth = get_ScreenWidth();
    CONSOLE_SCREEN_BUFFER_INFO cursor;
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    
    GetConsoleScreenBufferInfo(console, &cursor);
    int startFrom = cursor.dwCursorPosition.X;
    cursor.dwCursorPosition.Y = 0;
    SetConsoleCursorPosition(console, cursor.dwCursorPosition);
    
    print_Line(NULL, width);
    cursor.dwCursorPosition.Y++;
    SetConsoleCursorPosition(console, cursor.dwCursorPosition);

    if (overwrite == true)
    {
        for(int i = 0; i < line; i++)
        {
            GetConsoleScreenBufferInfo(console, &cursor);
            print_Line(" ", width);
            cursor.dwCursorPosition.Y++;
            SetConsoleCursorPosition(console, cursor.dwCursorPosition);
        }
    }
    else
    {
        cursor.dwCursorPosition.Y += line, cursor.dwCursorPosition.X = startFrom;
        SetConsoleCursorPosition(console, cursor.dwCursorPosition);
    }
    
    GetConsoleScreenBufferInfo(console, &cursor);
    if(offset == true)
        print_LineOffset(message, width, 2);
    else print_Line(message, width);
    
    if(overwrite == true)
    {
        for(int i = 0; i < 5 - line; i++)
        {
            cursor.dwCursorPosition.Y++, cursor.dwCursorPosition.X = startFrom;
            SetConsoleCursorPosition(console, cursor.dwCursorPosition);
            GetConsoleScreenBufferInfo(console, &cursor);
            print_Line(" ", width);
        }
        cursor.dwCursorPosition.Y++, cursor.dwCursorPosition.X = startFrom;
    }
    else cursor.dwCursorPosition.Y = 7, cursor.dwCursorPosition.X = startFrom;
    
    SetConsoleCursorPosition(console, cursor.dwCursorPosition);
    print_Line(NULL, width);

    if(offset == true)
    {
        cursor.dwCursorPosition.Y = line + 1;
        cursor.dwCursorPosition.X = startFrom + 2 + strlen(message);
        SetConsoleCursorPosition(console, cursor.dwCursorPosition);
    }
    if(wait == true)
        Sleep(TIME_STRATEGY);
    return;     
}

void print_UnitDesc(int descPos, char *descFile)
{
    char file[STRING_NAME] = "units/";
    strcat(file, descFile);
    FILE *desc = fopen(file, "r");
    if(desc == NULL)
    {
        printf("ERROR: Could not load file \n");
        return;
    }
    char buffer[STRING_DESC];
    int count = 0;
    do 
    {
        fgets(buffer, STRING_DESC, desc);
        if(buffer[0] == '#')
            count++;
        else if(feof(desc))
            break;
    } while(count <= descPos);
    
    print_Line(NULL, 0);
    print_Line(" ", 0);
    do
    {
        fgets(buffer, STRING_DESC, desc);
        buffer[strlen(buffer) -1] = '\0';
        
        if(buffer[0] == '#')
            break;
        print_Line(buffer, 0);
        
    } while (1);
    print_Line(" ", 0);
    print_Line(NULL, 0);
    
    fclose(desc);
    return;
}

char get_KeyPress(bool fLowerCase)
{
    char out = (char) getch();
    if(out >= 'A' && out <= 'Z' && fLowerCase == true)
        out += 32;
    return out;
}

int get_MapSprite_Graphic(B_tileData *tile, char *msg)
{
    int colorOut = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
    HANDLE nmd = GetStdHandle(STD_OUTPUT_HANDLE);
    if(*(tile->height) < 0 || *(tile->terrain) == Water)
    {
        for(int pixel = 0; pixel < 3; pixel++) msg[pixel] = '~';
        colorOut |= BACKGROUND_BLUE;
    }
    else if(*(tile->height) > 4 || *(tile->terrain) == Snow)
    {
        for(int pixel= 0; pixel < 3; pixel++) msg[pixel] = 219;
        colorOut |= BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;
    }
    else if(*(tile->terrain) == Mud)
    {
        for(int pixel= 0; pixel< 3; pixel++) msg[pixel] = 178;
        colorOut |= BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY;
    }
    else if(*(tile->terrain) == Rock)
    {
        for(int pixel= 0; pixel< 3; pixel++) msg[pixel] = '#';
        colorOut |= BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;
        colorOut ^= FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
    }
    else if(*(tile->terrain) == Sand)
    {
        for(int pixel= 0; pixel< 3; pixel++) msg[pixel] = 177;
        colorOut |= BACKGROUND_GREEN | BACKGROUND_RED;
    }
    else
    {
        for(int pixel= 0; pixel< 3; pixel++) msg[pixel] = 176;
        colorOut |= BACKGROUND_GREEN;
    }
    if(*(tile->veget) > Field)
    {
        colorOut ^= FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;
        switch (*(tile->veget))
        {
        case Sparse:
            msg[rand() % 3] = 213, msg[rand() % 3] = 213;
            break;
        case Grove:
            msg[rand() % 3] = 179, msg[rand() % 3] = 179;
            break;
        case Forest:
            msg[rand() % 3] = 215, msg[rand() % 3] = 215;
            break;
        case Jungle:
            msg[rand() % 3] = 186, msg[rand() % 3] = 186;
            break;
        }
    }
    return colorOut;
}

void print_MapGraphic(short int mHeight, short int mWidth, B_tileData *data)
{
    // Top Line
    HANDLE hnd = GetStdHandle(STD_OUTPUT_HANDLE);
    bool mapEdge = false, edge = false;
    int words_I = 0, color = 0;
    char msg[4] = {0};
    printf("     |");
    for(int i = 0; i < mWidth; i++)
    {
        printf("%3d|", i);
    }
    printf("\n     ");
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
        printf("[%3d]", i);
        putchar(179);
        for(short int j = 0; j < mWidth; j++)
        {
            if(data[i * mWidth + j].unit) printf("%.3s", data[i * mWidth + j].unit);
            else
            {
                color = get_MapSprite_Graphic(&data[i * mWidth + j], msg);
                SetConsoleTextAttribute(hnd, color);
                printf("%3s", msg);
            }
            SetConsoleTextAttribute(hnd, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
            putchar(179);
        }
        putchar('\n');
        printf("     ");
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

void print_MapStats(short int mHeight, short int mWidth, B_tileData *data)
{
    // Top Line
    bool mapEdge = false, edge = false;
    int words_I = 0, tile = 0;
    printf("     |");
    for(int i = 0; i < mWidth; i++)
    {
        printf("%3d|", i);
    }
    printf("\n     ");
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
        printf("[%3d]", i);
        putchar(179);
        for(short int j = 0; j < mWidth; j++)
        {
            if(data->spawn >= 0)
            {
                if(data[i * mWidth + j].unit) printf("%.3s", data[i * mWidth + j].unit);
                else
                {
                    switch (data[i * mWidth + j].spawn)
                    {
                    case 0:
                        printf("   ");
                        break;
                    case 1:
                        printf("<A>");
                        break;
                    case 2:
                        printf("<B>");
                        break;
                    }
                }
            }
            else
            {
                if(data->terrain)
                    tile = (int) *(data[i * mWidth + j].terrain);
                else if(data->veget != NULL)
                    tile = (int) *(data[i * mWidth + j].veget);
                else
                    tile = (int) *(data[i * mWidth + j].height);
                    
                if(data[i * mWidth + j].unit) printf("%.3s", data[i * mWidth + j].unit);
                else printf("%3d", tile);
            }
            putchar(179);
        }
        putchar('\n');
        printf("     ");
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

void set_MapCursor(short int xUpdate, short int yUpdate)
{
    HANDLE consoleInfo = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {MAP_OFFSET_X+xUpdate*4, MAP_OFFSET_Y+yUpdate*2};
    int hScreen = get_ScreenHeight();
    SetConsoleCursorPosition(consoleInfo, pos);
    return;
}

void update_Map(short int xUpdate, short int yUpdate, char *data)
{
    if(!data)
        return;
    HANDLE consoleInfo = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {MAP_OFFSET_X+xUpdate*4, MAP_OFFSET_Y+yUpdate*2};
    int hScreen = get_ScreenHeight();
    SetConsoleCursorPosition(consoleInfo, pos);
    printf("%.3s", data);
    if(pos.Y < hScreen)         // Top of terminal
        pos.X = 0, pos.Y = 0;
    else pos.Y -= (hScreen/2);  // Current Location - some offset
    SetConsoleTextAttribute(consoleInfo, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
    SetConsoleCursorPosition(consoleInfo, pos);
    return;
}

void info_Upper(char* mapName, MapMode mode, int turns, char *side, bool isPlayer, char *unitName, int Id, B_Pos pos, short int moves)
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
    // screenWidth /= 2.0f;  

    // Upper Line
    CONSOLE_SCREEN_BUFFER_INFO cursor;
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(console, &cursor);
    int size = (int)ceilf(screenWidth / 2.0f) - 1;
    cursor.dwCursorPosition.X = size;
    char msg[STRING_NAME];
    
    // Map name
    snprintf(msg, sizeof(msg), "             %s - %10s", mapName, get_MapMode(mode));
    print_Message(msg, cursor.dwCursorPosition.X + 1, 0, true, false, false);
    // -> Comands
    SetConsoleCursorPosition(console, cursor.dwCursorPosition);
    print_Message("[NumPad] Move Unit | [F] Fire At Enemy Unit", (int)floorf(screenWidth / 2.0f), 0, false, false, false);

    // Turn
    reset_Cursor();
    snprintf(msg, sizeof(msg), "Turn %d", turns);
    print_Message(msg, size + 1, 1, false, false, false);
    // -> Comands
    cursor.dwCursorPosition.X = size;
    SetConsoleCursorPosition(console, cursor.dwCursorPosition);
    print_Message("[Esc] Exit To Menu | [A] Set Tile As Target", (int)floorf(screenWidth / 2.0f), 1, false, false, false);
    
    // Side
    reset_Cursor();
    snprintf(msg, sizeof(msg), "%s%s%s", (isPlayer ? "(You) " : ""), side, (isPlayer ? "      " : ""));
    print_Message(msg, size + 1, 2, false, false, false);
    // -> Comands
    cursor.dwCursorPosition.X = size;
    SetConsoleCursorPosition(console, cursor.dwCursorPosition);
    print_Message("[W] View Unit Wiki | [S] Set Unit As Target", (int)floorf(screenWidth / 2.0f), 2, false, false, false);

    // Unit name
    reset_Cursor();
    snprintf(msg, sizeof(msg), "         [%04d] <||> %-15s", Id, unitName);
    print_Message(msg, size + 1, 3, false, false, false);
    // -> Comands
    cursor.dwCursorPosition.X = size;
    SetConsoleCursorPosition(console, cursor.dwCursorPosition);
    print_Message("[G] Get Tile Stats | [D] Current Unit Stats", (int)floorf(screenWidth / 2.0f), 3, false, false, false);

    // Unit position
    reset_Cursor();
    snprintf(msg, sizeof(msg), "%3dX <||> %3dY", pos.X, pos.Y);
    print_Message(msg, size + 1, 4, false, false, false);
    // -> Comands
    cursor.dwCursorPosition.X = size;
    SetConsoleCursorPosition(console, cursor.dwCursorPosition);
    print_Message("[T] Shift Map Mode | [Q] Get The Unit Stats", (int)floorf(screenWidth / 2.0f), 4, false, false, false);

    // Moves left
    reset_Cursor();
    if(moves > -1)
        snprintf(msg, sizeof(msg), "%d Moves Left", moves);
    else snprintf(msg, sizeof(msg), " ");
    print_Message(msg, size + 1, 5, false, false, false);
    // -> Comands
    cursor.dwCursorPosition.X = size;
    SetConsoleCursorPosition(console, cursor.dwCursorPosition);
    print_Message("[E] Build Trenches | [Enter] Skip Your Turn", (int)floorf(screenWidth / 2.0f), 5, false, false, false);
    
    return; // 8 lines 
}

void jukebox(char *sound, DWORD param)
{
    char play[STRING_NAME] = "sound/";
    strcat(play, sound);
    if(!muted) PlaySound(play, NULL, param | SND_NODEFAULT);
    else PlaySound(NULL, NULL, param);
    return;
}

void show_gUnit(B_Unit *unit)
{
    char msg[70];
    char buff[25];

    print_Line(NULL, 0);
    print_Line(" ", 0);
    if(unit->Game_ID >= 0)
    {
        snprintf(msg, sizeof(msg), "Unit %d", unit->Game_ID);
        print_Line(msg, 0);
    }
    snprintf(msg, sizeof(msg), "%s [%s]", unit->name, (unit->faction ? unit->faction : ""));
    print_Line(msg, 0);
    snprintf(msg, sizeof(msg), "Level %d", unit->level);
    print_Line(msg, 0);
    snprintf(msg, sizeof(msg), "Men: %d | %d", unit->men, unit->men_Max);
    print_Line(msg, 0);
    snprintf(msg, sizeof(msg), "Morale: %.2f", unit->morale);
    print_Line(msg, 0);
    print_Line("Melee Stats", 0);
    snprintf(msg, sizeof(msg), "%d OFS X %d DFS", unit->attack_MeleeP, unit->defend_MeleeP);
    print_Line(msg, 0);
    print_Line("Ranged Stats", 0);
    snprintf(msg, sizeof(msg), "%d OFS X %d DFS", unit->attack_RangeP, unit->defend_RangeP);
    print_Line(msg, 0);
    snprintf(msg, sizeof(msg), "Build Power: %d", unit->build_Cap);
    print_Line(msg, 0);

    if (unit->range > 0)
    {
        snprintf(msg, sizeof(msg), "Range: %d tiles | Ammo: %d left", unit->range, unit->ammo);
        print_Line(msg, 0);
    }
    print_Line(" ", 0);
    
    msg[0] = '\0';
    for (int i = 0; i < unit->buffs_S; i++)
    {
        snprintf(buff, sizeof(buff), "[+] %s ", get_UnitBuff(unit->buffs[i]));
        strcat(msg, buff);
        if (i > 5)
        {
            print_Line(msg, 0);
            msg[0] = '\0';
        }
    }
    if (msg[0] != '\0')
        print_Line(msg, 0);

    if(unit->chaseID != NULL)
    {
        snprintf(buff, sizeof(buff), "Chasing unit %d", *(unit->chaseID));
        print_Line(buff, 0);
    }
    if(unit->goal.X != NO_UNIT && unit->goal.Y != NO_UNIT)
    {
        snprintf(buff, sizeof(buff), "Going to %dX %dY", unit->goal.X, unit->goal.Y);
        print_Line(buff, 0);
    }

    print_Line(" ", 0);
    if (unit->engaged == true)
        print_Line("<!> Engaged In Combat", 0);
    if (unit->retreating == true)
        print_Line("<!> Retreating", 0);
    print_Line(NULL, 0);
}

int listScen()
{
    system("cls");
    int nScen = 0;
    WIN32_FIND_DATA fd;
    HANDLE handle = FindFirstFile("scenarios/*.txt", &fd);
    if(handle != INVALID_HANDLE_VALUE)
    {
        print_Line(NULL, 0);
        print_Line(" ", 0);
        do
        {
            if(fd.dwFileAttributes && FILE_ATTRIBUTE_DIRECTORY)
            {
                nScen++;
                print_LineOffset(fd.cFileName, 0, 7);
            }  
        } while (FindNextFile(handle, &fd));
        print_Line(" ", 0);
        print_Line(NULL, 0);
        FindClose(handle);
    }
    else
    {
        print_Message("Can't load the scenarios folder!", get_ScreenWidth(), 1, true, false, false);
        print_Message("Must have at least one scenario in there", get_ScreenWidth(), 2, false, false, true);
    }
    return nScen;
}

void show_TextFile(char *path)
{
    int width = get_ScreenWidth();
    FILE *text = fopen(path, "r");
    if(!text)
        return;
    
    char *word = (char *) malloc(width * sizeof(char));
    if(!word)
        return;
    
    print_Line(NULL, 0);
    while(!feof(text))
    {
        fgets(word, width - 4, text);
        word[strlen(word) - 1] = '\0';
        print_Line(word, 0);
    }
    print_Line(NULL, 0);
    
    fclose(text);
    free(word);
    return;
}

void screen_Credits(float version)
{
    char vers[10] = {0};
    snprintf(vers, sizeof(vers), "%2.2f", version);
    system("cls");

    print_Line(NULL, 0);
    print_Line(" ", 0);
    print_Line("Total Terminal War", 0);
    print_Line(vers, 0);
    print_Line("CostaCesar 2022", 0);
    print_Line(" ", 0);
    print_Line("Programador, Designer e Artista:", 0);
    print_Line("Caio Cesar Moraes Costa", 0);
    print_Line(" ", 0);
    print_Line("Apoiadores [<3]:", 0);
    print_Line("Bongo, Alvinho, Prates, Arthur Marceneiro", 0);
    print_Line(" ", 0);
    print_Line("Musica padrao: ", 0);
    print_Line("Civilization 2 - Theme", 0);
    print_Line("Age Of Empires I - Siege(Wally)", 0);
    print_Line(" ", 0);
    print_LineOffset("Aperte ENTER para continuar", 0, 5);
    print_Line(NULL, 0);
    while(get_KeyPress(false) != KEY_ENTER);
    { /* Nothing */ }
    return;
}

int screen_Scenery()
{
    int key = 0, nScen = 0;
    COORD pos = {3, 2};
    toggle_Cursor(false);
    nScen = listScen();
    do 
    {
        // pos.X = 3;
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
    print_Line(NULL, 0);
    print_Line(" ", 0);
    
    // Scenario Name
    snprintf(word, sizeof(word), "%c", 218);
    for(int i = 1; i < strlen(scn_Name)+3; i++)
        word[i] = 196;
    word[strlen(scn_Name)+3] = 191;
    print_Line(word, 0);
    snprintf(word, sizeof(word), "%c %s %c", 179, scn_Name, 179);
    print_Line(word, 0);
    for(int i = 1; i < strlen(scn_Name)+3; i++)
        word[i] = 196;
    word[0] = 192; word[strlen(scn_Name)+3] = 217;
    print_Line(word, 0);
    
    print_Line(" ", 0);
    snprintf(word, sizeof(word), "%c %s %c", 175, map_Name, 174);
    print_Line(word, 0);
    print_Line(" ", 0);
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
    
    for(int i = 0; i < sWidth / 2 - 4; i++)
        printf(" ");
    printf("Jogar");
    printf("\n");
    
    for(int i = 0; i < sWidth / 2 - 4; i++)
        printf(" ");
    printf("Tutorial");
    printf("\n");

    for(int i = 0; i < sWidth / 2 - 4; i++)
        printf(" ");
    printf("Creditos");
    printf("\n");
    
    for(int i = 0; i < sWidth / 2 - 4; i++)
        printf(" ");
    printf("Configurar");
    printf("\n");

    for(int i = 0; i < sWidth / 2 - 4; i++)
        printf(" ");
    printf("Sair");
    printf("\n");
    ShowCursor(FALSE);

    pos.X = (get_ScreenWidth() / 2 - 7), pos.Y = 6;
    do 
    {
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
        printf(">");
        int key = get_KeyPress(false);
        if(key == -32)
        {
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
            printf(" ");
            switch ((int) getch())
            {
            case 72: // Up
                pos.Y--;
                if(pos.Y < 6)
                    pos.Y = 10;
                break;
            case 80: // Down
                pos.Y++;
                if(pos.Y > 10)
                    pos.Y = 6;
                break;
            default:
                break;
            }
        }
        else if(key == KEY_ENTER) break;
    } while(1);
    return (pos.Y - 6);
}

void screen_Victory(B_endStats winner, B_endStats looser)
{
    system("cls");
    fflush(stdin);
    int sWidth = get_ScreenWidth();
    int sHeight = get_ScreenHeight();
    int auxHeight = 0, msg_len = 0;
    char msg[STRING_NAME];
    int aux1 = 0, aux2 = 0;
    switch ((int) sWidth % 4)
    {
    case 3:
        aux1++;
        break;
    case 1:
        aux2++;
        break;
    }
    
    // printf("           Winner X Looser\n");
    // printf("Deployed:  %6d   %6d\n", winner.deployed, looser.deployed);
    // printf("Killed:    %6d   %6d\n", winner.killed, looser.killed);
    // printf("Lost:      %6d   %6d\n", winner.loss, looser.loss);
    // printf("Remaining: %6d   %6d\n", winner.deployed - winner.loss, looser.deployed - looser.loss);

    
    //printf("#");
    //for(int i = 0; i < sWidth - 2; i++)
    //    printf("-");
    //printf("#\n");

    //printf("|");
    //for(int i = 0; i < sWidth - 2; i++)
    //    printf(" ");
    //printf("|\n");

    // printf("|");
    // for(int i = 0; i < (sWidth / 2) - floorf((strlen(winner.name) / 2.0f)) - 11; i++)
    //     printf(" ");
    // printf("%s has won the battle!", winner.name);
    // for(int i = 0; i < (sWidth / 2) - ceilf((strlen(winner.name) / 2.0f)) - 11; i++)
    //     printf(" ");
    // printf("|\n");

    snprintf(msg, sizeof(msg), "%s has won the battle!", winner.name);
    print_Line(NULL, 0);
    print_Line(" ", 0);
    print_Line(msg, 0);
    for(int i = 0; i < ceilf((sHeight - 13) / 2.0f); i++)
    {
        auxHeight++;
        print_Line(" ", 0);
        // printf("|");
        // for(int j = 0; j < sWidth - 2; j++)
        //     printf(" ");
        // printf("|\n");
    }
    
    // Winner X Looser
    // Name
    msg_len = strlen(winner.name);
    putchar(186);
    for(int i = 0; i < ceilf(sWidth / 4.0f) - floorf(msg_len / 2.0f) - 1; i++)
        printf(" ");
    printf("%s", winner.name);
    for(int i = 0; i < aux1 + floorf(sWidth / 4.0f) - ceilf(msg_len / 2.0f); i++)
        printf(" ");
    printf("||");
    msg_len = strlen(looser.name);
    for(int i = 0; i < ceilf(sWidth / 4.0f) - floorf(msg_len / 2.0f) - 1; i++)
        printf(" ");
    printf("%s", looser.name);
    for(int i = 0; i < floorf(sWidth /4.0f) - ceilf(msg_len / 2.0f) - 2 - aux2; i++)
        printf(" ");
    putchar(186);
    printf("\n");

    print_Line(" ", 0);

    // Deployed
    int nLen = get_Digits(winner.deployed);
    putchar(186);
    for(int i = 0; i < ceilf(sWidth / 4.0f) - floorf(nLen / 2.0f) - 1; i++)
    { printf(" "); }
    printf("%d", winner.deployed);
    for(int i = 0; i < aux1 + floorf(sWidth / 4.0f) - ceilf(nLen / 2.0f) - 3; i++)
        printf(" ");
    printf("Deployed"); nLen = get_Digits(looser.deployed);
    for(int i = 0; i < ceilf(sWidth / 4.0f) - floorf(nLen / 2.0f) - 4; i++)
        printf(" ");
    printf("%d", looser.deployed);
    for(int i = 0; i < floorf(sWidth / 4.0f) - ceilf(nLen / 2.0f) - 2 - aux2; i++)
        printf(" ");
    putchar(186);
    printf("\n");

    // Killed
    nLen = get_Digits(winner.killed);
    putchar(186);
    for(int i = 0; i < ceilf(sWidth / 4.0f) - floorf(nLen / 2.0f) - 1; i++)
    { printf(" "); }
    printf("%d", winner.killed);
    for(int i = 0; i < aux1 + floorf(sWidth / 4.0f) - ceilf(nLen / 2.0f) - 2; i++)
        printf(" ");
    printf("Killed"); nLen = get_Digits(looser.killed);
    for(int i = 0; i < ceilf(sWidth / 4.0f) - floorf(nLen / 2.0f) - 3; i++)
        printf(" ");
    printf("%d", looser.killed);
    for(int i = 0; i < floorf(sWidth / 4.0f) - ceilf(nLen / 2.0f) - 2 - aux2; i++)
        printf(" ");
    putchar(186);
    printf("\n");

    // Lost
    nLen = get_Digits(winner.loss);
    putchar(186);
    for(int i = 0; i < ceilf(sWidth / 4.0f) - floorf(nLen / 2.0f) - 1; i++)
    { printf(" "); }
    printf("%d", winner.loss);
    for(int i = 0; i < aux1 + floorf(sWidth / 4.0f) - ceilf(nLen / 2.0f) - 1; i++)
        printf(" ");
    printf("Lost"); nLen = get_Digits(looser.loss);
    for(int i = 0; i < ceilf(sWidth / 4.0f) - floorf(nLen / 2.0f) - 2; i++)
        printf(" ");
    printf("%d", looser.loss);
    for(int i = 0; i < floorf(sWidth / 4.0f) - ceilf(nLen / 2.0f) - 2 - aux2; i++)
        printf(" ");
    putchar(186);
    printf("\n");

    // Remaining
    nLen = get_Digits(winner.deployed - winner.loss);
    putchar(186);
    for(int i = 0; i < ceilf(sWidth / 4.0f) - floorf(nLen / 2.0f) - 1; i++)
    { printf(" "); }
    printf("%d", winner.deployed - winner.loss);
    for(int i = 0; i < aux1 + floorf(sWidth / 4.0f) - ceilf(nLen / 2.0f) - 3; i++)
        printf(" ");
    printf("Survived"); nLen = get_Digits(looser.deployed - looser.loss);
    for(int i = 0; i < ceilf(sWidth / 4.0f) - floorf(nLen / 2.0f) - 4; i++)
        printf(" ");
    printf("%d", looser.deployed - looser.loss);
    for(int i = 0; i < floorf(sWidth / 4.0f) - ceilf(nLen / 2.0f) - 2 - aux2; i++)
        printf(" ");
    putchar(186);
    printf("\n");

    fillSpace_ToBottom(3);
    print_Line("Press ENTER to continue...", 0);
    print_Line(" ", 0);
    print_Line(NULL, 0);
    
    while (get_KeyPress(false) != KEY_ENTER)
        continue;
    return;
}