//
//  decoder.cpp
//  hangeul
//
//  Created by Jeong YunWon on 2014. 6. 9..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#include <hangeul/decoder.h>

namespace hangeul {
    UnicodeVector BypassDecoder::decode(hangeul::State state) {
        UnicodeVector unicodes;
        unicodes.push_back(state[1]);
        return unicodes;
    }
}
