
#pragma once

template<class T> T & un_volatile(volatile T &t) { return static_cast<T>( t ); } // ???

using namespace std;

const wchar_t UNI_HDR = 0xFEFF; // � ������ ���������� ����� - ������ Unicode ������
const wchar_t UNI_CR = L'\r'; // � ����� ������ ������ - ���� �������� UNI_CR,UNI_LF
const wchar_t UNI_LF = L'\n';
const wchar_t UNI_TAB = L'\t';
const wchar_t UNI_SPACE = L' ';
const wchar_t UNI_NULL = L'\0';
extern const wchar_t *UNI_CRLF; // = L"\r\n"; // � ����� ������ ������ - ���� �������� UNI_CR,UNI_LF
extern const wchar_t *UNI_NULL_STR; // = L""; // ������� ������ ������

wstring get_textfile(const wchar_t *file);
wstring get_textfile_wstring(const wchar_t *file, int index);
bool get_textfile_wstring_array(wstring sarray[], int &nums, const wchar_t *file);

bool Unicode_save_wstring_array(wstring sarray[], int nums, const wchar_t *file);
bool Unicode_open_wifstream(wifstream &ifstr, const wchar_t *file);
bool Unicode_open_wofstream(wofstream &ofstr, const wchar_t *file);

string wchar2char(const wchar_t *wstr); // ��� ������� toNarrowString(wstr)
       string toNarrowString(const wchar_t *pStr, int len=-1);
inline string toNarrowString(const wstring &str) { return toNarrowString(str.c_str(), (int)str.length()); }
// inline string toNarrowString(const char *pStr, int len=-1) { return len<0? pStr:string(pStr, len); }

wstring char2wchar(const char *str); // ��� ������� toWideString(str)
       wstring toWideString(const char *pStr, int len=-1);
inline wstring toWideString(const string &str) { return toWideString(str.c_str(), (int)str.length()); }
// inline wstring toWideString(const wchar_t *pStr, int len=-1) { return len<0? pStr:wstring(pStr, len); }

// wstring2 � ����������� ���� ��� ����� string
class string2 : public string
{
public:
  string2() {}
  string2(const char *ch) : string(ch) {}
  string2(const string &str): string(str) {}
  string2(const wchar_t *wch) : string(toNarrowString(wch)) {}
  string2(const wstring &wstr): string(toNarrowString(wstr)) {}

  operator string() const { return *this; }               // string2 -> string
  operator const char *() const { return this->c_str(); } // string2 -> (const char *)
};

// ��� wstring � ���������� ��������������� � ���� (const wchar_t *) � ��.
// ��� ��������� ����. ������ ������� � ����������� (const wchar_t *)
// � �� ������ �� �� ����� ������� � ����������� wstring - ��������...
class wstring2 : public wstring // ������� ������ � ���. ������������� �����-������ � ��.
{
  string str; // str _������_ �������� ����� ����� ������� ������

  // �������������� wstring wstr -> str (�����. ����� ����� ���. ���.)
  void wstr2str(const wstring &wstr) { str = toNarrowString(wstr); }
  wstring str2wstr() { return toWideString(str); } // �������������� str -> wstring

  void this2str() { wstr2str( *(wstring*)this ); } // this -> str
  void str2this() { *(wstring*)this = str2wstr(); } // ������-� str -> ������� ����� *this

  // �������������� � wstring ��������� �����: char, wchar_t, int � �.�. (�� �� const char [N])
  template<class T> wstring to_wstring(const T &t) { wstring wstr; wstr = t; return wstr; }

  // �������������� � wstring ����� �����, ����� ��������� ����������� wstring2
  template<class T> wstring to_wstring_2(const T &t) { return (wstring)wstring2(t); }

public:
  // ======= ������������ - � �������� ����� ��������� �����������

  wstring2() {} // ��� wstring2():wstring() {}
  wstring2(const wstring  &wstr) { *this = wstr; }
  wstring2(const wstring2 &wstr2) { *this = wstr2; }
  wstring2(const char *str) { *this = str; }
  wstring2(      char *str) { *this = str; } // ?
  wstring2(const wchar_t *wstr) { *this = wstr; }
  wstring2(      wchar_t *wstr) { *this = wstr; } // ?
  wstring2(const string  &str)  { *this = str; }
  wstring2(const string2 &str2) { *this = (string) str2; }

