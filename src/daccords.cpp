
#include "stdafx.h"

inline bool SetPan( MIDIMultiTrack *tracks, int track_num, MIDIClockTime ticks, int chan, double dpan )
// dpan = -1 ... 0 ... +1 (лево, центр, право)
{
    MIDITimedBigMessage m;
    m.SetTime( ticks );
    m.SetPan( chan, dpan );
    return tracks->GetTrack( track_num )->PutEvent( m );
}

static const wchar_t *VOICE_PAUSE = L"X"; // маркер паузы голоса в файле аккорда
static const wchar_t *ACC_NUM_PREFIX = L";"; // префикс порядкового номера аккорда в файле аккорда

void ConvertAccordsToString(const DaccordsFile &df, wostringstream &ws, int pan_precision,
                            int add_accord_number, int add_accord_comment)
// упаковка содержимого файла аккордов df в стрим-строку ws
{
  // меняем точность и формат вывода чисел с пл. зап. для Chain Speed
  ws.precision(3);
  ws.setf(ios_base::fixed, ios_base::floatfield);

  // в самом начале файла - маркер Unicode текста, затем 1-я строка - номер версии формата файла
  ws << UNI_HDR << df.daccords_header << UNI_SPACE
                << df.dflt_version
                << L"  Commentary:" << UNI_CRLF;

  // затем 2-я строка - комментарий
  ws << df.comment << UNI_CRLF;

  // затем 3-я строка - названия и значения общих параметров секвенции аккордов
  ws << L"Transposition " << df.ch.transposition
     << L"  Chain_Speed " << df.ch.chain_speed
     << L"  Dont_Change_GM_Instrument " << df.ch.dont_change_gm_instrument
     << L"  Instrument_Number " << df.ch.timbre_number << UNI_CRLF;

  // затем 4-я строка - аббревиатуры числовых параметров аккорда
  ws << L"Instrum" << UNI_TAB << L"Durat-n" << UNI_TAB << L"TempBPS" << UNI_TAB << L"Voices"
     << UNI_TAB << L"Volume" << UNI_TAB << L"note pan" << UNI_CRLF;

  ws.precision(pan_precision); // меняем точность вывода чисел с пл. зап. для панорамы

  // затем начиная с 5-й строки идут числовые параметры каждого аккорда секвенции
  for (int n = 0; n < df.accords_number; ++n)
  {
    DichoticAccord &acc = df.accords[n];
    ws << acc.timbre << UNI_TAB << acc.duration << UNI_TAB << acc.temp << UNI_TAB << acc.voices_number;
    // если это не пауза аккорда - выводим все ноты/панорамы аккорда
    if (acc.voices_number > 0)
    {
      ws << UNI_TAB << acc.volume << UNI_TAB;
      for (int i=0; i < acc.voices_number; ++i)
      {
        if ( acc.dn[i].pause ) // если это пауза голоса
        {
          ws << UNI_SPACE << VOICE_PAUSE;
          continue;
        }
        // else это нота
        int nt = acc.dn[i].note;
        double p = acc.dn[i].pan;
        ws << UNI_SPACE << UNI_SPACE;
        if (10 > nt && nt >= 0) ws << UNI_SPACE;
        ws << nt << UNI_SPACE;
        ws << showpos << p << noshowpos; // показываем знак + только для панорамы
      }
    }

    if ( add_accord_number ) // выводить номер?
    {
      // после конца аккорда добавляем пробел, маркер и номер аккорда, номера начинаются с 1!
      ws << UNI_SPACE << ACC_NUM_PREFIX << (n+1);
    }

    if ( add_accord_comment == 1 ) // выводить комментарий аккорда (стандартно)?
    {
      // если есть непустой комментарий - добавляем его через пробел
      if ( acc.ok_comment() ) ws << UNI_SPACE << acc.comment;
    }
    else
    if ( add_accord_comment == 2 ) // выводить комментарий аккорда нестандартно?
    {
      if ( acc.ok_comment() )
      {
        // при отсутствии вывода номера аккорда выводить сначала префикс ';'
        if ( add_accord_number ) ws << UNI_SPACE << acc.comment;
        else                     ws << UNI_SPACE << ACC_NUM_PREFIX << acc.comment;
      }
    }

    ws << UNI_CRLF;
  }
}

// ========================== class ===========================

const wchar_t *DaccordsFile::daccords_header = L"Dichotic_Accords_File_Format_Version";

