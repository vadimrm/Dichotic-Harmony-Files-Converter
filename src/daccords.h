/*
*/

#pragma once

struct ChainHeader // общие параметры цепочки аккордов DichoticAccord
{
  int transposition; // абсолютный миди номер относительной ноты, равной 0
  double chain_speed; // скорость проигрывания секвенции, нормальная = 1.0
  int dont_change_gm_instrument; // если 1, то инструмент не берётся из каждлого аккорда, а
  int timbre_number; // берётся отсюда - общий миди номер инструмента на всю секвенцию
};

struct DichoticNote // одна дихотическая нота
{
  int pause; // если 1, то это не нота, а пауза!
  int note; // относительный номер ноты
  double pan; // панорама: -1 левый, 0 центр, 1 правый край
  int spare1; // "запасное число 1", используется для локальных нужд кода...
};

struct DichoticAccord // структура дихотического аккорда 
{
  // макс. число голосов аккорда, которые поддерживаются данной версией программы
  static const int MAX_ACC_VOICES = 256; // аппаратный максимум = 128 для XG level 3
  static const int COMMLEN = 16;

  int timbre; // тембр, миди номер инструмента
  int duration; // длительность в ударах метронома
  int temp; // параметр темпа, число ударов метронома в секунду
  int voices_number; // количество голосов в аккорде, 1...MAX_ACC_VOICES, если 0 - это пауза!
  wchar_t comment[COMMLEN]; // слово "комментария" к аккорду, завершающееся нулём, создаётся вручную в файле
  int spare1; // "запасное число 1", используется обычно как параметр сортировки...

  // если пауза, то эти параметры могут отсутствовать:
  int volume; // громкость, 1...127
  // в этом массиве должно быть место для максимально возможного числа голосов в аккорде
  DichoticNote dn[MAX_ACC_VOICES]; // дихотические ноты [0]...[voices_num-1] аккорда

  void clear_comment() { comment[0] = UNI_NULL; }

  int set_comment(const wchar_t *src)
  {
    int len = wcslen( src );
    len = min(len, COMMLEN-1);
    wcsncpy(comment, src, len);
    comment[len] = UNI_NULL;
    return len;
  }

  bool ok_comment() { return comment[0] != UNI_NULL; }
  // копируем src аккорд в объект без изменения исходного комментария объекта
  void copy_wo_comment(DichoticAccord &src)
  {
    wstring2 comm( comment );
    *this = src;
    wcsncpy(comment, comm.c_str(), COMMLEN-1);
    comment[COMMLEN-1] = UNI_NULL;
  }
};

class MidiFile;

class DaccordsFile
{
  static const int ADDS = 4; // служебные строки в начале файла (формат, комментарий, общие параметры, аббревиатуры)
  wstring header; // сырой заголовок файла (это ADDS суб-строк, разделённых между собой UNI_CRLF)
  ChainHeader ch; // декодированный заголовок файла
  wstring comment; // комментарий из заголовка файла
  Ar <DichoticAccord> accords; // массив аккордов из файла
  int errs; // ошибки при чтении daccords файла: если 0 то всё в порядке!
  int accords_number; // количество аккордов в массиве accords при конвертации из midi

  // функции добавления midi аккорда и паузы в daccords массив
  void write_accord(int dtms, vector <MIDITimedBigMessage> &accord_events, vector <int> &instr, vector <double> &pan);
public:
  DaccordsFile(const wchar_t *file = 0) // если есть - читаем .daccords файл
  {
    errs = 0;
    if (file != 0) Read(file);
  }

  void DeleteStartPause()
  {
    if (accords_number > 1 && accords[0].voices_number == 0)
    {
      --accords_number;
      accords.delete_element(0, true);
    }
  }

  bool Read(const wchar_t *file); // читаем и декодируем daccords файл
  // запись на диск daccords файла, pan_precision - число дес. знаков после запятой для вывода панорамы
  bool Write(const wchar_t *file, int pan_precision, int add_accord_number, int add_accord_comment) const;

  // преобразователь содержимого прочитанного MidiFile в daccords данные
  bool MidiToDaccords(const MidiFile &mfile, int ignore_percussion, double accord_time_lag, int delete_start_pause);

