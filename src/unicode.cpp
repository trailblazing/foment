/*

Foment

*/

#include <string.h>
#include "foment.hpp"
#include "unicode.hpp"
#include "unidata.hpp"
#include "convertutf.h"

// ---- System String Conversions ----

static unsigned int Utf16LengthOfString(FObject s)
{
    unsigned int sl = 0;

    for (unsigned int idx = 0; idx < StringLength(s); idx++)
    {
        sl += 1;

        if (AsString(s)->String[idx] > 0xFFFF)
            sl += 1;
    }

    return(sl);
}

SCh * ConvertToStringS(FObject s, SCh * b, unsigned int bl)
{
    FAssert(StringP(s));

    unsigned int sl = Utf16LengthOfString(s) + 1;
    if (bl < sl)
    {
        FObject bv = MakeBytevector(sl * sizeof(SCh));
        b = (SCh *) AsBytevector(bv)->Vector;
    }

    const UTF32 * utf32 = (UTF32 *) AsString(s)->String;
    UTF16 * utf16 = (UTF16 *) b;
    ConversionResult cr = ConvertUTF32toUTF16(&utf32,
            (UTF32 *) AsString(s)->String + StringLength(s), &utf16, (UTF16 *) b + sl - 1,
            lenientConversion);

    FAssert(cr == conversionOK);

    *utf16 = 0;
    return(b);
}

#define UNI_SUR_HIGH_START  (UTF32)0xD800
#define UNI_SUR_HIGH_END    (UTF32)0xDBFF
#define UNI_SUR_LOW_START   (UTF32)0xDC00
#define UNI_SUR_LOW_END     (UTF32)0xDFFF

static unsigned int ChLengthOfUtf16(SCh * ss, unsigned int ssl)
{
    unsigned int sl = 0;

    while (ssl > 0)
    {
        if (*ss < UNI_SUR_HIGH_START || *ss > UNI_SUR_HIGH_END)
            sl += 1;

        ss += 1;
        ssl -= 1;
    }

    return(sl);
}

FObject MakeStringS(SCh * ss)
{
    return(MakeStringS(ss, wcslen(ss)));
}

FObject MakeStringS(SCh * ss, unsigned int ssl)
{
    unsigned int sl = ChLengthOfUtf16(ss, ssl);
    FObject s = MakeString(0, sl);

    const UTF16 * utf16 = (UTF16 *) ss;
    UTF32 * utf32 = (UTF32 *) AsString(s)->String;
    ConversionResult cr = ConvertUTF16toUTF32(&utf16, utf16 + ssl, &utf32, utf32 + sl,
            lenientConversion);

    FAssert(cr == conversionOK);
    FAssert(utf32 == (UTF32 *) AsString(s)->String + sl);

    return(s);
}

// ---- Unicode ----

int WhitespaceP(FCh ch)
{
    // From:
    // PropList-6.2.0.txt
    // Date: 2012-05-23, 20:34:59 GMT [MD]

    if (ch >= 0x0009 && ch <= 0x000D)
        return(1);
    else if (ch == 0x0020)
        return(1);
    else if (ch == 0x0085)
        return(1);
    else if (ch == 0x00A0)
        return(1);
    else if (ch == 0x1680)
        return(1);
    else if (ch == 0x180E)
        return(1);
    else if (ch >= 0x2000 && ch <= 0x200A)
        return(1);
    else if (ch == 0x2028)
        return(1);
    else if (ch == 0x2029)
        return(1);
    else if (ch == 0x202F)
        return(1);
    else if (ch == 0x205F)
        return(1);
    else if (ch == 0x3000)
        return(1);

    return(0);
}

