
#pragma once

template<class T> T & un_volatile(volatile T &t) { return static_cast<T>( t ); } // ???

using namespace std;

const wchar_t UNI_HDR = 0xFEFF; // в начале текстового файла - маркер Unicode текста
const wchar_t UNI_CR = L'\r'; // в конце каждой строки - пара символов UNI_CR,UNI_LF
const wchar_t UNI_LF = L'\n';
const wchar_t UNI_TAB = L'\t';
const wchar_t UNI_SPACE = L' ';
const wchar_t UNI_NULL = L'\0';
extern const wchar_t *UNI_CRLF; // = L"\r\n"; // в конце каждой строки - пара символов UNI_CR,UNI_LF
extern const wchar_t *UNI_NULL_STR; // = L""; // широкая пустая строка

wstring get_textfile(const wchar_t *file);
wstring get_textfile_wstring(const wchar_t *file, int index);
bool get_textfile_wstring_array(wstring sarray[], int &nums, const wchar_t *file);

bool Unicode_save_wstring_array(wstring sarray[], int nums, const wchar_t *file);
bool Unicode_open_wifstream(wifstream &ifstr, const wchar_t *file);
bool Unicode_open_wofstream(wofstream &ofstr, const wchar_t *file);

string wchar2char(const wchar_t *wstr); // это синоним toNarrowString(wstr)
       string toNarrowString(const wchar_t *pStr, int len=-1);
inline string toNarrowString(const wstring &str) { return toNarrowString(str.c_str(), (int)str.length()); }
// inline string toNarrowString(const char *pStr, int len=-1) { return len<0? pStr:string(pStr, len); }

wstring char2wchar(const char *str); // это синоним toWideString(str)
       wstring toWideString(const char *pStr, int len=-1);
inline wstring toWideString(const string &str) { return toWideString(str.c_str(), (int)str.length()); }
// inline wstring toWideString(const wchar_t *pStr, int len=-1) { return len<0? pStr:wstring(pStr, len); }

// wstring2 в сокращенном виде для узких string
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

// это wstring с встроенным преобразованием к типу (const wchar_t *) и др.
// что позволяет напр. делать функции с аргументами (const wchar_t *)
// и не делать те же самые функции с аргументами wstring - экономия...
class wstring2 : public wstring // широкая строка с доп. возможностями ввода-вывода и др.
{
  string str; // str _всегда_ содержит узкую копию широкой строки

  // преобразование wstring wstr -> str (внутр. узкая копия шир. стр.)
  void wstr2str(const wstring &wstr) { str = toNarrowString(wstr); }
  wstring str2wstr() { return toWideString(str); } // преобразование str -> wstring

  void this2str() { wstr2str( *(wstring*)this ); } // this -> str
  void str2this() { *(wstring*)this = str2wstr(); } // преобр-е str -> базовая часть *this

  // преобразование в wstring некоторых типов: char, wchar_t, int и т.п. (но не const char [N])
  template<class T> wstring to_wstring(const T &t) { wstring wstr; wstr = t; return wstr; }

  // преобразование в wstring любых типов, через шаблонный конструктор wstring2
  template<class T> wstring to_wstring_2(const T &t) { return (wstring)wstring2(t); }

public:
  // ======= конструкторы - в основном через операторы копирования

  wstring2() {} // это wstring2():wstring() {}
  wstring2(const wstring  &wstr) { *this = wstr; }
  wstring2(const wstring2 &wstr2) { *this = wstr2; }
  wstring2(const char *str) { *this = str; }
  wstring2(      char *str) { *this = str; } // ?
  wstring2(const wchar_t *wstr) { *this = wstr; }
  wstring2(      wchar_t *wstr) { *this = wstr; } // ?
  wstring2(const string  &str)  { *this = str; }
  wstring2(const string2 &str2) { *this = (string) str2; }

  // шаблон для некот. др. типов аргументов
  template<class T> wstring2(const T &t) : wstring( to_wstring(t) ) { this2str(); }

  // ======= операторы сравнения

  // для родного типа - используем базовый оператор
  bool operator==(const wstring2 &t) const { return wstring( *this) == wstring( t ); }

  // для любых типов - преобразуем к родному типу
  template<class T> bool operator==(const T &t) const { return *this == wstring2( t ); }

  // ======= операторы копирования

  wstring2& operator=(const wstring2 &wstr2)
  {
    if (this == &wstr2) return *this; // самого себя не копируем
    // копируем _базовую_ часть источника путем преобразования типа указателей!!
    *(wstring*)this = *(wstring*)&wstr2;
    str = wstr2.str; // копируем узкую часть источника
    return *this;
  };

  wstring2& operator=(const wstring &wstr)
  {
    *(wstring*)this = wstr; // копируем источник в базовую часть объекта
    this2str(); // апдейт узкой части объекта
    return *this;
  };

  wstring2& operator=(const string &str)
  {
    this->str = str; // копируем источник в узкую часть объекта
    str2this(); // апдейт базовой части объекта
    return *this;
  };

