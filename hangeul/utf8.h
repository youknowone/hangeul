//
//  utf8.h
//  hangeul
//
//  Created by Jeong YunWon on 2014. 7. 6..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#ifndef __hangeul__utf8__
#define __hangeul__utf8__

#include <hangeul/type.h>
#include <string>

namespace hangeul {
    struct UTF8 {
        union {
            uint32_t data;
            uint8_t bytes[4];
            char chars[4];
        };

        UTF8(const char *str);
        UTF8(uint32_t data);

        size_t size();
        Unicode unicode(size_t size);
        Unicode unicode();

        static UTF8 from_unicode(Unicode unicode);
        static UnicodeVector to_unicodes(const char *str);
        static std::string from_unicodes(UnicodeVector unicodes);
    };
}

#endif /* defined(__hangeul__utf8__) */