int DigitValue(FCh ch)
{
    // From:
    // DerivedNumericType-6.2.0.txt
    // Date: 2012-08-13, 19:20:20 GMT [MD]

    // Return 0 to 9 for digit values and -1 if not a digit.

    if (ch < 0x1D00)
    {
        switch (ch >> 8)
        {
        case 0x00:
            if (ch >= 0x0030 && ch <= 0x0039)
                return(ch - 0x0030);
            return(-1);

        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
            return(-1);

        case 0x06:
            if (ch >= 0x0660 && ch <= 0x0669)
                return(ch - 0x0660);
            if (ch >= 0x06F0 && ch <= 0x06F9)
                return(ch - 0x06F0);
            return(-1);

        case 0x07:
            if (ch >= 0x07C0 && ch <= 0x07C9)
                return(ch - 0x07C0);
            return(-1);

        case 0x08:
            return(-1);

        case 0x09:
            if (ch >= 0x0966 && ch <= 0x096F)
                return(ch - 0x0966);
            if (ch >= 0x09E6 && ch <= 0x09EF)
                return(ch - 0x09E6);
            return(-1);

        case 0x0A:
            if (ch >= 0x0A66 && ch <= 0x0A6F)
                return(ch - 0x0A66);
            if (ch >= 0x0AE6 && ch <= 0x0AEF)
                return(ch - 0x0AE6);
            return(-1);

        case 0x0B:
            if (ch >= 0x0B66 && ch <= 0x0B6F)
                return(ch - 0x0B66);
            if (ch >= 0x0BE6 && ch <= 0x0BEF)
                return(ch - 0x0BE6);
            return(-1);

        case 0x0C:
            if (ch >= 0x0C66 && ch <= 0x0C6F)
                return(ch - 0x0C66);
            if (ch >= 0x0CE6 && ch <= 0x0CEF)
                return(ch - 0x0CE6);
            return(-1);

        case 0x0D:
            if (ch >= 0x0D66 && ch <= 0x0D6F)
                return(ch - 0x0D66);
            return(-1);

        case 0x0E:
            if (ch >= 0x0E50 && ch <= 0x0E59)
                return(ch - 0x0E50);
            if (ch >= 0x0ED0 && ch <= 0x0ED9)
                return(ch - 0x0ED0);
            return(-1);

        case 0x0F:
            if (ch >= 0x0F20 && ch <= 0x0F29)
                return(ch - 0x0F20);
            return(-1);

        case 0x10:
            if (ch >= 0x1040 && ch <= 0x1049)
                return(ch - 0x1040);
            if (ch >= 0x1090 && ch <= 0x1099)
                return(ch - 0x1090);
            return(-1);

        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
            return(-1);

        case 0x17:
            if (ch >= 0x17E0 && ch <= 0x17E9)
                return(ch - 0x17E0);
            return(-1);

        case 0x18:
            if (ch >= 0x1810 && ch <= 0x1819)
                return(ch - 0x1810);
            return(-1);

        case 0x19:
            if (ch >= 0x1946 && ch <= 0x194F)
                return(ch - 0x1946);
            if (ch >= 0x19D0 && ch <= 0x19D9)
                return(ch - 0x19D0);
            return(-1);

        case 0x1A:
            if (ch >= 0x1A80 && ch <= 0x1A89)
                return(ch - 0x1A80);
            if (ch >= 0x1A90 && ch <= 0x1A99)
                return(ch - 0x1A90);
            return(-1);

        case 0x1B:
            if (ch >= 0x1B50 && ch <= 0x1B59)
                return(ch - 0x1B50);
            if (ch >= 0x1BB0 && ch <= 0x1BB9)
                return(ch - 0x1BB0);
            return(-1);

        case 0x1C:
            if (ch >= 0x1C40 && ch <= 0x1C49)
                return(ch - 0x1C40);
            if (ch >= 0x1C50 && ch <= 0x1C59)
                return(ch - 0x1C50);
            return(-1);
        }
    }

    if (ch < 0xA620)
        return(-1);

    if (ch < 0xAC00)
    {
        switch (ch >> 8)
        {
        case 0xA6:
            if (ch >= 0xA620 && ch <= 0xA629)
                return(ch - 0xA620);
            return(-1);

        case 0xA7:
            return(-1);

        case 0xA8:
            if (ch >= 0xA8D0 && ch <= 0xA8D9)
                return(ch - 0xA8D0);
            return(-1);

        case 0xA9:
            if (ch >= 0xA900 && ch <= 0xA909)
                return(ch - 0xA900);
            if (ch >= 0xA9D0 && ch <= 0xA9D9)
                return(ch - 0xA9D0);
            return(-1);

        case 0xAA:
            if (ch >= 0xAA50 && ch <= 0xAA59)
                return(ch - 0xAA50);
            return(-1);

        case 0xAB:
            if (ch >= 0xABF0 && ch <= 0xABF9)
                return(ch - 0xABF0);
            return(-1);
        }
    }

    if (ch >= 0xFF10 && ch <= 0xFF19)
        return(ch - 0xFF10);

    if (ch >= 0x104A0 && ch <= 0x104A9)
        return(ch - 0x104A0);

    if (ch >= 0x11066 && ch <= 0x1106F)
        return(ch - 0x11066);

    if (ch >= 0x110F0 && ch <= 0x110F9)
        return(ch - 0x110F0);

    if (ch >= 0x11136 && ch <= 0x1113F)
        return(ch - 0x11136);

    if (ch >= 0x111D0 && ch <= 0x111D9)
        return(ch - 0x111D0);

    if (ch >= 0x116C0 && ch <= 0x116C9)
        return(ch - 0x116C0);

    if (ch >= 0x1D7CE && ch <= 0x1D7FF)
        return(ch - 0x1D7CE);

    return(-1);
}

