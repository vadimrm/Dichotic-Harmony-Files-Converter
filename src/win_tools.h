
#pragma once

// pip() � ������ �������� �� pipex() �.�. ��� ���������� XP pip() ��������!
void pip(); // 1000 Hz/ 20 msec �� �������!
void pip2(int freq, int time); // Hz/msec �� �������!
void pipex(); // ������ �� ����. ��������!

inline int wstr2int(wstring2 &wstr) { return _wtol(wstr); }
int EatMessages(HWND hWnd, UINT msgFilter);

extern int (WINAPI *MsgBoxFun)(HWND, LPCWSTR, LPCWSTR, UINT); // ��������� �� ������� ������-�����

// ��������� �����
void Mbox();
void MsgBox(wstring text);
template<class I> void Mbox(const I i1) { wstring2 str; str << i1; MsgBox(str); }
// delim - �������������� ������ ����� ������� ������ ����������
template<class I1, class I2> void Mbox(const I1 i1, const I2 i2, const wchar_t delim = UNI_LF)
{ wstring2 str; str << i1 << delim << i2; MsgBox(str); }
template<class I1, class I2, class I3> void Mbox(const I1 i1, const I2 i2, const I3 i3, const wchar_t delim = UNI_LF)
{ wstring2 str; str << i1 << delim << i2 << delim << i3; MsgBox(str); }
template<class I1,class I2,class I3,class I4> void Mbox(const I1 i1,const I2 i2,const I3 i3,const I4 i4,const wchar_t delim = UNI_LF)
{ wstring2 str; str << i1 << delim << i2 << delim << i3 << delim << i4; MsgBox(str); }
// �� �� ����� � ����������� ������ - � ������� �������
void Mbox0();
void MsgBox0(wstring text);
template<class I> void Mbox0(const I i1) { wstring2 str; str << i1; MsgBox0(str); }

// ��������� � ������� ��������� ������� "��������" ������� � �������������
// ���������� �������� = 1 ���� (�� ������ ������������?)
class TimerPeriod
{
  static int timerPeriod; // ���� 0, �� ���������� �����!

public:
  TimerPeriod(int period = 10) { setTimerPeriod(period); }

  int getTimerPeriod() const { return timerPeriod; } // � �������������

  bool setTimerPeriod(int period)
  {
    // �������� ���������� ������ ��������� "������" � ���������
    while ( !timer_resolution(period) ) if (++period > 100) return false;
    timerPeriod = period;
    return true;
  }

  // ��������� msec-�������� �������, ���� ���������� - ������ false, ����. ��� msec=0
  static bool timer_resolution(uint32 msec) // ��� W98,2K,XP ��������� ������ 1 ����
  { return TIMERR_NOERROR == timeBeginPeriod(msec); }
};

// =============================

// ������������� ����� ������� � ����������� �������
class Times
{
  int prec_timer_ok; // 1=���� ������ ����� ������� �������, 0=���
  LONGLONG t0_hi_res;
  DWORD    t0_low_res;
  double k_prop; // ��������� �������� ��������� ������� � ������� (���� �������, ���. ��������)

  double Time(bool reset);
  double PrecTime(bool reset);
  LONGLONG longPrecTime() const;
  bool SetupPrecTimer();

  // double ����� � ��� ���������� - � ��������
  double t0; // ������ ������� ������ ������� ����������� (dt=t-t0), �� ��� ���� �� ������������!
  double min_dt; // ������� �������� ������� �������

public:
  enum { def=0, mid, hi };
  // ��� resolution=def �������� ������� �� 5 �� 20 ���� (�� W2K 10 ����)
  // ��� resolution=mid �������� ����� 1.0 �����������
  // ��� resolution=hi  �������� ����� 0.3 �����������
  Times(int resolution = def);

  // ������ ���������� ������� � ������ reset() - ������ ����� t0 �� �������
  bool ResetResolution(int resolution);

  // ��� ������� ����� ������������ ��� ������������� Random ��������...
  static uint32 getTimerTime() { return timeGetTime(); }

  // ���������� ����� � ������ ����� ��������� �� �� "����������" t0
  double t(bool reset=false) { return prec_timer_ok? PrecTime(reset):Time(reset); }
  double time(bool reset=false) { return t(reset); } // ������� t()

  double dt() const; // ���������� ������� � �������� � ������� t0
  double dtms() const { return 1000.*dt(); } // �� �� � �������������
  int idtms() const { return int(dtms()); } // �� ��, �� � ���� ������

  void reset() { t0 = time(true); } // ������ ����� t0 �� �������

  double get_t0() const { return t0; }
  double get_min_dt() const { return min_dt; }
};

// =============================

