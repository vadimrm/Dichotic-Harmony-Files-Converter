
#pragma once

const size_t npos = string::npos;

typedef unsigned char    uint8;
typedef unsigned short   uint16;
typedef unsigned int     uint32;
typedef unsigned __int64 uint64;

typedef   signed char    int8;
typedef   signed short   int16;
typedef   signed int     int32;
typedef          __int64 int64;

// это для битовых полей
typedef unsigned int     uint; // конфликтует с libjdksmidi
typedef   signed int     sint;

// #define NULL 0 // @GW справедливо не разрешает переопределять NULL

const double two16 = 65536.; // 2^16
const double two32 = two16*two16; // 2^32 = 4294967296

const int MAX_INT  = 0x7FFFFFFF; // 2^31-1 = 2147483647
const int MAX_UINT = 0xFFFFFFFF; // 2^32-1 = 4294967295

const float MAX_FLOAT = FLT_MAX ; // 3.402823466e+38F
const float MIN_FLOAT = FLT_MIN ; // 1.175494351e-38F

const double MAX_DOUBLE = DBL_MAX ; // 1.7976931348623158e+308
const double MIN_DOUBLE = DBL_MIN ; // 2.2250738585072014e-308

// return x, bat (not <a) and (not >b) !
template<class I,class D>inline D minxmax(I a, D x, I b) { return (D)(x<(D)a? a:(x>(D)b? b:x)); }
// return true if  a <= x  and  x <= b
template<class I,class D>inline bool in_range(I mi,D x,I ma) { return x<(D)mi? false:(x>(D)ma? false:true); }

template<class I> inline I min2(I a, I b) { return (a<b? a:b); } // min(a,b)
template<class I> inline I max2(I a, I b) { return (a>b? a:b); } // max(a,b)
// синонимы
template<class I> inline I min_(I a, I b) { return min2(a, b); }
template<class I> inline I max_(I a, I b) { return max2(a, b); }
// для 3-х аргументов
template<class I> inline I min3(I a, I b, I c) { return min2( min2(a, b), c); }
template<class I> inline I max3(I a, I b, I c) { return max2( max2(a, b), c); }
// возвращает "среднее" из 3-х величин, т.е. то что остаётся между максимумом и минимумом
template<class I> inline I mid3(I a, I b, I c) { sort3(a, b, c); return b; }
// сортирует (a, b) в порядке возрастания
template<class I> inline void sort2(I &a, I &b) { if (a > b) swap(a, b); }
// сортирует (a, b, c) в порядке возрастания
template<class I> inline void sort3(I &a, I &b, I &c)
{
  if (a > b) swap(a, b);
  if (b > c)
  {
    swap(b, c);
    if (a > b) swap(a, b);
  }
}

// корректировка параметра на не выход за макс и/или мин значения
template <class T> inline void testmax(T &x, T &ma) { if ( x > ma ) x = ma; } // работает c volatile!
template <class T> inline void testmin(T &x, T &mi) { if ( x < mi ) x = mi; } // работает c volatile!
template <class T> inline void testmax(T &x, const T ma) { if ( x > ma ) x = ma; } // для констант
template <class T> inline void testmin(T &x, const T mi) { if ( x < mi ) x = mi; } // для констант
template <class I, class D> inline void test_max(D &x, I ma) { if ( x > (D)ma ) x = (D)ma; }
template <class I, class D> inline void test_min(D &x, I mi) { if ( x < (D)mi ) x = (D)mi; }
template <class I1, class D, class I2> inline void mintestmax(I1 mi, D &x, I2 ma) { test_max(x, ma); test_min(x, mi); }
template <class I1, class D, class I2> inline void testminxmax(I1 mi, D &x, I2 ma) { mintestmax(mi, x, ma); } // синоним

// абсол-е значение числа
template<class D>inline D abs_(D x) { return x<D(0)? -x:x;}
// проверка x==y с допустимой _относительной_ ошибкой err>=0, напр. для 7 знаков err=1.e-7
template<class D>inline bool d_eq(D x, D y, D err)
{
  D delta = abs_(x-y); // "разница" = модуль разности чисел
  D maxabs = max_( abs_(x), abs_(y) ); // максимальный модуль чисел
  return delta <= err*maxabs; // равно если "разница" <= погрешность*максмодуль
}
// проверка x>y с допустимой _относительной_ ошибкой err>=0
template<class D>inline bool d_gt(D x, D y, D err)
{
  D y1 = y + abs_(err*y); // y1 = y плюс [всегда положительная] добавка
  return x > y1;
}
// проверка x<y с допустимой _относительной_ ошибкой err>=0
template<class D>inline bool d_lt(D x, D y, D err)
{
  D y1 = y - abs_(err*y); // y1 = y минус [всегда положительная] добавка
  return x < y1;
}

