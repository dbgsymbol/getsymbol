#pragma once

#include "stdafx.h"
#include <SubAuth.h>
#include <winhttp.h>

#include <winnt.h>

#pragma comment(lib, "winhttp.lib")

#define APP_NAME L"GetSymbol"
#define APP_VERSION L"1.0.1"

#define UPDATE_CHECK_URL L"https://dbgsymbol.com/version"
#define WEBSITE_URL L"https://dbgsymbol.com"

HINTERNET CreateSession();

ULONG OpenURL(
	_In_ HINTERNET hsession,
	_In_ LPCWSTR url,
	_Out_ LPHINTERNET hconnect_ptr,
	_Out_ LPHINTERNET hrequest_ptr,
	_Out_opt_ PULONG total_length_ptr
);

BOOLEAN ReadRequest(
	_In_ HINTERNET hrequest,
	_Out_writes_bytes_(buffer_size) PVOID buffer,
	_In_ ULONG buffer_size,
	_Out_ PULONG readed_ptr,
	_Inout_opt_ PULONG total_readed_ptr
);

ULONG QueryContentLength(
	_In_ HINTERNET hrequest
);

ULONG QueryStatusCode(
	_In_ HINTERNET hrequest
);

ULONG BeginDownload(
	_In_ LPCWSTR url,
	_In_ HANDLE hFile,
	_In_ TCHAR** lpStringBuffer
);

