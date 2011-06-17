Dichotic Harmony Files Converter
Copyright (c) 2010 Vadim Madgazin
Версия 1.21 от 17 июня 2011 г.

Консольное win32 приложение для взаимной конвертации .daccords и .mid файлов,
а также для их дополнительной обработки - удаления начальной паузы, обрезания
и т.п. - см. ниже параметры строки запуска.

Usage for midi input:  "2h_converter.exe  INFILE.mid"

Usage for daccords input:  "2h_converter.exe  INFILE[.daccords]"

Usage for music generator output:  "2h_converter.exe  GEN"

optional parameters, their defaults for ([midi];[daccords]) and range min/max:

-col    (0;  )  collapse_midi_tracks 0/1 свернуть все midi треки в один трек
-colex  (0;  )  collapse_and_expand_midi_tracks 0/1 свернуть и развернуть
-pause  (1; 1)  use_start_pause of music 0/1 использовать начальную паузу
-clip   (0.; )  clip_music_time seconds 0.=infinite/max double обрезать музыку
-text   (0;  )  save midi_as_text 0/1 записать midi файл как текстовый
-pan    (2; 2)  panorame_precision digits 0/3 точность панорамы звука, дес. знаков
-perc   (0; 1)  use_percussion instruments 0/1 использовать канал ударных инструм.
-trans  (1;  )  optimize_transposition 0/1 оптимизировать общую транспозицию нот
-tlag   (1.; )  accord_time_lag 0./100. интервал одновременности нот, мсек
-tick   ( ;1.)  tick_time_msec midi tick 0.01/1000. точность midi времени, мсек
-rep    ( ; 1)  repeat_upto_number loop music 1/1e6 число повторов музыки
-anum   (1; 1)  add_accord_number text 0/1 добавлять номер аккорда в файл
-acomm  (1; 1)  add_accord_comment text 0/2 формат комментария аккорда
-head   ( ; 0)  add_daccords_header text as seq.specific 0/1 добавлять хедер

-ncol  (0)  notes color for music GEN:  0/1
-m     (7)  M for music GEN:  M <= 10
-n     (7)  N for music GEN:  N <= M
-k     (2)  K for music GEN:  2 <= K < N
-seed  (0)  random seed for music GEN:  -2^31...+2^31

Example:  "2h_converter.exe INFILE.mid -clip 60 -pause 0"

=================================================================================

int converter_mode = 0; // режим работы: 0 input midi файл, 1 input daccords файл

// параметры конвертации в converter_mode режиме: param[converter_mode]

int collapse_midi_tracks[2] = {0,0}; // 1 = сворачивать все midi треки в 0-й трек; [1] не используется
int collapse_and_expand_midi_tracks[2] = {0,0}; // 1 = сворачивать треки в 0 и разворачивать в 0-16; [1] не используется

int use_start_pause[2] = {1,1}; // 1 = оставлять начальную паузу музыки, 0 = удалять
double clip_music_time[2] = {0.,0.}; // время прерывания музыки (сек), если <=0. то неограничено; [1] не используется
int midi_as_text[2] = {0,0}; // 1 = вывод midi файла как текст в .txt файл; [1] не используется

// число дес. знаков после запятой для double панорамы, если 0 то это "целая" панорама -1, 0, +1
int panorame_precision[2] = {2,2}; // обычно 0 или 2, максимум 3.
int use_percussion[2] = {0,1}; // 1 = использовать ноты/канал ударных инструментов
int optimize_transposition[2] = {1,0}; // 1 = оптимизация общего параметра транспозиции нот; [1] не используется

// время в пределах которого последовательные нажатия (или отпускания) нот считаются одновременными
// т.о. весь поток музыкальных событий делится на участки в пределах этого времени и вне его пределов
double accord_time_lag[2] = {1.,0}; // обычно 1-10 msec; [1] не используется
double tick_time_msec[2] = {0,1.}; // 0.01-1000 число миллисекунд в одном midi тике; [0] не используется
int repeat_upto_number[2] = {0,1}; // <=1e6 число повторов входной музыки в выходном файле; [0] не используется
int add_accord_number[2] = {1,1}; // 1 = выводить в файл номер аккорда: в midi без префикса ';', в daccords с ';'

// 0 = не выводить в файл комментарий аккорда, 1 = выводить стандартно (через пробел), 2 = выводить нестандартно:
// в midi без пробела после выведенного номера аккорда, в daccords с префиксом ';' при отсутствии номера аккорда
// т.о. можно приклеить номер исх. аккорда к комментарию или использовать номер из комментария как номер аккорда
int add_accord_comment[2] = {1,1};
int add_daccords_header[2] = {0,0}; // 1 = выводить в файл daccords хедер как SEQUENCER_SPECIFIC; [0] не используется

