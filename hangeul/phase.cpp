//
//  phase.cpp
//  hangeul
//
//  Created by Jeong YunWon on 2014. 6. 7..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#include <hangeul/phase.h>
#include <chrono>
#include "_debug.h"

namespace hangeul {
    State empty_state;

    State simple_state(int32_t key, int32_t value) {
        State state;
        state[key] = value;
        return state;
    }

    Optional<CompositionRule> search_rule(const CompositionRule *rules, const unsigned result) {
        auto cursor = (CompositionRule *)rules;
        while ((*cursor)[0] != 0) {
            auto& rule = *cursor;
            if (result == rule[2]) {
                return Optional<CompositionRule>::SomeBytes(rule);
            }
            cursor++;
        }
        return Optional<CompositionRule>::None();
    }

    Optional<unsigned> search_rule(const CompositionRule *rules, unsigned input1, unsigned input2) {
        auto cursor = (CompositionRule *)rules;
        while ((*cursor)[0] != 0) {
            auto& rule = *cursor;
            if (input1 == rule[0] && input2 == rule[1]) {
                return Optional<unsigned>::Some(rule[2]);
            }
            cursor++;
        }
        return Optional<unsigned>::None();
    }

    PhaseResult CombinedPhase::put(State& state) {
        PhaseResult result = PhaseResult::Stop();
        for (auto& phase: this->phases) {
            result = phase->put(state);
            if (!result.processed) {
                break;
            }
        }
        return result;
    }

    PhaseResult PostProcessPhase::put(State& state) {
        for (auto& phase: this->phases) {
            auto result = phase->put(state);
            state = result.state;
        }
        return PhaseResult::Make(state, true);
    }

    PhaseResult BranchPhase::put(State& state) {
        auto branch = state[-1];
        return this->phases[branch]->put(state);
    }

    PhaseResult KeyStrokeStackPhase::put(State& state) {
        auto strokes = state.array(STROKES_IDX);
        auto stroke = state.latestKeyStroke();
        strokes.push_back(stroke);

        auto timestack = state.array(TIME_IDX);
        auto clock = std::chrono::system_clock::now();
        auto milliclock = std::chrono::duration_cast<std::chrono::milliseconds>(clock.time_since_epoch());
        timestack.push_back((int32_t)milliclock.count());
        assert(strokes.size() == timestack.size());

        dassert(strokes.back() == stroke);
        return PhaseResult::Make(state, true);
    }

    PhaseResult UnstrokeBackspacePhase::put(State& state) {
        auto strokes = state.array(STROKES_IDX);
        if (strokes.size() == 0) {
            return PhaseResult::Make(state, true); // do not need to unstroke anything
        }
        auto timestack = state.array(TIME_IDX);
        auto stroke = strokes.back();
        if (stroke == 0x0e) {
            strokes.pop_back();
            timestack.pop_back();
            if (strokes.size() > 0) {
                strokes.pop_back();
                timestack.pop_back();
            } else {
                return PhaseResult::Make(state, false);
            }
        }
        return PhaseResult::Make(state, true);
    }

    #define N(V) KeyPosition(V)
    #define K(V) KeyPosition ## V

    PhaseResult QwertyToKeyStrokePhase::put(State& state) {
        #define DDD 0
        #define S(V) KeyPosition(KeyPosition ## V + 0x20000)
        static KeyPosition map[] = {
            // 0x00
            N(0xff), N(0xff), N(0xff), N(0xff), N(0xff), N(0xff), N(0xff), N(0xff),
            N(0xff), K(Tab), N(0xff), N(0xff), N(0xff), K(Enter), N(0xff), N(0xff),
            // 0x10
            N(0xff), N(0xff), N(0xff), N(0xff), N(0xff), N(0xff), N(0xff), N(0xff),
            N(0xff), N(0xff), N(0xff), KeyPositionEsc, N(0xff), N(0xff), N(0xff), N(0xff),
            // 0x20
            K(Space), S(1), S(Quote), S(3), S(4), S(5), S(7), K(Quote),
            S(9), S(0), S(8), S(Equal), K(Comma), K(Minus), K(Period), K(Slash),
            // 0x30
            K(0), K(1), K(2), K(3), K(4), K(5), K(6), K(7),
            K(8), K(9), S(Colon), K(Colon), S(Comma), K(Equal), S(Period), S(Slash),
            // 0x40
            S(2), S(A), S(B), S(C), S(D), S(E), S(F), S(G),
            S(H), S(I), S(J), S(K), S(L), S(M), S(N), S(O),
            // 0x50
            S(P), S(Q), S(R), S(S), S(T), S(U), S(V), S(W),
            S(X), S(Y), S(Z), K(BracketLeft), S(Backslash), K(BracketRight), S(6), S(Minus),
            // 0x60
            K(Quote), K(A), K(B), K(C), K(D), K(E), K(F), K(G),
            K(H), K(I), K(J), K(K), K(L), K(M), K(N), K(O),
            // 0x70
            K(P), K(Q), K(R), K(S), K(T), K(U), K(V), K(W),
            K(X), K(Y), K(Z), S(BracketLeft), S(Backslash), S(BracketRight), S(Grave), K(Backspace),
        };
        #undef S
        auto inputsource = state[-1];
        auto stroke = KeyStroke(map[inputsource]);
        dlog(DDD, "inputsource: %02x / stroke: %02x", inputsource, stroke);
        state[0] = stroke;
        auto result = PhaseResult::Make(state, true);
        dassert(result.state[0] == stroke);
        return result;
        #undef DDD
    }
    
