#include "stdafx.h"
const wchar_t* ProgramTitle = L"Dichotic Harmony Files Converter";
const wchar_t* ProgramCopyright = L"Copyright (c) 2010 Vadim Madgazin";
const wchar_t* VER_NUM = L"1.10"; // версия от 7 июня 2011 г.
/*
  =Сделано:
  -работает с улучшенной libjdksmidi версии 14!
  -чуток улучшен код jdksmidi_fileread.cpp - см. текст Stephan.Huebler@tu-dresden.de
  -последние версии универсальных файлов скопированы из проекта 2h_accords_generator
  -установлена версия компилера с профайлером - Visual Studio 2005 Team Suite
  -включение и выключение профайлера: Linker / Advanced / Profile 
   но включать лучше так: Tools / Performance Tools / Performance Wizard далее всё просто
   потом на вкладке (в окне) Performance Explorer правой кнопкой на проекте выбираем Launch
   когда профиль готов - смотрим на "нижней" вкладке Functions время Elapsed Inclusive Time
   в миллисекундах - какая функция дольше всего выполнялась...
   чтобы посмотреть старый профайл ещё раз надо "открыть" .psess файл в File / Open
  -для больших файлов значительно увеличена скорость преобразования daccords -> midi:
   профайлер определил, что более 80% времени уходило на операции с wstring и особенно
   с wstring2 строками в функции DaccordsFile::Read(const wchar_t *file), в частности
   на конструирование, копирование и вывод (wstring2 >> в int, double и др. переменные)...
   поэтому самый медленный wstring код был заменён на wchar_t код, а затем везде в этой
   функции вместо wstring2 были использованы wistringstream и wstring, в результате чего
   скорость увеличилась ещё в 2 раза!
  -рекомендации на будущее для критичных по скорости участков кода:
   не использовать wstring2 функции, которые появляются обычно для удобства применения
   операторов ввода/вывода из/в простые переменные >> и <<
   не использовать код с многократным перевыделением памяти для wstring

  =Надо:
  -коммит libjdksmidi
  -новый проект 2h_converter на Github

  =Следующая версия:
  -добавить генератор мелодий: см. тетр. Идеи-7 от 18 ноября 2010 г.

  -опция вариатора midi инструментов: Идеи-7, запись от 31 дек. 2010 г.

  -кросс-платформ. вариант: без мессаг-бокса, только станд. вывод!
  -версия на GCC - minGW

  -рекомендации для 2-го формата daccords файлов:
   в заголовок ввести данные, обычно присутствующие в midi-партитурах (Сибелиус и др.):
   размер такта,
   название иструмента,
   копирайт и т.п.
   параметры времени accord_time_lag и tick_time_msec

   формат строки аккорда (возможно разрешить ему занимать несколько строк до знака ";"):
   вместо длительности (Duration и TempBPS) ввести значение времени (сек/мсек) начала аккорда
   (по флагу в хедере) от начала самого первого или от начала предыдущего аккорда,
   для каждой ноты ввести индивид-й номер инструмента и громкость (скорость нажатия),
   для перкуссионных нот ввести спец. флаг (без № инструмента, т.к. он кодируется номером ноты),
   для 2-х и более одинаковых (мелодических) нот в аккорде (одинаковый номер и инструмент ноты)
   ввести дополнительный "спец. номер ноты" - чтобы отличить какие именно ноты переходят далее
   в следующий аккорд, а какие нет.
   при первом появлении такой пары нот если одна из них была в предыдущем аккорде, то в текущем акк.
   она получает "спец. номер ноты" = 1, а другая (новая) - получает номер 2.
   для укладывания произвольно возникающих во времени нот в прокрустово ложе аккордов - без потерь -
   вводится ещё один параметр: номер стадии "легато" процесса звучания ноты.
   Список всевозможных номеров легато:
   0 = начало и конец (одновременно): нота, которая звучит только в текущем аккорде
   1 = начало: нота не заканчивается с концом текущего аккорда
   2 = продолжение: нота началась ранее и не заканчивается с концом текущего аккорда
   3 = конец: нота началась ранее и заканчивается с концом текущего аккорда.
   По умолчанию у нот "номер легато" = 0.

  -после написания c++midi lib разработать "текстовый midi" формат, годящийся как для текущей
   версии midi файлов (General Midi), так и для настоящих и будущих его расширений.
   вот что там может быть:
   время (абсолютное или от предыдущего события),
   длительность звука ноты
   номер трека (что-то вроде отдельной строки нотоносца)
   номер инструмента (мелодич./перкусс.)
   номер ноты
   скорость нажатия/отпускания
   панорама
   текстовые сообщения
   другие сообщения...
   причём можно сделать чтобы каждое последующее событие имело все параметры предыдущего,
   т.о. надо было бы выводить только отличия событий друг от друга, например вот так
   t 0. p 0. n 44 (первая нота первого аккорда: время, панорама, номер ноты)
   +n +2 +n +4 +n +3 (ещё три ноты с номерами 46, 50, 53, длительность аккорда - до следующего t)
   +t 0.5 n 40 +n +2 +n +3 (2-й аккорд спустя полсекунды ноты 40, 42, 45)

  =Потом:
  -переписать все массивы без класса Ar, через vector
  -упаковать все параметры вызова в одну структуру, которую и передавать в функции...
  -добавить возможность записывать текстовые события как Unicode строки, в том числе для
   мета-события META_SEQUENCER_SPECIFIC, и для последнего добавить manufacturer's ID
  -поместить все файлы проекта в Git, включая бинарный .sln но не .exe!
  -в будущем лучше компилировать итоговый вариант при помощи GCC/MinGW!

  =Важно:
  -рекомендации на будущее для критичных по скорости участков кода:
   не использовать wstring2 функции, которые появляются обычно для удобства применения
   операторов ввода/вывода из/в простые переменные >> и <<
   не использовать код с многократным перевыделением памяти для wstring
  -включение и выключение профайлера: Linker / Advanced / Profile 
   но включать лучше так: Tools / Performance Tools / Performance Wizard далее всё просто
   потом на вкладке (в окне) Performance Explorer правой кнопкой на проекте выбираем Launch
   когда профиль готов - смотрим на "нижней" вкладке Functions время Elapsed Inclusive Time
   в миллисекундах - какая функция дольше всего выполнялась...
   чтобы посмотреть старый профайл ещё раз надо "открыть" .psess файл в File / Open
  -любой файл без расширения ".mid" считается daccords файлом!
  -daccords файл должен быть в Unicode кодировке!
  -текст META_TRACK_NAME в треке 0 Сибелиус использует как заголовок музыкального произведения!
  -текст META_TRACK_NAME в треках 1,2,3 Сибелиус использует как левый заголовок строки нотоносца!
  -текст META_LYRIC_TEXT в треках 1,2,3 Сибелиус использует как подстрочный текст строки нотоносца!
  -баг в Сибелиусе вер. 4:
   если в одном аккорде есть 2 одинаковые ноты (одновременные, но с разной панорамой), то они
   интерпретируются неверно из midi файла формата 1 когда обе ноты расположены в одном треке -
   вместо двух нот нормальной длительности появляется одна минимально возможной длительности и
   пауза! но то же самое из midi файла с форматом 0 читается нормально!
   если же панорама одинаковая, то и из формата 0 одинаковые ноты дают неверный аккорд, одна
   нота нормальная, а другая - минимальной длительности+пауза!
  -баг в Сибелиусе вер. 4: midi файлы с небольшим числом нот (примерно <= 5) не открываются!
*/

