/*
 * utils.c
 *
 *  Created on: 2016年2月25日
 *      Author: jk
 */

#include <string.h>

#include "utils.h"

/*
 * 去掉字符串开头的空格
 *
 */
const unsigned char* string_trimLeft(const unsigned char* string)
{
    const unsigned char* p = string;

    while(*p == ' ') p++;

    return p;
}


/*
 * 去掉字符串末尾的空格
 *
 */
void string_trimRight(unsigned char* string)
{
    unsigned char* p = string + strlen(string);

    while(*p == ' ') p--;

    *(p + 1) = 0;

    return;
}

//equivalent to eat_acsii_to_ucs2
void ascii2unicode(unsigned short* out, const unsigned char* in)
{
    int i = 0;
    unsigned char* outp = (unsigned char*)out;
    const unsigned char* inp = in;

    while( inp[i] )
    {
        outp[i * 2] = inp[i];
        outp[i * 2 + 1] = 0x00;
        i++;
    }

    out[i] = 0;
}

void unicode2ascii(unsigned char* out, const unsigned short* in)
{
    int i = 0;

    while( in[i] )
    {
        out[i] = in[i] & 0xFF;
        i++;
    }

    out[i] = 0;
}