bool DaccordsFile::Read(const wchar_t *file)
{
  errs = 0;

  // определяем длину файла в байтах и выделяем буфер для чтения файла
  int file_bytes = get_file_length(file);
  Ar <int8> buf(file_bytes+2); // в конце на всяк. случ. дополнительное место для пары байт!
  int8 *file_buf = buf.memory();

  // читаем файл
  int bytes_read = read_bin(file, file_buf, file_bytes);
  if (bytes_read != file_bytes)
  {
    errs |= 1; // ошибка: сбой при чтении файла
    return false;
  }
  // после конца прочитанных данных добавляем пару нулевых байт - метка конца широкой строки!
  file_buf[bytes_read] = file_buf[bytes_read+1] = 0;

  // определяем примерное число строк в файле, считая что 1 строка занимает минимум 40 байт
  // (это 20 w-литер), хотя обычно бывает раза в 2-3 больше...    т.е. определяем с запасом!
  int max_file_strings = file_bytes / 40;

  // проверяем что прочлось и если ок - используем информацию по назначению

  // содержимое файла в виде широкой строки текста, Unicode заголовок опускаем
  wchar_t *fstr = (wchar_t *)(file_buf+2);
  wchar_t *fstr_end = (wchar_t *)(file_buf+bytes_read); // первый символ за концом строки fstr

  // проверяем наличие Unicode заголовка
  if ( *(fstr-1) != UNI_HDR )
  {
    errs |= 2; // ошибка: не Unicode файл
    return false;
  }

  // проверяем наличие daccords заголовка
  if ( wcsncmp( fstr, daccords_header, wcslen(daccords_header) ) != 0 )
  {
    errs |= 4; // ошибка: неверный daccords header
    return false;
  }

  // заменяем все концы строк UNI_CRLF на нули, попутно расставляя указатели на начало строк

  // указатели на начало строк (до очередного символа CR/LF)
  Ar <wchar_t *> pstr(max_file_strings);

  int i;
  for (i = 0; i <= max_file_strings; ++i)
  {
    if (i == max_file_strings) // количество строк превысило максимум
    {
      max_file_strings += 500; // было 100
      pstr.expand_to_nums(max_file_strings);
    }
    pstr[i] = fstr;

/*
    // этот код выполняется _очень_ медленно (преобразование больших файлов замедляется в 10 раз)!
    wstring ws(fstr); // копирование огромного wchar_t массива в новую wstring строку
    size_t n = ws.find(UNI_CRLF);
    if (n == npos) break; // конец строки не найден: это последняя строка
    // else найден, записываем туда нули и сдвигаем указатель
    fstr[n] = fstr[n+1] = UNI_NULL;
    fstr = fstr + n + 2;
*/
    // а этот эквивалентный код выполняется намного быстрее
    wchar_t *endstr = wcschr(fstr, *UNI_CRLF);
    if (endstr == 0) break; // конец строки не найден: это последняя строка
    // else найден, записываем туда нули и сдвигаем указатель
    *endstr = UNI_NULL;
    fstr = endstr + 2;

    if (fstr >= fstr_end) break; // достигнут конец содержимого файла
    // else ещё есть информация
  }

  // pstr[ADDS] это 1-й аккорд, определяем полное количество аккордов в файле
  int num_accords = i-ADDS+1;

  // восстанавливаем "сырой" заголовок из ADDS субстрок
  header.clear();
  for (int n = 0; n < ADDS; ++n)
  {
    header += pstr[n];
    if ( n != (ADDS-1) ) header += UNI_CRLF;
  }

  int version; // версия формата, прочитанная из файла, должна совпадать с dflt_version!

  // 1-я строка: вводим и проверяем заголовок, версию формата и число аккордов
  wstring header, str;
  wistringstream istr(pstr[0]);

  istr >> header >> version;
  if ( wcscmp(header.c_str(), daccords_header) != 0 // проверяем ещё раз: сразу за хедером должен быть пробел!
       || version != dflt_version
       || num_accords <= 0
       ) // неверный заголовок, номер версии формата или нет ни одного аккорда...
  {
    errs |= 8; // ошибка: неверный формат файла
    return false;
  }

  comment = pstr[1]; // 2-я строка: комментарий из заголовка файла!

  // 3-я строка: вводим общие параметры секвенции аккордов
  wistringstream istr2(pstr[2]);
  istr2 >> str >> ch.transposition >> str >> ch.chain_speed >>
           str >> ch.dont_change_gm_instrument >> str >> ch.timbre_number;

  // 4-я строка: аббревиатуры числовых параметров аккорда (не используем)

  // следующая строка: вводим параметры 1-го и каждого следующего аккорда секвенции
  // сначала делаем массив аккордов нужного размера (без стирания элементов)
  accords.renew(num_accords, false);
  for (i = 0; i < num_accords; ++i)
  {
    wistringstream is(pstr[i+ADDS]);
    DichoticAccord acc;

    is >> acc.timbre >> acc.duration >> acc.temp >> acc.voices_number;
    // приводим число голосов к рабочему диапазону данной версии программы
    mintestmax(0, acc.voices_number, DichoticAccord::MAX_ACC_VOICES);

    // если это не пауза - вводим все ноты/панорамы аккорда
    if (acc.voices_number > 0)
    {
      is >> acc.volume;
      for (int i=0; i < acc.voices_number; ++i)
      {
        // проверяем, нет ли паузы (литера X) вместо ноты голоса i
        wstring s2;
        is >> s2;
        if ( s2.compare(VOICE_PAUSE) == 0 )
        {
          acc.dn[i].pause = 1;
          continue;
        }
        acc.dn[i].pause = 0;
        wistringstream is2(s2);
        is2 >> acc.dn[i].note;
        is >> acc.dn[i].pan;
      }
    }

    // вводим комментарий аккорда, пропуская возможно отсутствующий номер аккорда с префиксом ACC_NUM_PREFIX
    wstring s3, comm;
    // вводим в строку s3 одно слово, следующее за концом аккорда 
    is >> s3;
    // если слово в s3 начинается с префикса номера аккорда, то комментарий будет в следующем слове!
    if ( s3.c_str()[0] == *ACC_NUM_PREFIX ) is >> comm; // вводим следующее слово
    else comm = s3; // если нет, то номер аккорда отсутствует, а слово в s3 - это и есть комментарий!

    // копируем comm в комментарий аккорда и добавляем ноль
    wcsncpy(acc.comment, comm.c_str(), DichoticAccord::COMMLEN-1);
    acc.comment[DichoticAccord::COMMLEN-1] = UNI_NULL;

    accords[i] = acc;
  }

  return true;
}