bool TEST = 0; // 1 = режим для профилирования кода на тестовом файле
int testmode = 1;
wstring testfilew = L"crash_converter.mid.daccords";

int converter_mode = 0; // режим работы: 0 input midi файл, 1 input daccords файл
static int& MODE = converter_mode;

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

wstring2 infilew, outfilew; // имя входного, выходного файлов

HWND MboxHWND = 0;
const wchar_t *MboxTitle = 0;
int WINAPI MyMsgBoxFun(HWND, LPCWSTR text, LPCWSTR, UINT)
{
  return MessageBoxW( MboxHWND, text, MboxTitle, MB_OK ); // модальный мессаг-бокс над окном MboxHWND
}

void wmain(int argc, wchar_t *argv[])
{
  // получаем хендл и дескриптор окна консоли
  HWND chwnd = GetConsoleWindow();
  HINSTANCE chinst = GetModuleHandle(0);

  MboxHWND = chwnd; // хендл родительского окна для Mbox() - окон
  MboxTitle = ProgramTitle; // их титульная строка
  // изменяем указатель на мою функцию мессаг-бокса, которая знает хендл текущего активного диалог бокса
  MsgBoxFun = MyMsgBoxFun;

  wstring2 bigtitle(ProgramTitle);
  bigtitle += L" ver ";
  bigtitle << VER_NUM << "\n" << ProgramCopyright;

  if (TEST) // тестовый режим
  {
    converter_mode = testmode;
    infilew = testfilew;
  }
  else // обычный режим
  {
    // разбираемся с аргументами
    if ( !ParseArgs(argc, argv) )
    {
      PrintHelp(bigtitle);
      return;
    }
  }

  wcout << endl << bigtitle << endl << endl;

  if ( converter_mode == 0 ) // input midi
  {
    // читаем midi файл

    MidiFile mfile;
    if ( !mfile.Read(infilew) )
    {
      Mbox(L"Error reading or processing midi file", infilew);
      return;
    }

    bool save_midi = false;

    if ( collapse_midi_tracks[MODE] || collapse_and_expand_midi_tracks[MODE] )
    {
      save_midi = true;
      // сворачиваем все треки в 0-й и если надо затем разворачиваем 0-й трек в треки 0-16
      mfile.CollapseAndExpandMultiTrack( 0 != collapse_and_expand_midi_tracks[MODE] );
    }

    if ( midi_as_text[MODE] ) // вывод входного midi файла как текст в .txt файл
    {
      string text = MultiTrackAsText( *mfile.GetMultiTrack() );
      wstring filew = infilew + L".txt";
      // сохраняем текст в файл
      write_bin(filew.c_str(), text.c_str(), text.length(), true);
      return;
    }

    if ( !use_start_pause[MODE] ) // первым делом удаляем начальную паузу музыки
    {
      // однако небольшая пауза в midi может остаться, она удаляется позже в daccord
      save_midi = true;
      mfile.CompressStartPause(!use_percussion[MODE]);
    }

    if (clip_music_time[MODE] > 0.) // только затем обрезаем время конца музыки
    {
      save_midi = true;
      mfile.ClipMultiTrack( clip_music_time[MODE] );
    }

    if (save_midi) // запись скорректированного midi файла на диск
    {
      outfilew = infilew + L".mid";
      infilew = outfilew; // этот выходной миди файл будет новым входным!
      int midi_tracks_number = 0;
      if ( !mfile.Write(outfilew, &midi_tracks_number) )
      {
        Mbox(L"Error writing midi file", outfilew);
      }
      else
      {
        cout << "Music duration in sec = " << GetMisicDurationInSeconds( *mfile.GetMultiTrack() ) << endl;
        wcout << L"OK writing file  " << outfilew << endl;
        cout << "Number of midi tracks is " << midi_tracks_number << endl << endl;
      }
    }

    // создаём daccords файл

    DaccordsFile dfile;
    bool res = dfile.MidiToDaccords(mfile, !use_percussion[MODE], accord_time_lag[MODE], !use_start_pause[MODE]);
    if (!res)
    {
      Mbox(L"Error in MidiToDaccords() converter!");
      return;
    }

    // делаем транспозицию = минимальный номер мелодической ноты
    if ( optimize_transposition[MODE] ) dfile.OptimizeTransposition();

    // write the output daccords file

    outfilew = infilew + L".daccords"; // имя выходного файла
    if ( !dfile.Write(outfilew, panorame_precision[MODE], add_accord_number[MODE], add_accord_comment[MODE]) )
    {
      Mbox(L"Error writing daccords file", outfilew);
    }
    else
    {
      wcout << L"OK writing file  " << outfilew << endl;
      cout << "Number of accords is " << dfile.get_accords_number() << endl;
    }
  }
  else // input daccords
  {
    // читаем daccords файл

    DaccordsFile dfile;
    if ( !dfile.Read(infilew) )
    {
      Mbox(L"Error reading or processing daccords file", infilew,
           L"\nError code", dfile.errors() );
      return;
    }
    // Mbox( dfile.header_comment(), L"Число аккордов в файле:", dfile.arr_accords().elements() );

    if ( !use_start_pause[MODE] ) dfile.DeleteStartPause(); // удаляем начальную паузу музыки

    // создаём midi файл

    MidiFile mfile;
    bool res = mfile.DaccordsToMidi(dfile, tick_time_msec[MODE], add_daccords_header[MODE], repeat_upto_number[MODE],
                     add_accord_number[MODE], add_accord_comment[MODE], panorame_precision[MODE], !use_percussion[MODE]);
    if (!res)
    {
      Mbox(L"Error in DaccordsToMidi() converter!");
      return;
    }

    // write the output midi file

    outfilew = infilew + L".mid"; // имя выходного файла
    int midi_tracks_number = 0;
    if ( !mfile.Write(outfilew, &midi_tracks_number) )
    {
      Mbox(L"Error writing midi file", outfilew);
    }
    else
    {
      wcout << L"OK writing file  " << outfilew << endl;
      cout << "Number of midi tracks is " << midi_tracks_number << endl;
    }
  }
}

