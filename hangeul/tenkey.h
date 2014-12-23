//
//  tenkey.h
//  hangeul
//
//  Created by Jeong YunWon on 2014. 12. 30..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#ifndef __hangeul__tenkey__
#define __hangeul__tenkey__

#include <utility>

#include <hangeul/phase.h>
#include <hangeul/decoder.h>

namespace hangeul {

namespace Tenkey {
    namespace AnnotationClass {
        enum Type {
            Consonant,
            Vowel,
            Punctuation,
            Function,
        };
    }

    struct Annotation {
        AnnotationClass::Type type;
        uint32_t data;
    };
    
    class MergeStrokesPhase: public Phase {
        virtual PhaseResult put(State& state);

        static std::string InputType() { return "keystroke"; }
        static std::string OutputType() { return "keystroke"; }
    };

    class UnstrokeBackspacePhase: public Phase {
        virtual PhaseResult put(State& state);

        static std::string InputType() { return "keystroke"; }
        static std::string OutputType() { return "keystroke"; }
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

    class Combinator: public hangeul::Combinator {
    public:
        static std::string InputType() { assert(false); return "inputsource-tenkey"; }
        static std::string OutputType() { assert(false); return "combination-cheonjiin"; }
        
        Combinator();
    };

}

}

#endif /* defined(__hangeul__tenkey__) */