static inline unsigned int MapCh(const unsigned int * mp, FCh bs, FCh ch)
{
    FAssert(ch >= bs);

    ch -= bs;
    return(mp[ch / 32] & (1 << (ch % 32)));
}

unsigned int AlphabeticP(FCh ch)
{
    // From:
    // DerivedCoreProperties-6.2.0.txt
    // Date: 2012-05-20, 00:42:31 GMT [MD]

    switch (ch >> 12)
    {
    case 0x00:
    case 0x01:
        return(MapCh(Alphabetic0x0000To0x1fff, 0, ch));

    case 0x02:
        if (ch >= 0x2060 && ch <= 0x219F)
            return(MapCh(Alphabetic0x2060To0x219f, 0x2060, ch));
        if (ch >= 0x24A0 && ch <= 0x24FF)
            return(MapCh(Alphabetic0x24a0To0x24ff, 0x24A0, ch));
        if (ch >= 0x2C00 && ch <= 0x2E3F)
            return(MapCh(Alphabetic0x2c00To0x2e3f, 0x2C00, ch));
        break;

    case 0x03:
        if (ch >= 0x3000 && ch <= 0x31FF)
            return(MapCh(Alphabetic0x3000To0x31ff, 0x3000, ch));

        // No break.

    case 0x04:
        if (ch >= 0x3400 && ch <= 0x4DB5)
            return(1);

        // No break.

    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
        if (ch >= 0x4E00 && ch <= 0x9FCC)
            return(1);
        break;

    case 0x0A:
        if (ch >= 0xA000 && ch <= 0xA48C)
            return(1);
        if (ch >= 0xA480 && ch <= 0xABFF)
            return(MapCh(Alphabetic0xa480To0xabff, 0xA480, ch));

        // No break.

    case 0x0B:
    case 0x0C:
    case 0x0D:
        if (ch >= 0xAC00 && ch <= 0xD7A3)
            return(1);
        if (ch >= 0xD7A0 && ch <= 0xD7FF)
            return(MapCh(Alphabetic0xd7a0To0xd7ff, 0xD7A0, ch));
        break;

    case 0x0E:
        break;

    case 0x0F:
    case 0x10:
        if (ch >= 0xF900 && ch <= 0x1049F)
            return(MapCh(Alphabetic0xf900To0x1049f, 0xF900, ch));
        if (ch >= 0x10800 && ch <=0x10C5F)
            return(MapCh(Alphabetic0x10800To0x10c5f, 0x10800, ch));
        break;

    case 0x11:
        if (ch >= 0x11000 && ch <= 0x111DF)
            return(MapCh(Alphabetic0x11000To0x111df, 0x11000, ch));
        if (ch >= 11680 && ch <= 0x116AF)
            return(1);
        break;

    case 0x12:
        if (ch >= 0x12000 && ch <= 0x1236E)
            return(1);
        if (ch >= 0x12400 && ch <= 0x12462)
            return(1);
        break;

    case 0x13:
        if (ch >= 0x13000 && ch <= 0x1342E)
            return(1);

    case 0x14:
    case 0x15:
        break;

    case 0x16:
        if (ch >= 0x16800 && ch <= 0x16A38)
            return(1);
        if (ch >= 0x16F00 && ch <= 0x16F9F)
            return(MapCh(Alphabetic0x16f00To0x16f9f, 0x16F00, ch));
        break;

    case 0x17:
    case 0x18:
    case 0x19:
    case 0x1A:
        break;

    case 0x1B:
        if (ch >= 0x1B000 && ch <= 0x1B001)
            return(1);
        break;

    case 0x1C:
        break;

    case 0x1D:
        if (ch >= 0x1D400 && ch <= 0x1D7DF)
            return(MapCh(Alphabetic0x1d400To0x1d7df, 0x1D400, ch));
        if (ch >= 0x1EE00 && ch <= 0x1EEBF)
            return(MapCh(Alphabetic0x1ee00To0x1eebf, 0x1EE00, ch));
        break;

    case 0x1E:
    case 0x1F:
        break;

    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x24:
    case 0x25:
    case 0x26:
    case 0x27:
    case 0x28:
    case 0x29:
    case 0x2A:
        if (ch >= 0x20000 && ch <= 0x2A6D6)
            return(1);

        // No break.

    case 0x2B:
        if (ch >= 0x2A700 && ch <= 0x2B734)
            return(1);
        if (ch >= 0x2B740 && ch <= 0x2B81D)
            return(1);
        break;

    case 0x2C:
    case 0x2D:
    case 0x2E:
        break;

    case 0x2F:
        if (ch >= 0x2F800 && ch <= 0x2FA1D)
            return(1);
        break;
    }

    return(0);
}

