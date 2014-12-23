//
//  cheonjiin.cpp
//  hangeul
//
//  Created by Jeong YunWon on 2014. 7. 5..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#include "cheonjiin.h"
#include "_debug.h"

namespace hangeul {

namespace Cheonjiin {
    State Decoder::combined(State& state) {
        auto rstate = State();
        auto strokes = state.array(STROKES_IDX);
        for (int i = 0; i < strokes.size(); i++) {
            auto stroke = strokes[i];
            rstate[0] = stroke;
            auto result = this->combinator->put(rstate);
            rstate = result.state;
            if (!result.processed) {
                //assert(false);
                //break;
            }
            //for (auto it: charstate) { dlog(1, "k:%d / v:%d", it.first, it.second); }
        }
        return rstate;
    }

    UnicodeVector Decoder::decode(State::ArrayProxy& stroke) {
        UnicodeVector unicodes;
        if (stroke[1] && stroke[2]) {
            auto a = stroke[1];
            auto b = stroke[2];
            auto c = stroke[3];
            if (b <= Vowel::I) {
                auto v = 0xac00;
                v += (Initial::FromConsonant[a] - 1) * 21 * 28;
                v += (b - 1) * 28;
                if (c) {
                    v += Final::FromConsonant[c];
                }
                unicodes.push_back(v);
            } else {
                auto v = 0x3131 + a - 1;
                unicodes.push_back(v);
                if (b == Vowel::Ao) {
                    unicodes.push_back(183);
                }
                else if (b == Vowel::Yao) {
                    unicodes.push_back(183);
                    unicodes.push_back(183);
                }
                else {
                    dassert(false);
                }
            }
        }
        else if (stroke[1] && !stroke[2]) {
            auto v = 0x3131 + stroke[1] - 1;
            unicodes.push_back(v);
        }
        else if (!stroke[1] && stroke[2]) {
            auto v = 0x314f + stroke[2] - 1;
            unicodes.push_back(v);
        }
        else {
            unicodes.push_back(stroke[10]);
        }
        return unicodes;
    }

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
            auto character = cstate.array(0x1000 + (i + 1) * 0x10);
            auto univector = this->decode(character);
            for (auto& uni: univector) {
                unicodes.push_back(uni);
            }
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
        if (strokes.size() == 1 && annotation.type == Tenkey::AnnotationClass::Function) {
            strokes.pop_back();
            stroke = strokes.back();
            annotation = AnnotationMap[0][stroke & 0xff];
        }
        return PhaseResult::Make(state, true);
    }

    Tenkey::Annotation Layout::translate(State& state) {
        auto stroke = state.latestKeyStroke();
        auto phase = stroke >> 8;
        auto masked = stroke & 0xff;
        Tenkey::Annotation annotation;
        annotation = AnnotationMap[phase][masked];
        return annotation;
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
        static Layout layout;

        auto annotation = layout.translate(state);
        auto characters = state.array(0x1000);
        auto strokes = state.array(0x2000);

        characters.push_back(annotation.type);
        strokes.push_back(annotation.data);
        auto character = state.array(0x1000 + characters.size() * 0x10);

        character[1] = 0;
        character[2] = 0;
        character[10] = 0;
        character[11] = 0;
        switch (annotation.type) {
            case Tenkey::AnnotationClass::Consonant:
                character[1] = annotation.data;
                break;
            case Tenkey::AnnotationClass::Vowel:
                character[2] = annotation.data;
                break;
            case Tenkey::AnnotationClass::Punctuation:
                character[10] = annotation.data;
                break;
            case Tenkey::AnnotationClass::Function:
                character[11] = annotation.data;
                break;
            default:
                assert(false);
                break;
        }

        auto res = PhaseResult::Make(state, true);
        return res;
    }