// проверка x>y (возврат +1), x<y (-1), x==y (0)
// с допустимой _относительной_ ошибкой err>=0 (но если y=0 то err=0)
template<class D>inline int d_gtlteq(D x, D y, D err)
{
  D d = abs_(err*y); // неотрицательная _абсолютная_ ошибка
  if ( x > (y+d) ) return  1;
  if ( x < (y-d) ) return -1;
  return 0;
}

// произвольное целое x сдвигается к интервалу (0 <= x < num) - меньше num!
// "порциями" по num величин, где num - целое/беззнаковое, num > 0 !
template <class I, class U> inline I modulo(I x, U num)
{
  if (x >= num) x = x % num;
  else
//if (x < 0)    x =   num - ( (-x) % num ); // было неверно: modulo(-10 10) = 10 вместо 0!
  if (x < 0)    x = ( num - ( (-x) % num ) ) % num; // теперь верно! исправил 10.06.2010г.
  // else 0 <= x < num
  return x;
}
// тоже самое для D = double/float
template <class D, class U> inline D modulo_d(D x, U num)
{
  if (x < 0.0)  x = (D) fmod( ( num - fmod(-x, num) ), num ); // исправил и не проверил!
  else
  if (x >= num) x = (D) fmod(x, num);
  return x;
}
// определяет количество "порций сдвига" в функции modulo (положительных или отрицательных)
template <class I, class U> inline I num_shifts(I x, U num)
{
  if (x >= 0) x = x / num;
  else
  if (x < 0)  x = ( x - modulo(x,num) ) / num;
  return x;
}

// границы 0 и num для &x (x по ссылке!) склеены в кольцо, т.о. всегда (0 <= x <= num)
template <class I> inline I xring(I &x, I num) { x = modulo(x, num+1); return x; }
// границы mi и ma для &x (x по ссылке!) склеены в кольцо, т.о. всегда (mi <= x <= ma)
template <class I> inline I xring(I mi, I &x, I ma) { x = mi + modulo(x-mi, ma-mi+1); return x; }
// то же самое для x по значению
template <class I> inline I ring(I x, I num) { return modulo(x, num+1); }
template <class I> inline I ring(I mi, I x, I ma) { return mi + modulo(x-mi, ma-mi+1); }

// округление float types в integer
template <class D> inline int float2int(D d) { return int( d >= D(0.0) ? (d+D(0.5)):(d-D(0.5))); }

// template <class D> inline unsigned int float2uint(D d) { return  unsigned int(d>=0.0? (d+0.5):(d-0.5)); } // VS
// @GW error: expected primary-expression before 'unsigned'; заменяем на uint32 - то же самое!
template <class D> inline uint32 float2uint(D d) { return uint32(d>=0.0? (d+0.5):(d-0.5)); } // @GW

// умножение любого на float
template <class X, class D> inline void xmulf(X &x, D d) { x = X(x*d); }

// float x, xscale
template <class D> inline void scale(D &x, D xscale) { x *= xscale; }
template <class D> inline void scale_int(int &x, D xscale) { x = float2int(x * xscale); }
// варианты без &
template <class D> inline D dscale(D x, D xscale) { return x * xscale; }
template <class D> inline int iscale_int(int x, D xscale) { return float2int(x * xscale); }
// scale "с неуменьшением до 0"
inline void safe_scale_int(int &x, double xscale)
{
  if ( x == 0 ) return;
  else if ( x > 0 )  x = max_(  1, float2int(x * xscale) );
  else   /* x < 0 */ x = min_( -1, float2int(x * xscale) );
}

// проверять указатель на 0 не нужно: (delete 0) не вызывает проблем! но делать obj = 0 может быть важно...
template <class I> inline void safe_delete_object(I *&obj) { delete obj; obj = 0; }
// все объекты, созданные при помощи new Type [number] надо удалять так:
template <class I> inline void safe_delete_array(I *&arr)  { delete [] arr; arr = 0; }

template<class I>inline I *new_copy(I *src) { if (!src) return 0; return new I (*src); }
template<class I>inline I *new_copy_const(const I *src){ if( !src) return 0; return new I(*src); }