unsigned int UppercaseP(FCh ch)
{
    // From:
    // DerivedCoreProperties-6.2.0.txt
    // Date: 2012-05-20, 00:42:31 GMT [MD]

    switch (ch >> 12)
    {
    case 0x00:
        if (ch <= 0x059F)
            return(MapCh(Uppercase0x0000To0x059f, 0, ch));
        break;

    case 0x01:
        if (ch >= 0x10A0 && ch <= 0x10DF)
            return(MapCh(Uppercase0x10a0To0x10df, 0x10A0, ch));
        if (ch >= 0x1E00 && ch <= 0x1FFF)
            return(MapCh(Uppercase0x1e00To0x1fff, 0x1E00, ch));
        break;

    case 0x02:
        if (ch >= 0x2100 && ch <= 0x219F)
            return(MapCh(Uppercase0x2100To0x219f, 0x2100, ch));
        if (ch >= 0x24B6 && ch <= 0x24CF)
            return(1);
        if (ch >= 0x2C00 && ch <= 0x2D3F)
            return(MapCh(Uppercase0x2c00To0x2d3f, 0x2C00, ch));
        break;

    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
        break;

    case 0x0A:
        if (ch >= 0xA640 && ch <= 0xA7BF)
            return(MapCh(Uppercase0xa640To0xa7bf, 0xA640, ch));
        break;

    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E:
        break;

    case 0x0F:
        if (ch >= 0xFF21 && ch <= 0xFF3A)
            return(1);
        break;

    case 0x10:
        if (ch >= 0x10400 && ch <= 0x10427)
            return(1);
        break;

    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x19:
    case 0x1A:
    case 0x1B:
    case 0x1C:
        break;

    case 0x1D:
        if (ch >= 0x1D400 && ch <= 0x1D7DF)
            return(MapCh(Uppercase0x1d400To0x1d7df, 0x1D400, ch));
        break;
    }

    return(0);
}

