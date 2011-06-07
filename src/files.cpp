
#include "stdafx.h"

// ==================== глобальные file-функции =======================

bool read_bin_ex(const wchar_t *file, void *buffer, int &maxbuflen)
{
  if ( maxbuflen <= 0 ) return false;

  FILEopen df( file, "rb" );
  if ( !df ) return false;

  maxbuflen = (int)fread( buffer, 1, maxbuflen, df );
//  if ( 0 == feof(df) ) return false; // not enough space for read??

  int err = ferror( df );
  return 0 == err;
}

int read_bin(const wchar_t *file, void *buffer, int maxbuflen)
// читает кусок файла с его начала, возвращает число прочитанных байт
{
  bool res = read_bin_ex(file, buffer, maxbuflen);
  if (!res) return 0;
  return maxbuflen;
}

int read_bin_offset(const wchar_t *file, void *buffer, int maxbuflen, int offset)
// читает кусок файла по текущему смещению, возврящает число прочитанных байт
{
  if ( maxbuflen <= 0 || offset < 0 ) return 0;

  FILEopen df(file, "rb");
  if ( !df ) return 0;

  if ( fseek(df, offset, SEEK_SET) != 0 ) return 0;

  maxbuflen = (int)fread(buffer, 1, maxbuflen, df);

  int err = ferror(df);
  if (err != 0) return 0;

  return maxbuflen;
}

bool write_bin_offset(const wchar_t *file, const void *buffer, int buflen, int offset)
// записывает кусок файла по текущему смещению из буфера, файл должен существовать!
// может писать в любую часть файла, в т.ч. с увеличением его первонач-го размера,
// пропущенные при этом куски заполняются нулями (если смещение больше длины файла)
{
  if ( buflen < 0 || offset < 0 ) return false;

  FILEopen df( file, "r+b" ); // w+b работает не так как надо!
  if ( !df ) return false;

  if ( fseek(df, offset, SEEK_SET) != 0 ) return false;

  int wnum = (int)fwrite( buffer, 1, buflen, df );
  return wnum == buflen;
}

bool write_bin_append(const wchar_t *file, const void *buffer, int buflen)
// создает файл с нуля, а если он есть - записывает в файл методом добавления к концу
{
  if ( buflen < 0 ) return false;

  FILEopen df( file, "a+b" );
  if ( !df ) return false;

  int wnum = (int)fwrite( buffer, 1, buflen, df );
  return wnum == buflen;
}

bool write_bin_append2(const wchar_t *file, const void *buffer, int buflen)
// это write_bin_append() с разрешением коллективного доступа к файлу на чтение/запись
{
  if ( buflen < 0 ) return false;

  FILEopen2 df(file, "a+b", _SH_DENYNO); // разрешаем коллективный доступ к файлу на чтение/запись
  if ( !df ) return false;

  int wnum = (int)fwrite( buffer, 1, buflen, df );
  return wnum == buflen;
}

bool write_bin(const wchar_t *file, const void *buffer, int buflen, bool create)
// если create=true - создает и пишет в файл, иначе файл уже должен существовать!
{
  if ( buflen < 0 ) return false;

  const wchar_t *mode = create? L"w+b":L"wb";
  FILEopen df( file, mode );
  if ( !df ) return false;

  int wnum = (int)fwrite( buffer, 1, buflen, df );
  return wnum == buflen;
}

int get_file_length(const wchar_t *file)
{
  FILEopen df(file, "rb");
  if ( !df ) return 0;

  if ( fseek(df, 0, SEEK_END) != 0 ) return 0;

  int len = ftell(df); // len = -1 if error!
  return len>0? len:0;
}

bool file_exist(const wchar_t *file)
{
  FILEopen df(file, "rb");
  return df != 0;
}

// ===================== class MFile ===========================

MFile::MFile(FILE *dfile)
{
  disk_file = true;
  file = dfile;
}

MFile::MFile(uint8 *ramemory, int ramemlength)
{
  disk_file = false;
  memory = ramemory;
  memlength = ramemlength;
  memoffset = 0;
  mem_eof = 0;
}

uint8 MFile::read_uint8()
{
  int data;

  if (disk_file)
  {
    data = fgetc( file );
    if (data == EOF) data = 0; // error
  }
  else
  {
    if (memoffset >= memlength) // error
    {
      data = 0;
      mem_eof = 1;
    }
    else
    {
      data = *(memory+memoffset);
      ++memoffset;
    }
  }

  return (uint8) data;
}

// в read-функциях сначала идет младший байт, затем старший (Intel CPU order)
uint32 MFile::read_uint16()
{
  uint32 val = read_uint8();
  return val | ( read_uint8() << 8 );
}

uint32 MFile::read_uint32()
{
  uint32 val = read_uint16();
  return val | ( read_uint16() << 16 );
}

int MFile::eof()
// макрос feof()
{
  return disk_file? feof(file) : mem_eof;
}

int MFile::seek(int offset, int origin)
{
  if (disk_file) return fseek(file, offset, origin);

/*
  Moves the file pointer to a specified location.
  offset = Number of bytes from origin,
  origin = Initial position:
  SEEK_CUR Current position of file pointer
  SEEK_END End of file
  SEEK_SET Beginning of file
*/
  int new_offset;
  switch (origin)
  {
    default:
    case SEEK_CUR: new_offset = memoffset + offset; break;
    case SEEK_END: new_offset = memlength - offset; break;
    case SEEK_SET: new_offset = offset; break;
  }

/*
  If successful, fseek returns 0.
  Otherwise, it returns a nonzero value.
*/
  if (new_offset < 0 || new_offset > memlength) return 1; // error

  memoffset = new_offset;
  return 0; // ok
}

