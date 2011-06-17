
#pragma once

#pragma pack(push, 1) // иначе по умолчанию выравнивание по 4 байта...

const int SLOW_ARRAY = 0;
const int FAST_ARRAY = 1;

// fast: метод работы с памятью массива 1=быстро, 0=медленно (и безопасно для любых типов T)
// fast: см. также длинный комментарий в теле функции Ar<T,fast>::clear()
// переносимость кода при fast=1 гарантируется только для целых типов T!!
template<class T, int fast = FAST_ARRAY> class Ar // 9 сентября 2010 г. поменял fast на 1 (ранее было 0)
{
  T* mem; // массив элементов типа class T
  int nums; // количество членов массива T[nums]
  int fastmem; // FAST_ARRAY = быстрый метод очистки и копирования памяти
  // для массивов одинаковых типов метод копирования выбирается по this массиву, а для разных типов он всегда медленный!

  T* new_memory(int elems); // выделяет новую память
  bool make_array(int elems, bool need_clear); // выделяет новую память под массив, если надо - стирает его

  // запрещенные функции:
  // template<class T2> void operator=(T2 &src); // запрет = из других типов! НО: не компилится...
public:
  Ar(const Ar<T,fast> &src); // конструктор копирования - нужен для передачи объекта по значению!
  Ar( int elems = 0); // конструктор по умолчанию - без инициализации!
  // конструктор массива T[elems] с заполнением всего массива константой val (по умолчанию нулевой)
  // лучше всегда делать need_fill = true чтобы обнулить память - см. комментарий в make_array()!
  Ar( int elems, bool need_fill, const T val = T() ); // need_fill введено для отличия от других конструкторов
  // конструктор копирования из обычного массива T src[], number - полный размер массива!
  template<int number> Ar(const T (&src)[number]) : fastmem(fast) // размер массива определяется автоматически!
  {
    make_array(number, false);
    copy_from(src);
  }
  ~Ar() { safe_delete_array( mem ); }

  // копируем в this весь массив src[0]...src[number-1] - размер массива определяется автоматически!
  template<int number> Ar<T,fast>& copy_from(const T (&src)[number]) { return copy_from(src, number); }
  // копируем this в весь массив dst[0]...dst[number-1] - размер массива определяется автоматически!
  template<int number> Ar<T,fast>& copy_to(T (&dst)[number]) const { return copy_to(dst, number); }

  // перевыделяет и стирает память для нового количества членов, если такое же - то просто стирает
  bool renew(int elems, bool need_clear = false); // не стирает если need_clear = false! ранее было true по умолчанию!

  // если массив мал, увеличивает его размер до нужного минимума без потери исх. содержимого (ост. элементы нулевые)
  void expand_to_nums(int min_elems, bool need_clear = true); // если не мал, то ничего не делает
  void change_array_memory(T* other_mem) { mem = other_mem; } // компилер требует public для этой функции!!

  void clear(); // заполняем память - нулями или пустыми конструкторами
  void fill(const T val); // (почленное) заполнение всего массива константой val
  // удаляет член [index] и "сжимает" массив, при decrement_nums = false размер массива не меняется
  void delete_element(int index, bool decrement_nums);

  int elements() const { return nums; } // число членов массива
  size_t element_size() const { return sizeof(T); } // размер одного члена в байтах
  size_t array_size() const { return elements()*element_size(); } // размер всего массива в байтах

  T* memory(int i = 0) const { return mem+i; } // указатель на i-й элемент массива (без защиты)
  T* memory_prot(int i) const { return in_range(0, i, elements()-1)? memory(i):0; } // указатель на i-й элемент массива или NULL за его пределами!

  T& operator[](int i)       { return mem[i]; } // скобки слева и справа от =
  T& operator[](int i) const { return mem[i]; } // скобки справа от =

  // operator[] с ограничением индекса в пределах массива
  T& operator()(int i)       { mintestmax(0, i, elements()-1); return (*this)[i]; }
  T& operator()(int i) const { mintestmax(0, i, elements()-1); return (*this)[i]; }

  operator       T*() const { return mem; } // преобразование объекта *this к указателю на тип его элементов T*
  operator const T*() const { return mem; } // не надо?

  Ar<T,fast>& operator=(const Ar<T,fast> &src); // копирование массивов одинаковых типов:
                      // здесь - и только здесь - размер массива this становится РАВНЫМ размеру src массива!
  // во всех остальных функциях недостаточный размер this увеличивается, чтобы скопировать указанное количество
  // src элементов, однако если места итак хватает - размер this не меняется (и т.о. может быть НЕ равным src)!

  // копируем в this первые number членов массива src, number < 0 означает весь src массив, причём
  // this[index0]   = src[0],
  // this[index0+1] = src[1], и т.д.
  Ar<T,fast>& copy_from(const Ar<T,fast> &src, int number = -1, int index0 = 0);

  // в C-массивах не должно быть меньше элементов чем копируемое, но этого не проверить...

  // копируем src->this первые number членов начиная с [0], src - это обычный C-массив элементов T src[number]
  Ar<T,fast>& copy_from(const T *src, int number); // 9.9.2010 убрал код с number=-1, теперь -1 не копирует ничего!!

  // копируем this->dst первые number членов начиная с [0], dst - это обычный C-массив элементов T dst[number]
  // не может быть скопировано более чем полное число членов this массива!!
  const Ar<T,fast>& copy_to(T *dst, int number) const; // 9.9.2010 убрал код с number=-1, теперь -1 не копирует ничего!!

  // копирование Ar и С массивов разных типов - с преобразованием типа элементов, аналогично предыдущим ф-м!
  template<class T2> Ar<T,fast>& copy_from(const Ar<T2> &src, int number = -1); // тут нет index0, он всегда 0!
  template<class T2> Ar<T,fast>& copy_from(const T2 *src, int number);
  template<class T2> const Ar<T,fast>& copy_to(T2 *dst, int number) const;

  // прибавление скаляра ко всем элементам массива this
  template<class T2> Ar<T,fast>& operator+=(const T2 &src) { for (int i=0; i<nums; i++) (*this)[i]+=src; return *this; }

  // прибавление массива src в конец массива this с увеличением его размера: this += src
  Ar<T,fast>& operator+=(const Ar<T,fast> &src)
  {
    int n0 = elements(); // длина this до прибавления src
    int n1 = src.elements(); // длина src
    expand_to_nums( n0 + n1, false ); // увеличиваем this до суммарной длины, без стирания
    copy_from(src, n1, n0); // копируем весь src массив в конец первоначальной длины this
    return *this;
  }
};

