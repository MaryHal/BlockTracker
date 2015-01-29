#include "StringUtils.hpp"

#include <cstdio>

std::string strformat(const char* format, ...)
{
    char buf[1024];

    va_list marker;
    va_start(marker, format);
    vsprintf(buf, format, marker);
    va_end(marker);

    return buf;
}
