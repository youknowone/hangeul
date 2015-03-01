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
            Symbol,
            Function,
        };
    }

    struct Annotation {
        AnnotationClass::Type type;
        uint32_t data;
    };

    class Decoder: public hangeul::Decoder {
    public:
        virtual State combined(State& state) = 0;
        virtual Unicode decode(int stroke) = 0;
        virtual UnicodeVector commited(State& state);
        virtual UnicodeVector composed(State& state);
    };



    class Layout {
    public:
        Tenkey::Annotation translate(State& state);
        Unicode label(Tenkey::Annotation annotation);
    };

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

namespace TableTenkey {
    typedef Tenkey::Annotation Table[][13];

    class Decoder: public Tenkey::Decoder {
        Table *_table;
    public:
        Decoder(Table *table) {
            this->_table = table;
        }
        virtual State combined(State& state) {
            auto rstate = State();
            auto strokes = state.array(STROKES_IDX);
            auto string = rstate.array(STRING_IDX);
            for (int i = 0; i < strokes.size(); i++) {
                auto stroke = strokes[i];
                auto character = this->decode(stroke);
                string.push_back(character);
            }
            return rstate;
        }
        virtual Unicode decode(int stroke) {
            auto level = stroke >> 8;
            auto position = stroke & 0xff;
            auto annotation = (*this->_table)[level][position];
            assert(annotation.type == Tenkey::AnnotationClass::Symbol);
            return annotation.data;
        }
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

        FromTenkeyHandler();
        //virtual ~FromTenkeyHandler();
    };

    extern Table AlphabetMap;
    extern Table NumberMap;
}

}

#endif /* defined(__hangeul__tenkey__) */