void DaccordsFile::OptimizeTransposition()
// делаем транспозицию = минимальный номер ноты
{
  int minnote = 128;
  int phase = 0; // 0 для поиска минимума, 1 для транспозиции нот

back:
  for (int i = 0; i < accords_number; ++i )
  {
    DichoticAccord &acc = accords[i];
    for (int n = 0; n < acc.voices_number; ++n )
      if (phase == 0)
      {
        int note = acc.dn[n].note;
        // игнорируем отрицательные ноты - ударных инструментов!
        if ( !acc.dn[n].pause && note >= 0 ) minnote = min(minnote, note);
      }
      else
      {
        if ( !acc.dn[n].pause ) acc.dn[n].note -= minnote;
      }
  }
  // если не было мелодических нот - нет и транспозиции
  if (minnote == 128) minnote = 0;

  if (phase == 0 && minnote > 0)
  {
    phase = 1;
    goto back;
  }

  ch.transposition += minnote;
}

void DaccordsFile::write_accord(int dtms, vector <MIDITimedBigMessage> &accord_events,
                                vector <int> &instrument, vector <double> &panorama)
// выводим аккорд и стираем отработанные ненужные события
{
  // локально копируем канальные массивы
  vector <int> instrumen2(instrument);
  vector <double> panoram2(panorama);

  int chann, note, vel, instr = 0;
  double pan;
  wstring comment; // комментарий к аккорду

  // копируем midi данные из стека аккорда accord_events в аккорд acc
  DichoticAccord acc;

  acc.duration = dtms;
  acc.temp = 1000;

  int size = (int) accord_events.size();
  int n = 0;
  int velsum = 0, velnum = 0, maxvel = 0;

  // цикл формирования аккорда из всех нажатых нот от начала стека до маркера end
  for (int i = 0; i < size; ++i)
  {
    MIDITimedBigMessage &ev = accord_events[i];
    if ( ev.IsUserAppMarker() ) // найден маркер end
    {
      ev.SetNoOp(); // стираем маркер и выходим из цикла
      break;
    }

    if ( ev.IsLyricText() ) // текст META_LYRIC_TEXT копируем в комментарий к аккорду
    {
      wstring2 wstr = ev.GetSysExString(); // преобразуем узкую строку в широкую
      comment = wstr;
      ev.SetNoOp(); // стираем событие
    }

    if ( !ev.IsChannelEvent() ) continue;
    // else это канальное событие

    chann = ev.GetChannel(); // номер канала события

    // апдейтим копии канальных массивов
    if ( ev.IsProgramChange() ) instrumen2[chann] = ev.GetPGValue(); // изменение инструмента в канале
    else if ( ev.IsPanChange() ) panoram2[chann] = ev.GetPan(); // изменение панорамы в канале
    else if ( ev.ImplicitIsNoteOn() ) // нажатие ноты
    {
      pan = panoram2[chann];
      note = ev.GetNote();
      vel = ev.GetVelocity();

      velsum += vel;
      ++velnum;
      // запоминаем инструмент самой громкой мелодической ноты аккорда
      if ( chann != MidiFile::CHANPERC )
      {
        if (maxvel < vel)
        {
          maxvel = vel;
          instr = instrumen2[chann];
        }
      }

      acc.dn[n].pause = 0;
      // меняем знак на "-" у номера ноты канала перкусионных
      acc.dn[n].note = chann==MidiFile::CHANPERC? -note:note;
      acc.dn[n].pan = pan;
      if ( ++n >= DichoticAccord::MAX_ACC_VOICES ) break;
    }
  }
  acc.voices_number = n;

  acc.set_comment( comment.c_str() ); // копируем комментарий в аккорд

  // для аккорда используем среднюю громкость всех нот и инструмент самой громкой мелодической ноты
  acc.volume = (velnum > 0)? (velsum/velnum):0;
  acc.timbre = instr;

  // добавляем аккорд acc к массиву аккордов музыки accords
//  if (dtms > 0) 
  {
    int anum = accords.elements();
    // если больше нет места в массиве - увеличиваем его на 100 аккордов
    if ( accords_number >= anum ) accords.expand_to_nums( anum + 100 );
    accords[accords_number++] = acc;
//    cout << " accords_number " << accords_number << "  dtms " << dtms;
  }
  // нулевая длительность аккорда: так бывает при (dur < 0.5) && (accord_time_lag = 0.0) !
  // или если все ноты перкуссионные, а их канал 10 запрещён - первая пауза будет с dtms=0
  if (dtms <= 0) cout << " accord number " << accords_number << "  dtms " << dtms << endl;

  // после вывода отпущенной ноты все её события должны быть стёрты

  for (int i = size-1; i >= 0; --i) // сканируем стек от конца к началу
  {
    MIDITimedBigMessage &ev = accord_events[i];

    if ( ev.IsAllNotesOff() ) // сброс всех нот в одном канале
    {
      chann = ev.GetChannel(); // номер канала
      ev.SetNoOp(); // стираем это событие и
      for (int j = i-1; j >= 0; --j) // все более ранние нотные события для этого канала
      {
        MIDITimedBigMessage &ev = accord_events[j];
        // находим соотв. нотное событие
        if ( ev.IsNote() && chann == ev.GetChannel() )
        {
          ev.SetNoOp(); // стираем это событие
//          cout << "  del note " << note;
        }
      }
    }
    else
    if ( ev.ImplicitIsNoteOff() ) // отпускание одной ноты
    {
      note = ev.GetNote(); // номер ноты и
      chann = ev.GetChannel(); // номер канала отпущенной ноты

      ev.SetNoOp(); // стираем это событие и
      for (int j = i-1; j >= 0; --j) // все более ранние нотные события для этой note/chann
      {
        MIDITimedBigMessage &ev = accord_events[j];
        // находим соотв. нотное событие
        if ( ev.IsNote() && note == ev.GetNote() && chann == ev.GetChannel() )
        {
          ev.SetNoOp(); // стираем это событие
//          cout << "  del note " << note;
        }
      }
    }
  }
//  cout << endl;

  // события pan/instr до первого нажатия ноты переносим в исходные массивы и стираем
  for (int i = 0; i < size; ++i)
  {
    MIDITimedBigMessage &ev = accord_events[i];

    if ( ev.ImplicitIsNoteOn() ) break; // нажатие ноты

    chann = ev.GetChannel(); // канал события

    if ( ev.IsProgramChange() ) // изменение инструмента в канале
    {
      instrument[chann] = ev.GetPGValue();
      ev.SetNoOp(); // стираем это событие
    }
    else if ( ev.IsPanChange() ) // изменение панорамы в канале
    {
      panorama[chann] = ev.GetPan();
      ev.SetNoOp(); // стираем это событие
    }
  }

  // очищаем стек от всех стёртых событий: это ускоряет конвертацию больших файлов на порядок!
  vector <MIDITimedBigMessage> accord_events2( accord_events );
  accord_events.clear();
  for (int i = 0; i < size; ++i)
  {
    MIDITimedBigMessage &ev = accord_events2[i];
    if ( !ev.IsNoOp() )
    {
      accord_events.push_back( ev );
    }
  }

/*
  // то же самое без копирования во 2-й массив - кажется на 10-20% медленнее
  for (int i = size-1; i >= 0; --i)
  {
    MIDITimedBigMessage &ev = accord_events[i];
    if ( ev.IsNoOp() )
    {
      accord_events.erase( accord_events.begin() + i );
    }
  }
*/
}

