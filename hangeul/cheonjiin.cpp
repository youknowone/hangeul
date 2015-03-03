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
        for (auto& stroke: strokes) {
            rstate[0] = stroke;
            auto result = this->combinator->put(rstate);
            rstate = result.state;
            if (!result.processed) {
                //assert(false);
                //break;
            }
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
                if (c != Consonant::None) {
                    auto v = 0x3131 + c - 1;
                    unicodes.push_back(v);
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

    Tenkey::Annotation Layout::translate(State& state) {
        Tenkey::Stroke stroke = state.latestKeyStroke();
        Tenkey::Annotation annotation;
        annotation = CheonjiinMap[stroke.phase][stroke.code];
        return annotation;
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
            case Tenkey::Annotation::Consonant:
                character[1] = annotation.data;
                break;
            case Tenkey::Annotation::Vowel:
                character[2] = annotation.data;
                break;
            case Tenkey::Annotation::Symbol:
                character[10] = annotation.data;
                break;
            case Tenkey::Annotation::Function:
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

    FromTenkeyHandler::FromTenkeyHandler(hangeul::Combinator *combinator) : CombinedPhase() {
        this->phases.push_back((Phase *)new KeyStrokeStackPhase());
        TableTenkey::Table *table = (TableTenkey::Table *)&CheonjiinMap;
        this->phases.push_back((Phase *)new TableTenkey::UnstrokeBackspacePhase(table));
        this->phases.push_back((Phase *)new TableTenkey::MergeStrokesPhase(table));
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

    TableTenkey::Table CheonjiinMap = {
        #define S(C) { Tenkey::Annotation::Symbol, C }
        #define _() Tenkey::Annotation::None
        #define F(C) { Tenkey::Annotation::Function, KeyPosition ## C }
        #define C(C) { Tenkey::Annotation::Consonant, Consonant:: C }
        #define V(C) { Tenkey::Annotation::Vowel, Vowel:: C }
        #define E() { Tenkey::Annotation::Function, 0 }
        {
            V(I), V(Ao), V(Eu),
            C(G), C(N), C(D),
            C(B), C(S), C(J),
            F(Right), C(NG), S('.'),
            S(' '),
        },
        {
            _(), V(Yao), _(),
            C(K), C(R), C(T),
            C(P), C(H), C(CH),
            _(), C(M), S(','),
            _(),
        },
        {
            _(), _(), _(),
            C(GG), _(), C(DD),
            C(BB), C(SS), C(JJ),
            _(), _(), S('?'),
            _(),
        },
        {
            _(), _(), _(),
            _(), _(), _(),
            _(), _(), _(),
            _(), _(), S('!'),
            _(),
        },
        {
        },
        #undef A
        #undef F
        #undef C
        #undef V
        #undef E
    };
}

}