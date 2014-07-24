//
//  context.h
//  hangeul
//
//  Created by Jeong YunWon on 2014. 6. 9..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#ifndef __HANGEUL_CONTEXT__
#define __HANGEUL_CONTEXT__

#include <vector>
#include <hangeul/phase.h>
#include <hangeul/decoder.h>

namespace hangeul {
    template <typename T> void append_vector(T& vec1, T& vec2) {
        for (auto& item: vec2) {
            vec1.push_back(item);
        }
    }

    class Context {
    protected:
        StateList _states;
        Phase *_processor;
        Decoder *_decoder;
    public:
        Context(Phase *processor, Decoder *decoder) { this->_processor = processor; this->_decoder = decoder; }

        StateList& states() { return this->_states; }
        Decoder& decoder() { return *this->_decoder; }

        bool put(InputSource input);
        void flush();
        UnicodeVector commited();
        UnicodeVector composed();
    };
}

#endif
