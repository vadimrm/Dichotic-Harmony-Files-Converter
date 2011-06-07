
#include "stdafx.h"

// ============================= ������� ============================================

// pip() � ������ �������� �� pipex() �.�. ��� ���������� XP pip() ��������!
void pip() { pip2(1000, 20); }
void pip2(int freq, int time) { Beep(freq, time); } // Hz/msec �� �������!
void pipex() { MessageBeep(UINT(-1)); } // �������� ������ �� ����. ��������!
// void pipex() { MessageBeep(MB_ICONEXCLAMATION); } // ���� ���������...
// void pipex() { MessageBeep(MB_ICONASTERISK); } // ���� ������� � ������

int EatMessages(HWND hWnd, UINT msgFilter)
// eat spurious messages msgFilter
{
  MSG msg;
  int cnt = 0;
  while ( PeekMessage(&msg, hWnd, msgFilter, msgFilter, PM_REMOVE) ) ++cnt;
  return cnt;
}

// ============================= ������ ============================================

int TimerPeriod::timerPeriod = 0; // 0=����������� ������!

Times::Times(int resolution)
{
  ResetResolution(resolution);
}

bool Times::ResetResolution(int resolution)
// ������ ���������� ������� � ������ reset() - ������ ����� t0 �� �������
{
  prec_timer_ok = 0;
  k_prop = 0.001;
  min_dt = 0.01; // �� ��������� �������� 10 �����������
  bool res = true; // ��������� ��������� ������ ��������

  // ������������� ������ ��������� ���������� �������
  if (resolution > 0)
  {
    // �������� �������� ������ ������������ �������
    if (resolution == hi) res = SetupPrecTimer();

    // resolution=1 ��� �� ���������� resolution=2 - �������� ������� ������
    if (resolution == mid || res == false)
    {
      k_prop = 0.001;
      TimerPeriod tp(1);
      int per = tp.getTimerPeriod();
      if (per > 0) min_dt = k_prop * per;
      // ��� resolution=1 ������ ������� �� ������ ���� ������ 1 ����
      if (resolution == mid && per > 1) res = false;
    }
  }

  // ���������� ������� ������ �������
  reset();
  return res;
}

double Times::PrecTime(bool reset) // get actual precision time in seconds
{
  if (!prec_timer_ok) return 0.;
  LONGLONG t = longPrecTime();
  if (reset) t0_hi_res = t;
  // ������ ����� � �������� (������ �� ��������� ����������)
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

  // ������� ��.�. ������� � ��
  double prec_freq = (double)precFreq.QuadPart;
  if ( prec_freq <= 0.0 ) prec_timer_ok = 0;

  // ���� ������ � ������� - ���������� ��������� �������
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

  return max_(0., dtime); // ������ �� ������������� ����-� �� ������ ��.
}

// =============================

void Mbox() { MsgBox(L""); }

// �� ��������� ������-���� ����� � ������� ������� ��������, �.�. �� ���������
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

