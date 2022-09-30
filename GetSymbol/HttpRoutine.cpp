
#include "stdafx.h"

#include "HttpRoutine.h"


HINTERNET CreateSession() {
	
	HINTERNET hSession;
	
	TCHAR userAgent[MAX_PATH];

	wsprintf(userAgent, L"%s/%s (+%s)", APP_NAME, APP_VERSION, WEBSITE_URL);
		
	hSession = WinHttpOpen(userAgent,
		WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0
		);

	if (!hSession)
		return NULL;

	ULONG ulOption = NULL;

	// enable secure protocols
	ulOption = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1 |
		WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3;
	WinHttpSetOption(
		hSession,
		WINHTTP_OPTION_SECURE_PROTOCOLS,
		&ulOption,
		sizeof(ULONG)
	);

	// disable redirect from https to http
	ulOption = WINHTTP_OPTION_REDIRECT_POLICY_DISALLOW_HTTPS_TO_HTTP;
	WinHttpSetOption(
		hSession,
		WINHTTP_OPTION_REDIRECT_POLICY,
		&ulOption,
		sizeof(ULONG)
	);

	// enable compression feature
	ulOption = WINHTTP_DECOMPRESSION_FLAG_GZIP | WINHTTP_DECOMPRESSION_FLAG_DEFLATE;
	WinHttpSetOption(
		hSession,
		WINHTTP_OPTION_DECOMPRESSION,
		&ulOption,
		sizeof(ULONG)
	);

	// enable http2 protocol (win10+)
	ulOption = WINHTTP_PROTOCOL_FLAG_HTTP2;
	WinHttpSetOption(
		hSession,
		WINHTTP_OPTION_ENABLE_HTTP_PROTOCOL,
		&ulOption,
		sizeof(ULONG)
	);
	
	return hSession;
}

_Success_(return == ERROR_SUCCESS)
ULONG OpenURL(
	_In_ HINTERNET hsession,
	_In_ LPCWSTR url,
	_Out_ LPHINTERNET hconnect_ptr,
	_Out_ LPHINTERNET hrequest_ptr,
	_Out_opt_ PULONG total_length_ptr
)
{
	HINTERNET hconnect;
	HINTERNET hrequest;
	ULONG attempts;
	ULONG flags;
	ULONG status;
	BOOL result;

	*hconnect_ptr = NULL;
	*hrequest_ptr = NULL;

	if (total_length_ptr)
		*total_length_ptr = 0;

	URL_COMPONENTS url_component = { 0, };

	TCHAR   lpszHostName[MAX_PATH];
	TCHAR	lpszUrlPath[MAX_PATH];

	RtlZeroMemory(lpszHostName, sizeof(lpszHostName));
	RtlZeroMemory(lpszUrlPath, sizeof(lpszUrlPath));

	url_component.lpszHostName = lpszHostName;
	url_component.dwHostNameLength = MAX_PATH;
	url_component.lpszUrlPath = lpszUrlPath;
	url_component.dwUrlPathLength = MAX_PATH;
	url_component.dwStructSize = sizeof(URL_COMPONENTS);
	url_component.dwSchemeLength = 1;

	if (!WinHttpCrackUrl(url, (ULONG)wcslen(url), 0, &url_component))
	{
		status = GetLastError();
		return status;
	}

	hconnect = WinHttpConnect(hsession, url_component.lpszHostName, url_component.nPort, 0);

	if (!hconnect)
	{
		status = GetLastError();
		goto Exit;
	}

	flags = WINHTTP_FLAG_REFRESH;

	if (url_component.nScheme == INTERNET_SCHEME_HTTPS)
		flags |= WINHTTP_FLAG_SECURE;

	hrequest = WinHttpOpenRequest(
		hconnect,
		NULL,
		url_component.lpszUrlPath,
		NULL,
		WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		flags
	);

	if (!hrequest)
	{
		status = GetLastError();

		WinHttpCloseHandle(hconnect);

		goto Exit;
	}

	ULONG ulOption = NULL;
	ulOption = WINHTTP_DISABLE_KEEP_ALIVE;
	WinHttpSetOption(
		hrequest,
		WINHTTP_OPTION_DISABLE_FEATURE,
		&ulOption,
		sizeof(ULONG)
	);


	attempts = 6;

	do
	{
		result = WinHttpSendRequest(
			hrequest,
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0,
			WINHTTP_NO_REQUEST_DATA,
			0,
			WINHTTP_IGNORE_REQUEST_TOTAL_LENGTH,
			0
		);

		if (!result)
		{
			status = GetLastError();

			if (status == ERROR_WINHTTP_RESEND_REQUEST)
			{
				continue;
			}
			else if (status == ERROR_WINHTTP_CONNECTION_ERROR)
			{
				ulOption = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1 |
					WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
				result = WinHttpSetOption(
					hsession,
					WINHTTP_OPTION_SECURE_PROTOCOLS,
					&ulOption,
					sizeof(ULONG)
				);

				if (!result)
					break;
			}
			else if (status == ERROR_WINHTTP_SECURE_FAILURE)
			{
				ulOption = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
				result = WinHttpSetOption(
					hrequest,
					WINHTTP_OPTION_SECURITY_FLAGS,
					&ulOption,
					sizeof(ULONG)
				);

				if (!result)
					break;
			}
			else
			{
				break;
			}
		}
		else
		{
			if (!WinHttpReceiveResponse(hrequest, NULL))
			{
				status = GetLastError();
			}
			else
			{
				if (QueryStatusCode(hrequest) != HTTP_STATUS_OK)
				{
					status = ERROR_WINHTTP_INVALID_SERVER_RESPONSE;
					break;
				}

				if (total_length_ptr)
					*total_length_ptr = QueryContentLength(hrequest);

				*hconnect_ptr = hconnect;
				*hrequest_ptr = hrequest;

				status = ERROR_SUCCESS;

				goto Exit;
			}
		}
	} while (--attempts);

	WinHttpCloseHandle(hrequest);
	WinHttpCloseHandle(hconnect);

Exit:
	return status;
}

