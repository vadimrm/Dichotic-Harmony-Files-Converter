#include "stdafx.h"
const wchar_t* ProgramTitle = L"Dichotic Harmony Files Converter";
const wchar_t* ProgramCopyright = L"Copyright (c) 2010 Vadim Madgazin";
const wchar_t* VER_NUM = L"1.10"; // ������ �� 7 ���� 2011 �.
/*
  =�������:
  -�������� � ���������� libjdksmidi ������ 14!
  -����� ������� ��� jdksmidi_fileread.cpp - ��. ����� Stephan.Huebler@tu-dresden.de
  -��������� ������ ������������� ������ ����������� �� ������� 2h_accords_generator
  -����������� ������ ��������� � ����������� - Visual Studio 2005 Team Suite
  -��������� � ���������� ����������: Linker / Advanced / Profile 
   �� �������� ����� ���: Tools / Performance Tools / Performance Wizard ����� �� ������
   ����� �� ������� (� ����) Performance Explorer ������ ������� �� ������� �������� Launch
   ����� ������� ����� - ������� �� "������" ������� Functions ����� Elapsed Inclusive Time
   � ������������� - ����� ������� ������ ����� �����������...
   ����� ���������� ������ ������� ��� ��� ���� "�������" .psess ���� � File / Open
  -��� ������� ������ ����������� ��������� �������� �������������� daccords -> midi:
   ��������� ���������, ��� ����� 80% ������� ������� �� �������� � wstring � ��������
   � wstring2 �������� � ������� DaccordsFile::Read(const wchar_t *file), � ���������
   �� ���������������, ����������� � ����� (wstring2 >> � int, double � ��. ����������)...
   ������� ����� ��������� wstring ��� ��� ������ �� wchar_t ���, � ����� ����� � ����
   ������� ������ wstring2 ���� ������������ wistringstream � wstring, � ���������� ����
   �������� ����������� ��� � 2 ����!
  -������������ �� ������� ��� ��������� �� �������� �������� ����:
   �� ������������ wstring2 �������, ������� ���������� ������ ��� �������� ����������
   ���������� �����/������ ��/� ������� ���������� >> � <<
   �� ������������ ��� � ������������ �������������� ������ ��� wstring

  =����:
  -������ libjdksmidi
  -����� ������ 2h_converter �� Github

  =��������� ������:
  -�������� ��������� �������: ��. ����. ����-7 �� 18 ������ 2010 �.

  -����� ��������� midi ������������: ����-7, ������ �� 31 ���. 2010 �.

  -�����-��������. �������: ��� ������-�����, ������ �����. �����!
  -������ �� GCC - minGW

  -������������ ��� 2-�� ������� daccords ������:
   � ��������� ������ ������, ������ �������������� � midi-���������� (�������� � ��.):
   ������ �����,
   �������� ����������,
   �������� � �.�.
   ��������� ������� accord_time_lag � tick_time_msec

   ������ ������ ������� (�������� ��������� ��� �������� ��������� ����� �� ����� ";"):
   ������ ������������ (Duration � TempBPS) ������ �������� ������� (���/����) ������ �������
   (�� ����� � ������) �� ������ ������ ������� ��� �� ������ ����������� �������,
   ��� ������ ���� ������ �������-� ����� ����������� � ��������� (�������� �������),
   ��� ������������� ��� ������ ����. ���� (��� � �����������, �.�. �� ���������� ������� ����),
   ��� 2-� � ����� ���������� (������������) ��� � ������� (���������� ����� � ���������� ����)
   ������ �������������� "����. ����� ����" - ����� �������� ����� ������ ���� ��������� �����
   � ��������� ������, � ����� ���.
   ��� ������ ��������� ����� ���� ��� ���� ���� �� ��� ���� � ���������� �������, �� � ������� ���.
   ��� �������� "����. ����� ����" = 1, � ������ (�����) - �������� ����� 2.
   ��� ����������� ����������� ����������� �� ������� ��� � ����������� ���� �������� - ��� ������ -
   �������� ��� ���� ��������: ����� ������ "������" �������� �������� ����.
   ������ ������������ ������� ������:
   0 = ������ � ����� (������������): ����, ������� ������ ������ � ������� �������
   1 = ������: ���� �� ������������� � ������ �������� �������
   2 = �����������: ���� �������� ����� � �� ������������� � ������ �������� �������
   3 = �����: ���� �������� ����� � ������������� � ������ �������� �������.
   �� ��������� � ��� "����� ������" = 0.

  -����� ��������� c++midi lib ����������� "��������� midi" ������, ��������� ��� ��� �������
   ������ midi ������ (General Midi), ��� � ��� ��������� � ������� ��� ����������.
   ��� ��� ��� ����� ����:
   ����� (���������� ��� �� ����������� �������),
   ������������ ����� ����
   ����� ����� (���-�� ����� ��������� ������ ���������)
   ����� ����������� (�������./�������.)
   ����� ����
   �������� �������/����������
   ��������
   ��������� ���������
   ������ ���������...
   ������ ����� ������� ����� ������ ����������� ������� ����� ��� ��������� �����������,
   �.�. ���� ���� �� �������� ������ ������� ������� ���� �� �����, �������� ��� ���
   t 0. p 0. n 44 (������ ���� ������� �������: �����, ��������, ����� ����)
   +n +2 +n +4 +n +3 (��� ��� ���� � �������� 46, 50, 53, ������������ ������� - �� ���������� t)
   +t 0.5 n 40 +n +2 +n +3 (2-� ������ ������ ���������� ���� 40, 42, 45)

  =�����:
  -���������� ��� ������� ��� ������ Ar, ����� vector
  -��������� ��� ��������� ������ � ���� ���������, ������� � ���������� � �������...
  -�������� ����������� ���������� ��������� ������� ��� Unicode ������, � ��� ����� ���
   ����-������� META_SEQUENCER_SPECIFIC, � ��� ���������� �������� manufacturer's ID
  -��������� ��� ����� ������� � Git, ������� �������� .sln �� �� .exe!
  -� ������� ����� ������������� �������� ������� ��� ������ GCC/MinGW!

  =�����:
  -������������ �� ������� ��� ��������� �� �������� �������� ����:
   �� ������������ wstring2 �������, ������� ���������� ������ ��� �������� ����������
   ���������� �����/������ ��/� ������� ���������� >> � <<
   �� ������������ ��� � ������������ �������������� ������ ��� wstring
  -��������� � ���������� ����������: Linker / Advanced / Profile 
   �� �������� ����� ���: Tools / Performance Tools / Performance Wizard ����� �� ������
   ����� �� ������� (� ����) Performance Explorer ������ ������� �� ������� �������� Launch
   ����� ������� ����� - ������� �� "������" ������� Functions ����� Elapsed Inclusive Time
   � ������������� - ����� ������� ������ ����� �����������...
   ����� ���������� ������ ������� ��� ��� ���� "�������" .psess ���� � File / Open
  -����� ���� ��� ���������� ".mid" ��������� daccords ������!
  -daccords ���� ������ ���� � Unicode ���������!
  -����� META_TRACK_NAME � ����� 0 �������� ���������� ��� ��������� ������������ ������������!
  -����� META_TRACK_NAME � ������ 1,2,3 �������� ���������� ��� ����� ��������� ������ ���������!
  -����� META_LYRIC_TEXT � ������ 1,2,3 �������� ���������� ��� ����������� ����� ������ ���������!
  -��� � ��������� ���. 4:
   ���� � ����� ������� ���� 2 ���������� ���� (�������������, �� � ������ ���������), �� ���
   ���������������� ������� �� midi ����� ������� 1 ����� ��� ���� ����������� � ����� ����� -
   ������ ���� ��� ���������� ������������ ���������� ���� ���������� ��������� ������������ �
   �����! �� �� �� ����� �� midi ����� � �������� 0 �������� ���������!
   ���� �� �������� ����������, �� � �� ������� 0 ���������� ���� ���� �������� ������, ����
   ���� ����������, � ������ - ����������� ������������+�����!
  -��� � ��������� ���. 4: midi ����� � ��������� ������ ��� (�������� <= 5) �� �����������!
*/

