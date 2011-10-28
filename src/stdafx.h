/*
  stdafx.h здесь все заголовки, которые никогда не меняются
*/

#pragma once

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#define _WIN32_WINNT 0x0500 // версия начиная с Windows 2000 - надо для консольных функций!
#include <windows.h>
//#include <windowsx.h>

#include <mmsystem.h>

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "../libjdksmidi/include/jdksmidi/world.h"
#include "../libjdksmidi/include/jdksmidi/track.h"
#include "../libjdksmidi/include/jdksmidi/multitrack.h"
#include "../libjdksmidi/include/jdksmidi/filereadmultitrack.h"
#include "../libjdksmidi/include/jdksmidi/fileread.h"
#include "../libjdksmidi/include/jdksmidi/fileshow.h"
#include "../libjdksmidi/include/jdksmidi/filewritemultitrack.h"
#include "../libjdksmidi/include/jdksmidi/sequencer.h"

using namespace jdksmidi;
