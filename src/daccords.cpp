
#include "stdafx.h"

inline bool SetPan( MIDIMultiTrack *tracks, int track_num, MIDIClockTime ticks, int chan, double dpan )
// dpan = -1 ... 0 ... +1 (����, �����, �����)
{
    MIDITimedBigMessage m;
    m.SetTime( ticks );
    m.SetPan( chan, dpan );
    return tracks->GetTrack( track_num )->PutEvent( m );
}

static const wchar_t *VOICE_PAUSE = L"X"; // ������ ����� ������ � ����� �������
static const wchar_t *ACC_NUM_PREFIX = L";"; // ������� ����������� ������ ������� � ����� �������

void ConvertAccordsToString(const DaccordsFile &df, wostringstream &ws, int pan_precision,
                            int add_accord_number, int add_accord_comment)
// �������� ����������� ����� �������� df � �����-������ ws
{
  // ������ �������� � ������ ������ ����� � ��. ���. ��� Chain Speed
  ws.precision(3);
  ws.setf(ios_base::fixed, ios_base::floatfield);

  // � ����� ������ ����� - ������ Unicode ������, ����� 1-� ������ - ����� ������ ������� �����
  ws << UNI_HDR << df.daccords_header << UNI_SPACE
                << df.dflt_version
                << L"  Commentary:" << UNI_CRLF;

  // ����� 2-� ������ - �����������
  ws << df.comment << UNI_CRLF;

  // ����� 3-� ������ - �������� � �������� ����� ���������� ��������� ��������
  ws << L"Transposition " << df.ch.transposition
     << L"  Chain_Speed " << df.ch.chain_speed
     << L"  Dont_Change_GM_Instrument " << df.ch.dont_change_gm_instrument
     << L"  Instrument_Number " << df.ch.timbre_number << UNI_CRLF;

  // ����� 4-� ������ - ������������ �������� ���������� �������
  ws << L"Instrum" << UNI_TAB << L"Durat-n" << UNI_TAB << L"TempBPS" << UNI_TAB << L"Voices"
     << UNI_TAB << L"Volume" << UNI_TAB << L"note pan" << UNI_CRLF;

  ws.precision(pan_precision); // ������ �������� ������ ����� � ��. ���. ��� ��������

  // ����� ������� � 5-� ������ ���� �������� ��������� ������� ������� ���������
  for (int n = 0; n < df.accords_number; ++n)
  {
    DichoticAccord &acc = df.accords[n];
    ws << acc.timbre << UNI_TAB << acc.duration << UNI_TAB << acc.temp << UNI_TAB << acc.voices_number;
    // ���� ��� �� ����� ������� - ������� ��� ����/�������� �������
    if (acc.voices_number > 0)
    {
      ws << UNI_TAB << acc.volume << UNI_TAB;
      for (int i=0; i < acc.voices_number; ++i)
      {
        if ( acc.dn[i].pause ) // ���� ��� ����� ������
        {
          ws << UNI_SPACE << VOICE_PAUSE;
          continue;
        }
        // else ��� ����
        int nt = acc.dn[i].note;
        double p = acc.dn[i].pan;
        ws << UNI_SPACE << UNI_SPACE;
        if (10 > nt && nt >= 0) ws << UNI_SPACE;
        ws << nt << UNI_SPACE;
        ws << showpos << p << noshowpos; // ���������� ���� + ������ ��� ��������
      }
    }

    if ( add_accord_number ) // �������� �����?
    {
      // ����� ����� ������� ��������� ������, ������ � ����� �������, ������ ���������� � 1!
      ws << UNI_SPACE << ACC_NUM_PREFIX << (n+1);
    }

    if ( add_accord_comment == 1 ) // �������� ����������� ������� (����������)?
    {
      // ���� ���� �������� ����������� - ��������� ��� ����� ������
      if ( acc.ok_comment() ) ws << UNI_SPACE << acc.comment;
    }
    else
    if ( add_accord_comment == 2 ) // �������� ����������� ������� ������������?
    {
      if ( acc.ok_comment() )
      {
        // ��� ���������� ������ ������ ������� �������� ������� ������� ';'
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

  // ���������� ����� ����� � ������ � �������� ����� ��� ������ �����
  int file_bytes = get_file_length(file);
  Ar <int8> buf(file_bytes+2); // � ����� �� ����. ����. �������������� ����� ��� ���� ����!
  int8 *file_buf = buf.memory();

  // ������ ����
  int bytes_read = read_bin(file, file_buf, file_bytes);
  if (bytes_read != file_bytes)
  {
    errs |= 1; // ������: ���� ��� ������ �����
    return false;
  }
  // ����� ����� ����������� ������ ��������� ���� ������� ���� - ����� ����� ������� ������!
  file_buf[bytes_read] = file_buf[bytes_read+1] = 0;

  // ���������� ��������� ����� ����� � �����, ������ ��� 1 ������ �������� ������� 40 ����
  // (��� 20 w-�����), ���� ������ ������ ���� � 2-3 ������...    �.�. ���������� � �������!
  int max_file_strings = file_bytes / 40;

  // ��������� ��� �������� � ���� �� - ���������� ���������� �� ����������

  // ���������� ����� � ���� ������� ������ ������, Unicode ��������� ��������
  wchar_t *fstr = (wchar_t *)(file_buf+2);
  wchar_t *fstr_end = (wchar_t *)(file_buf+bytes_read); // ������ ������ �� ������ ������ fstr

  // ��������� ������� Unicode ���������
  if ( *(fstr-1) != UNI_HDR )
  {
    errs |= 2; // ������: �� Unicode ����
    return false;
  }

  // ��������� ������� daccords ���������
  if ( wcsncmp( fstr, daccords_header, wcslen(daccords_header) ) != 0 )
  {
    errs |= 4; // ������: �������� daccords header
    return false;
  }

  // �������� ��� ����� ����� UNI_CRLF �� ����, ������� ���������� ��������� �� ������ �����

  // ��������� �� ������ ����� (�� ���������� ������� CR/LF)
  Ar <wchar_t *> pstr(max_file_strings);

  int i;
  for (i = 0; i <= max_file_strings; ++i)
  {
    if (i == max_file_strings) // ���������� ����� ��������� ��������
    {
      max_file_strings += 500; // ���� 100
      pstr.expand_to_nums(max_file_strings);
    }
    pstr[i] = fstr;

/*
    // ���� ��� ����������� _�����_ �������� (�������������� ������� ������ ����������� � 10 ���)!
    wstring ws(fstr); // ����������� ��������� wchar_t ������� � ����� wstring ������
    size_t n = ws.find(UNI_CRLF);
    if (n == npos) break; // ����� ������ �� ������: ��� ��������� ������
    // else ������, ���������� ���� ���� � �������� ���������
    fstr[n] = fstr[n+1] = UNI_NULL;
    fstr = fstr + n + 2;
*/
    // � ���� ������������� ��� ����������� ������� �������
    wchar_t *endstr = wcschr(fstr, *UNI_CRLF);
    if (endstr == 0) break; // ����� ������ �� ������: ��� ��������� ������
    // else ������, ���������� ���� ���� � �������� ���������
    *endstr = UNI_NULL;
    fstr = endstr + 2;

    if (fstr >= fstr_end) break; // ��������� ����� ����������� �����
    // else ��� ���� ����������
  }

  // pstr[ADDS] ��� 1-� ������, ���������� ������ ���������� �������� � �����
  int num_accords = i-ADDS+1;

  // ��������������� "�����" ��������� �� ADDS ��������
  header.clear();
  for (int n = 0; n < ADDS; ++n)
  {
    header += pstr[n];
    if ( n != (ADDS-1) ) header += UNI_CRLF;
  }

  int version; // ������ �������, ����������� �� �����, ������ ��������� � dflt_version!

  // 1-� ������: ������ � ��������� ���������, ������ ������� � ����� ��������
  wstring header, str;
  wistringstream istr(pstr[0]);

  istr >> header >> version;
  if ( wcscmp(header.c_str(), daccords_header) != 0 // ��������� ��� ���: ����� �� ������� ������ ���� ������!
       || version != dflt_version
       || num_accords <= 0
       ) // �������� ���������, ����� ������ ������� ��� ��� �� ������ �������...
  {
    errs |= 8; // ������: �������� ������ �����
    return false;
  }

  comment = pstr[1]; // 2-� ������: ����������� �� ��������� �����!

  // 3-� ������: ������ ����� ��������� ��������� ��������
  wistringstream istr2(pstr[2]);
  istr2 >> str >> ch.transposition >> str >> ch.chain_speed >>
           str >> ch.dont_change_gm_instrument >> str >> ch.timbre_number;

  // 4-� ������: ������������ �������� ���������� ������� (�� ����������)

  // ��������� ������: ������ ��������� 1-�� � ������� ���������� ������� ���������
  // ������� ������ ������ �������� ������� ������� (��� �������� ���������)
  accords.renew(num_accords, false);
  for (i = 0; i < num_accords; ++i)
  {
    wistringstream is(pstr[i+ADDS]);
    DichoticAccord acc;

    is >> acc.timbre >> acc.duration >> acc.temp >> acc.voices_number;
    // �������� ����� ������� � �������� ��������� ������ ������ ���������
    mintestmax(0, acc.voices_number, DichoticAccord::MAX_ACC_VOICES);

    // ���� ��� �� ����� - ������ ��� ����/�������� �������
    if (acc.voices_number > 0)
    {
      is >> acc.volume;
      for (int i=0; i < acc.voices_number; ++i)
      {
        // ���������, ��� �� ����� (������ X) ������ ���� ������ i
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

    // ������ ����������� �������, ��������� �������� ������������� ����� ������� � ��������� ACC_NUM_PREFIX
    wstring s3, comm;
    // ������ � ������ s3 ���� �����, ��������� �� ������ ������� 
    is >> s3;
    // ���� ����� � s3 ���������� � �������� ������ �������, �� ����������� ����� � ��������� �����!
    if ( s3.c_str()[0] == *ACC_NUM_PREFIX ) is >> comm; // ������ ��������� �����
    else comm = s3; // ���� ���, �� ����� ������� �����������, � ����� � s3 - ��� � ���� �����������!

    // �������� comm � ����������� ������� � ��������� ����
    wcsncpy(acc.comment, comm.c_str(), DichoticAccord::COMMLEN-1);
    acc.comment[DichoticAccord::COMMLEN-1] = UNI_NULL;

    accords[i] = acc;
  }

  return true;
}

void DaccordsFile::OptimizeTransposition()
// ������ ������������ = ����������� ����� ����
{
  int minnote = 128;
  int phase = 0; // 0 ��� ������ ��������, 1 ��� ������������ ���

back:
  for (int i = 0; i < accords_number; ++i )
  {
    DichoticAccord &acc = accords[i];
    for (int n = 0; n < acc.voices_number; ++n )
      if (phase == 0)
      {
        int note = acc.dn[n].note;
        // ���������� ������������� ���� - ������� ������������!
        if ( !acc.dn[n].pause && note >= 0 ) minnote = min(minnote, note);
      }
      else
      {
        if ( !acc.dn[n].pause ) acc.dn[n].note -= minnote;
      }
  }
  // ���� �� ���� ������������ ��� - ��� � ������������
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
// ������� ������ � ������� ������������ �������� �������
{
  // �������� �������� ��������� �������
  vector <int> instrumen2(instrument);
  vector <double> panoram2(panorama);

  int chann, note, vel, instr = 0;
  double pan;
  wstring comment; // ����������� � �������

  // �������� midi ������ �� ����� ������� accord_events � ������ acc
  DichoticAccord acc;

  acc.duration = dtms;
  acc.temp = 1000;

  int size = (int) accord_events.size();
  int n = 0;
  int velsum = 0, velnum = 0, maxvel = 0;

  // ���� ������������ ������� �� ���� ������� ��� �� ������ ����� �� ������� end
  for (int i = 0; i < size; ++i)
  {
    MIDITimedBigMessage &ev = accord_events[i];
    if ( ev.IsUserAppMarker() ) // ������ ������ end
    {
      ev.SetNoOp(); // ������� ������ � ������� �� �����
      break;
    }

    if ( ev.IsLyricText() ) // ����� META_LYRIC_TEXT �������� � ����������� � �������
    {
      wstring2 wstr = ev.GetSysExString(); // ����������� ����� ������ � �������
      comment = wstr;
      ev.SetNoOp(); // ������� �������
    }

    if ( !ev.IsChannelEvent() ) continue;
    // else ��� ��������� �������

    chann = ev.GetChannel(); // ����� ������ �������

    // �������� ����� ��������� ��������
    if ( ev.IsProgramChange() ) instrumen2[chann] = ev.GetPGValue(); // ��������� ����������� � ������
    else if ( ev.IsPanChange() ) panoram2[chann] = ev.GetPan(); // ��������� �������� � ������
    else if ( ev.ImplicitIsNoteOn() ) // ������� ����
    {
      pan = panoram2[chann];
      note = ev.GetNote();
      vel = ev.GetVelocity();

      velsum += vel;
      ++velnum;
      // ���������� ���������� ����� ������� ������������ ���� �������
      if ( chann != MidiFile::CHANPERC )
      {
        if (maxvel < vel)
        {
          maxvel = vel;
          instr = instrumen2[chann];
        }
      }

      acc.dn[n].pause = 0;
      // ������ ���� �� "-" � ������ ���� ������ ������������
      acc.dn[n].note = chann==MidiFile::CHANPERC? -note:note;
      acc.dn[n].pan = pan;
      if ( ++n >= DichoticAccord::MAX_ACC_VOICES ) break;
    }
  }
  acc.voices_number = n;

  acc.set_comment( comment.c_str() ); // �������� ����������� � ������

  // ��� ������� ���������� ������� ��������� ���� ��� � ���������� ����� ������� ������������ ����
  acc.volume = (velnum > 0)? (velsum/velnum):0;
  acc.timbre = instr;

  // ��������� ������ acc � ������� �������� ������ accords
//  if (dtms > 0) 
  {
    int anum = accords.elements();
    // ���� ������ ��� ����� � ������� - ����������� ��� �� 100 ��������
    if ( accords_number >= anum ) accords.expand_to_nums( anum + 100 );
    accords[accords_number++] = acc;
//    cout << " accords_number " << accords_number << "  dtms " << dtms;
  }
  // ������� ������������ �������: ��� ������ ��� (dur < 0.5) && (accord_time_lag = 0.0) !
  // ��� ���� ��� ���� �������������, � �� ����� 10 �������� - ������ ����� ����� � dtms=0
  if (dtms <= 0) cout << " accord number " << accords_number << "  dtms " << dtms << endl;

  // ����� ������ ���������� ���� ��� � ������� ������ ���� �����

  for (int i = size-1; i >= 0; --i) // ��������� ���� �� ����� � ������
  {
    MIDITimedBigMessage &ev = accord_events[i];

    if ( ev.IsAllNotesOff() ) // ����� ���� ��� � ����� ������
    {
      chann = ev.GetChannel(); // ����� ������
      ev.SetNoOp(); // ������� ��� ������� �
      for (int j = i-1; j >= 0; --j) // ��� ����� ������ ������ ������� ��� ����� ������
      {
        MIDITimedBigMessage &ev = accord_events[j];
        // ������� �����. ������ �������
        if ( ev.IsNote() && chann == ev.GetChannel() )
        {
          ev.SetNoOp(); // ������� ��� �������
//          cout << "  del note " << note;
        }
      }
    }
    else
    if ( ev.ImplicitIsNoteOff() ) // ���������� ����� ����
    {
      note = ev.GetNote(); // ����� ���� �
      chann = ev.GetChannel(); // ����� ������ ���������� ����

      ev.SetNoOp(); // ������� ��� ������� �
      for (int j = i-1; j >= 0; --j) // ��� ����� ������ ������ ������� ��� ���� note/chann
      {
        MIDITimedBigMessage &ev = accord_events[j];
        // ������� �����. ������ �������
        if ( ev.IsNote() && note == ev.GetNote() && chann == ev.GetChannel() )
        {
          ev.SetNoOp(); // ������� ��� �������
//          cout << "  del note " << note;
        }
      }
    }
  }
//  cout << endl;

  // ������� pan/instr �� ������� ������� ���� ��������� � �������� ������� � �������
  for (int i = 0; i < size; ++i)
  {
    MIDITimedBigMessage &ev = accord_events[i];

    if ( ev.ImplicitIsNoteOn() ) break; // ������� ����

    chann = ev.GetChannel(); // ����� �������

    if ( ev.IsProgramChange() ) // ��������� ����������� � ������
    {
      instrument[chann] = ev.GetPGValue();
      ev.SetNoOp(); // ������� ��� �������
    }
    else if ( ev.IsPanChange() ) // ��������� �������� � ������
    {
      panorama[chann] = ev.GetPan();
      ev.SetNoOp(); // ������� ��� �������
    }
  }

  // ������� ���� �� ���� ������ �������: ��� �������� ����������� ������� ������ �� �������!
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
  // �� �� ����� ��� ����������� �� 2-� ������ - ������� �� 10-20% ���������
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
// ��������������� ����������� ������������ MidiFile � daccords ������
{
  const MIDIMultiTrack *mt = mfile.GetMultiTrack();
  // OutputMultiTrackAsText(*mt, cout); return false; // ���������� ����������� �����������

  // ���������� ���������� midi ������� �� ���� ������
  int num_events = mt->GetNumEvents();
  // ������ ������ ������� �������� �� ������� 1 ������� = 1 ������ (��� �����)
  accords.renew(num_events, true); // � ������� �������!
  // ������� ������ ��� ������
  accords_number = 0;

  // ������ ��������� �������
  ch.transposition = 0;
  ch.chain_speed = 1.0;
  ch.dont_change_gm_instrument = 0; // ���������� ����� ������� �� ������� �������
  ch.timbre_number = 0;

  // ������ ������������ ������ �������

  // ����� ���� ����������� � �������� ��� ������� midi ������  0...15
  vector <int> instrument(MidiFile::NUMCHANS, 0);
  vector <double> panorama(MidiFile::NUMCHANS, 0.);
  // ���� midi ������� ������� (� ������� ����� ������� ��������� �� ����� ��������� ������� �������)
  vector <MIDITimedBigMessage> accord_events; // ��� ����� ��������� �������, UserAppMarker ��� LyricText
  accord_events.reserve(256); // ����� ����������� ����� ����� �� 256 ���������


  // ����� ������ � ����� (������� ��� �����): ������ ������� ���������� �������, ��� ��������� �������
  // ������ � ���� (� �������� accord_time_lag) ��������� ��������������, ������ ������ �� ���� �������
  // ���������� ������, ��� ��������� ������� � ���� ��������� ��������������, ������ ������ �� ����
  // �������� ��� ���������� (start_time, end_time) ���� ������� � ����������� start_time �� end_time
  double start_time = 0., end_time = start_time;
  bool start = true; // ��������� ��������� - ������� ������������� � start_time

  MIDISequencer seq(mt);
  seq.GoToZero(); // ������ ����� �� ������ �����������!
  MIDITimedBigMessage ev; // ������� ���� �������
  int ev_track; // ����� ���� �����
  double event_time = 0; // ����� ������� � �������������

  // ����� ������������ �������� � ��������������� ������ � ����
  double srcall = 0., dstall = 0.;

  // �������-������ "����� �������", ����� �������� ����� ������� ��� �� ����� �������� � ������� �������
  // (��� ����� �������� � ��������� ���.)
  MIDITimedBigMessage end;
  end.SetUserAppMarker();
  bool ignore_percussion_notes = false; // true ���� ���� ������������ ������ ��������� � ������ �������

  while ( seq.GetNextEvent( &ev_track, &ev ) )
  {
    if ( ev.IsEndOfTrack() || ev.IsBeatMarker() ) continue;
    // else ����� ������ �������, ����� ����� ����� � ���������� ������� ����������

    event_time = seq.GetCurrentTimeInMs(); // ���������� ����� ���� �������

    if ( ev_track == 0 && ev.IsTrackName() ) // ����� META_TRACK_NAME � ����� 0 ��� ����������� daccords ������
    {
      wstring2 wstr = ev.GetSysExString(); // ����������� ����� ������ � �������
      comment = wstr;
    }

    if ( ev.IsLyricText() ) // ����� META_LYRIC_TEXT ��������� � ����������� � �������
    {
      accord_events.push_back( ev ); // ���������� ������� � ���� �������
      continue;
    }

    if ( !ev.IsChannelEvent() ) continue;
    // else ��� ��������� �������

    int chan = ev.GetChannel(); // ����� ���� ������
    if (ignore_percussion && chan == MidiFile::CHANPERC)
    {
      if ( ev.IsNote() ) ignore_percussion_notes = true;
      continue; // ���������� ����������� ����� �������
    }
    // else ��� ������������ ������

    if ( ev.IsProgramChange() || ev.IsPanChange() ) // ��������� ����������� ��� �������� � ������
    {
      accord_events.push_back( ev ); // ���������� ������� � ���� �������
      continue;
    }

    if ( !ev.IsAllNotesOff() && !ev.IsNote() )
    {
      // ��������� ��������� ������� - ���� ����������... ����� ����� ������ � aftertouch:
      // ��������� �������� �� ������� IsChannelPressure() ��� IsPolyPressure(), � �����
      // ���� ���� ���-��� �� Controller Events (0xB), ����. Legato Footswitch (0x44)
      continue;
    }
    // else ����� ���� ��� � ������ ��� �������������� ������ �������

    // � ������ ������ ������� ����������� � start_time (=0)
    if (start)
    {
      double dt = event_time - start_time;
      if (dt <= accord_time_lag) // ���� ������� � �������� ����������� �� start_time
      {
        accord_events.push_back( ev ); // ���������� ������ ������� � ���� �������
      }
      else // ������� ������: ����� start ����!
      {
        // ��������� � ����� �������� ������� � end_time
        accord_events.push_back( end ); // ���������� � ���� ������ "����� �������"
        accord_events.push_back( ev );  // ���������� ������ ������� � ���� �������
        // start_time �� ������!
        end_time = event_time; // ����� ������� �� ���� ������ �� ������ �������
        start = false;
      }
      continue;
    }
    // else ����������� ������� � end_time

    double dt = event_time - end_time;
    if (dt <= accord_time_lag) // ���� ������� � �������� ����������� �� end_time
    {
      accord_events.push_back( ev ); // ���������� ������ ������� � ���� �������
    }
    else // ������� ������: ���� �������� ������!
    {
      // ������ ������������ �������
      double dur = end_time - start_time;
      // �� �� ��������� �� 1 ���� � �������������� �� ����������� ���������� ��������
      int idur = float2int( dur + (srcall - dstall) );
      // �������� ��������� �����
      srcall += dur;
      dstall += idur;

      // ������� ������ � ������� "������������" �������...
      write_accord(idur, accord_events, instrument, panorama);

      // ��������� � ���������� ������� ��� ���������� �������
      accord_events.push_back( end ); // ���������� � ���� ������ "����� �������"
      accord_events.push_back( ev ); // ���������� ������ ������� � ���� �������
      start_time = end_time; // ������ ������ ������� = ����� �������
      end_time = event_time; // ����� ������� �� ���� ������ �� ������ �������
    }
  }

  // ������� [����]��������� ������:
  // ��� ��, ��� �������� � ����� ����� ������� end � ��������� ������ write_accord() �� �����
  {
    double dur = end_time - start_time;
    int idur = float2int( dur + (srcall - dstall) );
    srcall += dur;
    dstall += idur;
    write_accord(idur, accord_events, instrument, panorama);
  }

  // ������� �������������� �����: ��� ���������� ������� � ������� ���������� ��������� ���� ��
  // ���������� �� ������� �������, ������� �� �������� ������ ����� (�� end_time �� event_time)
  {
    double dur = event_time - end_time;
    int idur = float2int( dur + (srcall - dstall) );
    srcall += dur;
    dstall += idur;
    if (idur > 0) write_accord(idur, accord_events, instrument, panorama);
  }

  // ������� ��������� �����
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
  // ���� �� ����� ����� ��������� � ��������� Unicode
  wostringstream wss;

  ConvertAccordsToString(*this, wss, pan_precision, add_accord_number, add_accord_comment);

  // ��� ����� �������������� ������������� ���������� ��� ������� �������!
  wstring ws = wss.str();

  // ����� ������ � ������ ������ � ������ 
  const void *file_buf = ws.c_str();
  int file_bytes = sizeof(wchar_t)*ws.size();

  return write_bin(file, file_buf, file_bytes, true);
}

// ========================== class ===========================

bool MidiFile::Read(const wchar_t *file)
// ������ � ���������� midi ����
{
  wstring2 file2 = file;
  return ReadMidiFile(file2, *tracks);
}

bool MidiFile::Write(const wchar_t *file, int *midi_tracks_number) const
// ������ MIDIMultiTrack �� ���� � midi ����
{
  int tracks_number = tracks->GetNumTracksWithEvents();
  if (midi_tracks_number != 0)
    *midi_tracks_number = tracks_number;
  wstring2 file2 = file;
  return WriteMidiFile(*tracks, file2);
}

bool MidiFile::DaccordsToMidi(const DaccordsFile &dfile, double tick_time_msec, int add_daccords_header,
  int repeat_upto_number, int add_accord_number, int add_accord_comment, int pan_precision, int ignore_percussion)
// ��������������� ��������� �������� � midi �����, � ������ ������ ���������� true
// ���� pan_precision = 0 �� ������������ ������ 3 �������� �������� � ����������� �� -1, 0, +1
// ���� pan_precision > 0 �� �� 16 ����� �������� �������� � ����� �����������-������������� �������
{
  // ������ ������ ����� ������ � ������� ��
  int tracks_number = pan_precision == 0? 4:17;
  tracks->ClearAndResize( tracks_number );

  // ���������� ���� ����� � ����� �������� (��� ����� ������ ���� � �������� 1...32767)
  double clksperbeat = 1000./tick_time_msec;
  // ��� tempo = 1e6 ���������� ������� (1 midi ���) ����� = (1000/clksperbeat) �����������
  // �.�. tick_time_msec = 1 ����/��� ��� clksperbeat=1000, � 10 ����/��� ��� clksperbeat=100
  // ������ ������� � 100, �� ��� ����� ������� ������������ ����� 1000, �.�. ������������ ��������� ���
  // ����� ���������� ����� 50 ����������� � 10-���� �� ��� ���� ����������� ���� ������������ � 20%...
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

  int main_track = 0; // ����� ����� ��� ���� ��-��������� ���������
  int chan_track[NUMCHANS]; // ������ "������" ������ ��� ������� 0...15
  for (int i = 0; i < NUMCHANS; ++i)
  {
    int tr = i+1; // 1...16
    tr = (tr < tracks_number)? tr:(tracks_number-1);
    chan_track[i] = tr;
  }

  time = 0;
  m.SetTime( time ); // ������� ����� ��� ���� ������� �����!

  if ( add_daccords_header )
  {
    // ��������� ����� ��������� daccords ����� � midi ���� ��� META_SEQUENCER_SPECIFIC event,
    // ������ � ���� char ������, ���� ���� �� ������� ������ 1-3 ����� ��� ID �������������
    // (manufacturer's ID), � ����� ������ ��������� ��� Unicode �����...
    wstring2 header = dfile.header_body();
    tracks->GetTrack( main_track )->PutTextEvent(time, META_SEQUENCER_SPECIFIC, header);
  }

  // ���� ���� - ��������� ����������� �� ��������� daccords ����� � midi ����, � ���� char ������...
  wstring2 comment = dfile.header_comment();
  if ( !comment.empty() )
  {
    // ����� META_TRACK_NAME � ����� 0 �������� ���������� ��� ��������� ������������ ������������!
    tracks->GetTrack( main_track )->PutTextEvent(time, META_TRACK_NAME, comment); // wstring2 -> char!
  }

  // ���������� ������ ����� 4/4 (������ ��������), ��� ������ ����� �� ���������
  // m.SetTimeSig( 4, 2 ); // ����������� 4 = 2^2!
  // tracks->GetTrack( main_track )->PutEvent( m );

  m.SetTempo( tempo );
  tracks->GetTrack( main_track )->PutEvent( m );

  // ��� ��������� ��-������ ������� ���������� � ������ �����: ���. �������� � ������!
  // ��� �������� ��������� ������������� ����� ��� �����!

  // ������ ������� �������� ������� �� ����-�������
  vector<double> chan_pan(NUMCHANS, 123.); // �������� ��������� �������� ����� ���������������!
  // ������ "���������" ������� �������� ������� �� ����-�������: true ��� ������ ���������
  vector<bool> chan_pan_changed(NUMCHANS, false);

  // ������ ��� 3-� �������� (�����) ��������: ����� ����� ��������� ������ � ��������� ��������
  if (pan_precision == 0)
  {
    // ����� META_TRACK_NAME � ����� > 0 �������� ���������� ��� ����� ��������� ������ ���������!
    // ����� META_LYRIC_TEXT � ����� > 0 �������� ���������� ��� ����������� ����� ������ ���������!
    tracks->GetTrack( chan_track[0] )->PutTextEvent(time, META_TRACK_NAME, "Left");
    tracks->GetTrack( chan_track[1] )->PutTextEvent(time, META_TRACK_NAME, "Centre");
    tracks->GetTrack( chan_track[2] )->PutTextEvent(time, META_TRACK_NAME, "Right");
    // ��������� ��������
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

  // ������ �������� ������ ����������� � ������ �� ������������ �������
  vector<int> chan_instr(NUMCHANS, 12345); // �������� ��������� �������� ����� ���������������!

  // ������ ������� ������� � ���������� ��� � ���������� �� � �����

  bool last_pause = false; // ���� true, �� ��������� ������ ��� ������!
  time = 0;

  while ( --repeat_upto_number >= 0 ) // ��������� ������ repeat_upto_number ���
  {
    for (int n = 0; n < num_accords; ++n)
    {
      DichoticAccord &acc = acc_chain[n];
      // ������������ ������� � ��������, � ������ chain_speed
      double dur_sec = acc.duration/(ch.chain_speed * (double)acc.temp);
      // ������������ ������� � midi-�����
      MIDIClockTime dur_tick = seconds2ticks( dur_sec );

      if (acc.voices_number == 0) // ���� ������ - ��� ����� 
      {
        last_pause = true;
        time += dur_tick;
      }
      else // ������ - �� ����� - ������� ��� ����/�������� �������
      {
        int note;
        last_pause = false;
        m.SetTime( time );

        // ������ ��������� ������� ������������� ������ (��� ������ ������ ��� ����� ���� � ����� ���������)
        vector<bool> chan_busy(NUMCHANS, false); // 0 = ����� ��������, 1 = �����
        chan_busy[CHANPERC] = 1; // ����� ������� ������ "�����" ��� ������������ ���, ������������ ������ ��� �������!

        // ������ ������ ��� ���������� ����������� �� 1 ���� �� ������ ����� ��� �������
        vector <bool> ok_add_accord_text(NUMCHANS, false);

        // ���� ������� ���

        for (int i = 0; i < acc.voices_number; ++i)
        {
          if ( acc.dn[i].pause ) continue; // ����� ������ - ����������
          // else ��� �� ����� ������
          note = ch.transposition + acc.dn[i].note;

          // �������� ����
          pan = acc.dn[i].pan;
          // ��������� �������� �� 3-� �����
          if (pan_precision == 0) pan = round_pan( pan );

          if ( note < 0 ) // ������� ���� ������� ������������
          {
            if ( ignore_percussion ) continue; // ���� ��������� ��������� - ����������
            chan = CHANPERC;
            note = abs(note);
            // ��� ��������� �������� ������� ��������� �������������� �������!
            if (chan_pan[chan] != pan)
            {
              chan_pan[chan] = pan;
              SetPan( tracks, chan_track[chan], time, chan, chan_pan[chan] );
            }
            goto noteon; // ��� ����� �� ������� ����
          }

          // ���������� ����� ������������ ���� �������, �������� �������� ����
          {
          // ���� �����, � ������� ��� ����������� ��������, ����������� � ��������� ������� ����
          int ip;
          for (ip = 0; ip < NUMCHANS; ++ip)
            if ( ip != CHANPERC && // �� ���������� ����� �������!
                 chan_pan[ip] == pan ) break;

          if ( ip >= NUMCHANS ) // ����� �������� �� �������, ���� ������ �����!
          {
            // ���� �����, � ������� ��� �� �������� �������� � �� ������ �� ����� ����
            int ic;
            for (ic = 0; ic < NUMCHANS; ++ic)
              if ( !chan_pan_changed[ic] && !chan_busy[ic] ) // (����� ������� ������ "�����")
                break;

            if ( ic >= NUMCHANS ) // ����� ����� �� ������, ����� �������� ������ ��������
            {
              // ���� ������ ����� ��������� ����� (����� ������� ������ "�����")
              ic = find( chan_busy.begin(), chan_busy.end(), false ) - chan_busy.begin();
              if (ic >= NUMCHANS)
              {
                // ������ �� ����� ����: ����� ������������-������ ������� ���������� ������ �������!
                cout << "\nMIDI warnings 1: all midi channels busy, Note Pan not right!\n" << endl;
                chan = 0; // ���� �����-�� �����...
              }
              else // ic = ������ ������
                chan = ic;
            }
            else // ����� ����� ������, ic = ������ ������
              chan = ic;

            // �������� ����� ����� � ������������� ������ �� ��������
            chan_busy[chan] = 1;
            chan_pan[chan] = pan;
            SetPan( tracks, chan_track[chan], time, chan, chan_pan[chan] );
            chan_pan_changed[chan] = 1;
          }
          else // �������� �������, ���������� �����-� �� �����
            chan = ip;
          }

          // ���������� ���������� ������������ ���� ������� � �������� ��� ����� � ������
          {
          int instr = ch.dont_change_gm_instrument? ch.timbre_number:acc.timbre;
          if ( chan_instr[chan] != instr )
          {
            chan_instr[chan] = instr;
            m.SetProgramChange( chan, chan_instr[chan] );
            tracks->GetTrack( chan_track[chan] )->PutEvent( m );
          }
          }

noteon:   // �������� ���� � ���������� � ����� - ��� ������������ ����������!

          acc.dn[i].spare1 = chan;
          m.SetNoteOn(chan, note, acc.volume);
          tracks->GetTrack( chan_track[chan] )->PutEvent( m );

          // � ���������� ����� �������� � ����������� ��� (��� �����):
          // ��������� ����� ������� �/��� ����������� � ���� ��� ����� �����
          if ( add_accord_number || add_accord_comment )
          {
            // ������ �� ������ ���� � ������ ���� ������ �� ���� ������
            if ( !ok_add_accord_text[chan] )
            {
              ok_add_accord_text[chan] = true;
              wstring2 text;
              if ( add_accord_number ) text << (n+1);

              if ( add_accord_comment == 1 ) // ����������� ����� ��������
              {
                // ���� ������� ����� �� ����� ���� ������ ������
                if ( !text.empty() ) text += UNI_SPACE;
                text << acc.comment;
              }
              else
              if ( add_accord_comment == 2 )  // ������������� ����� ��������
              {
                text << acc.comment; // ��� �������
              }

              tracks->GetTrack( chan_track[chan] )->PutTextEvent(time, META_LYRIC_TEXT, text);
            }
          }
        }

        // ���� ���������� ���

        time += dur_tick;
        m.SetTime( time );
        for (int i=0; i < acc.voices_number; ++i)
        {
          if ( acc.dn[i].pause == 0 ) // ��� �� ����� ������
          {
            note = ch.transposition + acc.dn[i].note;
            // ��������� ����� ������, � ������� ����� ���� ������ ��� ����
            chan = acc.dn[i].spare1;
            if (chan == CHANPERC) note = abs(note); // ���� ������� ������������
            m.SetNoteOff(chan, note, acc.volume);
            tracks->GetTrack( chan_track[chan] )->PutEvent( m );
          }
        }
      }
    }
  }

  // ���� ��������� �������� ���� ����� - ���� ��������� "�������" ����� ���� �� ��������� ������!
  if (last_pause)
  {
    // ��������� ������� ���� � ������� ���������
    m.SetTime( time );
    m.SetNoteOn(chan = 0, 0, 0);
    tracks->GetTrack( chan_track[chan] )->PutEvent( m );
  }

  cout << "Music duration in sec = " << GetMisicDurationInSeconds(*tracks) << endl;

  return true;
};