bool TEST = 0; // 1 = ����� ��� �������������� ���� �� �������� �����
int testmode = 1;
wstring testfilew = L"crash_converter.mid.daccords";

int converter_mode = 0; // ����� ������: 0 input midi ����, 1 input daccords ����
static int& MODE = converter_mode;

// ��������� ����������� � converter_mode ������: param[converter_mode]

int collapse_midi_tracks[2] = {0,0}; // 1 = ����������� ��� midi ����� � 0-� ����; [1] �� ������������
int collapse_and_expand_midi_tracks[2] = {0,0}; // 1 = ����������� ����� � 0 � ������������� � 0-16; [1] �� ������������

int use_start_pause[2] = {1,1}; // 1 = ��������� ��������� ����� ������, 0 = �������
double clip_music_time[2] = {0.,0.}; // ����� ���������� ������ (���), ���� <=0. �� ������������; [1] �� ������������
int midi_as_text[2] = {0,0}; // 1 = ����� midi ����� ��� ����� � .txt ����; [1] �� ������������

// ����� ���. ������ ����� ������� ��� double ��������, ���� 0 �� ��� "�����" �������� -1, 0, +1
int panorame_precision[2] = {2,2}; // ������ 0 ��� 2, �������� 3.
int use_percussion[2] = {0,1}; // 1 = ������������ ����/����� ������� ������������
int optimize_transposition[2] = {1,0}; // 1 = ����������� ������ ��������� ������������ ���; [1] �� ������������

