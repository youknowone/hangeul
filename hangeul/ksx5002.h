//
//  KSX5002.h
//  hangeul
//
//  Created by Jeong YunWon on 2014. 7. 5..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#ifndef __hangeul__KSX5002__
#define __hangeul__KSX5002__

#include <utility>

#include <hangeul/phase.h>
#include <hangeul/decoder.h>

namespace hangeul { namespace KSX5002 {

    namespace AnnotationClass {
        enum Type {
            ASCII,
            Function,
            Consonant,
            Vowel,
        };
    }

    const CompositionRule FinalCompositionRules[] = {
        #define R(A, B, X) {Consonant::A, Consonant::B, Consonant::X}
        R(G, S, GS),
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

    const CompositionRule VowelCompositionRules[] = {
        #define R(A, B, X) {Vowel::A, Vowel::B, Vowel::X}
        R(O, A, Wa),
        R(O, Ae, Wae),
        R(O, I, Oe),
        R(U, Eo, Weo),
        R(U, E, We),
        R(U, I, Wi),
        R(Eu, I, Ui),
        {0, 0, 0},
        #undef R
    };

    struct Annotation {
        AnnotationClass::Type type;
        uint32_t data;
    };

    class Decoder: public hangeul::Decoder {
    public:
        virtual UnicodeVector decode(State state);
    };

    //! state[2]->state[0, 'a', 'b', 'c']
    class KeyStrokeToAnnotationPhase: public AnnotationPhase {
    public:
        virtual PhaseResult put(StateList state);

        static std::string InputType() { assert(false); return "keyposition"; }
        static std::string OutputType() { assert(false); return "annotation-ksx5002"; }
    };

    class BackspacePhase: public Phase {
    public:
        virtual PhaseResult put(StateList state);

        static std::string InputType() { assert(false); return "annotation-ksx5002"; }
        static std::string OutputType() { assert(false); return "combination-ksx5002"; }
    };

    class JasoCompositionPhase: public Phase {
    public:
        virtual PhaseResult put(StateList state);

        static std::string InputType() { assert(false); return "annotation-ksx5002"; }
        static std::string OutputType() { assert(false); return "annotation-ksx5002"; }
    };

    class AnnotationToCombinationPhase: public Phase {
    public:
        virtual PhaseResult put(StateList state);

        static std::string InputType() { assert(false); return "annotation-ksx5002"; }
        static std::string OutputType() { assert(false); return "combination-ksx5002"; }
    };

    class ConsonantDecompositionPhase: public Phase {
    public:
        virtual PhaseResult put(StateList state);

        static std::string InputType() { assert(false); return "combination-ksx5002"; }
        static std::string OutputType() { assert(false); return "combination-ksx5002"; }
    };

    class FromQwertyPhase: public CombinedPhase {
    public:
        static std::string InputType() { assert(false); return "inputsource-qwerty"; }
        static std::string OutputType() { assert(false); return "combination-ksx5002"; }

        FromQwertyPhase();
    };

} }

#endif /* defined(__hangeul__KSX5002__) */
