//
//  context.cpp
//  hangeul
//
//  Created by Jeong YunWon on 2014. 6. 9..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#include <hangeul/context.h>

namespace hangeul {

    bool Context::put(InputSource input) {
        State state;
        state[1] = input;
        this->_states.push_front(state);
        auto res = this->_processor->put(this->_states);
        this->_states = res.states;
        return res.processed;
    }

    void Context::flush() {
        if (this->_states.size()) {
            this->_states.front()[-1] = 1;
        }
    }

    UnicodeVector Context::commited() {
        UnicodeVector result;
        while (this->_states.size() > 1) {
            // FIXME: not a STL way
            auto vec = this->_decoder->decode(this->_states.back());
            append_vector(result, vec);
            this->_states.pop_back();
        }
        if (this->_states.size() && this->_states.back()[-1]) {
            auto vec = this->_decoder->decode(this->_states.back());
            append_vector(result, vec);
            this->_states.pop_back();
        }
        return result;
    }

    UnicodeVector Context::composed() {
        StateList copied = this->_states;
        copied.reverse();
        UnicodeVector result;
        for (auto& state: copied) {
            auto vec = this->_decoder->decode(state);
            append_vector(result, vec);
            assert(result.back() == vec.back());
        }
        return result;
    }


}