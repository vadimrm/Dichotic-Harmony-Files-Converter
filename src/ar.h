
#pragma once

#pragma pack(push, 1) // ����� �� ��������� ������������ �� 4 �����...

const int SLOW_ARRAY = 0;
const int FAST_ARRAY = 1;

// fast: ����� ������ � ������� ������� 1=������, 0=�������� (� ��������� ��� ����� ����� T)
// fast: ��. ����� ������� ����������� � ���� ������� Ar<T,fast>::clear()
// ������������� ���� ��� fast=1 ������������� ������ ��� ����� ����� T!!
template<class T, int fast = FAST_ARRAY> class Ar // 9 �������� 2010 �. ������� fast �� 1 (����� ���� 0)
{
  T* mem; // ������ ��������� ���� class T
  int nums; // ���������� ������ ������� T[nums]
  int fastmem; // FAST_ARRAY = ������� ����� ������� � ����������� ������
  // ��� �������� ���������� ����� ����� ����������� ���������� �� this �������, � ��� ������ ����� �� ������ ���������!

  T* new_memory(int elems); // �������� ����� ������
  void make_array(int elems, bool need_clear); // �������� ����� ������ ��� ������, ���� ���� - ������� ���

  // ����������� �������:
  // template<class T2> void operator=(T2 &src); // ������ = �� ������ �����! ��: �� ����������...
public:
  Ar(const Ar<T,fast> &src); // ����������� ����������� - ����� ��� �������� ������� �� ��������!
  Ar( int elems = 0); // ����������� �� ��������� - ��� �������������!
  // ����������� ������� T[elems] � ����������� ����� ������� ���������� val (�� ��������� �������)
  // ����� ������ ������ need_fill = true ����� �������� ������ - ��. ����������� � make_array()!
  Ar( int elems, bool need_fill, const T val = T() ); // need_fill ������� ��� ������� �� ������ �������������
  // ����������� ����������� �� �������� ������� T src[], number - ������ ������ �������!
  template<int number> Ar(const T (&src)[number]) : fastmem(fast) // ������ ������� ������������ �������������!
  {
    make_array(number, false);
    copy_from(src);
  }
  ~Ar() { safe_delete_array( mem ); }

  // �������� � this ���� ������ src[0]...src[number-1] - ������ ������� ������������ �������������!
  template<int number> Ar<T,fast>& copy_from(const T (&src)[number]) { return copy_from(src, number); }
  // �������� this � ���� ������ dst[0]...dst[number-1] - ������ ������� ������������ �������������!
  template<int number> Ar<T,fast>& copy_to(T (&dst)[number]) const { return copy_to(dst, number); }

  // ������������ � ������� ������ ��� ������ ���������� ������, ���� ����� �� - �� ������ �������
  void renew(int elems, bool need_clear = true); // �� ������� ���� need_clear = false!

  // ���� ������ ���, ����������� ��� ������ �� ������� �������� ��� ������ ���. ����������� (���. �������� �������)
  void expand_to_nums(int min_elems, bool need_clear = true); // ���� �� ���, �� ������ �� ������
  void change_array_memory(T* other_mem) { mem = other_mem; } // �������� ������� public ��� ���� �������!!

  void clear(); // ��������� ������ - ������ ��� ������� ��������������
  void fill(const T val); // (���������) ���������� ����� ������� ���������� val
  // ������� ���� [index] � "�������" ������, ��� decrement_nums = false ������ ������� �� ��������
  void delete_element(int index, bool decrement_nums);

  int elements() const { return nums; } // ����� ������ �������
  size_t element_size() const { return sizeof(T); } // ������ ������ ����� � ������
  size_t array_size() const { return elements()*element_size(); } // ������ ����� ������� � ������

  T* memory(int i = 0) const { return mem+i; } // ��������� �� i-� ������� ������� (��� ������)
  T* memory_prot(int i) const { return in_range(0, i, elements()-1)? memory(i):0; } // ��������� �� i-� ������� ������� ��� NULL �� ��� ���������!

  T& operator[](int i)       { return mem[i]; } // ������ ����� � ������ �� =
  T& operator[](int i) const { return mem[i]; } // ������ ������ �� =

  // operator[] � ������������ ������� � �������� �������
  T& operator()(int i)       { mintestmax(0, i, elements()-1); return (*this)[i]; }
  T& operator()(int i) const { mintestmax(0, i, elements()-1); return (*this)[i]; }

  operator       T*() const { return mem; } // �������������� ������� *this � ��������� �� ��� ��� ��������� T*
  operator const T*() const { return mem; } // �� ����?

  Ar<T,fast>& operator=(const Ar<T,fast> &src); // ����������� �������� ���������� �����:
                      // ����� - � ������ ����� - ������ ������� this ���������� ������ ������� src �������!
  // �� ���� ��������� �������� ������������� ������ this �������������, ����� ����������� ��������� ����������
  // src ���������, ������ ���� ����� ���� ������� - ������ this �� �������� (� �.�. ����� ���� �� ������ src)!

  // �������� � this ������ number ������ ������� src, number < 0 �������� ���� src ������, ������
  // this[index0]   = src[0],
  // this[index0+1] = src[1], � �.�.
  Ar<T,fast>& copy_from(const Ar<T,fast> &src, int number = -1, int index0 = 0);

  // � C-�������� �� ������ ���� ������ ��������� ��� ����������, �� ����� �� ���������...

  // �������� src->this ������ number ������ ������� � [0], src - ��� ������� C-������ ��������� T src[number]
  Ar<T,fast>& copy_from(const T *src, int number); // 9.9.2010 ����� ��� � number=-1, ������ -1 �� �������� ������!!

  // �������� this->dst ������ number ������ ������� � [0], dst - ��� ������� C-������ ��������� T dst[number]
  // �� ����� ���� ����������� ����� ��� ������ ����� ������ this �������!!
  const Ar<T,fast>& copy_to(T *dst, int number) const; // 9.9.2010 ����� ��� � number=-1, ������ -1 �� �������� ������!!

  // ����������� Ar � � �������� ������ ����� - � ��������������� ���� ���������, ���������� ���������� �-�!
  template<class T2> Ar<T,fast>& copy_from(const Ar<T2> &src, int number = -1); // ��� ��� index0, �� ������ 0!
  template<class T2> Ar<T,fast>& copy_from(const T2 *src, int number);
  template<class T2> const Ar<T,fast>& copy_to(T2 *dst, int number) const;

  // ����������� ������� �� ���� ��������� ������� this
  template<class T2> Ar<T,fast>& operator+=(const T2 &src) { for (int i=0; i<nums; i++) (*this)[i]+=src; return *this; }

  // ����������� ������� src � ����� ������� this � ����������� ��� �������: this += src
  Ar<T,fast>& operator+=(const Ar<T,fast> &src)
  { 
    int n0 = elements(); // ����� this �� ����������� src
    int n1 = src.elements(); // ����� src
    expand_to_nums( n0 + n1, false ); // ����������� this �� ��������� �����, ��� ��������
    copy_from(src, n1, n0); // �������� ���� src ������ � ����� �������������� ����� this
    return *this;
  }
};