#pragma pack(pop)

template<class T,int fast> bool Ar<T,fast>::make_array(int elems, bool need_clear)
{
  mem = new_memory(elems);
  if (!mem) return false;

  /*
  В топике "new Operator (C++)" MSDN говорит что для VS2005:
  When new is used to allocate memory for a C++ class object,
  the object's constructor is called after the memory is allocated.
  Т.е. память инициализируется пустым конструктором объекта, возможно
  это и так для одиночных объектов, но массивы объектов оказываются
  заполнены мусором - проверено и на VS6, и на VS2005!
  И в Строутструпе 1999 г. на стр. 170 говорится, что стандартная
  реализация operator new() не инициализирует выделяемую память...
  */

  if (need_clear) clear(); // поэтому по умолчанию стираем память - см. конструктор Ar!
  return true;
}

template<class T,int fast> T* Ar<T,fast>::new_memory(int elems)
{
  nums = max2(0, elems);
  T *new_mem = nums>0? new T [nums] : 0;
  return new_mem;
}

template<class T,int fast> Ar<T,fast>::Ar(int elems) : fastmem(fast)
{
  make_array(elems, false);
}

template<class T,int fast> Ar<T,fast>::Ar(int elems, bool need_fill, const T val) : fastmem(fast)
{
  if ( make_array(elems, false) )
    if (need_fill) fill(val);
}

template<class T,int fast> Ar<T,fast>::Ar(const Ar<T,fast> &src)
{
  fastmem = src.fastmem;
  make_array(src.elements(), false);
  *this = src;
}

