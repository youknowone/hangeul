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
    struct Annotation {
        enum Type {
            Function,
            Consonant,
            Vowel,
            Symbol,
        };

        Type type;
        uint32_t data;

        bool operator==(Annotation& rhs) { return this->type == rhs.type && this->data == rhs.data; }
        bool operator!=(Annotation& rhs) { return !operator!=(rhs); }

        static Annotation None;
    };
    struct Stroke {
        union {
            uint16_t value;
            struct {
                uint8_t code;
                uint8_t phase;
            };
        };
        Stroke(uint16_t value): value(value) {}
        Stroke(uint8_t phase, uint8_t code): phase(phase), code(code) {}
        bool operator==(Stroke& rhs) { return this->value == rhs.value; }
        bool operator!=(Stroke& rhs) { return !operator==(rhs); }

    };

    class Decoder: public hangeul::Decoder {
    public:
        virtual State combined(State& state) = 0;
        virtual Annotation decode(int stroke) = 0;
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
            for (auto& stroke: strokes) {
                auto annotation = this->decode(stroke);
                if (annotation.type == Tenkey::Annotation::Symbol) {
                    string.push_back(annotation.data);
                }
            }
            return rstate;
        }
        virtual Tenkey::Annotation decode(int stroke) {
            auto level = stroke >> 8;
            auto position = stroke & 0xff;
            auto annotation = (*this->_table)[level][position];
            //assert(annotation.type == Tenkey::Annotation::Symbol);
            return annotation;
        }
    };

    class MergeStrokesPhase: public Phase {
        Table *_table;
        uint32_t _interval_ms;
    public:
        MergeStrokesPhase(Table *table, uint32_t interval_ms): _table(table), _interval_ms(interval_ms) {}

        virtual PhaseResult put(State& state);

        static std::string InputType() { return "keystroke"; }
        static std::string OutputType() { return "keystroke"; }
    };

    class UnstrokeBackspacePhase: public Phase {
        Table *_table;
    public:
        UnstrokeBackspacePhase(Table *table): _table(table) {}
        virtual PhaseResult put(State& state);

        static std::string InputType() { return "keystroke"; }
        static std::string OutputType() { return "keystroke"; }
    };

    class FromTenkeyHandler: public CombinedPhase {
    public:
        static std::string InputType() { assert(false); return "inputsource-tenkey"; }
        static std::string OutputType() { assert(false); return "combination-ksx5002"; }

        FromTenkeyHandler(Table *table);
        //virtual ~FromTenkeyHandler();
    };

    extern Table AlphabetMap;
    extern Table NumberMap;
}

}

#endif /* defined(__hangeul__tenkey__) */
