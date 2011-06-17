
#pragma once

bool ParseArgs(int argc, char *argv[]);
void PrintHelp(const wchar_t *title);
void MusicGenerator();

const int MAXM = 10;
struct MusGen // параметры генератора музыки
{
int notes0[MAXM]; // набор разрешённых нот музыки - чёрные клавиши
int notes1[MAXM]; // набор разрешённых нот музыки - белые клавиши
int m, n, k; // MAXM >= M >= N > K >= 2; при K > 2 музыка слишком однообразна...
int ind[MAXM];  // индексы начальных N нот музыки
int seed; // случайность
int transposition; // сдвиг 0-й ноты
int notes_num; // длина всей музыки в нотах
int duration_msec; // длительность одной ноты в мсек
};

