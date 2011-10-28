
#include "stdafx.h"
#include "stdafx2.h"

// ============================= Функции ============================================

// pip() в релизе заменять на pipex() т.к. под некоторыми XP pip() зависает!
void pip() { pip2(1000, 20); }
void pip2(int freq, int time) { Beep(freq, time); } // Hz/msec на спикере!
void pipex() { MessageBeep(UINT(-1)); } // короткий сигнал на звук. карточку!
// void pipex() { MessageBeep(MB_ICONEXCLAMATION); } // звук подлиннее...
// void pipex() { MessageBeep(MB_ICONASTERISK); } // звук длиннее и громче

int EatMessages(HWND hWnd, UINT msgFilter)
// eat spurious messages msgFilter
{
  MSG msg;
  int cnt = 0;
  while ( PeekMessage(&msg, hWnd, msgFilter, msgFilter, PM_REMOVE) ) ++cnt;
  return cnt;
}

// ============================= Классы ============================================

int TimerPeriod::timerPeriod = 0; // 0=неизвестный период!

Times::Times(int resolution)
{
  ResetResolution(resolution);
}

bool Times::ResetResolution(int resolution)
// меняем разрешение таймера и делаем reset() - меняем время t0 на текущее
{
  prec_timer_ok = 0;
  k_prop = 0.001;
  min_dt = 0.01; // по умолчанию точность 10 миллисекунд
  bool res = true; // результат установки нужной точности

  // устанавливаем лучшее временное разрешение таймера
  if (resolution > 0)
  {
    // пытаемся включить датчик сверхточного времени
    if (resolution == hi) res = SetupPrecTimer();

    // resolution=1 или не получилось resolution=2 - ускоряем обычный таймер
    if (resolution == mid || res == false)
    {
      k_prop = 0.001;
      TimerPeriod tp(1);
      int per = tp.getTimerPeriod();
      if (per > 0) min_dt = k_prop * per;
      // при resolution=1 период таймера не должен быть больше 1 мсек
      if (resolution == mid && per > 1) res = false;
    }
  }

  // запоминаем текущий момент времени
  reset();
  return res;
}

double Times::PrecTime(bool reset) // get actual precision time in seconds
{
  if (!prec_timer_ok) return 0.;
  LONGLONG t = longPrecTime();
  if (reset) t0_hi_res = t;
  // точное время в секундах (видимо от включения компьютера)
  return k_prop*double(t);
}

LONGLONG Times::longPrecTime() const
{
  LARGE_INTEGER tcount;
  QueryPerformanceCounter(&tcount);
  return tcount.QuadPart;
}

bool Times::SetupPrecTimer()
{
  prec_timer_ok = 0;

  LARGE_INTEGER precFreq;
  int res = QueryPerformanceFrequency(&precFreq);
  if (res) prec_timer_ok = 1;

  // частота св.т. датчика в Гц
  double prec_freq = (double)precFreq.QuadPart;
  if ( prec_freq <= 0.0 ) prec_timer_ok = 0;

  // если датчик в порядке - определяем параметры частоты
  if (prec_timer_ok)
  {
    min_dt = k_prop = 1./prec_freq;
    return true;
  }
  return false;
}

double Times::Time(bool reset)
{
  DWORD t = timeGetTime();
  if (reset) t0_low_res = t;
  return k_prop*t;
}

double Times::dt() const
{
  double dtime;

  if (prec_timer_ok)
  {
    uint64 t = longPrecTime();
    uint64 dt = t - uint64(t0_hi_res);
    dtime = k_prop * dt;
  }
  else
  {
    uint32 dt = timeGetTime() - t0_low_res;
    dtime = k_prop * dt;
  }

  return max_(0., dtime); // защита от отрицательных пром-в на всякий сл.
}

// =============================

void Mbox() { MsgBox(L""); }

// по умолчанию мессаг-бокс будет с нулевым хендлом родителя, т.е. не модальный
int (WINAPI *MsgBoxFun)(HWND, LPCWSTR, LPCWSTR, UINT) = MessageBoxW;
void MsgBox(wstring text)
{
  MsgBoxFun(0, text.c_str(), 0, MB_OK);
}

void Mbox0() { MsgBox0(L""); }

void MsgBox0(wstring text)
{
  MessageBoxW(0, text.c_str(), 0, MB_OK);
}