// ����� � �������� �������� ���������������� ������� (��� ����������) ��� ��������� ��������������
// �.�. ���� ����� ����������� ������� ������� �� ������� � �������� ����� ������� � ��� ��� ��������
double accord_time_lag[2] = {1.,0}; // ������ 1-10 msec; [1] �� ������������
double tick_time_msec[2] = {0,1.}; // 0.01-1000 ����� ����������� � ����� midi ����; [0] �� ������������
int repeat_upto_number[2] = {0,1}; // <=1e6 ����� �������� ������� ������ � �������� �����; [0] �� ������������
int add_accord_number[2] = {1,1}; // 1 = �������� � ���� ����� �������: � midi ��� �������� ';', � daccords � ';'

// 0 = �� �������� � ���� ����������� �������, 1 = �������� ���������� (����� ������), 2 = �������� ������������:
// � midi ��� ������� ����� ����������� ������ �������, � daccords � ��������� ';' ��� ���������� ������ �������
// �.�. ����� ��������� ����� ���. ������� � ����������� ��� ������������ ����� �� ����������� ��� ����� �������
int add_accord_comment[2] = {1,1};
int add_daccords_header[2] = {0,0}; // 1 = �������� � ���� daccords ����� ��� SEQUENCER_SPECIFIC; [0] �� ������������

wstring2 infilew, outfilew; // ��� ��������, ��������� ������

HWND MboxHWND = 0;
const wchar_t *MboxTitle = 0;
int WINAPI MyMsgBoxFun(HWND, LPCWSTR text, LPCWSTR, UINT)
{
  return MessageBoxW( MboxHWND, text, MboxTitle, MB_OK ); // ��������� ������-���� ��� ����� MboxHWND
}