unsigned int LowercaseP(FCh ch)
{
    // From:
    // DerivedCoreProperties-6.2.0.txt
    // Date: 2012-05-20, 00:42:31 GMT [MD]

    switch (ch >> 12)
    {
    case 0x00:
        if (ch <= 0x059F)
            return(MapCh(Lowercase0x0000To0x059f, 0, ch));
        break;

    case 0x01:
        if (ch >= 0x1D00 && ch <= 0x1FFF)
            return(MapCh(Lowercase0x1d00To0x1fff, 0x1D00, ch));
        break;

    case 0x02:
        if (ch >= 0x2060 && ch <= 0x219F)
            return(MapCh(Lowercase0x2060To0x219f, 0x2060, ch));
        if (ch >= 0x24D0 && ch <= 0x24E9)
            return(1);
        if (ch >= 0x2C20 && ch <= 0x2D3F)
            return(MapCh(Lowercase0x2c20To0x2d3f, 0x2C20, ch));
        break;

    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
        break;

    case 0x0A:
        if (ch >= 0xA640 && ch <= 0xA7FF)
            return(MapCh(Lowercase0xa640To0xa7ff, 0xA640, ch));
        break;

    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E:
        break;

    case 0x0F:
        if (ch >= 0xFB00 && ch <= 0xFB06)
            return(1);
        if (ch >= 0xFB13 && ch <= 0xFB17)
            return(1);
        if (ch >= 0xFF41 && ch <= 0xFF5A)
            return(1);
        break;

    case 0x10:
        if (ch >= 0x10428 && ch <= 0x1044F)
            return(1);
        break;

    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x19:
    case 0x1A:
    case 0x1B:
    case 0x1C:
        break;

    case 0x1D:
        if (ch >= 0x1D400 && ch <= 0x1D7DF)
            return(MapCh(Lowercase0x1d400To0x1d7df, 0x1D400, ch));
    }

    return(0);
}
unsigned int CharFullfoldLength(FCh ch)
{
    // From:
    // CaseFolding-6.2.0.txt
    // Date: 2012-08-14, 17:54:49 GMT [MD]

    if (ch < 0x2000)
    {
        if (FullfoldSet[ch / 32] & (1 << (ch % 32)))
        {
            if (ch == 0x00df)
                return(Fullfold0x00dfTo0x00df[0].Count);
            else if (ch >= 0x0130 && ch <= 0x0149)
                return(Fullfold0x0130To0x0149[ch - 0x0130].Count);
            else if (ch == 0x01f0)
                return(Fullfold0x01f0To0x01f0[0].Count);
            else if (ch >= 0x0390 && ch <= 0x03b0)
                return(Fullfold0x0390To0x03b0[ch - 0x0390].Count);
            else if (ch == 0x0587)
                return(Fullfold0x0587To0x0587[0].Count);
            else if (ch >= 0x1e96 && ch <= 0x1e9e)
                return(Fullfold0x1e96To0x1e9e[ch - 0x1e96].Count);
            else if (ch >= 0x1f50 && ch <= 0x1f56)
                return(Fullfold0x1f50To0x1f56[ch - 0x1f50].Count);
            else if (ch >= 0x1f80 && ch <= 0x1ffc)
                return(Fullfold0x1f80To0x1ffc[ch - 0x1f80].Count);
        }

        return(1);
    }
    else if (ch >= 0xfb00 && ch <= 0xfb17)
        return(Fullfold0xfb00To0xfb17[ch - 0xfb00].Count);

    return(1);
}

FCh * CharFullfold(FCh ch)
{
    // From:
    // CaseFolding-6.2.0.txt
    // Date: 2012-08-14, 17:54:49 GMT [MD]

    FAssert(ch >= 0x2000 || FullfoldSet[ch / 32] & (1 << (ch % 32)));

    if (ch == 0x00df)
        return(Fullfold0x00dfTo0x00df[0].Chars);
    else if (ch >= 0x0130 && ch <= 0x0149)
        return(Fullfold0x0130To0x0149[ch - 0x0130].Chars);
    else if (ch == 0x01f0)
        return(Fullfold0x01f0To0x01f0[0].Chars);
    else if (ch >= 0x0390 && ch <= 0x03b0)
        return(Fullfold0x0390To0x03b0[ch - 0x0390].Chars);
    else if (ch == 0x0587)
        return(Fullfold0x0587To0x0587[0].Chars);
    else if (ch >= 0x1e96 && ch <= 0x1e9e)
        return(Fullfold0x1e96To0x1e9e[ch - 0x1e96].Chars);
    else if (ch >= 0x1f50 && ch <= 0x1f56)
        return(Fullfold0x1f50To0x1f56[ch - 0x1f50].Chars);
    else if (ch >= 0x1f80 && ch <= 0x1ffc)
        return(Fullfold0x1f80To0x1ffc[ch - 0x1f80].Chars);

    FAssert(ch >= 0xfb00 && ch <= 0xfb17);

    return(Fullfold0xfb00To0xfb17[ch - 0xfb00].Chars);
}

