#include "gzip/GZipHelper.h"

bool GZipCompress(unsigned char* inbuf, unsigned int inbuf_size, unsigned char* outbuf, unsigned int& outbuf_size)
{
	CA2GZIP gz((char*)inbuf, inbuf_size);
	outbuf_size = gz.Length;
	if (!outbuf) {
		return false;
	}
	memcpy(outbuf, gz.pgzip, gz.Length);
	return true;
}

bool GZipDecompress(unsigned char* inbuf, unsigned int inbuf_size, unsigned char* outbuf, unsigned int& outbuf_size)
{
	CGZIP2A gz((LPGZIP)inbuf, inbuf_size);
	outbuf_size = gz.Length;
	if (!outbuf) {
		return false;
	}
	memcpy(outbuf, gz.psz, gz.Length);
	return true;
}