#pragma pack(pop)

template<class T,int fast> void Ar<T,fast>::make_array(int elems, bool need_clear)
{
  mem = new_memory(elems);

  /*
  � ������ "new Operator (C++)" MSDN ������� ��� ��� VS2005:
  When new is used to allocate memory for a C++ class object,
  the object's constructor is called after the memory is allocated.
  �.�. ������ ���������������� ������ ������������� �������, ��������
  ��� � ��� ��� ��������� ��������, �� ������� �������� �����������
  ��������� ������� - ��������� � �� VS6, � �� VS2005!
  � � ������������ 1999 �. �� ���. 170 ���������, ��� �����������
  ���������� operator new() �� �������������� ���������� ������...
  */

  if (need_clear) clear(); // ������� �� ��������� ������� ������ - ��. ����������� Ar!
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
  make_array(elems, false);
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

  // ��� fastmem=FAST_ARRAY � ������� ���� T �� ������ ���� ���������� � ����������� �������?
  //
  // ������ �������� ��� Ar<string> ����� ��� ������ � �������, float 0.f � double 0 �����
  // ������� �� ���� 4 ��� 8 ������� ���� (�� PC/Win32/VS2005)
  // ������ �� ������ ��������/���������� ��� ����� ���� �� ���, ������� ���� �� �������,
  // ������� ����� ������ ���������� (float/double) - �� ������ ����� ������ �������...
  //
  // ��� �������� string: �������� �� ��, ��� ����������� �� ��������� string S() ���������
  // ������ S � ����� ��������� ����, ������ ����� ��� ������ �����, �.�. ������ �� 16 �����
  // (15 ����� � 0) �������� ����� � ���� ������� � ����� �� ������������� ������� ��� �����
  // ���� S ��� ������ ������������ � ���������� ���������� "������ ������"!
  // �� ��� ���������� �������� string > 15+1 ����� �� ����������� ��������: � S ����������
  // ��������� �� ������� ������ � ���� ��� �������� ���� ������ � ��� �������� ��� ������ ��
  // ��������� - ���������� � "������", ���� ���� ������ ������ �� �������� ����� ��������!
  //
  // ��� �������� fastmem ����������� Ar<string>, �� ��� ����� ����������� ������� �����������
  // ���������� � ����� �� ������� � ���������� �������� �������� ����� � ��� �� ������...
  //
  // �.�. ���������� �������, ������� ����� ������ ������� �� ������ ������ ����������, � ������
  // �������� ��� ���������� �������, ������� ����� ������ ���������� �� ������ ������ �������!!
  if (fastmem == FAST_ARRAY)
  {
    size_t count = array_size();
    if (count > 0) memset( this->memory(), 0, count );
  }
  else // SLOW_ARRAY
  {
    // �������������� ��� ����� ��������, ��������� ������ �������������!
    fill( T() );
    // ������ ����� ��������� ������, ����� fast ��� ������ ���-�� ������
    // ������ �������, ������� ������ ����� �������� slow �������
    // ������������� � ������� ����� �� ������ �������
  }
}

