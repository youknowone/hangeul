//
//  decoder.h
//  hangeul
//
//  Created by Jeong YunWon on 2014. 6. 9..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#ifndef __hangeul__decoder__
#define __hangeul__decoder__

#include <hangeul/type.h>

namespace hangeul {
    class Decoder {
    public:
        virtual UnicodeVector decode(State state) = 0;
    };

    class BypassDecoder: public Decoder {
    public:
        virtual UnicodeVector decode(State state);
    };
}

#endif
