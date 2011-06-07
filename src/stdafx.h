// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "jdksmidi/world.h"
#include "jdksmidi/track.h"
#include "jdksmidi/multitrack.h"
#include "jdksmidi/filereadmultitrack.h"
#include "jdksmidi/fileread.h"
#include "jdksmidi/fileshow.h"
#include "jdksmidi/filewritemultitrack.h"
#include "jdksmidi/sequencer.h"

using namespace jdksmidi;

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

#include "unicode.h"
#include "tools.h"
#include "ar.h"
#include "files.h"
#include "win_tools.h"
#include "daccords.h"
#include "2h_converter.h"