unsigned int CharFullupLength(FCh ch)
{
    // From:
    // SpecialCasing-6.2.0.txt
    // Date: 2012-05-23, 20:35:15 GMT [MD]

    if (ch < 0x2000)
    {
        if (FullupSet[ch / 32] & (1 << (ch % 32)))
        {
            if (ch == 0x00df)
                return(Fullup0x00dfTo0x00df[0].Count);
            else if (ch == 0x0149)
                return(Fullup0x0149To0x0149[0].Count);
            else if (ch == 0x01f0)
                return(Fullup0x01f0To0x01f0[0].Count);
            else if (ch >= 0x0390 && ch <= 0x03b0)
                return(Fullup0x0390To0x03b0[ch - 0x0390].Count);
            else if (ch == 0x0587)
                return(Fullup0x0587To0x0587[0].Count);
            else if (ch >= 0x1e96 && ch <= 0x1e9a)
                return(Fullup0x1e96To0x1e9a[ch - 0x1e96].Count);
            else if (ch >= 0x1f50 && ch <= 0x1f56)
                return(Fullup0x1f50To0x1f56[ch - 0x1f50].Count);
            else if (ch >= 0x1f80 && ch <= 0x1ffc)
                return(Fullup0x1f80To0x1ffc[ch - 0x1f80].Count);
        }

        return(1);
    }
    else if (ch >= 0xfb00 && ch <= 0xfb17)
        return(Fullup0xfb00To0xfb17[ch - 0xfb00].Count);

    return(1);
}

FCh * CharFullup(FCh ch)
{
    // From:
    // SpecialCasing-6.2.0.txt
    // Date: 2012-05-23, 20:35:15 GMT [MD]

    FAssert(ch >= 0x2000 || FullupSet[ch / 32] & (1 << (ch % 32)));

    if (ch == 0x00df)
        return(Fullup0x00dfTo0x00df[0].Chars);
    else if (ch == 0x0149)
        return(Fullup0x0149To0x0149[0].Chars);
    else if (ch == 0x01f0)
        return(Fullup0x01f0To0x01f0[0].Chars);
    else if (ch >= 0x0390 && ch <= 0x03b0)
        return(Fullup0x0390To0x03b0[ch - 0x0390].Chars);
    else if (ch == 0x0587)
        return(Fullup0x0587To0x0587[0].Chars);
    else if (ch >= 0x1e96 && ch <= 0x1e9a)
        return(Fullup0x1e96To0x1e9a[ch - 0x1e96].Chars);
    else if (ch >= 0x1f50 && ch <= 0x1f56)
        return(Fullup0x1f50To0x1f56[ch - 0x1f50].Chars);
    else if (ch >= 0x1f80 && ch <= 0x1ffc)
        return(Fullup0x1f80To0x1ffc[ch - 0x1f80].Chars);

    FAssert(ch >= 0xfb00 && ch <= 0xfb17);

    return(Fullup0xfb00To0xfb17[ch - 0xfb00].Chars);
}

unsigned int CharFulldownLength(FCh ch)
{
    // From:
    // SpecialCasing-6.2.0.txt
    // Date: 2012-05-23, 20:35:15 GMT [MD]

    if (ch == 0x0130)
        return(2);
    return(1);
}

static FCh Fulldown0x0130[] = {0x0069, 0x0307};

FCh * CharFulldown(FCh ch)
{
    // From:
    // SpecialCasing-6.2.0.txt
    // Date: 2012-05-23, 20:35:15 GMT [MD]

    FAssert(ch == 0x0130);

    return(Fulldown0x0130);
}