void wmain(int argc, wchar_t *argv[])
{
  // �������� ����� � ���������� ���� �������
  HWND chwnd = GetConsoleWindow();
  HINSTANCE chinst = GetModuleHandle(0);

  MboxHWND = chwnd; // ����� ������������� ���� ��� Mbox() - ����
  MboxTitle = ProgramTitle; // �� ��������� ������
  // �������� ��������� �� ��� ������� ������-�����, ������� ����� ����� �������� ��������� ������ �����
  MsgBoxFun = MyMsgBoxFun;

  wstring2 bigtitle(ProgramTitle);
  bigtitle += L" ver ";
  bigtitle << VER_NUM << "\n" << ProgramCopyright;

  if (TEST) // �������� �����
  {
    converter_mode = testmode;
    infilew = testfilew;
  }
  else // ������� �����
  {
    // ����������� � �����������
    if ( !ParseArgs(argc, argv) )
    {
      PrintHelp(bigtitle);
      return;
    }
  }

  wcout << endl << bigtitle << endl << endl;

  if ( converter_mode == 0 ) // input midi
  {
    // ������ midi ����

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
      // ����������� ��� ����� � 0-� � ���� ���� ����� ������������� 0-� ���� � ����� 0-16
      mfile.CollapseAndExpandMultiTrack( 0 != collapse_and_expand_midi_tracks[MODE] );
    }

    if ( midi_as_text[MODE] ) // ����� �������� midi ����� ��� ����� � .txt ����
    {
      string text = MultiTrackAsText( *mfile.GetMultiTrack() );
      wstring filew = infilew + L".txt";
      // ��������� ����� � ����
      write_bin(filew.c_str(), text.c_str(), text.length(), true);
      return;
    }

    if ( !use_start_pause[MODE] ) // ������ ����� ������� ��������� ����� ������
    {
      // ������ ��������� ����� � midi ����� ��������, ��� ��������� ����� � daccord
      save_midi = true;
      mfile.CompressStartPause(!use_percussion[MODE]);
    }

    if (clip_music_time[MODE] > 0.) // ������ ����� �������� ����� ����� ������
    {
      save_midi = true;
      mfile.ClipMultiTrack( clip_music_time[MODE] );
    }

    if (save_midi) // ������ ������������������ midi ����� �� ����
    {
      outfilew = infilew + L".mid";
      infilew = outfilew; // ���� �������� ���� ���� ����� ����� �������!
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

    // ������ daccords ����

    DaccordsFile dfile;
    bool res = dfile.MidiToDaccords(mfile, !use_percussion[MODE], accord_time_lag[MODE], !use_start_pause[MODE]);
    if (!res)
    {
      Mbox(L"Error in MidiToDaccords() converter!");
      return;
    }

    // ������ ������������ = ����������� ����� ������������ ����
    if ( optimize_transposition[MODE] ) dfile.OptimizeTransposition();

    // write the output daccords file

    outfilew = infilew + L".daccords"; // ��� ��������� �����
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
    // ������ daccords ����

    DaccordsFile dfile;
    if ( !dfile.Read(infilew) )
    {
      Mbox(L"Error reading or processing daccords file", infilew,
           L"\nError code", dfile.errors() );
      return;
    }
    // Mbox( dfile.header_comment(), L"����� �������� � �����:", dfile.arr_accords().elements() );

    if ( !use_start_pause[MODE] ) dfile.DeleteStartPause(); // ������� ��������� ����� ������

    // ������ midi ����

    MidiFile mfile;
    bool res = mfile.DaccordsToMidi(dfile, tick_time_msec[MODE], add_daccords_header[MODE], repeat_upto_number[MODE],
                     add_accord_number[MODE], add_accord_comment[MODE], panorame_precision[MODE], !use_percussion[MODE]);
    if (!res)
    {
      Mbox(L"Error in DaccordsToMidi() converter!");
      return;
    }

    // write the output midi file

    outfilew = infilew + L".mid"; // ��� ��������� �����
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

  infilew = argv[1]; // 2-� �������� ������ ��� �������� ����� ��� �����!
  string infile = infilew;

  // ���������� ���������� �����
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

  // ����� ���� ��������� � �������!

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