bool ParseArgs(int argc, wchar_t *argv[])
{
  if ( argc < 2 ) return false;
  // else argc >= 2

  infilew = argv[1]; // 2-й аргумент всегда имя входного файла без ключа!
  string infile = infilew;

  // определяем расширение файла
  basic_string <char>::size_type  len = infile.length();
  basic_string <char>::size_type  pos = infile.rfind('.');
  if ( pos == (len-4) &&
       'm' == tolower( infile[pos+1] ) &&
       'i' == tolower( infile[pos+2] ) &&
       'd' == tolower( infile[pos+3] ) )
  {
    converter_mode = 0; // input from midi file
  }
  else
  {
    converter_mode = 1; // input from daccords file
  }

  if ( argc < 3 ) return true;
  // else argc >= 3

  // далее идут аргументы с ключами!

  for (int i = 2; i < argc; i += 2)
  {
    int ival = 0;
    double dval = 0.;
    if ( (i+1) < argc )
    {
      ival = _wtoi( argv[i+1] );
      dval = _wtof( argv[i+1] );
    }
    wstring key = argv[i];

    if ( key == L"-col" ) collapse_midi_tracks[MODE] = ival;
    else
    if ( key == L"-colex" ) collapse_and_expand_midi_tracks[MODE] = ival;
    else
    if ( key == L"-pause" ) use_start_pause[MODE] = ival;
    else
    if ( key == L"-clip" )  clip_music_time[MODE] = max(0., dval);
    else
    if ( key == L"-text" )  midi_as_text[MODE] = ival;
    else
    if ( key == L"-pan" )   panorame_precision[MODE] = max(0, ival);
    else
    if ( key == L"-perc" )  use_percussion[MODE] = ival;
    else
    if ( key == L"-trans" ) optimize_transposition[MODE] = ival;
    else
    if ( key == L"-tlag" )  accord_time_lag[MODE] = max(0., dval);
    else
    if ( key == L"-tick" )  tick_time_msec[MODE] = max(0.01, dval);
    else
    if ( key == L"-rep" )   repeat_upto_number[MODE] = (int) min( dval, 1e6 );
    else
    if ( key == L"-anum" )  add_accord_number[MODE] = ival;
    else
    if ( key == L"-acomm" ) add_accord_comment[MODE] = ival;
    else
    if ( key == L"-head" )  add_daccords_header[MODE] = ival;
    else
    {
      wcout << L"\a" << endl << L"Warning: ignore unknown key \"" << key << L"\"!" << endl;
    }
  }

  return true;
}