  // ������ ��� �����. ��. ����� ����������
  template<class T> wstring2(const T &t) : wstring( to_wstring(t) ) { this2str(); }

  // ======= ��������� ���������

  // ��� ������� ���� - ���������� ������� ��������
  bool operator==(const wstring2 &t) const { return wstring( *this) == wstring( t ); }

  // ��� ����� ����� - ����������� � ������� ����
  template<class T> bool operator==(const T &t) const { return *this == wstring2( t ); }

  // ======= ��������� �����������

  wstring2& operator=(const wstring2 &wstr2)
  {
    if (this == &wstr2) return *this; // ������ ���� �� ��������
    // �������� _�������_ ����� ��������� ����� �������������� ���� ����������!!
    *(wstring*)this = *(wstring*)&wstr2;
    str = wstr2.str; // �������� ����� ����� ���������
    return *this;
  };

  wstring2& operator=(const wstring &wstr)
  {
    *(wstring*)this = wstr; // �������� �������� � ������� ����� �������
    this2str(); // ������ ����� ����� �������
    return *this;
  };

  wstring2& operator=(const string &str)
  {
    this->str = str; // �������� �������� � ����� ����� �������
    str2this(); // ������ ������� ����� �������
    return *this;
  };

  wstring2& operator=(const wchar_t *wch)
  {
    wstring wstr(wch); // ���������� �������� � ������������ ������� ������
    *this = wstr; // ���������� �������� ����������� wstring2 = wstring 
    return *this;
  };

  wstring2& operator=(const char *ch)
  {
    string str(ch); // ���������� �������� � ������������ ����� ������
    *this = str; // ���������� �������� ����������� wstring2 = string 
    return *this;
  };

  // ======= ��������� �������������� �����

  operator wstring() const { return *(wstring*)this; } // wstring2 -> wstring
  operator string()  const { return str; } // wstring2 -> string
  operator const wchar_t *() const { return this->c_str(); } // wstring2 -> (const wchar_t *)
  operator const char *()    const { return str.c_str(); } // wstring2 -> (const char *)
  //  wstring2 -> ������������ ��� T - ���������� ����� *this, �.�. �������� >> ������ ������
  template<class T> operator T() const { T val; wstring2 wstr2(*this); wstr2 >> val ; return val; }

  // �������� ������ _��_ ������ ������ � ��� ��������� �������, ��� ������ *this
  // ���������� "����" �� ���� ������������� ������!

  template<class T> wstring2& operator>>(T &t)
  {
    t = T(); // ���� ������ ��� ���� - � t ����� "�������" �������� ������� ������������ T()
    wistringstream wis(*this);
    wis >> t;
    istream::pos_type n = wis.tellg(); // n = ������� ������� ������ ������
    // ������� n ��������� �������� �� ������� ����� *this
    if ( n != istream::pos_type(-1) ) *(wstring*)this = this->wstring::substr(n);
    // ���� ��� ws=" 1 " ����� (ws >> x) � ws �������� �� ��� �� 1-���, �.�. ���� ������,
    // � ���� ws="   1.1" �� ����� (ws >> y) ������ ws ���������� ������!
    this2str(); // ������ ����� ����� �������!
    return *this;
  }
  // ���� T = volatile int, �� � wistringstream ������ ��� (??) ��������� >>
  template<class T> wstring2& operator>>(volatile T &t)
  {
    T t2; // ������� � ������ ������ �� ����, �� ��� volatile
    *this >> t2;
    t = t2;
    return *this;
  }

  // �������� ����� _�_ ������ ������: ������ � ������������ ����������!