template<class T,int fast> void Ar<T,fast>::clear()
{
  if ( memory() == 0 || elements() <= 0 ) return;

  // для fastmem=FAST_ARRAY в объекте типа T не должно быть указателей и виртуальных функций?
  //
  // однако например для Ar<string> часто все бывает в порядке, float 0.f и double 0 также
  // состоят из всех 4 или 8 нулевых байт (на PC/Win32/VS2005)
  // однако на других системах/компилерах это может быть не так, поэтому даже те объекты,
  // которые можно быстро копировать (float/double) - не всегда можно быстро стирать...
  //
  // что касается string: несмотря на то, что конструктор по умолчанию string S() порождает
  // объект S с кучей ненулевых байт, скорее всего это просто мусор, т.к. строки до 16 литер
  // (15 литер и 0) хранятся прямо в теле объекта и когда мы принудительно стираем все байты
  // этой S она просто превращается в совершенно нормальную "пустую строку"!
  // но при увеличении размеров string > 15+1 литер всё кардинально меняется: в S появляется
  // указатель на область памяти в куче где хранится тело строки и при стирании эта память не
  // удаляется - происходит её "утечка", хотя сама стёртая строка по прежнему будет работать!
  //
  // что касается fastmem копирования Ar<string>, то тут будут происходить явления копирования
  // указателей с такой же утечкой и проблемами двойного удаления одной и той же памяти...
  //
  // т.о. существуют объекты, которые можно быстро стирать но нельзя быстро копировать, и вполне
  // возможно что существуют объекты, которые можно быстро копировать но нельзя быстро стирать!!
  if (fastmem == FAST_ARRAY)
  {
    size_t count = array_size();
    if (count > 0) memset( this->memory(), 0, count );
  }
  else // SLOW_ARRAY
  {
    // инициализируем все члены объектом, созданным пустым конструктором!
    fill( T() );
    // однако легко придумать пример, когда fast код сотрет что-то важное
    // внутри объекта, поэтому сделан более надежный slow вариант
    // инициализации в обычном цикле по членам массива
  }
}

template<class T,int fast> void Ar<T,fast>::fill(const T val)
{
  if (fastmem == FAST_ARRAY)
  {
    T zero_obj = T();
    // если константа val совпадает с "пустым" объектом zero_obj
    // (который при fastmem должен состоять из всех нулевых байт)
    // то делаем быструю очистку массива (устан-ем все члены в 0)
    if ( 0 == memcmp( &zero_obj, &val, sizeof(T)) ) clear();
  }
  else // SLOW_ARRAY
  {
    // почленно копируем val
    for (int i = 0; i < elements(); i++) (*this)[i] = val;
  }
}

template<class T,int fast> bool Ar<T,fast>::renew(int elems, bool need_clear)
{
  int new_elems = max2(0, elems);
  if ( new_elems != elements() )
  {
    safe_delete_array( mem );
    return make_array(new_elems, need_clear);
  }
  else if (need_clear) clear(); // для единообразия стираем массив
  return true;
}

template<class T,int fast> Ar<T,fast>& Ar<T,fast>::operator=(const Ar<T,fast> &src)
{
  renew( src.elements() ); // удаляем старый this и делаем размер this = размеру src!
  return copy_from( src, src.elements(), 0 ); // явно указываем какую функцию копирования использовать
}

template<class T,int fast> void Ar<T,fast>::delete_element(int index, bool decrement_nums)
{
  if ( !in_range(0, index, elements()-1) ) return;
  // "опускаем" все верхние над num члены массива на один индекс ниже
  for (int i = index; i < elements()-1; i++) (*this)[i] = (*this)[i+1];
  if (decrement_nums) --nums;
}

template<class T,int fast> void Ar<T,fast>::expand_to_nums(int min_elems, bool need_clear)
{
  if ( elements() < min_elems ) // если ЭТОТ (this) массив меньше чем нужный минимум
  {
    Ar<T> new_arr( min_elems, need_clear); // делаем новый массив нужной длины (и возм. стираем его)

    // копируем ЭТОТ массив (this) в новый (new_arr) бОльшего размера:
    // тут явно берётся функция copy_from(const T *src,int number) с указанием number, что
    // особенно важно, если this массив был сначала пустой (т.е. с number = 0), т.к. иначе
    // компилер выбирает не ту функцию, которая подразумевается мной, и не с тем number !!
    new_arr.copy_from( memory(), elements() );

    // меняем местами память массивов: swap(new_arr.mem, mem) требует public mem!
    T* tmp = memory();
    mem = new_arr.memory();
    new_arr.change_array_memory( tmp );

    // меняем старый размер массива на новый:
    nums = new_arr.elements();
  } // тут старая память ЭТОГО массива удаляется, а новая память остаётся!
}

