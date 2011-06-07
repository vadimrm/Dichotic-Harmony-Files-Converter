/*
  wchar_t ������ ������ �������� �������...
*/

#pragma once

const int FILE_STRLEN = MAX_PATH+64; // 260+64

bool file_exist(const wchar_t *file);
int  get_file_length(const wchar_t *file); // if error return 0!
int  read_bin(const wchar_t *file, void *buffer, int maxbuflen);
bool read_bin_ex(const wchar_t *file, void *buffer, int &maxbuflen);

// ������ ����� ����� �� �������� ��������, ���������� ����� ����������� ����
int  read_bin_offset(const wchar_t *file, void *buffer, int maxbuflen, int offset);

// ���� create=1 - ������� � ����� � ����, ����� ���� ��� ������ ������������!
bool write_bin(const wchar_t *file, const void *buffer, int buflen, bool create);

// ������� ���� � ����, � ���� �� ���� - ���������� � ���� ������� ���������� � �����
bool write_bin_append(const wchar_t *file, const void *buffer, int buflen);
// ��� write_bin_append() � ����������� ������������� ������� � ����� �� ������/������
bool write_bin_append2(const wchar_t *file, const void *buffer, int buflen);

// ���������� ����� ����� �� �������� �������� �� ������, ���� ������ ������������!
// ����� ������ � ����� ����� �����, � �.�. � ����������� ��� ��������-�� �������,
// ����������� ��� ���� ����� ����������� ������ (���� �������� ������ ����� �����)
bool write_bin_offset(const wchar_t *file, const void *buffer, int buflen, int offset);

// ==========================================================================

class FILEopen // ������� "�����������" ����� � �������������
{
  FILE *pfile;
public:
  FILEopen(const wstring2 filename, const wstring2 mode) { _wfopen_s(&pfile, filename, mode); }
  // ���������� ��������� ����, ��� ����� ������� � �� ����������� �������: (*this).~FILEopen();
  ~FILEopen() { if (pfile) fclose( pfile ); pfile = NULL; }
  operator FILE*() { return pfile; }  // ������ � *pfile  ��� ������ (FILE*)(*this)
  FILE *operator->() { return pfile; } // ������ � pfile-> ��� ������ (*this)->
  // ���� ������ ��� ������, ���� (*this != 0) ��� ( this->openok() == true )
  bool openok() { return pfile != NULL; }
};

class FILEopen2 // FILEopen � ������������� ������� � �����
{
  FILE *pfile;
public:
/*
The argument shflag is a constant expression consisting of one of the following constants, defined in Share.h:
_SH_COMPAT Sets Compatibility mode for 16-bit applications.
_SH_DENYNO Permits read and write access.
_SH_DENYRD Denies read access to the file.
_SH_DENYRW Denies read and write access to the file.
_SH_DENYWR Denies write access to the file.
*/
  FILEopen2(const wstring2 filename, const wstring2 mode, int shflag) { pfile = _wfsopen(filename, mode, shflag); }
  // ���������� ��������� ����, ��� ����� ������� � �� ����������� �������: (*this).~FILEopen();
  ~FILEopen2() { if (pfile) fclose( pfile ); pfile = NULL; }
  operator FILE*() { return pfile; }  // ������ � *pfile  ��� ������ (FILE*)(*this)
  FILE *operator->() { return pfile; } // ������ � pfile-> ��� ������ (*this)->
  // ���� ������ ��� ������, ���� (*this != 0) ��� ( this->openok() == true )
  bool openok() { return pfile != NULL; }
};

// ==========================================================================

