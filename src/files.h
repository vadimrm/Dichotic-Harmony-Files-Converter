/*
  wchar_t версии старых файловых функций...
*/

#pragma once

const int FILE_STRLEN = MAX_PATH+64; // 260+64

bool file_exist(const wchar_t *file);
int  get_file_length(const wchar_t *file); // if error return 0!
int  read_bin(const wchar_t *file, void *buffer, int maxbuflen);
bool read_bin_ex(const wchar_t *file, void *buffer, int &maxbuflen);

// читает кусок файла по текущему смещению, возврящает число прочитанных байт
int  read_bin_offset(const wchar_t *file, void *buffer, int maxbuflen, int offset);

// если create=1 - создает и пишет в файл, иначе файл уже должен существовать!
bool write_bin(const wchar_t *file, const void *buffer, int buflen, bool create);

// создает файл с нуля, а если он есть - записывает в файл методом добавления к концу
bool write_bin_append(const wchar_t *file, const void *buffer, int buflen);
// это write_bin_append() с разрешением коллективного доступа к файлу на чтение/запись
bool write_bin_append2(const wchar_t *file, const void *buffer, int buflen);

// записывает кусок файла по текущему смещению из буфера, файл должен существовать!
// может писать в любую часть файла, в т.ч. с увеличением его первонач-го размера,
// пропущенные при этом куски заполняются нулями (если смещение больше длины файла)
bool write_bin_offset(const wchar_t *file, const void *buffer, int buflen, int offset);

// ==========================================================================

class FILEopen // простой "открыватель" файла с самозакрытием
{
  FILE *pfile;
public:
  FILEopen(const wstring2 filename, const wstring2 mode) { _wfopen_s(&pfile, filename, mode); }
  // деструктор закрывает файл, это можно сделать и до уничтожения объекта: (*this).~FILEopen();
  ~FILEopen() { if (pfile) fclose( pfile ); pfile = NULL; }
  operator FILE*() { return pfile; }  // доступ к *pfile  при помощи (FILE*)(*this)
  FILE *operator->() { return pfile; } // доступ к pfile-> при помощи (*this)->
  // файл открыт без ошибок, если (*this != 0) или ( this->openok() == true )
  bool openok() { return pfile != NULL; }
};

class FILEopen2 // FILEopen с расшариванием доступа к файлу
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
  // деструктор закрывает файл, это можно сделать и до уничтожения объекта: (*this).~FILEopen();
  ~FILEopen2() { if (pfile) fclose( pfile ); pfile = NULL; }
  operator FILE*() { return pfile; }  // доступ к *pfile  при помощи (FILE*)(*this)
  FILE *operator->() { return pfile; } // доступ к pfile-> при помощи (*this)->
  // файл открыт без ошибок, если (*this != 0) или ( this->openok() == true )
  bool openok() { return pfile != NULL; }
};

// ==========================================================================

class MFile // метафайл - файл на диске или его образ в RAM (созданный вовне)
{
  bool disk_file; // true если файл на диске, иначе файл в RAM
  FILE *file; // файловый указатель
  uint8 *memory; // начало образа файла в RAM
  int memlength; // количество байт в RAM образе
  int memoffset; // номер текущего байта RAM от начала memory
  int mem_eof; // !0 после первой попытки прочесть за концом memory, иначе 0

public:
  MFile(FILE *dfile); // конструктор объекта для настоящего дискового файла
  MFile(uint8 *ramemory, int ramemlength); // то же, для образа файла из RAM

  // функции чтения 1, 2 или 4-х байт из метафайла
  uint8   read_uint8();
  uint32 read_uint16();
  uint32 read_uint32();

  int eof(); // макрос feof()
  int seek(int offset, int origin); // функция fseek()
};

// ==========================================================================

// читаем файл с диска прямо в конструкторе, выдаем содержимое и его размеры
class BinFile
{
  Ar <uint8> buf; // массив байт - содержимое файла с диска
  int len; // длина файла - надо, т.к. размер массива Ar другой!
  wstring2 file; // имя файла из конструктора
  BinFile &operator=(const BinFile &) {}; // избавляемся от warning C4512: assignment operator could not be generated

public:
  const int &length; // количество файловых байт в контенте
  uint8 *content() const { return (len==0)? 0:buf.memory(); }
  // описание параметров перекодировки см. TurnSecureCode()
  BinFile(wstring2 filename, bool secure = false, int key = 0, int width = 32, bool key_from_numbytes = false)
         : length(len)
  {
    file = filename;
    len = get_file_length(file);
    if (len <= 0) { len = 0; return; }
 
    // оставляем в конце буфера 2 нулевых байта - для нужд текстовых файлов!
    buf.renew(len+2);
    read_bin(file, buf.memory(), len);
    buf[len] = buf[len+1] = 0;

    // перекодируем прочитанные данные если надо
    if ( secure ) turn_secure_state(key, width, key_from_numbytes);
  }

  // перекодировка прочитанных из файла данных
  bool turn_secure_state(int key = 0, int width = 32, bool key_from_numbytes = false)
  {
    return TurnSecureCode(buf.memory(), len, key, width, key_from_numbytes);
  }

  // запись файла на диск - поверх исходного
  bool save() { return write_bin(file, buf.memory(), len, true); }
};


// текстовый файл любого типа - Unicode или ANSI - читается в широкую строку,
// однако трансляция литер CR/LF не производится!
class TextFile
{
  wstring file; // широкая строка-копия текста из файла
  bool uni; // true для Unicode, false для ANSI файла
  size_t len; // длина строки
  static const wchar_t UNI_HDR = 0xFEFF; // маркер Unicode текста в начале файла
  TextFile &operator=(const TextFile &) {}; // избавляемся от warning C4512: assignment operator could not be generated

public:
  const size_t &length; // количество литер в контенте
  const bool &is_unicode; // true для Unicode, false для ANSI файла
  const wstring &content; // широкая строка-копия текста из файла
  // описание параметров перекодировки см. TurnSecureCode()
  TextFile(wstring2 filename, bool secure = false, int key = 0, int width = 32, bool key_from_numbytes = false)
          : is_unicode(uni), content(file), length(len)
  {
    file.clear();
    uni = false;
    BinFile df(filename, secure, key, width, key_from_numbytes);
    len = df.length;
    if (len <= 0) { len = 0; return; }

    // проверяем тип текста - у юникодового должен быть хедер и четная длина
    if (len%2 == 0 && len > 2)
    {
      wchar_t *text = (wchar_t *) df.content();
      if ( text[0] == UNI_HDR )
      {
        uni = true;
        file = &text[1]; // пропускаем хедер!
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