template<class T,int fast> Ar<T,fast>& Ar<T,fast>::copy_from(const Ar<T,fast> &src, int number, int index0)
// копируем в this первые number членов массива src, number < 0 означает весь src массив, причём
// this[index0]   = src[0],
// this[index0+1] = src[1], и т.д.
{
  if (index0 < 0) index0 = 0; // просто защита!
  if (number < 0) number = src.elements(); // number < 0 заменяем на размер src массива!

  number = min2( src.elements(), number ); // number не должно быть больше чем размер src массива!
  // если number=0 (0 из аргумента или если src массив не содержит ни одного элемента) - ничего не копируем!
  if (number == 0) return *this;
  // else number > 0, надо копировать!

  int n = index0 + number; // n = минимально необходимый размер this массива!
  // n может быть больше размера this массива!
  if (n > 0) // ничего не копируем при n = 0
  {
    expand_to_nums( n, false ); // если в this не хватает мета - увеличиваем его длину!

    // быстро или медленно копируем src[0]->this[index0], src[1]->this[index0+1] и т.д. всего number раз
    if (fastmem == FAST_ARRAY)
    {
      void *dstm = memory(index0);
      void *srcm = src.memory();
      size_t bytes = number*element_size();
      memcpy(dstm, srcm, bytes);
    }
    else
    {
      for (int i = 0; i < number; i++)
        (*this)[index0+i] = src[i];
    }
  }
  return *this;
}

template<class T,int fast> Ar<T,fast>& Ar<T,fast>::copy_from(const T *src, int number)
{
  int n = number; // n может быть больше размера this массива!
  if (n > 0) // ничего не копируем при n = 0
  {
    expand_to_nums( n, false ); // если в this не хватает мета - увеличиваем его длину!
    // быстро или медленно копируем src->this
    if (fastmem == FAST_ARRAY)
    {
      void *dst = memory();
      size_t size = n*element_size();
      memcpy(dst, src, size);
    }
    else
    {
      for (int i = 0; i < n; i++)
        (*this)[i] = src[i];
    }
  }
  return *this;
}

template<class T,int fast> const Ar<T,fast>& Ar<T,fast>::copy_to(T *dst, int number) const
{
  int n = min2( elements(), number ); // n НЕ может быть больше размера this массива!
  if (n > 0) // ничего не копируем при n = 0
  {
    // быстро или медленно копируем this->dst
    if (fastmem == FAST_ARRAY)
      memcpy( dst, memory(), n*element_size() );
    else
      for (int i = 0; i < n; i++) dst[i] = (*this)[i];
  }
  return *this;
}

template<class T,int fast> template<class T2> Ar<T,fast>& Ar<T,fast>::copy_from(const Ar<T2> &src, int number)
{
  if (number < 0) number = src.elements(); // number < 0 заменяем на размер src массива!
  int n = min2( src.elements(), number ); // n = number, но не больше чем размер src массива!
  // n может быть больше размера this массива!
  if (n > 0) // ничего не копируем при n = 0
  {
    expand_to_nums( n, false ); // если в this не хватает мета - увеличиваем его длину!
    // медленно копируем src->this
    for (int i = 0; i < n; i++) (*this)[i] = src[i];
  }
  return *this;
}

template<class T,int fast> template<class T2> Ar<T,fast>& Ar<T,fast>::copy_from(const T2 *src, int number)
{
  int n = number; // n может быть больше размера this массива!
  if (n > 0) // ничего не копируем при n = 0
  {
    expand_to_nums( n, false ); // если в this не хватает мета - увеличиваем его длину!
    // медленно копируем src->this
    for (int i = 0; i < n; i++) (*this)[i] = src[i];
  }
  return *this;
}


template<class T,int fast> template<class T2> const Ar<T,fast>& Ar<T,fast>::copy_to(T2 *dst, int number) const
{
  int n = min2( elements(), number ); // n НЕ может быть больше размера this массива!
  if (n > 0) // ничего не копируем при n = 0
  {
    // медленно копируем this->dst
    for (int i = 0; i < n; i++) dst[i] = (*this)[i];
  }
  return *this;
}

