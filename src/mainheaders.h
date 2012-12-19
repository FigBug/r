#pragma once

#include <stdio.h>
#include <tchar.h>

#include <string>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#include <Winhttp.h>
#endif

#ifdef __APPLE__
#include <curl/curl.h>
#endif

using namespace std;