  void OptimizeTransposition(); // делаем транспозицию = минимальный номер ноты, а последний = 0

  int get_accords_number() const { return accords_number; }
  const Ar <DichoticAccord>& arr_accords() const { return accords; }
  const wstring& header_body() const { return header; }
  const wstring& header_comment() const { return comment; }
  ChainHeader chain_header() const { return ch; }
  int errors() const { return errs; }

  static const wchar_t *daccords_header; // первое слово в начале файла (заголовок)
  static const int dflt_version = 0; // версия формата файла, который воспринимает код

  // упаковка содержимого файла аккордов df в стрим-строку ws
  // pan_precision - число дес. знаков после запятой для вывода панорамы, если 0 то даже точки нет!
  friend void ConvertAccordsToString(const DaccordsFile &df, wostringstream &ws, int pan_precision,
                                     int add_accord_number, int add_accord_comment);
};


class MidiFile
{
  MIDIMultiTrack *tracks; // the object which will hold all the midi tracks

public:
  MidiFile() { tracks = new MIDIMultiTrack(1); }
  ~MidiFile() { delete tracks; }

  bool Read(const wchar_t *file); // читаем и декодируем midi файл

  // сворачиваем все треки в 0-й и если надо затем разворачиваем 0-й трек в треки 0-16
  bool CollapseAndExpandMultiTrack(bool expand)
  {
    MIDIMultiTrack *tracks2 = new MIDIMultiTrack(1);
    if (!tracks || !tracks2)
    {
      delete tracks2;
      return false;
    }

    if (expand) jdksmidi::CollapseAndExpandMultiTrack( *tracks, *tracks2 );
    else        jdksmidi::CollapseMultiTrack( *tracks, *tracks2 );

    delete tracks; // удаляем исходный объект
    tracks = tracks2; // указатель на исходный объект теперь указывает на новый объект в tracks2
    return true;
  }

  bool CompressStartPause(bool ignore_percussion)
  {
    MIDIMultiTrack *tracks2 = new MIDIMultiTrack(1);
    if (!tracks || !tracks2)
    {
      delete tracks2;
      return false;
    }
    int ignore_channel = ignore_percussion? CHANPERC:-1;
    jdksmidi::CompressStartPause( *tracks, *tracks2, ignore_channel );
    delete tracks;
    tracks = tracks2;
    return true;
  }

  bool ClipMultiTrack(double max_time_sec)
  {
    MIDIMultiTrack *tracks2 = new MIDIMultiTrack(1);
    if (!tracks || !tracks2)
    {
      delete tracks2;
      return false;
    }
    jdksmidi::ClipMultiTrack( *tracks, *tracks2, max_time_sec );
    delete tracks;
    tracks = tracks2;
    return true;
  }

  // преобразователь содержимого прочитанного DaccordsFile в MIDIMultiTrack
  bool DaccordsToMidi(const DaccordsFile &dfile, double tick_time_msec, int add_daccords_header,
    int repeat_upto_number, int add_accord_number, int add_accord_comment, int pan_precision, int ignore_percussion);

  // запись MIDIMultiTrack на диск в midi файл, в *midi_tracks_number возвращается число треков
  bool Write(const wchar_t *file, int *midi_tracks_number=0) const;

  const MIDIMultiTrack * GetMultiTrack() const { return tracks; }

  static double round_pan(double pan)
  {
    // округление pan до 3-х точек: всё что по модулю меньше pan_porog ставим в центр, остальное - по краям панорамы
    const double pan_porog = 0.5;
    if (pan <= -pan_porog) pan = -1.; // лево
    else
    if (pan >= +pan_porog) pan = +1.; // право
    else                   pan =  0.; // центр
    return pan;
  }

  // промежуток времени секунды -> midi тики: справедливо только при tempo = 1e6 !
  MIDIClockTime seconds2ticks(double seconds) const
  {
    return MIDIClockTime( 0.5 + tracks->GetClksPerBeat()*seconds );
  }

  static const uint32 tempo = 1000000; // темп 1 000 000 usec = 1 sec в четверти

  static const int NUMCHANS = 16; // макс. количество миди-каналов, включая канал ударных CHANPERC
  static const int CHANPERC = 9; // номер midi канала ударных инструментов
};

