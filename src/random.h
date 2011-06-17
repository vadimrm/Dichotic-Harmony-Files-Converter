/*
    Random number generator
*/

#pragma once

// генератор псевдо-случайных чисел с разрядностью 1...32 бит
class Random
{
public:
  // разрядность генератора bitlen устанавливается в конструкторе!
  Random(uint32 bitlen=32);
  // то же с начальной рандомизацией датчика
  Random(uint32 bitlen, bool randomize);
  // глобальная ф-я !
  static void UrandComput(uint32 &rbitlen, uint32 &rmul, uint32 &radd);

  // генерируют очередное псевдослучайное seed и возврящают его значение
  // с приведением к нужному диапазону 8/16/32 бит, но не более bitlen !
  uint32 get_rand8();
  uint32 get_rand16();
  uint32 get_rand32();

  // random number from 0/0. to maxval, include maxval!
  int    get_rand_ex(int maxval);
  uint32 get_rand_ex(uint32 maxval);
  double get_rand_ex(double maxval);
  // random number from 0/0. to maxval, do NOT include maxval!
  double get_rand_exclude(double maxval);

  uint32 get_seed() { return seed; }
  void set_seed(uint32 new_seed) { seed = new_seed & rmax1; };

  // рандомизация seed временем в секундах из сишной функции
  // time_t time() = seconds elapsed since midnight (00:00:00), January 1, 1970
  // time_t теперь 32 или 64 бита, warning в VS2005
  void randomize_seed() { uint32 seconds = (uint32) time(0); seed = seconds & rmax1; }
  // быстрая смена зерна, >=50 раз в сек (т.к. шаг таймера по умолчанию 10-20 мсек)
  void randomize_seed_msec() { uint32 msec = timeGetTime(); seed = msec & rmax1; }

  uint32 rand() { return seed = (rmul*seed + radd) & rmax1; };

private:
  void Setup(uint32 bitlen, bool randomize);

  // эти ф-и позволяют изменить коэфф-ты генератора, осторожно!
  void set_rmul(uint32 rmul) { this->rmul = rmul & rmax1; }
  void set_radd(uint32 radd) { this->radd = radd & rmax1; }

  // 8 старших бит seed <- младший байт seed8
  void set_seed8(uint32 seed8) { seed = (seed8 & 255) << rsh8; };
  uint32 get_seed8() { return this->seed >> rsh8; }

  // ф-я определяет (длину повторения-1) псевдослуч-й последовательности,
  // при заданном значении зерна from_seed (медленно для bitlen > 24...)
  uint32 get_rand_length1(uint32 from_seed=1);

private:
  uint32 seed;
  uint32 rmul;
  uint32 radd;
  uint32 bitlen;
  // rmax1 = (макс длина случ-й последовательности - 1) для заданного bitlen
  uint32 rmax1; // = 2^bitlen-1
  // величина сдвига вправо (для уменьшения) в get_rand8/16
  uint32 rsh8, rsh16;
  // величина сдвига влево (для увеличения) в get_rand8/16/32
  uint32 lsh8, lsh16, lsh32;
};

inline uint32 Random::get_rand8()
{
  if      (bitlen ==  8)   return rand();
  else if (bitlen >   8)   return rand() >> rsh8;
  /* else  bitlen <   8 */ return rand() << lsh8;
};

inline uint32 Random::get_rand16()
{
  if      (bitlen == 16)   return rand();
  else if (bitlen >  16)   return rand() >> rsh16;
  /* else  bitlen <  16 */ return rand() << lsh16;
};

inline uint32 Random::get_rand32()
{
  return rand() << lsh32; // сдвиг на 0 быстрее чем проверка на == 32 !
};

inline uint32 Random::get_rand_ex(uint32 maxval)
// random number from 0 to maxval, include maxval
{
  // равномерное распределение по всем значениям, включая концы!
  return (uint32) get_rand_exclude( 1.0 + double(maxval) );
}

inline int Random::get_rand_ex(int maxval)
// random number from 0 to maxval >= 0, include maxval
{
  // равномерное распределение по всем значениям, включая концы!
  return (int) get_rand_exclude( 1.0 + double(maxval) );
}

inline double Random::get_rand_ex(double maxval)
// random number from 0. to maxval, INclude maxval!
{
  // равномерное распределение по всем значениям, но на maxval м.б. отличие...
  return (get_rand32()*maxval)/0xFFFFFFFFul;
}

inline double Random::get_rand_exclude(double maxval)
// random number from 0. to maxval, EXclude maxval!
{
  const double k = 1./(1.0 + double(0xFFFFFFFFul)); // 2^-32
  // равномерное распределение по всем значениям (maxval не достигается)!
  return get_rand32()*maxval*k;
}