bool DaccordsFile::MidiToDaccords(const MidiFile &mfile, int ignore_percussion, double accord_time_lag, int delete_start_pause)
// преобразователь содержимого прочитанного MidiFile в daccords данные
{
  const MIDIMultiTrack *mt = mfile.GetMultiTrack();
  // OutputMultiTrackAsText(*mt, cout); return false; // распечатка содержимого мультитрека

  // определяем количество midi событий во всех треках
  int num_events = mt->GetNumEvents();
  // меняем размер массива аккордов из расчёта 1 событие = 1 аккорд (или пауза)
  accords.renew(num_events, true); // и стираем аккорды!
  // сначала массив без музыки
  accords_number = 0;

  // делаем заголовок аккорда
  ch.transposition = 0;
  ch.chain_speed = 1.0;
  ch.dont_change_gm_instrument = 0; // инструмент будет браться из каждого аккорда
  ch.timbre_number = 0;

  // теперь конвертируем нотные события

  // номер миди инструмента и панорама для каждого midi канала  0...15
  vector <int> instrument(MidiFile::NUMCHANS, 0);
  vector <double> panorama(MidiFile::NUMCHANS, 0.);
  // стек midi событий аккорда (в будущем лучше хранить указатели на члены исходного массива событий)
  vector <MIDITimedBigMessage> accord_events; // тут некие канальные события, UserAppMarker или LyricText
  accord_events.reserve(256); // сразу резервируем объём стека до 256 элементов


  // время начала и конца (аккорда или паузы): первое собитие становится началом, все остальные которые
  // близки к нему (в пределах accord_time_lag) считаются одновременными, первое далёкое от него событие
  // становится концом, все остальные близкие к нему считаются одновременными, первое далёкое от него
  // означает что промежуток (start_time, end_time) надо вывести и передвинуть start_time на end_time
  double start_time = 0., end_time = start_time;
  bool start = true; // начальное состояние - события привязываются к start_time

  MIDISequencer seq(mt);
  seq.GoToZero(); // ставим время на начало мультитрека!
  MIDITimedBigMessage ev; // текущее миди событие
  int ev_track; // номер миди трека
  double event_time = 0; // время события в миллисекундах

  // общая длительность исходной и преобразованной музыки в мсек
  double srcall = 0., dstall = 0.;

  // событие-маркер "конца аккорда", после которого любые нажатия нот не будут выведены в текущем аккорде
  // (они будут выведены в следующем акк.)
  MIDITimedBigMessage end;
  end.SetUserAppMarker();
  bool ignore_percussion_notes = false; // true если было игнорировано нотное сообщение в канале ударных

  while ( seq.GetNextEvent( &ev_track, &ev ) )
  {
    if ( ev.IsEndOfTrack() || ev.IsBeatMarker() ) continue;
    // else любое другое событие, кроме конца трека и служебного маркера секвенцера

    event_time = seq.GetCurrentTimeInMs(); // запоминаем время миди события

    if ( ev_track == 0 && ev.IsTrackName() ) // текст META_TRACK_NAME в треке 0 это комментарий daccords хедера
    {
      wstring2 wstr = ev.GetSysExString(); // преобразуем узкую строку в широкую
      comment = wstr;
    }

    if ( ev.IsLyricText() ) // текст META_LYRIC_TEXT запишется в комментарий к аккорду
    {
      accord_events.push_back( ev ); // записываем событие в стек аккорда
      continue;
    }

    if ( !ev.IsChannelEvent() ) continue;
    // else это канальное событие

    int chan = ev.GetChannel(); // номер миди канала
    if (ignore_percussion && chan == MidiFile::CHANPERC)
    {
      if ( ev.IsNote() ) ignore_percussion_notes = true;
      continue; // игнорируем запрещённый канал ударных
    }
    // else это мелодические каналы

    if ( ev.IsProgramChange() || ev.IsPanChange() ) // изменение инструмента или панорамы в канале
    {
      accord_events.push_back( ev ); // записываем событие в стек аккорда
      continue;
    }

    if ( !ev.IsAllNotesOff() && !ev.IsNote() )
    {
      // остальные канальные события - пока игнорируем... потом можно учесть и aftertouch:
      // изменение давления на клавиши IsChannelPressure() или IsPolyPressure(), а может
      // быть даже кое-что из Controller Events (0xB), напр. Legato Footswitch (0x44)
      continue;
    }
    // else сброс всех нот в канале или индивидуальное нотное событие

    // в начале нотные события привязываем к start_time (=0)
    if (start)
    {
      double dt = event_time - start_time;
      if (dt <= accord_time_lag) // если событие в пределах допустимого от start_time
      {
        accord_events.push_back( ev ); // записываем нотное событие в стек аккорда
      }
      else // событие далеко: конец start фазы!
      {
        // переходим в режим привязки событий к end_time
        accord_events.push_back( end ); // записываем в стек маркер "конца аккорда"
        accord_events.push_back( ev );  // записываем нотное событие в стек аккорда
        // start_time не меняем!
        end_time = event_time; // конец аккорда на этом далёком от начала событии
        start = false;
      }
      continue;
    }
    // else привязываем события к end_time

    double dt = event_time - end_time;
    if (dt <= accord_time_lag) // если событие в пределах допустимого от end_time
    {
      accord_events.push_back( ev ); // записываем нотное событие в стек аккорда
    }
    else // событие далеко: надо выводить аккорд!
    {
      // точная длительность аккорда
      double dur = end_time - start_time;
      // то же округлённо до 1 мсек с корректировкой на погрешность предыдущих аккордов
      int idur = float2int( dur + (srcall - dstall) );
      // апдейтим суммарное время
      srcall += dur;
      dstall += idur;

      // выводим аккорд и стираем "отработанные" события...
      write_accord(idur, accord_events, instrument, panorama);

      // переходим к накоплению событий для следующего аккорда
      accord_events.push_back( end ); // записываем в стек маркер "конца аккорда"
      accord_events.push_back( ev ); // записываем нотное событие в стек аккорда
      start_time = end_time; // начало нового аккорда = конец старого
      end_time = event_time; // конец аккорда на этом далёком от начала событии
    }
  }

  // выводим [пред]последний аккорд:
  // это то, что осталось в стеке после маркера end в последнем вызове write_accord() из цикла
  {
    double dur = end_time - start_time;
    int idur = float2int( dur + (srcall - dstall) );
    srcall += dur;
    dstall += idur;
    write_accord(idur, accord_events, instrument, panorama);
  }

  // выводим заключительную паузу: это промежуток времени с момента отпускания последней ноты до
  // последнего по времени события, которое не является концом трека (от end_time до event_time)
  {
    double dur = event_time - end_time;
    int idur = float2int( dur + (srcall - dstall) );
    srcall += dur;
    dstall += idur;
    if (idur > 0) write_accord(idur, accord_events, instrument, panorama);
  }

  // удаляем начальную паузу
  if (delete_start_pause && accords_number > 1 && accords[0].voices_number == 0)
  {
    double dur_msec = 1e3*accords[0].duration/(ch.chain_speed * (double)accords[0].temp);
    srcall -= dur_msec;
    dstall -= dur_msec;
    --accords_number;
    accords.delete_element(0, true);
  }

  if ( ignore_percussion_notes )
    cout << "ignore_percussion_note = " << boolalpha << ignore_percussion_notes << endl;
  cout << "Music duration in sec = " << (0.001*srcall) << "  +remainder in msec " << (dstall-srcall) << endl;

  return true;
}

