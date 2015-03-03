//
//  tenkey.cpp
//  hangeul
//
//  Created by Jeong YunWon on 2014. 12. 30..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#include "tenkey.h"

namespace hangeul {
namespace Tenkey {

    UnicodeVector Decoder::commited(State& state) {
        auto unicodes = this->composed(state);
        return unicodes;
    }

    UnicodeVector Decoder::composed(State& state) {
        auto unicodes = UnicodeVector();
        //state._debug();
        State cstate = this->combined(state);
        auto string = cstate.array(STRING_IDX);
        for (auto& character: string) {
            unicodes.push_back(character);
        }
        return unicodes;
    }

    Unicode Layout::label(Tenkey::Annotation annotation) {
        switch (annotation.type) {
            case Tenkey::AnnotationClass::Consonant:
                return 0x3131 + annotation.data - 1;
            case Tenkey::AnnotationClass::Vowel:
                return 0x314f + annotation.data - 1;
            case Tenkey::AnnotationClass::Symbol:
                return annotation.data;
            case Tenkey::AnnotationClass::Function:
                return 'X';
            default:
                assert(false);
                break;
        }
        return 0;
    }

    PhaseResult AnnotationPhase::put(State& state) {
        auto characters = state.array(0x1000);
        characters.push_back(state.latestKeyStroke());

        auto res = PhaseResult::Make(state, true);
        return res;
    }
}


namespace TableTenkey {

    PhaseResult MergeStrokesPhase::put(State& state) {
        auto strokes = state.array(STROKES_IDX);
        if (strokes.size() < 2) {
            return PhaseResult::Make(state, true); // do not need to merge strokes
        }
        auto s1 = strokes[-1];
        auto s1_phase = s1 >> 8;
        auto s1_code = s1 & 0xff;
        auto s2 = strokes[-2];
        auto s2_code = s2 & 0xff;
        auto annotation = AlphabetMap[0][s1_code];
        if (annotation.type == Tenkey::AnnotationClass::Function && s1 == s2) {
            strokes.pop_back();
        }
        else if (s1_phase == 0 && s1_code == s2_code) {
            auto s2_phase = s2 >> 8;
            auto sn_phase = s2_phase + 1;
            if (sn_phase >= 4) {
                sn_phase = 0;
            }
            else if (sn_phase >= 2) {
                auto sn_annotation = AlphabetMap[sn_phase][s2_code];
                auto s0_annotation = AlphabetMap[0][s2_code];
                if (sn_annotation.type == s0_annotation.type && sn_annotation.data == s0_annotation.data) {
                    sn_phase = 0;
                }
            }
            if (s2_phase != 0 || sn_phase != 0) {
                auto sn_annotation = AlphabetMap[sn_phase][s2_code];
                if (annotation.type != sn_annotation.type || annotation.data != sn_annotation.data) {
                    auto sn = (sn_phase << 8) + s2_code;
                    strokes.pop_back();
                    strokes.pop_back();
                    strokes.push_back(sn);
                }
            }
        }
        return PhaseResult::Make(state, true);
    }

    PhaseResult UnstrokeBackspacePhase::put(State& state) {
        auto strokes = state.array(STROKES_IDX);
        if (strokes.size() == 0) {
            return PhaseResult::Make(state, true); // do not need to unstroke anything
        }
        auto stroke = strokes.back();
        if (stroke == 0x0e) {
            strokes.pop_back();
            stroke = strokes.back();
            auto annotation = AlphabetMap[0][stroke & 0xff];
            while (strokes.size() > 0 && annotation.type == Tenkey::AnnotationClass::Function) {
                strokes.pop_back();
                stroke = strokes.back();
                annotation = AlphabetMap[0][stroke & 0xff];
            }
            if (strokes.size() > 0) {
                strokes.pop_back();
            } else {
                return PhaseResult::Make(state, false);
            }
        }
        auto annotation = AlphabetMap[0][stroke & 0xff];
        if (strokes.size() == 1 && annotation.type == Tenkey::AnnotationClass::Function) {
            strokes.pop_back();
            stroke = strokes.back();
            annotation = AlphabetMap[0][stroke & 0xff];
        }
        return PhaseResult::Make(state, true);
    }

    FromTenkeyHandler::FromTenkeyHandler() : CombinedPhase() {
        this->phases.push_back((Phase *)new KeyStrokeStackPhase());
        this->phases.push_back((Phase *)new UnstrokeBackspacePhase());
        this->phases.push_back((Phase *)new MergeStrokesPhase());
    }

    Table AlphabetMap = {
#define S(C) { Tenkey::AnnotationClass::Symbol, C }
#define F(C) { Tenkey::AnnotationClass::Function, KeyPosition ## C }
#define E() { Tenkey::AnnotationClass::Function, 0 }
        {
            S('@'), S('a'), S('d'),
            S('g'), S('j'), S('m'),
            S('p'), S('t'), S('w'),
            F(Right), S('.'), S('.'),
        },
        {
            S('#'), S('b'), S('e'),
            S('h'), S('k'), S('n'),
            S('q'), S('u'), S('x'),
            F(Right), S(','), S('.'),
        },
        {
            S('/'), S('c'), S('f'),
            S('i'), S('l'), S('o'),
            S('r'), S('v'), S('y'),
            F(Right), S('?'), S('.'),
        },
        {
            S('&'), S('a'), S('d'),
            S('g'), S('j'), S('m'),
            S('s'), S('t'), S('z'),
            F(Right), S('!'), S('.'),
        },
        {
            S('_'), S('a'), S('d'),
            S('g'), S('j'), S('m'),
            S('p'), S('t'), S('w'),
            F(Right), S('.'), S('.'),
        },
#undef A
#undef F
#undef E
    };

    Table NumberMap = {
#define S(C) { Tenkey::AnnotationClass::Symbol, C }
#define F(C) { Tenkey::AnnotationClass::Function, KeyPosition ## C }
#define E() { Tenkey::AnnotationClass::Function, 0 }
        {
            S('1'), S('2'), S('3'),
            S('4'), S('5'), S('6'),
            S('7'), S('8'), S('9'),
            F(Right), S('0'), S('.'),
        },
        {
            S('1'), S('2'), S('3'),
            S('4'), S('5'), S('6'),
            S('7'), S('8'), S('9'),
            F(Right), S('0'), S('.'),
        },
#undef A
#undef F
#undef E
    };
}
}