    PhaseResult JasoCompositionPhase::put(State& state) {
        auto characters = state.array(0x1000);
        if (characters.size() < 2) {
            return PhaseResult::Make(state, true);
        }

        auto c1 = state.array(0x1000 + (characters.size() - 0) * 0x10);
        auto c2 = state.array(0x1000 + (characters.size() - 1) * 0x10);

        if (c1[11]) {
            return PhaseResult::Make(state, false);
        }

        bool combined = false;
        if (c1[1] && c2[3]) {
            auto j1 = c2[3];
            auto j2 = c1[1];
            auto composed = search_rule(FinalCompositionRules, j1, j2);
            if (!composed.is_none) {
                c2[3] = composed.some;
                combined = true;
            }
        }
        else if (!c2[3] && c1[2] && c2[2]) {
            auto j1 = c2[2];
            auto j2 = c1[2];
            auto composed = search_rule(VowelCompositionRules, j1, j2);
            if (!composed.is_none) {
                c2[2] = composed.some;
                combined = true;
            }
        }
        else if (c2[1] && !c2[2] && !c2[3] && c1[1]) {
            auto j1 = c2[1];
            auto j2 = c1[1];
            auto composed = search_rule(InitialCompositionRules, j1, j2);
            if (!composed.is_none) {
                c2[1] = composed.some;
                combined = true;
            }
        }
        if (combined) {
            characters.pop_back();
            return PhaseResult::Make(state, false);
        }
        return PhaseResult::Make(state, true);
    }

    PhaseResult AnnotationToCombinationPhase::put(State& state) {
        auto characters = state.array(0x1000);
        auto c1 = state.array(0x1000 + (characters.size() - 0) * 0x10);
        if (!c1[1] && !c1[2] && !c1[3]) {
            return PhaseResult::Make(state, true);
        }

        if (characters.size() < 2) {
            return PhaseResult::Make(state, false);
        }

        auto c2 = state.array(0x1000 + (characters.size() - 1) * 0x10);
        if (c1[1] && c1[2]) {

        }
        else if (c1[1]) {
            if (c2[1] && c2[2] && !c2[3]) {
                c2[3] = c1[1];
                characters.pop_back();
                return PhaseResult::Make(state, true);
            }
            else {
                return PhaseResult::Make(state, true);
            }
        }
        else if (c1[2]) {
            if (c2[1] && !c2[2]) {
                c2[2] = c1[2];
                characters.pop_back();
                return PhaseResult::Make(state, true);
            }
            else {
                return PhaseResult::Make(state, true);
            }
        }
        else {
            assert(false);
        }
        return PhaseResult::Make(state, true);
    }

    PhaseResult ConsonantDecompositionPhase::put(State& state) {
        auto strokes = state.array(0x1000);
        if (strokes.size() < 2) {
            return PhaseResult::Make(state, true);
        }

        auto c1 = state.array(0x1000 + (strokes.size() - 0) * 0x10);
        auto c2 = state.array(0x1000 + (strokes.size() - 1) * 0x10);

        auto c = c2[3];
        if (c1[2] && !c1[1] && c) {
            auto strokes = state.array(0x2000);
            auto s2 = strokes[-2];
            bool decomposed = false;
            for (auto& rule: FinalCompositionRules) {
                if (c == rule[2] && rule[1] == s2) {
                    c2[3] = rule[0];
                    c1[1] = rule[1];
                    decomposed = true;
                    break;
                }
            }
            if (!decomposed) {
                c1[1] = c;
                c2[3] = 0;
            }
            return PhaseResult::Make(state, true);
        }
        return PhaseResult::Make(state, true);
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
        hangul_phase->phases.push_back((Phase *)new JasoCompositionPhase());
        hangul_phase->phases.push_back((Phase *)new AnnotationToCombinationPhase());
        hangul_phase->phases.push_back((Phase *)new ConsonantDecompositionPhase());

        auto success_phase = new SuccessPhase(hangul_phase);
        this->phases.push_back(success_phase);
    }

}

}