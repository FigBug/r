#include "mainheaders.h"

#ifdef __APPLE__
static size_t outputCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	int sz = size * nmemb;

	std::string* output = (std::string*)data;
	output->append(string((char*)ptr, sz));

	return sz;
}

string fetchUrl(string server, string path)
{
	CURL *curl;
	CURLcode res;

	string url = string("http://") + server + path;
	string output;

	curl = curl_easy_init();

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, outputCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&output);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "penis-browser/1.1");	

		res = curl_easy_perform(curl);

		curl_easy_cleanup(curl);
	}

	return output;
}
#endif

#ifdef _WIN32
string fetchUrl(string server, string path)
{
	BOOL bResults = FALSE;

	HINTERNET hSession = NULL;
	HINTERNET hConnect = NULL;
	HINTERNET hRequest = NULL;

	string result;

	wstring wserver(server.begin(), server.end());
	wstring wpath(path.begin(), path.end());

	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen(L"Penis Browser/1.1", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

	// Specify an HTTP server.
	if (hSession)
		hConnect = WinHttpConnect(hSession, wserver.c_str(), INTERNET_DEFAULT_HTTP_PORT, 0);

	// Create an HTTP request handle.
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"GET", wpath.c_str(), NULL, WINHTTP_NO_REFERER, NULL, NULL);

	// Send a request.
	if (hRequest)
		bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

	// End the request.
	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);

	// Keep checking for data until there is nothing left.
	if (bResults)
	{
		DWORD dwSize = 0;

		do 
		{
			DWORD dwDownloaded = 0;
			LPSTR pszOutBuffer;	

			// Check for available data.		
			WinHttpQueryDataAvailable(hRequest, &dwSize);

			// Allocate space for the buffer.
			pszOutBuffer = new char[dwSize + 1];

			// Read the Data.
			ZeroMemory(pszOutBuffer, dwSize + 1);

			if (WinHttpReadData( hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
				result.append(string(pszOutBuffer, dwDownloaded));

			// Free the memory allocated to the buffer.
			delete[] pszOutBuffer;

		} while (dwSize > 0);
	}

	// Close any open handles.
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);

	return result;
}
#endif
