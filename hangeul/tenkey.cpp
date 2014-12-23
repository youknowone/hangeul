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
        auto string = state.array(STRING_IDX);
        if (state[0] == -1) {
            string.set_size(0);
            return unicodes;
        } else {
            if (unicodes.size() > 0) {
                unicodes.pop_back();
            }
            if (string.size() > 0) {
                string.erase(0, string.size() - 1);
            }
        }
        return unicodes;
    }

    UnicodeVector Decoder::composed(State& state) {
        auto unicodes = UnicodeVector();
        State cstate = this->combined(state);
        auto characters = cstate.array(0x1000);
        for (auto i = 0; i < characters.size(); i++) {
            unicodes.push_back(characters[i]);
        }
        return unicodes;
    }

    static Tenkey::Annotation AnnotationMap[][13] = {
#define P(C) { Tenkey::AnnotationClass::Punctuation, C }
#define F(C) { Tenkey::AnnotationClass::Function, KeyPosition ## C }
#define C(C) { Tenkey::AnnotationClass::Consonant, Consonant:: C }
#define V(C) { Tenkey::AnnotationClass::Vowel, Vowel:: C }
#define E() { Tenkey::AnnotationClass::Function, 0 }
        {
            V(I), V(Ao), V(Eu),
            C(G), C(N), C(D),
            C(B), C(S), C(J),
            F(Right), C(NG), P('.'),
            P(' '),
        },
        {
            V(I), V(Yao), V(Eu),
            C(K), C(R), C(T),
            C(P), C(H), C(CH),
            F(Right), C(M), P(','),
            P(' '),
        },
        {
            V(I), V(Ao), V(Eu),
            C(GG), C(N), C(DD),
            C(BB), C(SS), C(JJ),
            F(Right), C(NG), P('?'),
            P(' '),
        },
        {
            V(I), V(Ao), V(Eu),
            C(G), C(N), C(D),
            C(B), C(S), C(J),
            F(Right), C(NG), P('!'),
            P(' '),
        },
#undef A
#undef F
#undef C
#undef V
#undef E
    };

    PhaseResult UnstrokeBackspacePhase::put(State& state) {
        auto strokes = state.array(STROKES_IDX);
        if (strokes.size() == 0) {
            return PhaseResult::Make(state, true); // do not need to unstroke anything
        }
        auto stroke = strokes.back();
        if (stroke == 0x0e) {
            strokes.pop_back();
            stroke = strokes.back();
            auto annotation = AnnotationMap[0][stroke & 0xff];
            while (strokes.size() > 0 && annotation.type == Tenkey::AnnotationClass::Function) {
                strokes.pop_back();
                stroke = strokes.back();
                annotation = AnnotationMap[0][stroke & 0xff];
            }
            if (strokes.size() > 0) {
                strokes.pop_back();
            } else {
                return PhaseResult::Make(state, false);
            }
        }
        auto annotation = AnnotationMap[0][stroke & 0xff];
        while (strokes.size() > 0 && annotation.type == Tenkey::AnnotationClass::Function) {
            strokes.pop_back();
            stroke = strokes.back();
            annotation = AnnotationMap[0][stroke & 0xff];
        }
        return PhaseResult::Make(state, true);
    }

    Unicode Layout::label(Tenkey::Annotation annotation) {
        switch (annotation.type) {
            case Tenkey::AnnotationClass::Consonant:
                return 0x3131 + annotation.data - 1;
            case Tenkey::AnnotationClass::Vowel:
                return 0x314f + annotation.data - 1;
            case Tenkey::AnnotationClass::Punctuation:
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

    PhaseResult MergeStrokesPhase::put(State& state) {
        auto strokes = state.array(STROKES_IDX);
        if (strokes.size() < 2) {
            return PhaseResult::Make(state, true); // do not need to merge strokes
        }
        auto s1 = strokes[-1];
        if (s1 == 0 || s1 == 2) {
            return PhaseResult::Make(state, true); // do not need to merge strokes
        }
        auto s1_phase = s1 >> 8;
        auto s1_code = s1 & 0xff;
        auto s2 = strokes[-2];
        auto s2_code = s2 & 0xff;
        auto annotation = AnnotationMap[0][s1_code];
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
                auto sn_annotation = AnnotationMap[sn_phase][s2_code];
                auto s0_annotation = AnnotationMap[0][s2_code];
                if (sn_annotation.type == s0_annotation.type && sn_annotation.data == s0_annotation.data) {
                    sn_phase = 0;
                }
            }
            if (s2_phase != 0 || sn_phase != 0) {
                auto sn = (sn_phase << 8) + s2_code;
                strokes.pop_back();
                strokes.pop_back();
                strokes.push_back(sn);
            }
        }
        return PhaseResult::Make(state, true);
    }

    FromTenkeyHandler::FromTenkeyHandler(hangeul::Combinator *combinator) : CombinedPhase() {
        this->phases.push_back((Phase *)new KeyStrokeStackPhase());
        this->phases.push_back((Phase *)new UnstrokeBackspacePhase());
        this->phases.push_back((Phase *)new MergeStrokesPhase());
        this->phases.push_back((Phase *)new CombinatorPhase((Phase *)combinator, false));
    }

    Combinator::Combinator() : hangeul::Combinator() {
        this->phases.push_back((Phase *)new AnnotationPhase());

        auto hangul_phase = new CombinedPhase();

        auto success_phase = new SuccessPhase(hangul_phase);
        this->phases.push_back(success_phase);
    }

}
}