  template<class T> wstring2& operator<<(const T &t)
  {
    wostringstream wostr;
    wostr << t;
    // �� �����������, � ���������� ����� ����� � ������� ����� �������!!!
    *(wstring*)this += wostr.str();
    // ����� �� ��������� ����� ���������� ���������� � ����� ���������!
    // ����� ����� ����� ���� ��� ����� wostringstream...
    // �.�. ��� ��������� ������ ���� ��������� �� ������ ������ (UNI_NULL_STR)
    this2str(); // ������ ����� ����� �������!
    return *this;
  }
  wstring2& operator<<(const string &str) // ��������� ��� ����� string - ����� �� ���������
  {
    wstring wstr = wstring2(str); // ����������� �������� -> wstring2 -> wstring
    *this << wstr; // ���������� ��������� operator<<(const wstring &)
    return *this;
  }
/*
  // ��������� ������� ��� C-�����, ����� ������ ���������� �������� ��������� - �� ������ ok!!
  wstring2& operator<<(const char *ch)
  {
    wstring wstr = wstring2(ch); // ����������� �������� -> wstring2 -> wstring
    *this << wstr; // ���������� ��������� operator<<(const wstring &)
    return *this;
  }
*/

  // �������� *this += t ��� ������������� ���� ������� ��������� T
  template<class T> wstring2& operator+=(const T &t)
  {
    // ���������� �������� += �� �������� ������ wstring, � t ����������� � wstring
    *(wstring*)this += to_wstring_2(t);
    // this->wstring::operator+=( to_wstring_2(t) ); // �� �� ����� ��-�������
    // to_wstring_2() ���������� ������ ������������ wstring2, ������� ����������
    // ��������� ������� to_wstring(), �� �������� ������� �� - �� ���� ��������!
    this2str(); // ������ ����� ����� �������!
    return *this;
  }

  // �������� (wstring2) + (����� ���);  friend - �� ���� ������ - �� ������ *this!

  template<class T> friend wstring2 operator+ (const wstring2 &wstr, const T &t)
  {
    wstring2 ws = wstr;
    ws += t;
    return ws;
  }
};

/*
  // �.�. �������� ����. ����� wstring2-������ � << � +=
  {
  wstring2 text, t1('A'), t2("t2");
  int val = 40;
  (text = "random ") << val; // ��� ����� ������, ����� = �������� ����� <<
  text << " char1 " << val/20 << " char2 ";
  text += " !";
  scrbox(text, t1, t2);
  }
*/


/*
  ����� ��:
  Upgrading an STL-based application to use Unicode. By Taka Muraoka.
  http://www.codeproject.com/vcpp/stl/upgradingstlappstounicode.asp

  �� ������ ������
  P. J. Plaugher just wrote about this subject in the
  "Standard C++" column of the C/C++ User's Journal.
  ��. ����� ���� ������ �
  http://groups.google.com/group/comp.std.c++/msg/960feb01524a8f2d?hl=en&lr=&ie=UTF-8&oe=UTF-8

  ��. ����� ����� Development\Unicode\rsdn.ru\_wifstream on C++\codecvt\*.h/*.cpp
*/

typedef codecvt<wchar_t, char, mbstate_t> NullCodecvtBase;

class NullCodecvt:public NullCodecvtBase
{
public:
  typedef wchar_t   _E;
  typedef char      _To;
  typedef mbstate_t _St;

  explicit NullCodecvt(size_t _R = 0):NullCodecvtBase(_R) {}

protected:
  virtual result do_in(_St& _State, const _To *_F1, const _To *_L1, const _To *&_Mid1,
                       _E *F2, _E *_L2, _E *&_Mid2) const { return noconv; }
  virtual result do_out(_St& _State, const _E *_F1, const _E *_L1, const _E *&_Mid1,
                        _To *F2, _E *_L2, _To *&_Mid2) const { return noconv; }
  virtual result do_unshift(_St& _State, _To *_F2, _To *_L2, _To *&_Mid2) const { return noconv; }
  virtual int do_length(_St& _State, const _To *_F1, const _To *_L1, size_t _N2) const _THROW0()
              { return (int)(_N2 < (size_t)(_L1 - _F1)? _N2 : _L1 - _F1); }
  virtual bool do_always_noconv() const _THROW0() { return true; }
  virtual int do_max_length() const _THROW0() { return 2; }
  virtual int do_encoding() const _THROW0() { return 2; }
};

