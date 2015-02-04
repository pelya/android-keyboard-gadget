/*
 * Copyright (C) 2015 Sergii Pylypenko
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _XSDL_TOOLS_H_
#define _XSDL_TOOLS_H_

// Convert 
static inline int UnicodeToUtf8(unsigned int src, char * dest)
{
    int len = 0;
    if ( src <= 0x007f) {
        *dest++ = (char)src;
        len = 1;
    } else if (src <= 0x07ff) {
        *dest++ = (char)0xc0 | (src >> 6);
        *dest++ = (char)0x80 | (src & 0x003f);
        len = 2;
    } else if (src == 0xFEFF) {
        // nop -- zap the BOM
    } else if (src >= 0xD800 && src <= 0xDFFF) {
        // surrogates not supported
    } else if (src <= 0xffff) {
        *dest++ = (char)0xe0 | (src >> 12);
        *dest++ = (char)0x80 | ((src >> 6) & 0x003f);
        *dest++ = (char)0x80 | (src & 0x003f);
        len = 3;
    } else if (src <= 0xffff) {
        *dest++ = (char)0xf0 | (src >> 18);
        *dest++ = (char)0x80 | ((src >> 12) & 0x3f);
        *dest++ = (char)0x80 | ((src >> 6) & 0x3f);
        *dest++ = (char)0x80 | (src & 0x3f);
        len = 4;
    } else {
        // out of Unicode range
    }
    *dest = 0;
    return len;
}

/**
 * Reads the next UTF-8-encoded character from the byte array ranging
 * from {@code *pstart} up to, but not including, {@code end}. If the
 * conversion succeeds, the {@code *pstart} iterator is advanced,
 * the codepoint is stored into {@code *pcp}, and the function returns
 * 0. Otherwise the conversion fails, {@code errno} is set to
 * {@code EILSEQ} and the function returns -1.
 */
static inline int unicode_from_utf8(const char **pstart, const char *end, unsigned int *pcp) {
        size_t len, i;
        unsigned int cp, min;
        const char *buf;

        buf = *pstart;
        if (buf == end)
                goto error;

        if (buf[0] < 0x80) {
                len = 1;
                min = 0;
                cp = buf[0];
        } else if (buf[0] < 0xC0) {
                goto error;
        } else if (buf[0] < 0xE0) {
                len = 2;
                min = 1 << 7;
                cp = buf[0] & 0x1F;
        } else if (buf[0] < 0xF0) {
                len = 3;
                min = 1 << (5 + 6);
                cp = buf[0] & 0x0F;
        } else if (buf[0] < 0xF8) {
                len = 4;
                min = 1 << (4 + 6 + 6);
                cp = buf[0] & 0x07;
        } else {
                goto error;
        }

        if (buf + len > end)
                goto error;

        for (i = 1; i < len; i++) {
                if ((buf[i] & 0xC0) != 0x80)
                        goto error;
                cp = (cp << 6) | (buf[i] & 0x3F);
        }

        if (cp < min)
                goto error;

        if (0xD800 <= cp && cp <= 0xDFFF)
                goto error;

        if (0x110000 <= cp)
                goto error;

        *pstart += len;
        *pcp = cp;
        return 0;

error:
        return -1;
}

static inline unsigned int UnicodeFromUtf8(const char **src)
{
	unsigned int ret = 0;
	const char *end = *src + strlen(*src);
	if( end == *src )
		return 0;
	if( unicode_from_utf8(src, end, &ret) == 0 )
		return ret;
	return 0;
}

#endif