void PrintHelp(const wchar_t *title)
{
  Mbox(
      title,
      L"\n\nMidi input (first default par):"
      L"\n2h_converter.exe  INFILE.mid"
      L"\nor"
      L"\nDaccords input (second default par):"
      L"\n2h_converter.exe  INFILE[.daccords]"
      L"\n\noptional parameters, their default(s) ([first];[second]) and range min/max:\n"
      L"\n-col    (0;  )  collapse_midi_tracks 0/1"
      L"\n-colex  (0;  )  collapse_and_expand_midi_tracks 0/1"
      L"\n-pause  (1; 1)  use_start_pause of music 0/1"
      L"\n-clip   (0.; )  clip_music_time seconds  0.=infinite/max double"
      L"\n-text   (0;  )  save midi_as_text  0/1"
      L"\n-pan    (2; 2)  panorame_precision digits  0/3"
      L"\n-perc   (0; 1)  use_percussion instruments  0/1"
      L"\n-trans  (1;  )  optimize_transposition  0/1"
      L"\n-tlag   (1.; )  accord_time_lag  0./100."
      L"\n-tick   ( ;1.)  tick_time_msec midi tick  0.01/1000."
      L"\n-rep    ( ; 1)  repeat_upto_number loop music  1/1e6"
      L"\n-anum   (1; 1)  add_accord_number text  0/1"
      L"\n-acomm  (1; 1)  add_accord_comment text  0/2"
      L"\n-head   ( ; 0)  add_daccords_header text as seq.specific  0/1"
      L"\n\nExample:"
      L"\n2h_converter.exe INFILE.mid -clip 60 -pause 0",
      UNI_SPACE
      );
}