bool DaccordsFile::Write(const wchar_t *file, int pan_precision, int add_accord_number, int add_accord_comment) const
{
  // файл на диске будет текстовым в кодировке Unicode
  wostringstream wss;

  ConvertAccordsToString(*this, wss, pan_precision, add_accord_number, add_accord_comment);

  // без этого промежуточного приравнивания дальнейший код работал неверно!
  wstring ws = wss.str();

  // буфер текста и размер буфера в байтах 
  const void *file_buf = ws.c_str();
  int file_bytes = sizeof(wchar_t)*ws.size();

  return write_bin(file, file_buf, file_bytes, true);
}

// ========================== class ===========================

bool MidiFile::Read(const wchar_t *file)
// читаем и декодируем midi файл
{
  wstring2 file2 = file;
  return ReadMidiFile(file2, *tracks);
}

bool MidiFile::Write(const wchar_t *file, int *midi_tracks_number) const
// запись MIDIMultiTrack на диск в midi файл
{
  int tracks_number = tracks->GetNumTracksWithEvents();
  if (midi_tracks_number != 0)
    *midi_tracks_number = tracks_number;
  wstring2 file2 = file;
  return WriteMidiFile(*tracks, file2);
}

bool MidiFile::DaccordsToMidi(const DaccordsFile &dfile, double tick_time_msec, int add_daccords_header,
  int repeat_upto_number, int add_accord_number, int add_accord_comment, int pan_precision, int ignore_percussion)