class MFile // �������� - ���� �� ����� ��� ��� ����� � RAM (��������� �����)
{
  bool disk_file; // true ���� ���� �� �����, ����� ���� � RAM
  FILE *file; // �������� ���������
  uint8 *memory; // ������ ������ ����� � RAM
  int memlength; // ���������� ���� � RAM ������
  int memoffset; // ����� �������� ����� RAM �� ������ memory
  int mem_eof; // !0 ����� ������ ������� �������� �� ������ memory, ����� 0

public:
  MFile(FILE *dfile); // ����������� ������� ��� ���������� ��������� �����
  MFile(uint8 *ramemory, int ramemlength); // �� ��, ��� ������ ����� �� RAM

  // ������� ������ 1, 2 ��� 4-� ���� �� ���������
  uint8   read_uint8();
  uint32 read_uint16();
  uint32 read_uint32();

  int eof(); // ������ feof()
  int seek(int offset, int origin); // ������� fseek()
};

// ==========================================================================

// ������ ���� � ����� ����� � ������������, ������ ���������� � ��� �������
class BinFile
{
  Ar <uint8> buf; // ������ ���� - ���������� ����� � �����
  int len; // ����� ����� - ����, �.�. ������ ������� Ar ������!
  wstring2 file; // ��� ����� �� ������������
  BinFile &operator=(const BinFile &) {}; // ����������� �� warning C4512: assignment operator could not be generated

public:
  const int &length; // ���������� �������� ���� � ��������
  uint8 *content() const { return (len==0)? 0:buf.memory(); }
  // �������� ���������� ������������� ��. TurnSecureCode()
  BinFile(wstring2 filename, bool secure = false, int key = 0, int width = 32, bool key_from_numbytes = false)
         : length(len)
  {
    file = filename;
    len = get_file_length(file);
    if (len <= 0) { len = 0; return; }
 
    // ��������� � ����� ������ 2 ������� ����� - ��� ���� ��������� ������!
    buf.renew(len+2);
    read_bin(file, buf.memory(), len);
    buf[len] = buf[len+1] = 0;

    // ������������ ����������� ������ ���� ����
    if ( secure ) turn_secure_state(key, width, key_from_numbytes);
  }

  // ������������� ����������� �� ����� ������
  bool turn_secure_state(int key = 0, int width = 32, bool key_from_numbytes = false)
  {
    return TurnSecureCode(buf.memory(), len, key, width, key_from_numbytes);
  }

  // ������ ����� �� ���� - ������ ���������
  bool save() { return write_bin(file, buf.memory(), len, true); }
};


// ��������� ���� ������ ���� - Unicode ��� ANSI - �������� � ������� ������,
// ������ ���������� ����� CR/LF �� ������������!
class TextFile
{
  wstring file; // ������� ������-����� ������ �� �����
  bool uni; // true ��� Unicode, false ��� ANSI �����
  size_t len; // ����� ������
  static const wchar_t UNI_HDR = 0xFEFF; // ������ Unicode ������ � ������ �����
  TextFile &operator=(const TextFile &) {}; // ����������� �� warning C4512: assignment operator could not be generated

public:
  const size_t &length; // ���������� ����� � ��������
  const bool &is_unicode; // true ��� Unicode, false ��� ANSI �����
  const wstring &content; // ������� ������-����� ������ �� �����
  // �������� ���������� ������������� ��. TurnSecureCode()
  TextFile(wstring2 filename, bool secure = false, int key = 0, int width = 32, bool key_from_numbytes = false)
          : is_unicode(uni), content(file), length(len)
  {
    file.clear();
    uni = false;
    BinFile df(filename, secure, key, width, key_from_numbytes);
    len = df.length;
    if (len <= 0) { len = 0; return; }

    // ��������� ��� ������ - � ����������� ������ ���� ����� � ������ �����
    if (len%2 == 0 && len > 2)
    {
      wchar_t *text = (wchar_t *) df.content();
      if ( text[0] == UNI_HDR )
      {
        uni = true;
        file = &text[1]; // ���������� �����!
      }
    }

    if (uni == false)
    {
      string text = (char *)df.content();
      file = wstring2( text );
    }

    len = file.size();
  }
};

