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
        UnicodeVector Decoder::decode(State state) {
            UnicodeVector unicodes;
            if (state['a'] && state['b']) {
                auto a = state['a'];
                auto c = state['c'];
                auto v = 0xac00;
                v += (Initial::FromConsonant[a] - 1) * 21 * 28;
                v += (state['b'] - 1) * 28;
                if (c) {
                    v += Final::FromConsonant[c];
                }
                unicodes.push_back(v);
            }
            else if (state['a'] && !state['b']) {
                auto v = 0x3131 + state['a'] - 1;
                unicodes.push_back(v);
            }
            else if (!state['a'] && state['b']) {
                auto v = 0x314f + state['b'] - 1;
                unicodes.push_back(v);
            }
            else {
                unicodes.push_back(state[0]);
            }
            return unicodes;
        }

        Annotation Layout::translate(KeyStroke stroke, StateList states) {
            #define A(C) { AnnotationClass::ASCII, C }
            #define F(C) { AnnotationClass::Function, KeyPosition ## C }
            #define C(C) { AnnotationClass::Consonant, Consonant:: C }
            #define V(C) { AnnotationClass::Vowel, Vowel:: C }
            #define E() { AnnotationClass::Function, 0 }

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
                case AnnotationClass::Consonant:
                    return 0x3131 + annotation.data - 1;
                case AnnotationClass::Vowel:
                    return 0x314f + annotation.data - 1;
                case AnnotationClass::ASCII:
                    return annotation.data;
                case AnnotationClass::Function:
                    return 'X';
                default:
                    assert(false);
                    break;
            }
            return 0;
        }

        PhaseResult KeyStrokeToAnnotationPhase::put(StateList states) {
            #define DDD 0
            static Layout layout;

            auto& state = states.front();
            auto stroke = state[2];
            auto annotation = layout.translate(stroke, states);

            switch (annotation.type) {
                case AnnotationClass::Consonant:
                    state['a'] = annotation.data;
                    break;
                case AnnotationClass::Vowel:
                    state['b'] = annotation.data;
                    break;
                case AnnotationClass::ASCII:
                    state[0] = annotation.data;
                    break;
                case AnnotationClass::Function:
                    state[2] = annotation.data;
                    if (annotation.data == KeyPositionBackspace) {
                        state[-1] = 1;
                    }
                    break;
                default:
                    assert(false);
                    break;
            }

            auto res = PhaseResult::Make(states, state['a'] || state['b'] || state[-1]);
            return res;
            #undef DDD
        }

        PhaseResult BackspacePhase::put(StateList states) {
            states.pop_front();
            if (states.size() == 0) {
                return PhaseResult::Make(states, false);
            } else {
                auto& state = states.front();
                if (state['c']) {
                    auto pair = search_rule(FinalCompositionRules, state['c']);
                    if (!pair.is_none) {
                        state['c'] = pair.some[0];
                    } else {
                        state['c'] = 0;
                    }
                }
                else if (state['b']) {
                    auto pair = search_rule(VowelCompositionRules, state['b']);
                    if (!pair.is_none) {
                        state['b'] = pair.some[0];
                    } else {
                        if (state['a']) {
                            state['b'] = 0;
                        } else {
                            states.pop_front();
                        }
                    }
                }
                else if (state['a']) {
                    states.pop_front();
                }
                return PhaseResult::Make(states, true);
            }
        }

        PhaseResult JasoCompositionPhase::put(StateList states) {
            if (states.size() == 1) {
                return PhaseResult::Make(states, true);
            }
            auto iter = states.begin();
            auto& state = *iter;
            iter++;
            auto& secondary = *iter;
            bool combined = false;
            if (state['a'] && secondary['c']) {
                auto c1 = secondary['c'];
                auto c2 = state['a'];
                auto composed = search_rule(FinalCompositionRules, c1, c2);
                if (!composed.is_none) {
                    secondary['c'] = composed.some;
                    combined = true;
                }
            }
            else if (!secondary['c'] && state['b'] && secondary['b']) {
                auto c1 = secondary['b'];
                auto c2 = state['b'];
                auto composed = search_rule(VowelCompositionRules, c1, c2);
                if (!composed.is_none) {
                    secondary['b'] = composed.some;
                    combined = true;
                }
            }
            if (combined) {
                states.erase(states.cbegin());
                return PhaseResult::Make(states, false);
            }
            return PhaseResult::Make(states, true);
        }

        PhaseResult AnnotationToCombinationPhase::put(StateList states) {
            auto iter = states.begin();
            auto& state = states.front();
            if (!state['a'] && !state['b'] && !state['c']) {
                return PhaseResult::Make(states, true);
            }
            if (states.size() == 1) {
                return PhaseResult::Make(states, false);
            }
            iter++;
            auto& secondary = *iter;
            if (state['a']) {
                if (secondary['a'] && secondary['b'] && !secondary['c']) {
                    secondary['c'] = state['a'];
                    states.erase(states.cbegin());
                    return PhaseResult::Make(states, true);
                }
                else {
                    return PhaseResult::Make(states, true);
                }
            }
            else if (state['b']) {
                if (secondary['a'] && !secondary['b']) {
                    secondary['b'] = state['b'];
                    states.erase(states.cbegin());
                    return PhaseResult::Make(states, true);
                }
                else {
                    return PhaseResult::Make(states, true);
                }
            }
            else {
                assert(false);
            }
            return PhaseResult::Make(states, true);
        }

        PhaseResult ConsonantDecompositionPhase::put(StateList states) {
            if (states.size() < 2) {
                return PhaseResult::Make(states, true);
            }
            auto iter = states.begin();
            auto& state = states.front();
            iter++;
            auto& secondary = *iter;
            auto c = secondary['c'];
            if (state['b'] && !state['a'] && c) {
                bool decomposed = false;
                for (auto& rule: FinalCompositionRules) {
                    if (c == rule[2]) {
                        secondary['c'] = rule[0];
                        state['a'] = rule[1];
                        decomposed = true;
                        break;
                    }
                }
                if (!decomposed) {
                    state['a'] = c;
                    secondary['c'] = 0;
                }
                return PhaseResult::Make(states, true);
            }
            return PhaseResult::Make(states, true);
        }

        FromQwertyPhase::FromQwertyPhase() : CombinedPhase() {
            this->phases.push_back((Phase *)new QwertyToKeyStrokePhase());
            this->phases.push_back((Phase *)new KeyStrokeToAnnotationPhase());

            auto branch = new BranchPhase();
            this->phases.push_back(branch);

            {
                auto hangul_phase = new CombinedPhase();
                hangul_phase->phases.push_back((Phase *)new JasoCompositionPhase());
                hangul_phase->phases.push_back((Phase *)new AnnotationToCombinationPhase());
                hangul_phase->phases.push_back((Phase *)new ConsonantDecompositionPhase());

                auto success_phase = new SuccessPhase(hangul_phase);
                branch->phases.push_back(success_phase);
            }

            {
                auto backspace = new BackspacePhase();
                branch->phases.push_back(backspace);
            }
        }
    }

    namespace Danmoum {
        KSX5002::Annotation Layout::translate(KeyStroke stroke, StateList states) {
            #define A(C) { KSX5002::AnnotationClass::ASCII, C }
            #define F(C) { KSX5002::AnnotationClass::Function, KeyPosition ## C }
            #define C(C) { KSX5002::AnnotationClass::Consonant, Consonant:: C }
            #define V(C) { KSX5002::AnnotationClass::Vowel, Vowel:: C }
            #define E() { KSX5002::AnnotationClass::Function, 0 }

            static KSX5002::Annotation map1[] = {
                A('`'), A('1'), A('2'), A('3'), A('4'), A('5'), A('6'), A('7'), A('8'), A('9'), A('0'), A('-'), A('='), A('\\'), F(Backspace),  E(),
                A('\t'), C(B), C(J), C(D), C(G), C(S), V(O), V(Yeo), V(Ya), V(Ae), V(E), A('['),  A(']'), E(), E(), E(),
                E(), C(M), C(N), C(NG), C(R), C(H), V(Yo), V(Eo), V(A), V(I), A(';'), A('\''), A('\n'), E(), E(), E(),
                E(), C(K), C(T), C(CH), C(P), V(Yu), V(U), V(Eu), A(','), A('.'), A('/'), E(), E(), E(), E(), E(),
                A(' '),
            };

            static KSX5002::Annotation map2[] = {
                A('~'), A('!'), A('@'), A('#'), A('$'), A('%'), A('^'), A('&'), A('*'), A('('), A(')'), A('_'), A('+'), A('|'), F(Backspace),  E(),
                A('\t'), C(BB), C(JJ), C(DD), C(GG), C(SS), V(O), V(Yeo), V(Ya), V(Yae), V(Ye), A('{'),  A('}'), E(), E(), E(),
                E(), C(M), C(N), C(NG), C(R), C(H), V(Yo), V(Eo), V(A), V(I), A(':'), A('"'), A('\n'), E(), E(), E(),
                E(), C(K), C(T), C(CH), C(P), V(Yu), V(U), V(Eu), A('<'), A('>'), A('?'), E(), E(), E(), E(), E(),
                A(' '),
            };

            #undef A
            #undef F
            #undef C
            #undef V
            #undef E

            auto masked = stroke & 0xff;
            KSX5002::Annotation annotation;
            if (stroke & 0x20000) {
                annotation = map2[masked];
            } else {
                annotation = map1[masked];
            }
            return annotation;
        }

        PhaseResult KeyStrokeToAnnotationPhase::put(StateList states) {
            #define DDD 0
            static Layout layout;

            auto& state = states.front();
            auto stroke = state[2];
            auto annotation = layout.translate(stroke, states);

            switch (annotation.type) {
                case KSX5002::AnnotationClass::Consonant:
                    state['a'] = annotation.data;
                    break;
                case KSX5002::AnnotationClass::Vowel:
                    state['b'] = annotation.data;
                    break;
                case KSX5002::AnnotationClass::ASCII:
                    state[0] = annotation.data;
                    break;
                case KSX5002::AnnotationClass::Function:
                    state[2] = annotation.data;
                    if (annotation.data == KeyPositionBackspace) {
                        state[-1] = 1;
                    }
                    break;
                default:
                    assert(false);
                    break;
            }

            auto res = PhaseResult::Make(states, state['a'] || state['b'] || state[-1]);
            return res;
            #undef DDD
        }

        PhaseResult BackspacePhase::put(StateList states) {
            states.pop_front();
            if (states.size() == 0) {
                return PhaseResult::Make(states, false);
            } else {
                auto& state = states.front();
                if (state['c']) {
                    auto pair = search_rule(KSX5002::FinalCompositionRules, state['c']);
                    if (!pair.is_none) {
                        state['c'] = pair.some[0];
                    } else {
                        state['c'] = 0;
                    }
                }
                else if (state['b']) {
                    auto pair = search_rule(VowelCompositionRules, state['b']);
                    if (!pair.is_none) {
                        state['b'] = pair.some[0];
                    } else {
                        if (state['a']) {
                            state['b'] = 0;
                        } else {
                            states.pop_front();
                        }
                    }
                }
                else if (state['a']) {
                    states.pop_front();
                }
                return PhaseResult::Make(states, true);
            }
        }

        PhaseResult JasoCompositionPhase::put(StateList states) {
            if (states.size() == 1) {
                return PhaseResult::Make(states, true);
            }
            auto iter = states.begin();
            auto& state = *iter;
            iter++;
            auto& secondary = *iter;
            bool combined = false;
            if (state['a'] && secondary['c']) {
                auto c1 = secondary['c'];
                auto c2 = state['a'];
                auto composed = search_rule(KSX5002::FinalCompositionRules, c1, c2);
                if (!composed.is_none) {
                    secondary['c'] = composed.some;
                    combined = true;
                }
            }
            else if (!secondary['c'] && state['b'] && secondary['b']) {
                auto c1 = secondary['b'];
                auto c2 = state['b'];
                auto composed = search_rule(VowelCompositionRules, c1, c2);
                if (!composed.is_none) {
                    secondary['b'] = composed.some;
                    combined = true;
                }
            }
            if (combined) {
                states.erase(states.cbegin());
                return PhaseResult::Make(states, false);
            }
            return PhaseResult::Make(states, true);
        }

        FromQwertyPhase::FromQwertyPhase() : CombinedPhase() {
            this->phases.push_back((Phase *)new QwertyToKeyStrokePhase());
            this->phases.push_back((Phase *)new KeyStrokeToAnnotationPhase());

            auto branch = new BranchPhase();
            this->phases.push_back(branch);

            {
                auto hangul_phase = new CombinedPhase();
                hangul_phase->phases.push_back((Phase *)new JasoCompositionPhase());
                hangul_phase->phases.push_back((Phase *)new KSX5002::AnnotationToCombinationPhase());
                hangul_phase->phases.push_back((Phase *)new KSX5002::ConsonantDecompositionPhase());

                auto success_phase = new SuccessPhase(hangul_phase);
                branch->phases.push_back(success_phase);
            }

            {
                auto backspace = new BackspacePhase();
                branch->phases.push_back(backspace);
            }
        }
    }
}