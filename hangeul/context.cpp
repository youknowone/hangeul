//
//  context.cpp
//  hangeul
//
//  Created by Jeong YunWon on 2014. 6. 9..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#include <hangeul/context.h>
#include <cdebug/debug.h>
#include <cstdio>
#include <chrono>

namespace hangeul {

    bool Context::put(InputSource input) {
        auto clock = std::chrono::system_clock::now();
        auto milliclock = std::chrono::duration_cast<std::chrono::milliseconds>(clock.time_since_epoch());
        this->_state[-'t'] = (int32_t)milliclock.count();
        this->_state[-1] = input;
        this->_state[0] = input;
        auto res = this->_handler->put(this->_state);
        this->_state = res.state;
        //for (auto& it: this->_state) { dlog(1, "key: %x / val: %x", it.first, it.second); }
        return res.processed;
    }

    void Context::flush() {
        this->_state[-1] = -1;
        this->_state[0] = -1;
        this->_handler->put(this->_state);
    }

    void Context::truncate() {
        this->_state = State();
    }

    UnicodeVector Context::commited() {
        
        return this->decoder().commited(this->_state);
    }

    UnicodeVector Context::composed() {
        //this->_state._debug();
        return this->decoder().composed(this->_state);
    }


}