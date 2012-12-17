#pragma once

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <tchar.h>

#include <string>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#include <Winhttp.h>
#endif

#ifdef __APPLE__
#include <curl/curl.h>
#endif

using namespace std;
