//
//  ksx5002.cpp
//  hangeul
//
//  Created by Jeong YunWon on 2014. 7. 5..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#include "KSX5002.h"
#include "_debug.h"

namespace hangeul {

    namespace KSX5002 {
        State Decoder::combined(State& state) {
            auto rstate = State();
            auto strokes = state.array(STROKES_IDX);
            auto timestack = state.array(TIME_IDX);
            assert(strokes.size() == timestack.size());
            for (auto i = 0; i < strokes.size(); i++) {
                rstate[0] = strokes[i];
                rstate[-'t'] = timestack[i];
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
                auto c = stroke[3];
                auto v = 0xac00;
                v += (Initial::FromConsonant[a] - 1) * 21 * 28;
                v += (stroke[2] - 1) * 28;
                if (c) {
                    v += Final::FromConsonant[c];
                }
                unicodes.push_back(v);
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
                auto to_idx = string.size() - 1;
                if (string.size() > 0) {
                    string.erase(0, to_idx);
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

        Annotation Layout::translate(State& state) {
            #define A(C) { Annotation::ASCII, C }
            #define F(C) { Annotation::Function, KeyPosition ## C }
            #define C(C) { Annotation::Consonant, Consonant:: C }
            #define V(C) { Annotation::Vowel, Vowel:: C }
            #define E() { Annotation::Function, 0 }

            static Annotation map1[] = {
                A('`'), A('1'), A('2'), A('3'), A('4'), A('5'), A('6'), A('7'), A('8'), A('9'), A('0'), A('-'), A('='), A('\\'), F(Backspace),  E(),
                A('\t'), C(B), C(J), C(D), C(G), C(S), V(Yo), V(Yeo), V(Ya), V(Ae), V(E), A('['),  A(']'), E(), E(), E(),
                E(), C(M), C(N), C(NG), C(R), C(H), V(O), V(Eo), V(A), V(I), A(';'), A('\''), A('\n'), E(), E(), E(),
                E(), C(K), C(T), C(CH), C(P), V(Yu), V(U), V(Eu), A(','), A('.'), A('/'), E(), E(), E(), E(), E(),
                A(' '),
            };

            static Annotation map2[] = {
                A('~'), A('!'), A('@'), A('#'), A('$'), A('%'), A('^'), A('&'), A('*'), A('('), A(')'), A('_'), A('+'), A('|'), F(Backspace),  E(),
                A('\t'), C(BB), C(JJ), C(DD), C(GG), C(SS), V(Yo), V(Yeo), V(Ya), V(Yae), V(Ye), A('{'),  A('}'), E(), E(), E(),
                E(), C(M), C(N), C(NG), C(R), C(H), V(O), V(Eo), V(A), V(I), A(':'), A('"'), A('\n'), E(), E(), E(),
                E(), C(K), C(T), C(CH), C(P), V(Yu), V(U), V(Eu), A('<'), A('>'), A('?'), E(), E(), E(), E(), E(),
                A(' '),
            };

            #undef A
            #undef F
            #undef C
            #undef V
            #undef E

            auto stroke = state.latestKeyStroke();
            auto masked = stroke & 0xff;
            Annotation annotation;
            if (stroke & 0x20000) {
                annotation = map2[masked];
            } else {
                annotation = map1[masked];
            }
            return annotation;
        }

        Unicode Layout::label(Annotation annotation) {
            switch (annotation.type) {
                case Annotation::Consonant:
                    return 0x3131 + annotation.data - 1;
                case Annotation::Vowel:
                    return 0x314f + annotation.data - 1;
                case Annotation::ASCII:
                    return annotation.data;
                case Annotation::Function:
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
            auto timestack = state.array(0x3000);

            characters.push_back(annotation.type);
            strokes.push_back(annotation.data);
            timestack.push_back(state.latestKeyStrokeTime());
            assert(strokes.size() == timestack.size());
            auto character = state.array(0x1000 + characters.ssize() * 0x10);

            character[1] = 0;
            character[2] = 0;
            character[10] = 0;
            character[11] = 0;
            switch (annotation.type) {
                case Annotation::Consonant:
                    character[1] = annotation.data;
                    break;
                case Annotation::Vowel:
                    character[2] = annotation.data;
                    break;
                case Annotation::ASCII:
                    character[10] = annotation.data;
                    break;
                case Annotation::Function:
                    character[11] = annotation.data;
                    break;
                default:
                    assert(false);
                    break;
            }

            auto res = PhaseResult::Make(state, character[1] || character[2]);
            return res;
        }

        PhaseResult JasoCompositionPhase::put(State& state) {
            auto characters = state.array(0x1000);
            if (characters.size() < 2) {
                return PhaseResult::Make(state, true);
            }

            auto c1 = state.array(0x1000 + (characters.ssize() - 0) * 0x10);
            auto c2 = state.array(0x1000 + (characters.ssize() - 1) * 0x10);

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
            auto c1 = state.array(0x1000 + (characters.ssize() - 0) * 0x10);
            if (!c1[1] && !c1[2] && !c1[3]) {
                return PhaseResult::Make(state, true);
            }

            if (characters.size() < 2) {
                return PhaseResult::Make(state, false);
            }

            auto c2 = state.array(0x1000 + (characters.ssize() - 1) * 0x10);
            if (c1[1] && c1[2]) {

            }
            else if (c1[1]) {
                if (c2[1] && c2[2] && !c2[3] && Final::FromConsonant[c1[1]] != Final::None) {
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

            auto c1 = state.array(0x1000 + (strokes.ssize() - 0) * 0x10);
            auto c2 = state.array(0x1000 + (strokes.ssize() - 1) * 0x10);

            auto c = c2[3];
            if (c1[2] && !c1[1] && c) {
                auto strokes = state.array(0x2000);
                auto timestack = state.array(0x3000);
                assert(strokes.size() == timestack.size());
                auto s2 = strokes[-2];
                auto decomposed = false;
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

        FromQwertyHandler::FromQwertyHandler(hangeul::Combinator *combinator) : CombinedPhase() {
            this->phases.push_back((Phase *)new QwertyToKeyStrokePhase());
            this->phases.push_back((Phase *)new KeyStrokeStackPhase());
            this->phases.push_back((Phase *)new UnstrokeBackspacePhase());
            this->phases.push_back((Phase *)new CombinatorPhase((Phase *)combinator, false));
        }

//        FromQwertyHandler::~FromQwertyHandler() {
//            this->phases.pop_back();
//            MultiplePhase::~MultiplePhase();
//        }

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

    namespace Danmoum {
        PhaseResult JasoCompositionPhase::put(State& state) {
            auto characters = state.array(0x1000);
            if (characters.size() < 2) {
                return PhaseResult::Make(state, true);
            }

            auto c1 = state.array(0x1000 + ((int32_t)characters.size() - 0) * 0x10);
            auto c2 = state.array(0x1000 + ((int32_t)characters.size() - 1) * 0x10);

            bool combined = false;
            if (c1[1] && c2[3]) {
                auto j1 = c2[3];
                auto j2 = c1[1];
                auto composed = search_rule(KSX5002::FinalCompositionRules, j1, j2);
                if (!composed.is_none) {
                    c2[3] = composed.some;
                    combined = true;
                } else {
                    auto timestack = state.array(0x3000);
                    auto timecond = timestack.size() >= 2 && timestack[-1] - timestack[-2] < this->_interval;
                    if (timecond) {
                        auto combination = search_rule(KSX5002::FinalCompositionRules, j1);
                        if (!combination.is_none) {
                            j1 = combination.some[1];
                        }
                        composed = search_rule(KSX5002::InitialCompositionRules, j1, j2);
                        if (!composed.is_none) {
                            c2[3] = combination.is_none ? 0 : combination.some[0];
                            c1[1] = composed.some;
                        }
                    }
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
                auto composed = search_rule(KSX5002::InitialCompositionRules, j1, j2);
                if (!composed.is_none) {
                    c2[1] = composed.some;
                    combined = true;
                }
            }
            if (combined) {
                characters.pop_back();
            }
            return PhaseResult::Make(state, true);
        }

        PhaseResult ConsonantDecompositionPhase::put(State& state) {
            auto strokes = state.array(0x1000);
            if (strokes.size() < 2) {
                return PhaseResult::Make(state, true);
            }

            auto c1 = state.array(0x1000 + ((int32_t)strokes.size() - 0) * 0x10);
            auto c2 = state.array(0x1000 + ((int32_t)strokes.size() - 1) * 0x10);

            auto c = c2[3];
            if (c1[2] && !c1[1] && c) {
                auto strokes = state.array(0x2000);
                auto timestack = state.array(0x3000);
                assert(strokes.size() == timestack.size());
                auto s2 = strokes[-2];
                auto decomposed = false;
                auto timecond = timestack.size() >= 3 && timestack[-2] - timestack[-3] < this->_interval;
                for (auto& rule: KSX5002::FinalCompositionRules) {
                    if (c == rule[2] && rule[1] == s2) {
                        if (rule[0] == rule[1] && timecond) {
                            break;
                        }
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

        FromQwertyHandler::FromQwertyHandler(hangeul::Combinator *combinator) : CombinedPhase() {
            this->phases.push_back((Phase *)new QwertyToKeyStrokePhase());
            this->phases.push_back((Phase *)new KeyStrokeStackPhase());
            this->phases.push_back((Phase *)new UnstrokeBackspacePhase());
            this->phases.push_back((Phase *)new CombinatorPhase((Phase *)combinator, false));
        }

        Combinator::Combinator() : hangeul::Combinator() {
            this->phases.push_back((Phase *)new KSX5002::AnnotationPhase());

            auto hangul_phase = new CombinedPhase();
            hangul_phase->phases.push_back((Phase *)new JasoCompositionPhase(250));
            hangul_phase->phases.push_back((Phase *)new KSX5002::AnnotationToCombinationPhase());
            hangul_phase->phases.push_back((Phase *)new ConsonantDecompositionPhase(250));

            auto success_phase = new SuccessPhase(hangul_phase);
            this->phases.push_back(success_phase);
        }
    }
}