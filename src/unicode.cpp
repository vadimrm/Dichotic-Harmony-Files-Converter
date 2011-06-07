
#include "stdafx.h"

const wchar_t *UNI_CRLF = L"\r\n"; // � ����� ������ ������ - ���� �������� UNI_CR,UNI_LF
const wchar_t *UNI_NULL_STR = L""; // ������� ������ ������

string wchar2char(const wchar_t *wstr)
// ����������� Unicode wchar_t ������ � string char ������
{
//*
  return toNarrowString(wstr);
/*/
  // ��� ��������������� ���������� sprintf - ���� ��������...
  char st[777];
  sprintf(st, "%S", wstr);
  string str = st;
  return str;
//*/
}

wstring char2wchar(const char *str)
// ����������� char ������ � Unicode wstring ������
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
  if (nChars == 0) return ""; // ����� ������ ������!

  // convert the wide string to a narrow string
  // nb: slightly naughty to write directly into the string like this
  string buf;
  buf.resize(nChars);
  WideCharToMultiByte(CP_ACP, 0, pStr, len, const_cast<char*>(buf.c_str()), nChars, NULL, NULL); 
  return buf; 
}

/*
www.RSDN.ru
����������� std::string � std::wstring

******* ��. ������ 10 CC1MG **********
*/

// ====================================================================

bool Unicode_open_wifstream(wifstream &ifstr, const wchar_t *file)
// ��������� wifstream ��� "����������" ������ � Unicode ��������� ������ ��� ������
{
  // ������������ ������ wifstream � Unicode ��� �����������
  locale loc = _ADDFAC(locale::classic(), new NullCodecvt);
  ifstr.imbue(loc);
  // ��������� ����� � �������� ������
  ifstr.open(file, ios_base::binary);
  return ifstr.is_open();
}

bool Unicode_open_wofstream(wofstream &ofstr, const wchar_t *file)
// ��������� wofstream ��� "����������" ������ � Unicode ��������� ������ ��� ������
{
  locale loc = _ADDFAC(locale::classic(), new NullCodecvt);
  ofstr.imbue(loc);
  ofstr.open(file, ios_base::binary);
  return ofstr.is_open();
}

wstring get_textfile(const wchar_t *file)
// ������ �� �����. ����� ����� ����������� -> ������� � ����� wstring ������
{
  wifstream fin;
  if (!Unicode_open_wifstream(fin, file))
  {
    wstring2 str;
    str << "Error open <" << file << "> file!";
    return str;
  }

  // ���� ���� ����� ������ "��������", �.�. ���� � ��������� ����� ���!
  // ������ � ������ ��������� ��������� ����� UNI_HDR � ��� CR/LF �����
  // � ���� "����������" ��������!

  wchar_t delim = 0; // 0 - "���������" ������ ��� �����
  wstring text;
  getline(fin, text, delim);
  return text;
}

wstring get_textfile_wstring(const wchar_t *file, int index=0)
// ������ �� Unicode ���������� ����� ����� ������ (�� "\r\n") � ������� index
{
  wifstream fin;
  if (!Unicode_open_wifstream(fin, file))
  {
    wstring2 str;
    str << "Error open <" << file << "> file!";
    return str;
  }

  // ���� ������ ������ �� ������ LF, ����� ������� �������� CR
  wchar_t delim = UNI_LF; // "���������" ������ ��� �����
  wstring text;
  // �� ��������� ������� ��������� ������ � �����...
  for (int i = 0; i <= index; ++i) getline(fin, text, delim);

  // ����������� ������ Unicode ��������� � ������ �����
  if (index == 0 && text[0] == UNI_HDR) text = text.substr(1);
   // ���������� ����� � ����� ����� 1
  size_t sz1 = text.size() - 1;
  // ����������� ������ CR � ����� ������
  if (text[sz1] == UNI_CR) text = text.substr(0, sz1);

  return text;
}

bool get_textfile_wstring_array(wstring sarray[], int &nums, const wchar_t *file)
// ���������� ������ ������ nums ����� �� Unicode �����. ����� -> sarray[nums]
// ���� ����� ������, ���������� false � ������������� nums � ����� �����������
{
  wifstream fin;
  if (!Unicode_open_wifstream(fin, file)) return false;

  // ��������� �������� ���� � ������
  for (int i = 0; i < nums; ++i)
  {
    wstring text;
    wchar_t delim = UNI_LF;
    if (!getline(fin, text, delim))
    {
      nums = i;
      return false; // ���� ����� ������ ��� ����
    }
    // else ok
    if (i == 0 && text[0] == UNI_HDR) text = text.substr(1);
    size_t sz1 = text.size() - 1;
    // ����������� \r � ����� ������
    if (text[sz1] == UNI_CR) text = text.substr(0, sz1);
    sarray[i] = text;
  }

  return true;
}

bool Unicode_save_wstring_array(wstring sarray[], int nums, const wchar_t *file)
// ���������� ������ ������ nums ����� �� sarray[nums] -> Unicode �����. ����
{
  wofstream fout;
  if (!Unicode_open_wofstream(fout, file)) return false;

  fout << UNI_HDR; // ������� ����� ��������� Unicode ������

  // ��������� �������� ������ � ����
  for (int i = 0; i < nums; ++i)
  {
    fout << sarray[i];
    // ������� ������� ����� ������, ������ ��������!!
    fout << UNI_CR;
    fout << UNI_LF;
  }

  return true;
}

