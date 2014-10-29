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

namespace hangeul {
    extern State empty_state;
    typedef unsigned CompositionRule[3];

    Optional<CompositionRule> search_rule(const CompositionRule *rules, const unsigned result);
    Optional<unsigned> search_rule(const CompositionRule *rules, unsigned input1, unsigned input2);

    struct PhaseResult {
        StateList states;
        bool processed;

        static PhaseResult Stop() { PhaseResult result; result.processed = false; return result; }
        static PhaseResult Make(StateList states, bool processed) {
            PhaseResult res = { states, processed };
            return res;
        }
    };

    class Phase { // abstract
    public:
        virtual PhaseResult put(StateList states) = 0;
        virtual PhaseResult put_state(StateList states, State new_state); 

        static std::string InputType() { assert(false); return ""; }
        static std::string OutputType() { assert(false); return ""; }

        bool validate_get(uint32_t key) { return true; }
        bool validate_set(uint32_t key, uint32_t value) { return true; }

        virtual ~Phase() { }
    };

    class BypassPhase: public Phase {
        virtual PhaseResult put(StateList states) { return PhaseResult::Make(states, true); }
    };

    class BlockPhase: public Phase {
        virtual PhaseResult put(StateList states) { return PhaseResult::Make(states, false); }
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
        virtual PhaseResult put(StateList state);
    };

    //! try-failthen
    class FallbackPhase: public MultiplePhase {
    public:
        virtual PhaseResult put(StateList state);
    };

    //! try-finally
    class PostProcessPhase: public MultiplePhase {
    public:
        virtual PhaseResult put(StateList states);
    };

    class BranchPhase: public MultiplePhase {
    public:
        virtual PhaseResult put(StateList states);
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
        virtual PhaseResult put(StateList states) {
            if (this->phase) {
                auto result = this->phase->put(states);
                states = result.states;
            }
            return PhaseResult::Make(states, true);
        }
    };

    //! state[1]->state[1] 변환
    class InputSourceTransformationPhase: public Phase { // abstract
        //virtual PhaseResult put(StateList states) { return PhaseResult::Make(states, false); }
    };

    //! state[1]->state[2] 변환
    class KeyStrokePhase: public Phase { // abstract
        static std::string OutputType() { return "keystroke"; }
    };

    //! state[2]->... 변환
    class AnnotationPhase: public Phase { // abstract
    };

    class QwertyToKeyStrokePhase: public KeyStrokePhase {
    public:
        virtual PhaseResult put(StateList states);

        static std::string InputType() { return "inputsource-qwerty"; }
    };

    class MacKeycodeToKeyStrokePhase: public KeyStrokePhase {
    public:
        virtual PhaseResult put(StateList state);

        static std::string InputType() { return "inputsource-keycode-mac"; }
    };

}

#endif