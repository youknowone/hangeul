//
//  phase.h
//  hangeul
//
//  Created by Jeong YunWon on 2014. 6. 7..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#ifndef __HANGEUL_PHASE__
#define __HANGEUL_PHASE__

#include <cassert>
#include <string>

#include <hangeul/util.h>
#include <hangeul/data.h>
#include <hangeul/type.h>

#define STROKES_IDX 0x10000
#define TIME_IDX 0x15000
#define STRING_IDX 0x20000

namespace hangeul {
    extern State empty_state;
    typedef unsigned CompositionRule[3];

    Optional<CompositionRule> search_rule(const CompositionRule *rules, const unsigned result);
    Optional<unsigned> search_rule(const CompositionRule *rules, unsigned input1, unsigned input2);

    struct PhaseResult {
        State state;
        bool processed;

        static PhaseResult Stop() { PhaseResult result; result.processed = false; return result; }
        static PhaseResult Make(State state, bool processed) {
            PhaseResult res = { state, processed };
            return res;
        }
    };

    class Phase { // abstract
    public:
        virtual PhaseResult put(State& state) = 0;

        static std::string InputType() { assert(false); return ""; }
        static std::string OutputType() { assert(false); return ""; }

        bool validate_get(uint32_t key) { return true; }
        bool validate_set(uint32_t key, uint32_t value) { return true; }

        virtual ~Phase() { }
    };

    class BypassPhase: public Phase {
        virtual PhaseResult put(State& state) { return PhaseResult::Make(state, true); }
    };

    class BlockPhase: public Phase {
        virtual PhaseResult put(State& state) { return PhaseResult::Make(state, false); }
    };

    class MultiplePhase: public Phase {
        bool needs_delete;
    public:
        std::vector<Phase *> phases;
        MultiplePhase(bool needs_delete = true) {
            this->needs_delete = needs_delete;
        }
        MultiplePhase(std::vector<Phase *> phases, bool needs_delete = true) {
            this->phases = phases;
            this->needs_delete = needs_delete;
        }
        virtual ~MultiplePhase() {
            if (needs_delete) {
                for (auto& phase: phases) {
                    delete phase;
                }
            }
        }
    };

    class CombinedPhase: public MultiplePhase {
    public:
        virtual PhaseResult put(State& state);
    };

    //! try-failthen
    class FallbackPhase: public MultiplePhase {
    public:
        virtual PhaseResult put(State& state);
    };

    //! try-finally
    class PostProcessPhase: public MultiplePhase {
    public:
        virtual PhaseResult put(State& state);
    };

    class BranchPhase: public MultiplePhase {
    public:
        virtual PhaseResult put(State& state);
    };

    class MetaPhase: public Phase {
        bool needs_delete;
    public:
        Phase *phase = nullptr;
        MetaPhase(bool needs_delete = true) {
            this->needs_delete = needs_delete;
        }
        MetaPhase(Phase *phase, bool needs_delete = true) {
            this->phase = phase;
            this->needs_delete = needs_delete;
        }
        virtual ~MetaPhase() {
            if (needs_delete) {
                delete this->phase;
            }
        }
    };

    class SuccessPhase: public MetaPhase {
    public:
        SuccessPhase(Phase *phase, bool needs_delete = true): MetaPhase(phase, needs_delete) { }
        virtual PhaseResult put(State& state) {
            if (this->phase) {
                auto result = this->phase->put(state);
                state = result.state;
            }
            return PhaseResult::Make(state, true);
        }
    };

    //! state[0]->state[0] 변환
    class InputSourceTransformationPhase: public Phase { // abstract
        //virtual PhaseResult put(State& state) { return PhaseResult::Make(states, false); }
    };

    //! state[0]->state[1] 변환
    class ToKeyStrokePhase: public Phase { // abstract
        static std::string OutputType() { return "keystroke"; }
    };

    class KeyStrokeStackPhase: public Phase {
        virtual PhaseResult put(State& state);

        static std::string InputType() { return "keystroke"; }
        static std::string OutputType() { return "keystroke"; }
    };

    class UnstrokeBackspacePhase: public Phase {
        virtual PhaseResult put(State& state);

        static std::string InputType() { return "keystroke"; }
        static std::string OutputType() { return "keystroke"; }
    };

    //! state[2]->... 변환
    class ToAnnotationPhase: public Phase { // abstract
    };

    class QwertyToKeyStrokePhase: public ToKeyStrokePhase {
    public:
        virtual PhaseResult put(State& state);

        static std::string InputType() { return "inputsource-qwerty"; }
    };

    class MacKeycodeToKeyStrokePhase: public ToKeyStrokePhase {
    public:
        virtual PhaseResult put(State& state);

        static std::string InputType() { return "inputsource-keycode-mac"; }
    };

    class Combinator: public CombinedPhase {
    public:
        static std::string InputType() { assert(false); return "keystroke"; }
        static std::string OutputType() { assert(false); return "combination"; }
    };

    class CombinatorPhase: public MetaPhase {
    public:
        CombinatorPhase(Phase *phase, bool needs_delete = true) : MetaPhase(phase, needs_delete) { }
        virtual PhaseResult put(State& state);
        static std::string InputType() { assert(false); return "keystroke"; }
        static std::string OutputType() { assert(false); return "combination"; }
    };
}

#endif