  wstring2& operator=(const wchar_t *wch)
  {
    wstring wstr(wch); // используем источник в конструкторе широкой строки
    *this = wstr; // используем оператор копирования wstring2 = wstring 
    return *this;
  };

  wstring2& operator=(const char *ch)
  {
    string str(ch); // используем источник в конструкторе узкой строки
    *this = str; // используем оператор копирования wstring2 = string 
    return *this;
  };

  // ======= операторы преобразования типов

  operator wstring() const { return *(wstring*)this; } // wstring2 -> wstring
  operator string()  const { return str; } // wstring2 -> string
  operator const wchar_t *() const { return this->c_str(); } // wstring2 -> (const wchar_t *)
  operator const char *()    const { return str.c_str(); } // wstring2 -> (const char *)
  //  wstring2 -> произвольный тип T - используем копию *this, т.к. оператор >> меняет объект
  template<class T> operator T() const { T val; wstring2 wstr2(*this); wstr2 >> val ; return val; }

  // оператор вывода _из_ данной строки в тип аргумента шаблона, сам объект *this
  // постепенно "тает" по мере многократного вывода!

  template<class T> wstring2& operator>>(T &t)
  {
    t = T(); // если объект уже пуст - в t будет "введено" значение пустого конструктора T()
    wistringstream wis(*this);
    wis >> t;
    istream::pos_type n = wis.tellg(); // n = текущая позиция чтения потока
    // удаляем n начальных символов из базовой части *this
    if ( n != istream::pos_type(-1) ) *(wstring*)this = this->wstring::substr(n);
    // напр при ws=" 1 " после (ws >> x) в ws остается то что за 1-кой, т.е. один пробел,
    // а если ws="   1.1" то после (ws >> y) строка ws становится пустой!
    this2str(); // апдейт узкой части объекта!
    return *this;
  }
  // если T = volatile int, то в wistringstream видимо нет (??) оператора >>
  template<class T> wstring2& operator>>(volatile T &t)
  {
    T t2; // выводим в объект такого же типа, но без volatile
    *this >> t2;
    t = t2;
    return *this;
  }

  // оператор ввода _в_ данную строку: шаблон с произвольным аргументом!

  template<class T> wstring2& operator<<(const T &t)
  {
    wostringstream wostr;
    wostr << t;
    // не присваиваем, а прибавляем новый вывод к базовой части объекта!!!
    *(wstring*)this += wostr.str();
    // иначе не получится вывод нескольких аргументов в одном операторе!
    // точно также ведет себя сам класс wostringstream...
    // т.о. для обнуления строки надо присвоить ей пустую строку (UNI_NULL_STR)
    this2str(); // апдейт узкой части объекта!
    return *this;
  }
  wstring2& operator<<(const string &str) // уточнение для узких string - иначе не компиляет
  {
    wstring wstr = wstring2(str); // преобразуем источник -> wstring2 -> wstring
    *this << wstr; // используем шаблонный operator<<(const wstring &)
    return *this;
  }
/*
  // уточнение шаблона для C-строк, иначе раньше выводилось значение указателя - но теперь ok!!
  wstring2& operator<<(const char *ch)
  {
    wstring wstr = wstring2(ch); // преобразуем источник -> wstring2 -> wstring
    *this << wstr; // используем шаблонный operator<<(const wstring &)
    return *this;
  }
*/

  // оператор *this += t для произвольного типа правого аргумента T
  template<class T> wstring2& operator+=(const T &t)
  {
    // используем оператор += из базового класса wstring, а t преобразуем в wstring
    *(wstring*)this += to_wstring_2(t);
    // this->wstring::operator+=( to_wstring_2(t) ); // то же самое по-другому
    // to_wstring_2() использует шаблон конструктора wstring2, который использует
    // шаблонную функцию to_wstring(), но напрямую ставить ее - не дает компилер!
    this2str(); // апдейт узкой части объекта!
    return *this;
  }

  // оператор (wstring2) + (любой тип);  friend - не член класса - не меняет *this!

  template<class T> friend wstring2 operator+ (const wstring2 &wstr, const T &t)
  {
    wstring2 ws = wstr;
    ws += t;
    return ws;
  }
};

/*
  // т.о. возможны напр. такие wstring2-игрища с << и +=
  {
  wstring2 text, t1('A'), t2("t2");
  int val = 40;
  (text = "random ") << val; // тут нужны скобки, иначе = делается после <<
  text << " char1 " << val/20 << " char2 ";
  text += " !";
  scrbox(text, t1, t2);
  }
*/


/*
  взято из:
  Upgrading an STL-based application to use Unicode. By Taka Muraoka.
  http://www.codeproject.com/vcpp/stl/upgradingstlappstounicode.asp

  на основе статьи
  P. J. Plaugher just wrote about this subject in the
  "Standard C++" column of the C/C++ User's Journal.
  см. часть этой статьи в
  http://groups.google.com/group/comp.std.c++/msg/960feb01524a8f2d?hl=en&lr=&ie=UTF-8&oe=UTF-8

  см. также папку Development\Unicode\rsdn.ru\_wifstream on C++\codecvt\*.h/*.cpp
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

