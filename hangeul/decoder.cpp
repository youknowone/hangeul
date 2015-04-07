//
//  decoder.cpp
//  hangeul
//
//  Created by Jeong YunWon on 2014. 6. 9..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#include <hangeul/decoder.h>
#include <hangeul/data.h>

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

    UnicodeVector nfc_to_nfd(UnicodeVector& string) {
        UnicodeVector unicodes;
        for (auto& chr: string) {
            if (0xac00 <= chr && chr <= 0xd7a3) {
                auto code = chr - 0xac00;
                auto a = code / (21 * 28);
                auto b = (code / 28) % 21;
                auto c = code % 28;
                unicodes.push_back(0x1100 + a);
                unicodes.push_back(0x1161 + b);
                if (c > 0) {
                    unicodes.push_back(0x11a7 + c);
                }
            }
            else if (0x3131 <= chr && chr <= 0x314e) {
                auto consonant = (Consonant::Type)(chr - 0x3131 + 1);
                auto initial = Initial::FromConsonant[consonant];
                unicodes.push_back(0x1100 + initial - 1);
            }
            else if (0x314f <= chr && chr <= 0x3163) {
                auto vowel = chr - 0x314f;
                unicodes.push_back(0x1161 + vowel);
            }
            else {
                unicodes.push_back(chr);
            }
        }
        return unicodes;
    }
}