// преобразователь секвенции аккордов в midi треки, в случае успеха возвращает true
// если pan_precision = 0 то используется только 3 значения панорама с округлением до -1, 0, +1
// если pan_precision > 0 то до 16 любых значений панорамы в одном мелодически-перкуссионном аккорде
{
  // делаем нужное число треков и очищаем их
  int tracks_number = pan_precision == 0? 4:17;
  tracks->ClearAndResize( tracks_number );

  // количество миди тиков в одной четверти (это число должно быть в пределах 1...32767)
  double clksperbeat = 1000./tick_time_msec;
  // при tempo = 1e6 разрешение времени (1 midi тик) будет = (1000/clksperbeat) миллисекунд
  // т.е. tick_time_msec = 1 мсек/тик при clksperbeat=1000, и 10 мсек/тик при clksperbeat=100
  // обычно хватает и 100, но для очень быстрых произведений лучше 1000, т.к. длительность отдельных нот
  // может составлять всего 50 миллисекунд и 10-мсек на тик даст погрешность этой длительности в 20%...
  mintestmax(1., clksperbeat, 32767.);
  tracks->SetClksPerBeat( float2int(clksperbeat) );

  ChainHeader ch = dfile.chain_header();
  wstring ch_comment = dfile.header_comment();
  const Ar <DichoticAccord> &acc_chain = dfile.arr_accords();
  int num_accords = acc_chain.elements();

  MIDITimedBigMessage m;
  MIDIClockTime time; // time in midi ticks
  int chan; // chan = 0, 1, ... 15
  double pan;

  int main_track = 0; // номер трека для всех не-канальных сообщений
  int chan_track[NUMCHANS]; // номера "нотных" треков для каналов 0...15
  for (int i = 0; i < NUMCHANS; ++i)
  {
    int tr = i+1; // 1...16
    tr = (tr < tracks_number)? tr:(tracks_number-1);
    chan_track[i] = tr;
  }

  time = 0;
  m.SetTime( time ); // нулевое время для всех событий трека!

  if ( add_daccords_header )
  {
    // добавляем сырой заголовок daccords файла в midi файл как META_SEQUENCER_SPECIFIC event,
    // просто в виде char текста, хотя надо бы сначала писать 1-3 байта как ID производителя
    // (manufacturer's ID), а потом писать заголовок как Unicode текст...
    wstring2 header = dfile.header_body();
    tracks->GetTrack( main_track )->PutTextEvent(time, META_SEQUENCER_SPECIFIC, header);
  }

  // если есть - добавляем комментарий из заголовка daccords файла в midi файл, в виде char текста...
  wstring2 comment = dfile.header_comment();
  if ( !comment.empty() )
  {
    // текст META_TRACK_NAME в треке 0 Сибелиус использует как заголовок музыкального произведения!
    tracks->GetTrack( main_track )->PutTextEvent(time, META_TRACK_NAME, comment); // wstring2 -> char!
  }

  // записываем размер такта 4/4 (четыре четверти), это размер такта по умолчанию
  // m.SetTimeSig( 4, 2 ); // знаменатель 4 = 2^2!
  // tracks->GetTrack( main_track )->PutEvent( m );

  m.SetTempo( tempo );
  tracks->GetTrack( main_track )->PutEvent( m );

  // все канальные не-нотные события записываем в нотные треки: уст. панорамы и тембра!
  // так Сибелиус правильно воспроизводит тембр нот трека!

  // массив текущей панорамы каждого из миди-каналов
  vector<double> chan_pan(NUMCHANS, 123.); // неверное начальное значение будет переустановлено!
  // массив "изменения" текущей панорамы каждого из миди-каналов: true при первом изменении
  vector<bool> chan_pan_changed(NUMCHANS, false);

  // только для 3-х точечной (целой) панорамы: пишем имена канальных треков и фиксируем панораму
  if (pan_precision == 0)
  {
    // текст META_TRACK_NAME в треке > 0 Сибелиус использует как левый заголовок строки нотоносца!
    // текст META_LYRIC_TEXT в треке > 0 Сибелиус использует как подстрочный текст строки нотоносца!
    tracks->GetTrack( chan_track[0] )->PutTextEvent(time, META_TRACK_NAME, "Left");
    tracks->GetTrack( chan_track[1] )->PutTextEvent(time, META_TRACK_NAME, "Centre");
    tracks->GetTrack( chan_track[2] )->PutTextEvent(time, META_TRACK_NAME, "Right");
    // установка панорамы
    chan_pan[0] = -1.;
    chan_pan[1] =  0.;
    chan_pan[2] = +1.;
    chan = 0, SetPan( tracks, chan_track[chan], time, chan, chan_pan[chan] );
    chan = 1, SetPan( tracks, chan_track[chan], time, chan, chan_pan[chan] );
    chan = 2, SetPan( tracks, chan_track[chan], time, chan, chan_pan[chan] );
    chan_pan_changed[0] = 1;
    chan_pan_changed[1] = 1;
    chan_pan_changed[2] = 1;
  }

  // массив текущего номера инструмента в каждом из мелодических каналов
  vector<int> chan_instr(NUMCHANS, 12345); // неверное начальное значение будет переустановлено!

  // создаём события нажатия и отпускания нот и записываем их в треки

  bool last_pause = false; // если true, то последний аккорд был паузой!
  time = 0;

  while ( --repeat_upto_number >= 0 ) // повторяем музыку repeat_upto_number раз
  {
    for (int n = 0; n < num_accords; ++n)
    {
      DichoticAccord &acc = acc_chain[n];
      // длительность аккорда в секундах, с учётом chain_speed
      double dur_sec = acc.duration/(ch.chain_speed * (double)acc.temp);
      // длительность аккорда в midi-тиках
      MIDIClockTime dur_tick = seconds2ticks( dur_sec );

      if (acc.voices_number == 0) // весь аккорд - это пауза 
      {
        last_pause = true;
        time += dur_tick;
      }
      else // аккорд - не пауза - выводим все ноты/панорамы аккорда
      {
        int note;
        last_pause = false;
        m.SetTime( time );

        // массив занятости каналов мелодическими нотами (для поиска канала для новой ноты с новой панорамой)
        vector<bool> chan_busy(NUMCHANS, false); // 0 = канал свободен, 1 = занят
        chan_busy[CHANPERC] = 1; // канал ударных всегда "занят" для мелодических нот, используется только для ударных!

        // массив флагов для добавления комментария по 1 разу на каждый канал нот аккорда
        vector <bool> ok_add_accord_text(NUMCHANS, false);

        // цикл нажатия нот

        for (int i = 0; i < acc.voices_number; ++i)
        {
          if ( acc.dn[i].pause ) continue; // пауза голоса - пропускаем
          // else это не пауза голоса
          note = ch.transposition + acc.dn[i].note;

          // панорама ноты
          pan = acc.dn[i].pan;
          // округляем панораму до 3-х точек
          if (pan_precision == 0) pan = round_pan( pan );

          if ( note < 0 ) // признак ноты ударных инструментов
          {
            if ( ignore_percussion ) continue; // если перкуссия запрещена - пропускаем
            chan = CHANPERC;
            note = abs(note);
            // как оказалось панорама ударных нормально отрабатывается железом!
            if (chan_pan[chan] != pan)
            {
              chan_pan[chan] = pan;
              SetPan( tracks, chan_track[chan], time, chan, chan_pan[chan] );
            }
            goto noteon; // идём сразу на нажатие ноты
          }

          // определяем канал мелодической ноты аккорда, апдейтим панораму ноты
          {
          // ищем канал, в котором уже установлена панорама, совпадающая с панорамой текущей ноты
          int ip;
          for (ip = 0; ip < NUMCHANS; ++ip)
            if ( ip != CHANPERC && // не используем канал ударных!
                 chan_pan[ip] == pan ) break;

          if ( ip >= NUMCHANS ) // такая панорама не найдена, надо делать новую!
          {
            // ищем канал, в котором ещё не менялась панорама и не звучит ни одной ноты
            int ic;
            for (ic = 0; ic < NUMCHANS; ++ic)
              if ( !chan_pan_changed[ic] && !chan_busy[ic] ) // (канал ударных всегда "занят")
                break;

            if ( ic >= NUMCHANS ) // такой канал не найден, будем повторно менять панораму
            {
              // ищем просто любой свободный канал (канал ударных всегда "занят")
              ic = find( chan_busy.begin(), chan_busy.end(), false ) - chan_busy.begin();
              if (ic >= NUMCHANS)
              {
                // такого не может быть: число одномоментно-разных панорам ограничено числом каналов!
                cout << "\nMIDI warnings 1: all midi channels busy, Note Pan not right!\n" << endl;
                chan = 0; // берём какой-то канал...
              }
              else // ic = индекс канала
                chan = ic;
            }
            else // такой канал найден, ic = индекс канала
              chan = ic;

            // занимаем канал нотой и устанавливаем нужную ей панораму
            chan_busy[chan] = 1;
            chan_pan[chan] = pan;
            SetPan( tracks, chan_track[chan], time, chan, chan_pan[chan] );
            chan_pan_changed[chan] = 1;
          }
          else // панорама найдена, используем соотв-й ей канал
            chan = ip;
          }

          // определяем инструмент мелодической ноты аккорда и апдейтим его номер в канале
          {
          int instr = ch.dont_change_gm_instrument? ch.timbre_number:acc.timbre;
          if ( chan_instr[chan] != instr )
          {
            chan_instr[chan] = instr;
            m.SetProgramChange( chan, chan_instr[chan] );
            tracks->GetTrack( chan_track[chan] )->PutEvent( m );
          }
          }

noteon:   // нажимаем ноту и запоминаем её канал - для последующего отпускания!

          acc.dn[i].spare1 = chan;
          m.SetNoteOn(chan, note, acc.volume);
          tracks->GetTrack( chan_track[chan] )->PutEvent( m );

          // в промежутке между нажатием и отпусканием нот (это важно):
          // добавляем номер аккорда и/или комментарий к нему как лирик текст
          if ( add_accord_number || add_accord_comment )
          {
            // только по одному разу в каждом миди канале на один аккорд
            if ( !ok_add_accord_text[chan] )
            {
              ok_add_accord_text[chan] = true;
              wstring2 text;
              if ( add_accord_number ) text << (n+1);

              if ( add_accord_comment == 1 ) // стандартный вывод коммента
              {
                // если выведен номер то после него ставим пробел
                if ( !text.empty() ) text += UNI_SPACE;
                text << acc.comment;
              }
              else
              if ( add_accord_comment == 2 )  // нестандартный вывод коммента
              {
                text << acc.comment; // без пробела
              }

              tracks->GetTrack( chan_track[chan] )->PutTextEvent(time, META_LYRIC_TEXT, text);
            }
          }
        }

        // цикл отпускания нот

        time += dur_tick;
        m.SetTime( time );
        for (int i=0; i < acc.voices_number; ++i)
        {
          if ( acc.dn[i].pause == 0 ) // это не пауза голоса
          {
            note = ch.transposition + acc.dn[i].note;
            // извлекаем номер канала, в котором ранее была нажата эта нота
            chan = acc.dn[i].spare1;
            if (chan == CHANPERC) note = abs(note); // нота ударных инструментов
            m.SetNoteOff(chan, note, acc.volume);
            tracks->GetTrack( chan_track[chan] )->PutEvent( m );
          }
        }
      }
    }
  }

  // если последним аккордом была пауза - надо добавлять "пустоту" чтобы звук не обрывался раньше!
  if (last_pause)
  {
    // добавляем нажатие ноты с нулевой скоростью
    m.SetTime( time );
    m.SetNoteOn(chan = 0, 0, 0);
    tracks->GetTrack( chan_track[chan] )->PutEvent( m );
  }

  cout << "Music duration in sec = " << GetMisicDurationInSeconds(*tracks) << endl;

  return true;
};

