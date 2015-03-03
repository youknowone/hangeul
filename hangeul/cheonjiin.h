//
//  cheonjiin.h
//  hangeul
//
//  Created by Jeong YunWon on 2014. 7. 5..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#ifndef __hangeul__cheonjiin__
#define __hangeul__cheonjiin__

#include <utility>

#include <hangeul/tenkey.h>

namespace hangeul {

namespace Cheonjiin {

    const CompositionRule InitialCompositionRules[] = {
        #define R(A, B, X) {Consonant::A, Consonant::B, Consonant::X}
        R(G, G, K),
        R(GG, G, G),
        R(N, N, R),
        R(D, D, T),
        R(DD, D, D),
        R(R, N, N),
        R(M, NG, NG),
        R(B, B, P),
        R(BB, B, B),
        R(S, S, H),
        R(SS, S, S),
        R(NG, NG, M),
        R(J, J, CH),
        R(JJ, J, J),
        R(CH, J, JJ),
        R(K, G, GG),
        R(T, D, DD),
        R(P, B, BB),
        R(H, S, SS),
        {0, 0, 0},
        #undef R
    };

    const CompositionRule VowelCompositionRules[] = {
        #define R(A, B, X) {Vowel::A, Vowel::B, Vowel::X}
        R(A, I, Ae),
        R(A, Ao, Ya),
        R(I, Yao, Ya),
        R(Ya, I, Yae),
        R(Ya, Ao, A),
        R(Eo, I, E),
        R(Yeo, I, Ye),
        R(Ao, I, Eo),
        R(Wa, I, Wae),
        R(Oe, A, Wa),
        R(O, I, Oe),
        R(Oe, Ao, Wa),
        R(U, I, Wi),
        R(U, Ao, Yu),
        R(Eu, Yao, Yu),
        R(Weo, I, We),
        R(Yu, I, Weo),
        R(Yu, Ao, U),
        R(Eu, I, Ui),
        R(Eu, Ao, U),
        R(I, Ao, A),
        R(Ao, Eu, O),
        R(Yao, I, Yeo),
        R(Ao, I, Eo),
        R(Ao, Ao, Yao),
        R(Yao, Eu, Yo),
        R(Yao, I, Yeo),
        R(Yao, Ao, Ao),
        {0, 0, 0},
        #undef R
    };

    const CompositionRule FinalCompositionRules[] = {
        #define R(A, B, X) {Consonant::A, Consonant::B, Consonant::X}
        R(G, S, GS),
        R(G, SS, G),
        R(G, H, G),
        R(N, S, N),
        R(N, J, NJ),
        R(N, H, NH),
        R(R, G, RG),
        R(R, M, RM),
        R(R, B, RB),
        R(R, S, RS),
        R(R, T, RT),
        R(R, P, RP),
        R(R, H, RH),
        R(B, S, BS),
        {0, 0, 0},
        #undef R
    };

    class FromTenkeyHandler: public CombinedPhase {
    public:
        static std::string InputType() { assert(false); return "inputsource-tenkey"; }
        static std::string OutputType() { assert(false); return "combination-ksx5002"; }

        FromTenkeyHandler(Combinator *combinator);
        //virtual ~FromTenkeyHandler();
    };

    class Decoder: public hangeul::Decoder {
        Combinator *combinator = nullptr;
    public:
        Decoder(Combinator *combinator) { this->combinator = combinator; } // is copying ok?
        virtual State combined(State& state);
        virtual UnicodeVector decode(State::ArrayProxy& stroke);
        virtual UnicodeVector commited(State& state);
        virtual UnicodeVector composed(State& state);
    };

    class Layout {
    public:
        Tenkey::Annotation translate(State& state);
        Unicode label(Tenkey::Annotation annotation);
    };

    //! state[2]->state[0, 'a', 'b', 'c']
    class AnnotationPhase: public hangeul::ToAnnotationPhase {
    public:
        virtual PhaseResult put(State& state);

        static std::string InputType() { assert(false); return "keyposition"; }
        static std::string OutputType() { assert(false); return "annotation-cheonjiin"; }
    };

    class JasoCompositionPhase: public Phase {
    public:
        virtual PhaseResult put(State& state);

        static std::string InputType() { assert(false); return "annotation-cheonjiin"; }
        static std::string OutputType() { assert(false); return "annotation-cheonjiin"; }
    };

    class AnnotationToCombinationPhase: public Phase {
    public:
        virtual PhaseResult put(State& state);

        static std::string InputType() { assert(false); return "annotation-cheonjiin"; }
        static std::string OutputType() { assert(false); return "combination-cheonjiin"; }
    };

    class ConsonantDecompositionPhase: public Phase {
    public:
        virtual PhaseResult put(State& state);

        static std::string InputType() { assert(false); return "combination-cheonjiin"; }
        static std::string OutputType() { assert(false); return "combination-cheonjiin"; }
    };

    class Combinator: public hangeul::Combinator {
    public:
        static std::string InputType() { assert(false); return "inputsource-tenkey"; }
        static std::string OutputType() { assert(false); return "combination-cheonjiin"; }
        
        Combinator();
    };

    extern TableTenkey::Table CheonjiinMap;
} }

#endif /* defined(__hangeul__cheonjiin__) */
