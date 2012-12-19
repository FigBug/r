#pragma once

#include <stdio.h>

#include <string>
#include <sstream>
#include <vector>

#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#include <Winhttp.h>
#include <Shlobj.h>
#endif

#ifdef __APPLE__
#include <curl/curl.h>
#endif

using namespace std;
