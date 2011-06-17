/*
  Random Generator
*/

#include "stdafx.h"

// алгоритм из книги Дж.Форсайт и др. "Машинные методы мат. вычислений."
// по разрядности датчика - от 1 до 32 бита - вычисляем параметры
// линейно - конгруэнтного датчика пс. случайных чисел.
// Мои уточнения (В.М.) - 6 ноября 2001 г. !
void Random::UrandComput(uint32 &rbitlen, uint32 &rmul, uint32 &radd)
{
  double halfm = 1.;

  mintestmax(1, rbitlen, 32);

  // halfm = (макс uint число с длиной rbitlen бит + 1)/2
  for (uint32 i = 1; i < rbitlen; ++i) halfm *= 2.;

  // тут функции с плавающей запятой должны давать точность хотя бы 32 бита,
  // поэтому для однозначности результата безразлична модель проца (386/486/...),
  // наличие копроц-а (387/487...), и точность double-вычислений (64 или 80 бит),
  // т.к. в _этой_ редакции функции промежуточные double-вычисления округляются
  // (при помощи _0.5_ + halfm * ...)!
  rmul = 5 + 8 * (uint32) ( 0.5 + halfm * atan (1.) / 8. );
  radd = 1 + 2 * (uint32) ( 0.5 + halfm * ( 0.5 - sqrt(3.) / 6. ) );

  // формула получения сл. числа: seed = (rmul * seed + radd) mod (2^rbitlen)
}

Random::Random(uint32 bitlen)
{
  Setup(bitlen, false);
}

Random::Random(uint32 bitlen, bool randomize)
{
  Setup(bitlen, randomize);
}

void Random::Setup(uint32 bitlen, bool randomize)
{
  mintestmax(1, bitlen, 32);

  UrandComput(bitlen, rmul, radd);

  this->bitlen = bitlen;

  rmax1 = 0xFFFFFFFFul;
  if (bitlen < 32) rmax1 = rmax1 >> (32-bitlen);

  // для bitlen <= 3 это существенно
  rmul &= rmax1;
  radd &= rmax1;

  // величина сдвига вправо (для уменьшения) в get_rand8/16
  rsh8  = (bitlen >  8)? (bitlen -  8):0;
  rsh16 = (bitlen > 16)? (bitlen - 16):0;

  // величина сдвига влево (для увеличения) в get_rand8/16/32
  lsh8  = (bitlen <  8)? ( 8 - bitlen):0;
  lsh16 = (bitlen < 16)? (16 - bitlen):0;
  lsh32 = (bitlen < 32)? (32 - bitlen):0;

  if (randomize) randomize_seed();
  else           seed = 1;
}

// ф-я определяет (длину повторения-1) псевдослуч-й последовательности,
// при заданном значении зерна seed (довольно медленно для bitlen > 24)
uint32 Random::get_rand_length1(uint32 from_seed)
{
  seed = from_seed & rmax1;
  register uint32 i = 0;
  register uint32 seed1 = seed;
  register uint32 seed0 = seed;

  for (;;)
  {
    seed1 = rand();
    // совпадение с начальным зерном!
    if (seed1 == seed0) break;
    // исчерпание макс-го кол-ва чисел, важно напр для 32-х битного датчика...
    if (i == rmax1) break;
    ++i;
  }

  return i;
}

bool TurnSecureCode(void *buffer, int numbytes, int key, int width, bool key_from_numbytes)
// XOR-перекодировка массива байт buffer[] длинной numbytes байт
// key - ключ шифра - зерно случайного генератора, 32 битное целое
// width - разрядность генератора шифра, лучше в диапазоне 25...32
// если key_from_numbytes = true, то ключ шифра берётся из numbytes
{
  if ( buffer == 0 || numbytes < 0 || width < 1 || width > 32 ) return false;

  if ( key_from_numbytes ) key = numbytes;

  Random gen(width); // width-битный генератор сл. чисел
  gen.set_seed(key); // начальное зерно - как ключ шифра
 
  uint8* buf = (uint8*) buffer;

  // побайтное XOR массива с последовательностью случайных байт генератора
  for (int i = 0; i < numbytes; ++i)
  {
    int bbyte = buf[i];
    bbyte ^= gen.get_rand8();
    buf[i] = (uint8) bbyte;
  }

  return true;
}

