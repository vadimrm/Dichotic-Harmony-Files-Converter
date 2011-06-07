/*
*/

#pragma once

struct ChainHeader // ����� ��������� ������� �������� DichoticAccord
{
  int transposition; // ���������� ���� ����� ������������� ����, ������ 0
  double chain_speed; // �������� ������������ ���������, ���������� = 1.0
  int dont_change_gm_instrument; // ���� 1, �� ���������� �� ������ �� �������� �������, �
  int timbre_number; // ������ ������ - ����� ���� ����� ����������� �� ��� ���������
};

struct DichoticNote // ���� ������������ ����
{
  int pause; // ���� 1, �� ��� �� ����, � �����!
  int note; // ������������� ����� ����
  double pan; // ��������: -1 �����, 0 �����, 1 ������ ����
  int spare1; // "�������� ����� 1", ������������ ��� ��������� ���� ����...
};

struct DichoticAccord // ��������� ������������� ������� 
{
  // ����. ����� ������� �������, ������� �������������� ������ ������� ���������
  static const int MAX_ACC_VOICES = 256; // ���������� �������� = 128 ��� XG level 3
  static const int COMMLEN = 16;

  int timbre; // �����, ���� ����� �����������
  int duration; // ������������ � ������ ���������
  int temp; // �������� �����, ����� ������ ��������� � �������
  int voices_number; // ���������� ������� � �������, 1...MAX_ACC_VOICES, ���� 0 - ��� �����!
  wchar_t comment[COMMLEN]; // ����� "�����������" � �������, ������������� ����, �������� ������� � �����
  int spare1; // "�������� ����� 1", ������������ ������ ��� �������� ����������...

  // ���� �����, �� ��� ��������� ����� �������������:
  int volume; // ���������, 1...127
  // � ���� ������� ������ ���� ����� ��� ����������� ���������� ����� ������� � �������
  DichoticNote dn[MAX_ACC_VOICES]; // ������������ ���� [0]...[voices_num-1] �������

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
  // �������� src ������ � ������ ��� ��������� ��������� ����������� �������
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
  static const int ADDS = 4; // ��������� ������ � ������ ����� (������, �����������, ����� ���������, ������������)
  wstring header; // ����� ��������� ����� (��� ADDS ���-�����, ���������� ����� ����� UNI_CRLF)
  ChainHeader ch; // �������������� ��������� �����
  wstring comment; // ����������� �� ��������� �����
  Ar <DichoticAccord> accords; // ������ �������� �� �����
  int errs; // ������ ��� ������ daccords �����: ���� 0 �� �� � �������!
  int accords_number; // ���������� �������� � ������� accords ��� ����������� �� midi

  // ������� ���������� midi ������� � ����� � daccords ������
  void write_accord(int dtms, vector <MIDITimedBigMessage> &accord_events, vector <int> &instr, vector <double> &pan);
public:
  DaccordsFile(const wchar_t *file = 0) // ���� ���� - ������ .daccords ����
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

  bool Read(const wchar_t *file); // ������ � ���������� daccords ����
  // ������ �� ���� daccords �����, pan_precision - ����� ���. ������ ����� ������� ��� ������ ��������
  bool Write(const wchar_t *file, int pan_precision, int add_accord_number, int add_accord_comment) const;

  // ��������������� ����������� ������������ MidiFile � daccords ������
  bool MidiToDaccords(const MidiFile &mfile, int ignore_percussion, double accord_time_lag, int delete_start_pause);

  void OptimizeTransposition(); // ������ ������������ = ����������� ����� ����, � ��������� = 0

  int get_accords_number() const { return accords_number; }
  const Ar <DichoticAccord>& arr_accords() const { return accords; }
  const wstring& header_body() const { return header; }
  const wstring& header_comment() const { return comment; }
  ChainHeader chain_header() const { return ch; }
  int errors() const { return errs; }

  static const wchar_t *daccords_header; // ������ ����� � ������ ����� (���������)
  static const int dflt_version = 0; // ������ ������� �����, ������� ������������ ���

  // �������� ����������� ����� �������� df � �����-������ ws
  // pan_precision - ����� ���. ������ ����� ������� ��� ������ ��������, ���� 0 �� ���� ����� ���!
  friend void ConvertAccordsToString(const DaccordsFile &df, wostringstream &ws, int pan_precision,
                                     int add_accord_number, int add_accord_comment);
};


class MidiFile
{
  MIDIMultiTrack *tracks; // the object which will hold all the midi tracks

public:
  MidiFile() { tracks = new MIDIMultiTrack(1); }
  ~MidiFile() { delete tracks; }

  bool Read(const wchar_t *file); // ������ � ���������� midi ����

  // ����������� ��� ����� � 0-� � ���� ���� ����� ������������� 0-� ���� � ����� 0-16
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

    delete tracks; // ������� �������� ������
    tracks = tracks2; // ��������� �� �������� ������ ������ ��������� �� ����� ������ � tracks2
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

  // ��������������� ����������� ������������ DaccordsFile � MIDIMultiTrack
  bool DaccordsToMidi(const DaccordsFile &dfile, double tick_time_msec, int add_daccords_header,
    int repeat_upto_number, int add_accord_number, int add_accord_comment, int pan_precision, int ignore_percussion);

  // ������ MIDIMultiTrack �� ���� � midi ����, � *midi_tracks_number ������������ ����� ������
  bool Write(const wchar_t *file, int *midi_tracks_number=0) const;

  const MIDIMultiTrack * GetMultiTrack() const { return tracks; }

  static double round_pan(double pan)
  {
    // ���������� pan �� 3-� �����: �� ��� �� ������ ������ pan_porog ������ � �����, ��������� - �� ����� ��������
    const double pan_porog = 0.5;
    if (pan <= -pan_porog) pan = -1.; // ����
    else
    if (pan >= +pan_porog) pan = +1.; // �����
    else                   pan =  0.; // �����
    return pan;
  }

  // ���������� ������� ������� -> midi ����: ����������� ������ ��� tempo = 1e6 !
  MIDIClockTime seconds2ticks(double seconds) const
  {
    return MIDIClockTime( 0.5 + tracks->GetClksPerBeat()*seconds );
  }

  static const uint32 tempo = 1000000; // ���� 1 000 000 usec = 1 sec � ��������

  static const int NUMCHANS = 16; // ����. ���������� ����-�������, ������� ����� ������� CHANPERC
  static const int CHANPERC = 9; // ����� midi ������ ������� ������������
};