_Success_(return)
BOOLEAN ReadRequest(
	_In_ HINTERNET hrequest,
	_Out_writes_bytes_(buffer_size) PVOID buffer,
	_In_ ULONG buffer_size,
	_Out_ PULONG readed_ptr,
	_Inout_opt_ PULONG total_readed_ptr
)
{
	ULONG readed;

	if (!WinHttpReadData(hrequest, buffer, buffer_size, &readed))
	{
		*readed_ptr = 0;

		return FALSE;
	}

	if (!readed)
	{
		*readed_ptr = 0;

		return FALSE;
	}

	*readed_ptr = readed;

	if (total_readed_ptr)
		*total_readed_ptr += readed;

	return TRUE;
}

ULONG QueryContentLength(
	_In_ HINTERNET hrequest
)
{
	ULONG content_length;
	ULONG size;

	size = sizeof(ULONG);

	if (WinHttpQueryHeaders(
		hrequest,
		WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
		WINHTTP_HEADER_NAME_BY_INDEX,
		&content_length,
		&size,
		WINHTTP_NO_HEADER_INDEX))
	{
		return content_length;
	}

	return 0;
}



ULONG QueryStatusCode(
	_In_ HINTERNET hrequest
)
{
	ULONG status;
	ULONG size;

	size = sizeof(ULONG);

	if (WinHttpQueryHeaders(
		hrequest,
		WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		WINHTTP_HEADER_NAME_BY_INDEX,
		&status,
		&size,
		WINHTTP_NO_HEADER_INDEX))
	{
		return status;
	}

	return 0;
}



_Success_(return == ERROR_SUCCESS)
ULONG BeginDownload(
	_In_ LPCWSTR url,
	_In_ HANDLE hFile,
	_In_ TCHAR ** lpStringBuffer
)
{

	HINTERNET hconnect;
	HINTERNET hrequest;
	CHAR* lpBuffer;
	ULONG buffer_length;
	ULONG total_readed;
	ULONG total_length;
	ULONG readed_length;
	ULONG unused;
	LONG status;


	if (!hFile && !lpStringBuffer) {
		return ERROR_INVALID_PARAMETER;
	}

	HINTERNET hsession = CreateSession();

	if (!hsession) {
		status = GetLastError();
		return status;
	}

	status = OpenURL(hsession, url, &hconnect, &hrequest, &total_length);

	if (status != ERROR_SUCCESS) {
		WinHttpCloseHandle(hsession);
		return status;
	}
		

	if (!hFile) {
		*lpStringBuffer = (TCHAR*)LocalAlloc(LPTR, total_length);
	}

	buffer_length = 64 * 1024;

	lpBuffer = (CHAR *)LocalAlloc(LPTR, buffer_length);

	total_readed = 0;

	while (ReadRequest(
		hrequest,
		lpBuffer,
		buffer_length,
		&readed_length,
		&total_readed))
	{
		if (hFile) {
			if (!WriteFile(hFile, lpBuffer, readed_length, &unused, NULL))
			{
				status = GetLastError();
				break;
			}
		}
		else {
			memcpy(*lpStringBuffer + (total_readed - readed_length), lpBuffer, readed_length);
		}
			
	}

	LocalFree(lpBuffer);

	
	WinHttpCloseHandle(hrequest);
	WinHttpCloseHandle(hconnect);
	WinHttpCloseHandle(hsession);

	return status;
}

