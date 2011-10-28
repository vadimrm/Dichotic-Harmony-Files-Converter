#include "stdafx.h"
#include "stdafx2.h"
const wchar_t* ProgramTitle = L"Dichotic Harmony Files Converter";
const wchar_t* ProgramCopyright = L"Copyright (c) 2010 Vadim Madgazin";
const wchar_t* VER_NUM = L"1.30"; // версия от 28 октября 2011 г.
/*
  =Сделано:
  -параметры в хелпе идут в порядке процессинга!
  -новые умолчания для входных midi файлов:
   -точность панорамы = 0 (остаётся только целая часть)
   -удаляется начальная пауза музыки
  -"инвертированы" параметры:
    use_start_pause -> delete_start_pause
    use_percussion -> delete_percussion
  -канал ударных удаляется в отдельном midi обработчике
  -теперь пустые комментарии не вставляются в выходные midi файлы
  -в парсер добавлен код для распознавания файлов с расширением ".midi"
  -новый параметр solo_from_midi - конвертация в daccords одного самого высокого голоса,
   все низшие голоса отбрасываются уже на midi уровне (до преобразования в аккорды)
  -переменная save_midi преобразована в параметр программы save_midi_file
  -код синхронизирован с DHAG версией 4.00
  -работает с libjdksmidi новой версии 16:
   -добавил утилиты SoloMelodyConverter() и ClearChannel()
   -нек. др. мелкие улучшения

  =Надо:
  =Следующая версия:
  -кросс-платформ. вариант: без мессаг-бокса, только станд. вывод!
  -засунуть в мидюки копирайты программы...
  -опция вариатора midi инструментов: Идеи-7, запись от 31 дек. 2010 г.

  -можно попробовать переделать одноголосную музыку в многоголосье - брать аккорды из A >= 2
   подряд идущих нот генератора со сдвигом первой ноты следующего аккорда на B нот от первой
   ноты предыдущего аккорда, где 1 <= B <= A; например - трёхголосье: A = 3, B = 1...3

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

  =Важно:
  -у пауз номер аккорда не копируется из daccords в midi файл, т.к. в последнем пауза не
   является самостоятельным объектом, это просто "пустота" между нотами...
  -после двойного преобразования midi файлы увеличиваются в размерах, это происходит
   из-за принятой модели "1 голос аккорда = 1 миди канал", предназначенной для работы
   с произвольной панорамой каждого голоса
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
  -код загадочно зависает из-за мусора в MIDIMultiTrack::clks_per_beat
  -любой файл без расширения ".mid[i]" считается daccords файлом!
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

int music_gen = 0; // !0 = режим генерации (M,N,K)-фонической музыки
int notes_color = 0; // 0=чёрные клавиши рояля; 1=белые
MusGen mg =
{
  {  1,3,  6,8,10,    13, 15, 18,20,22 }, // набор номеров нот музыки - чёрные клавиши рояля
  { 0,2,4,5,7,9, 11,12, 14, 16 }, // --//-- белые клавиши рояля
  // 10 >= M >= N > K >= 2; при K > 2 музыка слишком однообразна...
           5,   4,  2, // это неплохо для чёрных клавиш
  //       7,   5,  2, // а это - для белых...
  // индексы первых N нот музыки: от них тоже кое-что зависит, поэтому лучше бы читать их из файла...
  { 0,1,2,3,4,5,6,7,8,9 },
    0, // seed случайного генератора - можно менять из параметров вызова!
   50, // сдвиг нот: нота номер 0 становится этой миди нотой
  300, // длина генерируемой музыки в нотах
  333, // длительность одной ноты в мсек
};

bool TEST = 0; // 1 = режим для работы кода на тестовом файле
int testmode = 0; // converter_mode для теста
wstring testfilew = L"test.mid";

int converter_mode = 0; // режим работы: 0 input midi файл, 1 input daccords файл
static int& MODE = converter_mode;

// параметры конвертации в converter_mode режиме: param[converter_mode]

int separate_midi_track_number[2] = {-1,0}; // >=0 конвертировать только этот трек, -1 все треки; [1] не используется
int delete_percussion[2] = {1,0}; // 1 удалить все ноты (из канала) ударных инструментов

int collapse_midi_tracks[2] = {0,0}; // 1 сворачивать все midi треки в 0-й трек; [1] не используется
int collapse_and_expand_midi_tracks[2] = {0,0}; // 1 сворачивать треки в 0 и разворачивать в 0-16; [1] не используется

int delete_start_pause[2] = {1,0}; // 1 удалять начальную паузу музыки
double clip_music_time[2] = {0.,0.}; // время прерывания музыки (сек), если <=0. то неограничено; [1] не используется

int solo_from_midi[2] = {0,0}; // 1 конвертировать один (самый высокий) голос; [1] не используется
int save_midi_file[2] = {0,0}; // 1 запись (скорректированного) midi файла на диск; [1] не используется
int midi_as_text[2] = {0,0}; // 1 вывод midi файла как текст в .txt файл; [1] не используется

int optimize_transposition[2] = {1,0}; // 1 оптимизация общего параметра транспозиции нот; [1] не используется
// число дес. знаков после запятой для double панорамы, если 0 то это "целая" панорама -1, 0, +1
int panorame_precision[2] = {0,2}; // обычно 0 или 2, максимум 3.

// время в пределах которого последовательные нажатия (или отпускания) нот считаются одновременными
// т.о. весь поток музыкальных событий делится на участки в пределах этого времени и вне его пределов
double accord_time_lag[2] = {1.,0}; // обычно 1-10 msec; [1] не используется
double tick_time_msec[2] = {0,1.}; // 0.01-1000 число миллисекунд в одном midi тике; [0] не используется
int repeat_upto_number[2] = {0,1}; // <=1e6 число повторов входной музыки в выходном файле; [0] не используется
int add_accord_number[2] = {1,0}; // 1 выводить в файл номер аккорда: в midi без префикса ';', в daccords с ';'

// 0 не выводить в файл комментарий аккорда, 1 выводить стандартно (через пробел), 2 выводить нестандартно:
// в midi без пробела после выведенного номера аккорда, в daccords с префиксом ';' при отсутствии номера аккорда
// т.о. можно приклеить номер исх. аккорда к комментарию или использовать номер из комментария как номер аккорда
int add_accord_comment[2] = {1,1};
int add_daccords_header[2] = {0,0}; // 1 выводить в файл daccords хедер как SEQUENCER_SPECIFIC; [0] не используется

wstring2 infilew, outfilew; // имя входного, выходного файлов

HWND MboxHWND = 0;
const wchar_t *MboxTitle = 0;
int WINAPI MyMsgBoxFun(HWND, LPCWSTR text, LPCWSTR, UINT)
{
  return MessageBoxW( MboxHWND, text, MboxTitle, MB_OK ); // модальный мессаг-бокс над окном MboxHWND
}

// void wmain(int argc, wchar_t *argv[]) // VS (Visual Studio)
int main(int argc, char *argv[]) // @GW (MinGW) не может сделать широкую версию main!
{
  // получаем хендл и дескриптор окна консоли
  HWND chwnd = GetConsoleWindow();
  // HINSTANCE chinst = GetModuleHandle(0);

  MboxHWND = chwnd; // хендл родительского окна для Mbox() - окон
  MboxTitle = ProgramTitle; // их титульная строка
  // изменяем указатель на мою функцию мессаг-бокса, которая знает хендл текущего активного диалог бокса
  MsgBoxFun = MyMsgBoxFun;

  wstring2 bigtitle(ProgramTitle);
  bigtitle += L" ver ";
  bigtitle << VER_NUM << L"\n" << ProgramCopyright;

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
      return 0;
    }
  }

  wcout << endl << bigtitle << endl << endl;

  if ( music_gen )
  {
    MusicGenerator();
    return 0;
  }

  if ( converter_mode == 0 ) // input midi
  {
    if ( collapse_midi_tracks[MODE] || collapse_and_expand_midi_tracks[MODE]) save_midi_file[MODE] = 1;
    // одноголосное преобразование улучшается если до того свернуть все треки в один
    if (solo_from_midi[MODE]) collapse_midi_tracks[MODE] = 1;

    // читаем midi файл

    MidiFile mfile;
    if ( !mfile.Read(infilew) )
    {
      Mbox(L"Error reading or processing midi file", infilew);
      return 0;
    }

    int add_track = -1; // добавить этот номер к имени файла если он >= 0

    if ( separate_midi_track_number[MODE] >= 0 )
    {
      // оставляем только нужный трек, остальные удаляем
      if ( !mfile.SeparateTrack( separate_midi_track_number[MODE] ) ) return 0;
      add_track = separate_midi_track_number[MODE];
    }

    if ( delete_percussion[MODE] ) // удаляем всё в канале ударных
    {
      mfile.ClearChannel( MidiFile::CHANPERC );
    }

    if ( collapse_midi_tracks[MODE] || collapse_and_expand_midi_tracks[MODE] )
    {
      // сворачиваем все треки в 0-й и если надо затем разворачиваем 0-й трек в треки 0-16
      mfile.CollapseAndExpandMultiTrack( 0 != collapse_and_expand_midi_tracks[MODE] );
    }

    if (delete_start_pause[MODE] ) // удаляем начальную паузу музыки
    {
      // однако небольшая пауза в midi может остаться, она удаляется позже в daccord
      mfile.CompressStartPause();
    }

    if (clip_music_time[MODE] > 0.) // обрезаем время конца музыки
    {
      mfile.ClipMultiTrack( clip_music_time[MODE] );
    }

    if (solo_from_midi[MODE]) // отбрасываем все ноты кроме самой высокой
    {
      mfile.SoloMelodyConverter();
    }

    if (save_midi_file[MODE]) // запись (скорректированного) midi файла на диск
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

    if ( midi_as_text[MODE] ) // вывод входного midi файла как текст в .txt файл
    {
      string text = MultiTrackAsText( *mfile.GetMultiTrack() );
      wstring filew = infilew + L".txt";
      // сохраняем текст в файл
      write_bin(filew.c_str(), text.c_str(), text.length(), true);
      return 0;
    }

    // создаём daccords файл

    DaccordsFile dfile;
    bool res = dfile.MidiToDaccords(mfile, 0, accord_time_lag[MODE], delete_start_pause[MODE]);
    if (!res)
    {
      Mbox(L"Error in MidiToDaccords() converter!");
      return 0;
    }

    // отбрасываем все ноты кроме самой высокой - только в первом аккорде (постфактум-устранение бага)
    if (solo_from_midi[MODE]) dfile.SoloMelodyConverter();

    // делаем транспозицию = минимальный номер мелодической ноты
    if ( optimize_transposition[MODE] ) dfile.OptimizeTransposition();

    // write the output daccords file

    outfilew = infilew; // имя выходного файла
    if (add_track >= 0) outfilew << L".track" << add_track;
    outfilew += L".daccords";
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
      return 0;
    }
    // Mbox( dfile.header_comment(), L"Число аккордов в файле:", dfile.arr_accords().elements() );

    if ( delete_start_pause[MODE] ) dfile.DeleteStartPause(); // удаляем начальную паузу музыки

    // создаём midi файл

    MidiFile mfile;
    bool res = mfile.DaccordsToMidi(dfile, tick_time_msec[MODE], add_daccords_header[MODE], repeat_upto_number[MODE],
                     add_accord_number[MODE], add_accord_comment[MODE], panorame_precision[MODE], delete_percussion[MODE]);
    if (!res)
    {
      Mbox(L"Error in DaccordsToMidi() converter!");
      return 0;
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

  return 0;
}

bool ParseArgs(int argc, char *argv[])
{
  if ( argc < 2 ) return false;
  // else argc >= 2

  infilew = argv[1]; // 2-й аргумент всегда имя входного файла или GEN
  string infile = infilew;

  // определяем расширение файла: все кроме .mid или .midi будут daccords
  converter_mode = 1; // input from daccords file
  basic_string <char>::size_type  len = infile.length();
  basic_string <char>::size_type  pos = infile.rfind('.');
  if ( pos == (len-4) &&
       'm' == tolower( infile[pos+1] ) &&
       'i' == tolower( infile[pos+2] ) &&
       'd' == tolower( infile[pos+3] ) ) converter_mode = 0; // input from midi file
  if ( pos == (len-5) &&
       'm' == tolower( infile[pos+1] ) &&
       'i' == tolower( infile[pos+2] ) &&
       'd' == tolower( infile[pos+3] ) &&
       'i' == tolower( infile[pos+4] ) ) converter_mode = 0; // input from midi file

  if ( infile == "GEN" ) music_gen = 1;

  if ( argc < 3 ) return true;
  // else argc >= 3

  // далее идут аргументы с ключами!

  for (int i = 2; i < argc; i += 2)
  {
    int ival = 0;
    double dval = 0.;
    if ( (i+1) < argc )
    {
      ival = atoi( argv[i+1] );
      dval = atof( argv[i+1] );
    }
    string key = argv[i];

         if ( key == "-col" )   collapse_midi_tracks[MODE] = ival;
    else if ( key == "-colex" ) collapse_and_expand_midi_tracks[MODE] = ival;
    else if ( key == "-sep" )   separate_midi_track_number[MODE] = ival;
    else if ( key == "-solo" )  solo_from_midi[MODE] = ival;
    else if ( key == "-save" )  save_midi_file[MODE] = ival;
    else if ( key == "-wopause" ) delete_start_pause[MODE] = ival;
    else if ( key == "-clip" )  clip_music_time[MODE] = max(0., dval);
    else if ( key == "-text" )  midi_as_text[MODE] = ival;
    else if ( key == "-pan" )   panorame_precision[MODE] = max(0, ival);
    else if ( key == "-woperc" ) delete_percussion[MODE] = ival;
    else if ( key == "-trans" ) optimize_transposition[MODE] = ival;
    else if ( key == "-tlag" )  accord_time_lag[MODE] = max(0., dval);
    else if ( key == "-tick" )  tick_time_msec[MODE] = max(0.01, dval);
    else if ( key == "-rep" )   repeat_upto_number[MODE] = (int) min( dval, 1e6 );
    else if ( key == "-anum" )  add_accord_number[MODE] = ival;
    else if ( key == "-acomm" ) add_accord_comment[MODE] = ival;
    else if ( key == "-head" )  add_daccords_header[MODE] = ival;

    else if ( key == "-ncol" )  notes_color = ival;
    else if ( key == "-m" )  mg.m = ival;
    else if ( key == "-n" )  mg.n = ival;
    else if ( key == "-k" )  mg.k = ival;
    else if ( key == "-seed" )  mg.seed = ival;
    else
    {
      cout << "\a" << endl << "Warning: ignore unknown key \"" << key << "\"!" << endl;
    }
  }

  return true;
}

void PrintHelp(const wchar_t *title)
{
  Mbox(
      title,
      L"\n\nUsage for midi input:  2h_converter.exe  INFILE.mid"
      L"\n\nUsage for daccords input:  2h_converter.exe  INFILE[.daccords]"
      L"\n\nUsage for music generator output:  2h_converter.exe  GEN"
      L"\n\noptional parameters, their defaults for ([midi];[daccords]) and range min/max:"
      L"\n"
      L"\n-sep    (-1;  )  separate_midi_track_number -1,>=0"
      L"\n-woperc  (1; 0)  delete_percussion instruments 0/1"
      L"\n-col     (0;  )  collapse_midi_tracks 0/1"
      L"\n-colex   (0;  )  collapse_and_expand_midi_tracks 0/1"
      L"\n-wopause (1; 0)  delete_start_pause 0/1"
      L"\n-clip    (0.; )  clip_music_time seconds 0.=infinite/max double"
      L"\n-solo    (0;  )  solo_from_midi 0/1"
      L"\n-save    (0;  )  save_midi_file 0/1"
      L"\n-text    (0;  )  save midi_as_text 0/1"
      L"\n"
      L"\n-trans   (1;  )  optimize_transposition 0/1"
      L"\n-pan     (0; 2)  panorame_precision digits 0/3"
      L"\n-tlag    (1.; )  accord_time_lag 0./100."
      L"\n-tick    ( ;1.)  tick_time_msec midi tick 0.01/1000."
      L"\n-rep     ( ; 1)  repeat_upto_number loop music 1/1e6"
      L"\n-anum    (1; 0)  add_accord_number text 0/1"
      L"\n-acomm   (1; 1)  add_accord_comment text 0/2"
      L"\n-head    ( ; 0)  add_daccords_header text as seq.specific 0/1"
      L"\n"
      L"\n-ncol (0) notes color for music GEN: 0/1"
      L"\n-m    (5) M for music GEN: M <= 10"
      L"\n-n    (4) N for music GEN: N <= M"
      L"\n-k    (2) K for music GEN: 2 <= K < N"
      L"\n-seed (0) random seed for music GEN: -+2^31"
      L"\n"
      L"\nExample: 2h_converter.exe INFILE.mid -clip 60 -pause 0",
      UNI_SPACE
      );
}

void MusicGenerator()
{
  // корректируем параметры генератора
  mg.m = min(mg.m, MAXM);
  mg.n = min(mg.n, mg.m);
  mg.k = min(mg.k, mg.n-1);
  if (mg.k < 2) // ошибка данных
  {
    Mbox("Error in M, N, K =", mg.m, mg.n, mg.k, UNI_SPACE);
    return;
  }

  // создаём данные для daccords файла
  DaccordsFile df;

  df.comment = L"Music GEN";
  df.ch.chain_speed = 1.0;
  df.ch.dont_change_gm_instrument = 1;
  df.ch.timbre_number = 1;
  df.ch.transposition = mg.transposition;

  // выделяем память под аккорды
  if ( !df.accords.renew(mg.notes_num) ) return; // ошибка

  DichoticAccord acc; // "стандартный" аккорд
  acc.timbre = df.ch.timbre_number;
  acc.temp = 1000;
  acc.duration = mg.duration_msec;
  acc.voices_number = 1;
  acc.clear_comment();
  acc.volume = 100;
  acc.dn[0].pause = 0;
  acc.dn[0].note = 0;
  acc.dn[0].pan = 0;

/*
  -(M,N,K)-фония: см. тетр. Идеи-7 от 18 ноября 2010 г. запись I-3)
   множество разрешённых нот музыки состоит из M разных нот,
   ноты генерятся случайным образом или путём полного перебора вариантов,
   на любом отрезке музыки из N нот ровно K нот должны быть одинаковыми,
   остальные (N-K) нот должны быть разными, отличными и друг от друга и от тех K нот...
   10 >= M >= N > K >= 2; при K > 2 музыка слишком однообразна...

   алгоритм:
   вводим первый N-отрезок вручную (он может быть произвольным!) - см. MusGen::ind[]
   сдвигаемся на 1 ноту, генерим случайную новую ноту и проверяем новый N-отрезок:
   количество разных нот должно быть равно N-K+1, а все остальные K-1 нот должны быть одинаковы
   и равны какой-то одной (любой) ноте из тех разных... если это не так - генерим другую ноту!

   замечание: массив разрешённых нот MusGen::notes0/1[] может иметь повторы нот, которые просто будут
   увеличивать вероятность выпадения этих нот в музыке - т.о. можно подстроиться под определённую
   музыку - напр. как у известных произведений!
*/

  int M = mg.m, N = mg.n, K = mg.k;
  int maxval = M-1; // random number from 0 to maxval, include maxval!
  Random rg;
  rg.set_seed( mg.seed );
  // выбираем чёрные или белые клавиши...
  int *notes = notes_color==0? mg.notes0:mg.notes1;

  for (int n = 0; n < mg.notes_num; ++n)
  {
    ++df.accords_number;
    DichoticAccord &ac = df.accords[n];
    ac = acc;

    if (n < N) // копируем первый N-отрезок из фиксированных данных
    {
      ac.dn[0].note = notes[ mg.ind[n] ];
      continue;
    }

    int loop = 0;
back:
    if (++loop > 1000) // защита от зацикливания
    {
      Mbox("Error in music GEN, infinite loop!");
      break;
    }

    // случайный генератор индекса ноты 0...(M-1)
    int index = rg.get_rand_ex( maxval );
    ac.dn[0].note = notes[index];

    // проверяем новый N-отрезок: от df.accords[n-N+1] до df.accords[n]
    // количество разных нот должно быть равно N-K+1, а все остальные K-1 нот должны быть одинаковы
    // и равны какой-то одной (любой) ноте из тех разных... если это не так - генерим другую ноту!
    vector<int> nmus(N); // N-отрезок музыки
    for (int i = 0; i < N; ++i) nmus[i] = df.accords[i+n-N+1].dn[0].note;
    // анализ делать намного легче если nmus отсортировать в порядке возрастания (или убывания) нот
    sort( nmus.begin(), nmus.end() );
    int eqnum = 0; // количество одинаковых нот
    int eqnote = 0; // сама "одинаковая" нота
    for (int i = 1; i < N; ++i)
    {
      int act = nmus[i]; // текущая нота
      int prev = nmus[i-1]; // предыдущая нота
      if (act != prev) continue;
      // else ноты совпадают
      if (eqnum == 0) // первое совпадение
      {
        eqnote = act; // запоминаем первую совпавшую ноту
        eqnum = 2; // 2 одинаковых ноты - это всегда нормально, т.к. K >= 2
        continue;
      }
      // else не первое совпадение
      // все совпадающие ноты должны быть равны друг другу, а их число <= K
      if (eqnote != act || ++eqnum > K) goto back; // иначе делаем новую ноту ещё раз
    }
    // одинаковых нот на всём N-отрезке должно быть ровно K
    if (eqnum != K) goto back; // иначе делаем новую ноту ещё раз
  }

  // имя выходного файла - делаем из параметров генератора
  wstring2 filew;
  filew << "col" << notes_color << "m" << mg.m << "n" << mg.n << "k" << mg.k << "seed" << mg.seed;
  filew += ".daccords";
  if ( !df.Write(filew, 0, 1, 0) )
  {
    Mbox(L"Error writing daccords file", filew);
  }
  else
  {
    wcout << L"OK writing file  " << filew << endl;
    cout << "Number of accords is " << df.get_accords_number() << endl;
  }
}

