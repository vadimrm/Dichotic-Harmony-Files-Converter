
#include "stdafx.h"
#include "stdafx2.h"

const wchar_t *UNI_CRLF = L"\r\n"; // в конце каждой строки - пара символов UNI_CR,UNI_LF
const wchar_t *UNI_NULL_STR = L""; // широкая пустая строка

string wchar2char(const wchar_t *wstr)
// конвертация Unicode wchar_t строки в string char строку
{
//*
  return toNarrowString(wstr);
/*/
  // это микрософтовское расширение sprintf - тоже работает...
  char st[777];
  sprintf(st, "%S", wstr);
  string str = st;
  return str;
//*/
}

wstring char2wchar(const char *str)
// конвертация char строки в Unicode wstring строку
{
//*
  return toWideString(str);
/*/
  wchar_t wst[777];
  swprintf(wst, L"%S", str);
  wstring wstr = wst;
  return wstr;
//*/
}

wstring toWideString(const char *pStr, int len)
{
//  ASSERT_PTR( pStr ) ;
//  ASSERT( len >= 0 || len == -1 , _T("Invalid string length: ") << len ) ;

  // figure out how many wide characters we are going to get
  int nChars = MultiByteToWideChar(CP_ACP, 0, pStr, len, NULL, 0);
  if (len == -1)   --nChars;
  if (nChars == 0) return UNI_NULL_STR;

  // convert the narrow string to a wide string
  // nb: slightly naughty to write directly into the string like this
  wstring buf;
  buf.resize(nChars);
  // CP_ACP - default to ANSI code page
  MultiByteToWideChar(CP_ACP, 0, pStr, len, const_cast<wchar_t*>(buf.c_str()), nChars);
  return buf;
}

string toNarrowString(const wchar_t *pStr, int len)
{
//  ASSERT_PTR( pStr ) ;
//  ASSERT( len >= 0 || len == -1 , _T("Invalid string length: ") << len ) ;

  // figure out how many narrow characters we are going to get
  int nChars = WideCharToMultiByte(CP_ACP, 0, pStr, len, NULL, 0, NULL, NULL);
  if (len == -1)   --nChars;
  if (nChars == 0) return ""; // узкая пустая строка!

  // convert the wide string to a narrow string
  // nb: slightly naughty to write directly into the string like this
  string buf;
  buf.resize(nChars);
  WideCharToMultiByte(CP_ACP, 0, pStr, len, const_cast<char*>(buf.c_str()), nChars, NULL, NULL);
  return buf;
}

/*
www.RSDN.ru
конвертация std::string в std::wstring

******* см. версию 10 CC1MG **********
*/

// ====================================================================

#ifndef _ADDFAC
#define _ADDFAC(loc, pfac) locale(loc, pfac) // в GW нет этого дефайна - берём его из VS хедера <xlocale>
#endif

bool Unicode_open_wifstream(wifstream &ifstr, const char *file)
// открываем wifstream для "прозрачной" работы с Unicode текстовым файлом для чтения
{
  // обеспечиваем работу wifstream с Unicode без конвертации
  locale loc = _ADDFAC(locale::classic(), new NullCodecvt);
  ifstr.imbue(loc);
  // оказывается mingw не поддерживает ни wchar_t, ни wstring, по крайней мере в fstream классах...
  // для решения этой проблемы обычно советуют брать библиотеку STLport, но я это делать не буду...
  // открываем поток в двоичном режиме
  ifstr.open(file, ios_base::binary);
  return ifstr.is_open();
}

bool Unicode_open_wofstream(wofstream &ofstr, const char *file)
// открываем wofstream для "прозрачной" работы с Unicode текстовым файлом для записи
{
  locale loc = _ADDFAC(locale::classic(), new NullCodecvt);
  ofstr.imbue(loc);
  ofstr.open(file, ios_base::binary);
  return ofstr.is_open();
}

wstring get_textfile(const char *file)
// чтение из текст. файла всего содержимого -> возврат в одной wstring строке
{
  wifstream fin;
  if (!Unicode_open_wifstream(fin, file))
  {
    wstring2 str;
    str << "Error open <" << file << "> file!";
    return str;
  }

  // ввод всех строк текста "внавалку", т.к. нуля в текстовом файле нет!
  // однако в тексте останутся заголовок файла UNI_HDR и все CR/LF знаки
  // в виде "нечитаемых" символов!

  wchar_t delim = 0; // 0 - "граничная" литера для ввода
  wstring text;
  getline(fin, text, delim);
  return text;
}

wstring get_textfile_wstring(const char *file, int index=0)
// чтение из Unicode текстового файла одной строки (до "\r\n") с номером index
{
  wifstream fin;
  if (!Unicode_open_wifstream(fin, file))
  {
    wstring2 str;
    str << "Error open <" << file << "> file!";
    return str;
  }

  // ввод строки текста до литеры LF, перед которой остается CR
  wchar_t delim = UNI_LF; // "граничная" литера для ввода
  wstring text;
  // не проверяем наличие очередной строки в файле...
  for (int i = 0; i <= index; ++i) getline(fin, text, delim);

  // отбрасываем литеру Unicode заголовка в начале файла
  if (index == 0 && text[0] == UNI_HDR) text = text.substr(1);
   // количество литер в сроке минус 1
  size_t sz1 = text.size() - 1;
  // отбрасываем литеру CR в конце строки
  if (text[sz1] == UNI_CR) text = text.substr(0, sz1);

  return text;
}

bool get_textfile_wstring_array(wstring sarray[], int &nums, const char *file)
// построчное чтение первых nums строк из Unicode текст. файла -> sarray[nums]
// если строк меньше, возвращает false и устанавливает nums в число прочитанных
{
  wifstream fin;
  if (!Unicode_open_wifstream(fin, file)) return false;

  // построчно копируем файл в массив
  for (int i = 0; i < nums; ++i)
  {
    wstring text;
    wchar_t delim = UNI_LF;
    if (!getline(fin, text, delim))
    {
      nums = i;
      return false; // если строк меньше чем надо
    }
    // else ok
    if (i == 0 && text[0] == UNI_HDR) text = text.substr(1);
    size_t sz1 = text.size() - 1;
    // отбрасываем \r в конце строки
    if (text[sz1] == UNI_CR) text = text.substr(0, sz1);
    sarray[i] = text;
  }

  return true;
}

bool Unicode_save_wstring_array(wstring sarray[], int nums, const char *file)
// построчная запись первых nums строк из sarray[nums] -> Unicode текст. файл
{
  wofstream fout;
  if (!Unicode_open_wofstream(fout, file)) return false;

  fout << UNI_HDR; // сначала пишем заголовок Unicode текста

  // построчно копируем массив в файл
  for (int i = 0; i < nums; ++i)
  {
    fout << sarray[i];
    // выводим символы конца строки, каждый отдельно!!
    fout << UNI_CR;
    fout << UNI_LF;
  }

  return true;
}