template<class T,int fast> void Ar<T,fast>::fill(const T val)
{
  if (fastmem == FAST_ARRAY)
  {
    T zero_obj = T();
    // ���� ��������� val ��������� � "������" �������� zero_obj
    // (������� ��� fastmem ������ �������� �� ���� ������� ����)
    // �� ������ ������� ������� ������� (�����-�� ��� ����� � 0)
    if ( 0 == memcmp( &zero_obj, &val, sizeof(T)) ) clear();
  }
  else // SLOW_ARRAY
  {
    // �������� �������� val
    for (int i = 0; i < elements(); i++) (*this)[i] = val;
  }
}

template<class T,int fast> void Ar<T,fast>::renew(int elems, bool need_clear)
{
  int new_elems = max2(0, elems);
  if ( new_elems != elements() )
  {
    safe_delete_array( mem );
    make_array(new_elems, need_clear);
  }
  else if (need_clear) clear(); // ��� ������������ ������� ������
}

template<class T,int fast> Ar<T,fast>& Ar<T,fast>::operator=(const Ar<T,fast> &src)
{
  renew( src.elements() ); // ������� ������ this � ������ ������ this = ������� src!
  return copy_from( src, src.elements(), 0 ); // ���� ��������� ����� ������� ����������� ������������
}

template<class T,int fast> void Ar<T,fast>::delete_element(int index, bool decrement_nums)
{
  if ( !in_range(0, index, elements()-1) ) return;
  // "��������" ��� ������� ��� num ����� ������� �� ���� ������ ����
  for (int i = index; i < elements()-1; i++) (*this)[i] = (*this)[i+1];
  if (decrement_nums) --nums;
}

