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

    Annotation Annotation::None = { Function, 0 };

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
            case Tenkey::Annotation::Consonant:
                return 0x3131 + annotation.data - 1;
            case Tenkey::Annotation::Vowel:
                return 0x314f + annotation.data - 1;
            case Tenkey::Annotation::Symbol:
                return annotation.data;
            case Tenkey::Annotation::Function:
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
        auto s1 = Tenkey::Stroke(strokes[-1]);
        auto s2 = Tenkey::Stroke(strokes[-2]);
        auto annotation = (*_table)[0][s1.code];
        if (annotation.type == Tenkey::Annotation::Function && s1 == s2) {
            strokes.pop_back();
        }
        else if (s1.phase == 0 && s1.code == s2.code) {
            auto sn = s2;
            sn.phase += 1;
            auto sn_annotation = (*_table)[sn.phase][sn.code];

            if (sn_annotation == Tenkey::Annotation::None) {
                sn.phase = 0;
                sn_annotation = (*_table)[sn.phase][sn.code];
            }
            if (s2 != sn) {
                strokes.pop_back();
                strokes.pop_back();
                strokes.push_back(sn.value);
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
            auto annotation = (*_table)[0][stroke & 0xff];
            while (strokes.size() > 0 && annotation.type == Tenkey::Annotation::Function) {
                strokes.pop_back();
                stroke = strokes.back();
                annotation = (*_table)[0][stroke & 0xff];
            }
            if (strokes.size() > 0) {
                strokes.pop_back();
            } else {
                return PhaseResult::Make(state, false);
            }
        }
        auto annotation = (*_table)[0][stroke & 0xff];
        if (strokes.size() == 1 && annotation.type == Tenkey::Annotation::Function) {
            strokes.pop_back();
            stroke = strokes.back();
            annotation = (*_table)[0][stroke & 0xff];
        }
        return PhaseResult::Make(state, true);
    }

    FromTenkeyHandler::FromTenkeyHandler(Table *table) : CombinedPhase() {
        this->phases.push_back((Phase *)new KeyStrokeStackPhase());
        this->phases.push_back((Phase *)new UnstrokeBackspacePhase(table));
        this->phases.push_back((Phase *)new MergeStrokesPhase(table));
    }

    Table AlphabetMap = {
#define _() Tenkey::Annotation::None
#define S(C) { Tenkey::Annotation::Symbol, C }
#define F(C) { Tenkey::Annotation::Function, KeyPosition ## C }
#define E() { Tenkey::Annotation::Function, 0 }
        {
            S('@'), S('a'), S('d'),
            S('g'), S('j'), S('m'),
            S('p'), S('t'), S('w'),
            F(Right), S('.'), F(Function),
            S(' '),
        },
        {
            S('#'), S('b'), S('e'),
            S('h'), S('k'), S('n'),
            S('q'), S('u'), S('x'),
            _(), S(','), _(),
        },
        {
            S('/'), S('c'), S('f'),
            S('i'), S('l'), S('o'),
            S('r'), S('v'), S('y'),
            _(), S('?'), _(),
        },
        {
            S('&'), _(), _(),
            _(), _(), _(),
            S('s'), _(), S('z'),
            _(), S('!'), _(),
        },
        {
        },
#undef A
#undef F
#undef E
    };

    Table NumberMap = {
#define S(C) { Tenkey::Annotation::Symbol, C }
#define F(C) { Tenkey::Annotation::Function, KeyPosition ## C }
#define E() { Tenkey::Annotation::Function, 0 }
        {
            S('1'), S('2'), S('3'),
            S('4'), S('5'), S('6'),
            S('7'), S('8'), S('9'),
            F(Right), S('0'), S('.'),
        },
        {

        }
#undef A
#undef F
#undef E
    };
}
}