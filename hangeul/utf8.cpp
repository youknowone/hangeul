//
//  utf8.cpp
//  hangeul
//
//  Created by Jeong YunWon on 2014. 7. 6..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#include <hangeul/utf8.h>

namespace hangeul {

    UTF8::UTF8(const char *str) {
        uint32_t value = *((uint32_t *)str);
        this->data = value;
    }

    UTF8::UTF8(uint32_t data) {
        this->data = data;
    }

    size_t UTF8::size() {
        if ((data & 0x80) == 0) {
            return 1;
        }
        if ((data & 0xc0e0) == 0x80c0) {
            return 2;
        }
        if ((data & 0xc0c0f0) == 0x8080e0) {
            return 3;
        }
        if ((data & 0xc0c0c0f8) == 0x808080f0) {
            return 4;
        }
        return 0;
    }

    Unicode UTF8::unicode(size_t size) {
        switch (size) {
            case 1:
                return data & 0x7f;
            case 2:
                return ((data & 0x1f) << 6) + ((data & 0x3f00) >> 8);
            case 3:
                return ((data & 0x0f) << 12) + ((data & 0x3f00) >> 2) + ((data & 0x3f0000) >> 16);
            case 4: {
                return ((data & 0x07) << 18) + ((data & 0x3f00) << 4) + ((data & 0x3f0000) >> 10) + ((data & 0x3f0000) >> 24);
            }
            default:
                return -1;
        }
    }

    Unicode UTF8::unicode() {
        return unicode(this->size());
    }

    UTF8 UTF8::from_unicode(Unicode unicode) {
        if (unicode < 0x80) {
            return UTF8(unicode);
        }
        if (unicode < 0x800) {
            return UTF8(0x80c0 + ((unicode & 0x07c0) >> 6) + ((unicode & 0x3f) << 8));
        }
        if (unicode < 0x10000) {
            return UTF8(0x8080e0 + ((unicode & 0xf000) >> 12) + ((unicode & 0x0fc0) << 2) + ((unicode & 0x003f) << 16));
        }
        if (unicode < 0x110000) {
            return UTF8(0x808080f0 + ((unicode & 0x1c0000) >> 18) + ((unicode & 0x03f000) >> 4) + ((unicode & 0x0fc0) << 10) + ((unicode & 0x003f) << 24));
        }
        return UTF8(-1);
    }

    UnicodeVector UTF8::to_unicodes(const char *utf8_str) {
        UnicodeVector unicodes;
        const char *cursor = utf8_str;
        while (true) {
            UTF8 utf8 = UTF8(cursor);
            size_t size = utf8.size();
            if (size == 0) {
                break;
            }
            Unicode unicode = utf8.unicode(size);
            if (unicode == 0) {
                break;
            }
            unicodes.push_back(unicode);
            cursor += size;
        }
        return unicodes;
    }

    std::string UTF8::from_unicodes(UnicodeVector unicodes) {
        std::string result;
        char buffer[5];
        uint32_t *ref = (uint32_t *)buffer;
        for (auto& unicode: unicodes) {
            auto utf8 = UTF8::from_unicode(unicode);
            auto size = utf8.size();
            if (size == 0) {
                break;
            }
            *ref = utf8.data;
            buffer[size] = 0;
            result.append(buffer);
        }
        return result;
    }

}