    PhaseResult MacKeycodeToKeyStrokePhase::put(State& state) {
        static KeyPosition map[] = {
            // 35 7a 78 63 76 60 61 62 63 64 65 6d 67 6f 69 6b 71 6a 40 4f 50 5a
            // 32 12 13 14 15 17 16 1a 1c 19 1d 1b 18 2a 33(75)
            //  30 0c 0d 0e 0f 11 10 20 22 1f 23 21 1e(4c)
            //      00 01 02 03 05 04 26 28 25 29 27 24
            // 38    06 07 08 09 0b 2d 2e 2b 2f 2c   3c    7e    (   74   )
            // 3f 3B 3A 37       31       ?? 3d 3e      7b 7d 7c (73 79 77)
            K(A), K(S), K(D), K(F), K(H), K(G), K(Z), K(X), // 0x00 ~ 0x07
            K(C), K(V), N(0xff), K(B), K(Q), K(W), K(E), K(R), // 0x08 ~ 0x0f
            K(Y), K(T), K(1), K(2), K(3), K(4), K(6), K(5), // 0x10 ~ 0x17
            K(Equal), K(9), K(7), K(Minus), K(8), K(0), K(BracketRight), K(O), // 0x18 ~ 0x1f
            K(U), K(BracketLeft), K(I), K(P), K(L), K(L), K(J), K(Quote), // 0x20 ~ 0x27
            K(K), K(Colon), K(Backslash), K(Comma), K(Slash), K(N), K(M), K(Period), // 0x28 ~ 0x2f
            K(Tab), K(Space), K(Grave), K(Backspace), N(0xff), K(Esc), N(0xff), K(LeftOS), // 0x30 ~ 0x37
            K(LeftShift), N(0xff), K(LeftAlt), K(LeftControl), K(RightShift), K(RightAlt), K(RightControl), K(Function), // 0x38 ~ 0x3f
            K(F17), K(PadDecimal), N(0xff), K(PadMultiply), N(0xff), K(PadPlus), N(0xff), K(PadClear), // 0x40 ~ 0x47
            N(0xfe), N(0xff), N(0xff), K(PadDivide), K(PadEnter), N(0xff), K(PadMinus), K(F18), // 0x48 ~ 0x4f
            K(F19), K(PadEqual), K(Pad0), K(Pad1), K(Pad2), K(Pad3), K(Pad4), K(Pad5), // 0x50 ~ 0x57
            K(Pad6), K(Pad7), K(F20), K(Pad8), K(Pad9), K(Yen), K(Underscore), K(PadComma), // 0x58 ~ 0x5f
            K(F5), K(F6), K(F7), K(F3), K(F8), K(F9), K(Eisu), K(F11), // 0x60 ~ 0x67
            K(Kana), K(F13), K(F16), K(F14), N(0xff), K(F10), N(0xff), K(F12), // 0x68 ~ 0x6f
            N(0xff), K(F15), N(0xff), K(Home), K(PageUp), K(Delete), K(F4), K(End), // 0x70 ~ 0x77
            K(F2), K(PageDown), K(F1), K(Left), K(Right), K(Down), K(Up), N(0xff), // 0x78 ~ 0x7f
        };
        auto input = state[-1];
        KeyStroke stroke = 0xffff0000 & input + 0xff & map[input];
        state[0] = stroke;
        return PhaseResult::Make(state, true);
    }


    PhaseResult CombinatorPhase::put(State& state) {
        auto strokes = state.array(STROKES_IDX);
        auto timestack = state.array(TIME_IDX);
        auto string = state.array(STRING_IDX);
        string.erase(0, string.size());
        assert(string.size() == 0);

        State substate;
        auto substrokes = substate.array(STROKES_IDX);
        auto subtimestack = substate.array(TIME_IDX);
        int boundary = 0;
        auto result = PhaseResult::Stop();
        for (int i = 0; i < strokes.size(); i++) {
            auto stroke = strokes[i];
            substrokes.push_back(stroke);
            subtimestack.push_back(timestack[i]);
            substate[0] = stroke;
            result = phase->put(substate);
            substate = result.state;
            if (!result.processed) {
                string.push_back(boundary);
                auto charstrokes = state.array(STRING_IDX + string.size() * 0x100);
                charstrokes.import(substrokes);
                boundary += substrokes.size();
                substate = State();
                i = boundary - 1;
            }
        }
        if (string.back() != boundary) {
            string.push_back(boundary);
            auto charstrokes = state.array(STRING_IDX + string.size() * 0x100);
            charstrokes.import(substrokes);
        }
        return result;
    }
}
