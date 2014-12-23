//
//  decoder.cpp
//  hangeul
//
//  Created by Jeong YunWon on 2014. 6. 9..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#include <hangeul/decoder.h>

namespace hangeul {
    UnicodeVector BypassDecoder::commited(State& state) {
        UnicodeVector unicodes;
        unicodes.push_back(state[0]);
        return unicodes;
    }

    UnicodeVector BypassDecoder::composed(State& state) {
        UnicodeVector unicodes;
        unicodes.push_back(state[0]);
        return unicodes;
    }
}
