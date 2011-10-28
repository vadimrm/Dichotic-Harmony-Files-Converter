
#pragma once

// pip() в релизе заменять на pipex() т.к. под некоторыми XP pip() зависает!
void pip(); // 1000 Hz/ 20 msec на спикере!
void pip2(int freq, int time); // Hz/msec на спикере!
void pipex(); // сигнал на звук. карточку!

inline int wstr2int(wstring2 &wstr) { return _wtol(wstr); }
int EatMessages(HWND hWnd, UINT msgFilter);

extern int (WINAPI *MsgBoxFun)(HWND, LPCWSTR, LPCWSTR, UINT); // указатель на функцию массаг-бокса

// модальные боксы
void Mbox();
void MsgBox(wstring text);
template<class I> void Mbox(const I i1) { wstring2 str; str << i1; MsgBox(str); }
// delim - разделительный символ между выводом разных переменных
template<class I1, class I2> void Mbox(const I1 i1, const I2 i2, const wchar_t delim = UNI_LF)
{ wstring2 str; str << i1 << delim << i2; MsgBox(str); }
template<class I1, class I2, class I3> void Mbox(const I1 i1, const I2 i2, const I3 i3, const wchar_t delim = UNI_LF)
{ wstring2 str; str << i1 << delim << i2 << delim << i3; MsgBox(str); }
template<class I1,class I2,class I3,class I4> void Mbox(const I1 i1,const I2 i2,const I3 i3,const I4 i4,const wchar_t delim = UNI_LF)
{ wstring2 str; str << i1 << delim << i2 << delim << i3 << delim << i4; MsgBox(str); }
template<class I1,class I2,class I3,class I4,class I5>
  void Mbox(const I1 i1,const I2 i2,const I3 i3,const I4 i4,const I5 i5,const wchar_t delim = UNI_LF)
{ wstring2 str; str << i1 << delim << i2 << delim << i3 << delim << i4 << delim << i5; MsgBox(str); }
// то же самое с немодальным боксом - с нулевым хендлом
void Mbox0();
void MsgBox0(wstring text);
template<class I> void Mbox0(const I i1) { wstring2 str; str << i1; MsgBox0(str); }

// установка и возврат интервала времени "обычного" таймера в миллисекундах
// наименьший интервал = 1 мсек (не всегда гарантирован?)
class TimerPeriod
{
  static int timerPeriod; // если 0, то неизвестно какой!

public:
  TimerPeriod(int period = 10) { setTimerPeriod(period); }

  int getTimerPeriod() const { return timerPeriod; } // в миллисекундах

  bool setTimerPeriod(int period)
  {
    // пытаемся установить период ближайший "сверху" к заданному
    while ( !timer_resolution(period) ) if (++period > 100) return false;
    timerPeriod = period;
    return true;
  }

  // установка msec-точности таймера, если невозможно - выдает false, напр. при msec=0
  static bool timer_resolution(uint32 msec) // под W98,2K,XP нормально ставит 1 мсек
  { return TIMERR_NOERROR == timeBeginPeriod(msec); }
};

// =============================

// универсальный класс времени и промежутков времени
class Times
{
  int prec_timer_ok; // 1=есть датчик сверх точного времени, 0=нет
  LONGLONG t0_hi_res;
  DWORD    t0_low_res;
  double k_prop; // константа перевода показаний времени в секунды (того датчика, кот. работает)

  double Time(bool reset);
  double PrecTime(bool reset);
  LONGLONG longPrecTime() const;
  bool SetupPrecTimer();

  // double время и его промежутки - в секундах
  double t0; // момент времени начала отсчета промежутков (dt=t-t0), но оно само не используется!
  double min_dt; // текущая точность датчика времени

public:
  enum { def=0, mid, hi };
  // при resolution=def точность времени от 5 до 20 мсек (на W2K 10 мсек)
  // при resolution=mid точность около 1.0 миллисекунд
  // при resolution=hi  точность около 0.3 микросекунд
  Times(int resolution = def);

  // меняем разрешение таймера и делаем reset() - меняем время t0 на текущее
  bool ResetResolution(int resolution);

  // эту функцию можно использовать для инициализации Random датчиков...
  static uint32 getTimerTime() { return timeGetTime(); }

  // возвращает время и делает ресет счетчиков на их "внутренние" t0
  double t(bool reset=false) { return prec_timer_ok? PrecTime(reset):Time(reset); }
  double time(bool reset=false) { return t(reset); } // синоним t()

  double dt() const; // промежуток времени в секундах с момента t0
  double dtms() const { return 1000.*dt(); } // то же в миллисекундах
  int idtms() const { return int(dtms()); } // то же, но в виде целого

  void reset() { t0 = time(true); } // меняем время t0 на текущее

  double get_t0() const { return t0; }
  double get_min_dt() const { return min_dt; }
};

// =============================