template<class I,class D>inline I *new_copy2(I*src,D x,D y){ if( !src) return 0; return new I(*src,x,y); }
template <class I, class D> inline I *new_copy_const2(const I *src, D x, D y)
{ if (!src) return 0; return new I (*src, x, y); }

// функция почленного копирования dst[]=src[] массивов объектов одинаковых
// или разных типов, допускающих преобразование src типа в dst;
// num - число копируемых элементов
template<class TD, class TS> void copy_array(TD *dst, const TS *src, int num)
{
  for (int i = 0; i < num; i++) dst[i] = src[i];
}

// очистка широкой строки (например типа wstring2) и вывод в нее значения val (например числа)
template <class S, class D> inline void out2str(S &str, const D &val){ str = UNI_NULL_STR; str << val; }

// значения val (например числа) конвертируется (путем "печати") в строку текста
template <class D> inline wstring2 out2str(const D &val){ wstring2 str; str << val; return str; }

// даёт строку с побайтным содержимым объекта obj
// если выравнивание при компиляции класса объекта было грубее 1 байта в конце его может быть мусор!
template <class T> string object_as_text_string(const T &obj)
{
  const uint8 *objbyte = reinterpret_cast <const uint8 *> (&obj);
//  const uint8 *objbyte = (const uint8 *) &obj;
  ostringstream out;
  size_t nbytes = sizeof(T);
  for (size_t i = 0; i < nbytes; ++i)
  {
    out << "byte";
    out.width(3);
    out << right << dec << i;
    out << "  val ";
    out.width(4);
    out << right << hex << uppercase << showbase << int ( objbyte[i] ) << endl;
  }
  return out.str();
}

// ================ прототипы и классы =================

// копирует 4 байта src->dst с изменением порядка байтов на обратный: src[0]->dst[3]...
uint32 reverse_copy_dword(void *dst, uint32 src); // возвращает dst в виде dword

// число сочетаний из n по k: C = n!/k!*(n-k)! = (k+1)*(k+2)*...*n/1*2*...*(n-k)
uint64 C_n_k(int n, int k); // эта функция "загибается" уже при n = 21, k = 2
uint64 fC_n_k(int n, int k); // вариант внутри с плав. запятой, даёт гораздо больший рабочий диапазон!

void pip();
void SetBit(int &mask, int bitnum, int bitval);
bool TurnSecureCode(void *buffer, int numbytes, int key, int width, bool key_from_numbytes = false);

extern inline void mem32cpy32(void *dst, void *src, int numdwords);
extern inline void mem32set32(void *dst, int setval, int numdwords);

extern inline uint32 Randu(uint32 &seed);
extern inline int Randu31(uint32 &seed);
extern inline int Randu16(uint32 &seed);
extern inline int  Randu8(uint32 &seed);

struct in3 // число из 3-х int компонент, например для RGB масок
{
  int v1, v2, v3; // компоненты

  in3() : v1(0), v2(0), v3(0) {}
  in3(int eqval) : v1(eqval), v2(eqval), v3(eqval) {} // одинаковые компоненты
  in3(int v_1, int v_2, int v_3) : v1(v_1), v2(v_2), v3(v_3) {} // разные

  bool operator==(const int val) const { return (v1==val && v2==val && v3==val); }
  bool operator!=(const int val) const { return (v1!=val || v2!=val || v3!=val); }
};

struct db3 // число из 3-х double компонент, например для RGB коэффициентов
{
  double v1, v2, v3; // компоненты

  db3() : v1(0.), v2(0.), v3(0.) {}
  db3(double eqval) : v1(eqval), v2(eqval), v3(eqval) {} // одинаковые компоненты
  db3(double v_1, double v_2, double v_3) : v1(v_1), v2(v_2), v3(v_3) {} // разные

  bool operator==(const double val) const { return (v1==val && v2==val && v3==val); }
  bool operator!=(const double val) const { return (v1!=val || v2!=val || v3!=val); }
};

// объект из нужного количества байт, заполненных константой
class FillBytes
{
  uint8 *mem;
public:
  FillBytes(int bytes_number, int const_to_fill = 0)
  {
    mem = new uint8 [bytes_number];
    if (mem) memset(mem, const_to_fill, bytes_number);
  }
  ~FillBytes() { safe_delete_array(mem); }
  operator void*() { return mem; }
};

class Size // вместо недоделанной стандартной структуры SIZE
{
  int cx, cy;
public:
  Size() { cx = cy = 0; }
  Size(int dx, int dy) : cx(dx), cy(dy) {}
  SIZE getSize() { SIZE s; s.cx = cx; s.cy = cy; return s; }
};