template<class T,int fast> void Ar<T,fast>::expand_to_nums(int min_elems, bool need_clear)
{
  if ( elements() < min_elems ) // ���� ���� (this) ������ ������ ��� ������ �������
  {
    Ar<T> new_arr( min_elems, need_clear); // ������ ����� ������ ������ ����� (� ����. ������� ���)

    // �������� ���� ������ (this) � ����� (new_arr) �������� �������:
    // ��� ���� ������ ������� copy_from(const T *src,int number) � ��������� number, ���
    // �������� �����, ���� this ������ ��� ������� ������ (�.�. � number = 0), �.�. �����
    // �������� �������� �� �� �������, ������� ��������������� ����, � �� � ��� number !!
    new_arr.copy_from( memory(), elements() );

    // ������ ������� ������ ��������: swap(new_arr.mem, mem) ������� public mem!
    T* tmp = memory();
    mem = new_arr.memory();
    new_arr.change_array_memory( tmp );

    // ������ ������ ������ ������� �� �����:
    nums = new_arr.elements();
  } // ��� ������ ������ ����� ������� ���������, � ����� ������ �������!
}

template<class T,int fast> Ar<T,fast>& Ar<T,fast>::copy_from(const Ar<T,fast> &src, int number, int index0)
// �������� � this ������ number ������ ������� src, number < 0 �������� ���� src ������, ������
// this[index0]   = src[0],
// this[index0+1] = src[1], � �.�.
{
  if (index0 < 0) index0 = 0; // ������ ������!
  if (number < 0) number = src.elements(); // number < 0 �������� �� ������ src �������!

  number = min2( src.elements(), number ); // number �� ������ ���� ������ ��� ������ src �������!
  // ���� number=0 (0 �� ��������� ��� ���� src ������ �� �������� �� ������ ��������) - ������ �� ��������!
  if (number == 0) return *this;
  // else number > 0, ���� ����������!

  int n = index0 + number; // n = ���������� ����������� ������ this �������!
  // n ����� ���� ������ ������� this �������!
  if (n > 0) // ������ �� �������� ��� n = 0
  {
    expand_to_nums( n, false ); // ���� � this �� ������� ���� - ����������� ��� �����!

    // ������ ��� �������� �������� src[0]->this[index0], src[1]->this[index0+1] � �.�. ����� number ���
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
  int n = number; // n ����� ���� ������ ������� this �������!
  if (n > 0) // ������ �� �������� ��� n = 0
  {
    expand_to_nums( n, false ); // ���� � this �� ������� ���� - ����������� ��� �����!
    // ������ ��� �������� �������� src->this
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
  int n = min2( elements(), number ); // n �� ����� ���� ������ ������� this �������!
  if (n > 0) // ������ �� �������� ��� n = 0
  {
    // ������ ��� �������� �������� this->dst
    if (fastmem == FAST_ARRAY)
      memcpy( dst, memory(), n*element_size() );
    else
      for (int i = 0; i < n; i++) dst[i] = (*this)[i];
  }
  return *this;
}

template<class T,int fast> template<class T2> Ar<T,fast>& Ar<T,fast>::copy_from(const Ar<T2> &src, int number)
{
  if (number < 0) number = src.elements(); // number < 0 �������� �� ������ src �������!
  int n = min2( src.elements(), number ); // n = number, �� �� ������ ��� ������ src �������!
  // n ����� ���� ������ ������� this �������!
  if (n > 0) // ������ �� �������� ��� n = 0
  {
    expand_to_nums( n, false ); // ���� � this �� ������� ���� - ����������� ��� �����!
    // �������� �������� src->this
    for (int i = 0; i < n; i++) (*this)[i] = src[i];
  }
  return *this;
}

template<class T,int fast> template<class T2> Ar<T,fast>& Ar<T,fast>::copy_from(const T2 *src, int number)
{
  int n = number; // n ����� ���� ������ ������� this �������!
  if (n > 0) // ������ �� �������� ��� n = 0
  {
    expand_to_nums( n, false ); // ���� � this �� ������� ���� - ����������� ��� �����!
    // �������� �������� src->this
    for (int i = 0; i < n; i++) (*this)[i] = src[i];
  }
  return *this;
}


template<class T,int fast> template<class T2> const Ar<T,fast>& Ar<T,fast>::copy_to(T2 *dst, int number) const
{
  int n = min2( elements(), number ); // n �� ����� ���� ������ ������� this �������!
  if (n > 0) // ������ �� �������� ��� n = 0
  {
    // �������� �������� this->dst
    for (int i = 0; i < n; i++) dst[i] = (*this)[i];
  }
  return *this;
}

