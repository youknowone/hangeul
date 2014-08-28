//
//  capi.cpp
//  hangeul
//
//  Created by Jeong YunWon on 8/5/14.
//  Copyright (c) 2014 youknowone.org. All rights reserved.
//

#include "capi.h"

#include <hangeul/hangeul.h>

#define ALLOCFUNC(TYPE, PREFIX) TYPE *PREFIX ## _create() { return new hangeul:: TYPE(); }
#define DEALLOCFUNC(TYPE, PREFIX) void PREFIX ## _free(TYPE *state) { delete (hangeul:: TYPE *)state; }
#define ALLOCFUNCS(TYPE, PREFIX) ALLOCFUNC(TYPE, PREFIX) DEALLOCFUNC(TYPE, PREFIX)

#define GETVFUNC(OBJTYPE, PREFIX, RETURNTYPE, FIELDNAME) RETURNTYPE PREFIX ## _get_ ## FIELDNAME(OBJTYPE *obj) { return (((hangeul:: OBJTYPE *)obj)->FIELDNAME); }
#define GETPFUNC(OBJTYPE, PREFIX, RETURNTYPE, FIELDNAME) RETURNTYPE *PREFIX ## _get_ ## FIELDNAME(OBJTYPE *obj) { return &(((hangeul:: OBJTYPE *)obj)->FIELDNAME); }
#define GETCFUNC(OBJTYPE, PREFIX, RETURNTYPE, FIELDNAME) RETURNTYPE *PREFIX ## _get_ ## FIELDNAME(OBJTYPE *obj) { return &(((hangeul:: OBJTYPE *)obj)->FIELDNAME()); }


ALLOCFUNCS(State, state)
ALLOCFUNCS(StateList, statelist)

void state_set(State *state, int32_t key, int32_t value) {
    (*((hangeul::State *)state))[key] = value;
}

void statelist_append(StateList *states, State *state) {
    ((hangeul::StateList *)states)->push_front(*(hangeul::State *)state);
}

ALLOCFUNCS(UnicodeVector, unicodevector)

size_t unicodevector_size(UnicodeVector *unicodes) {
    return ((hangeul::UnicodeVector *)unicodes)->size();
}

Unicode unicodevector_get(UnicodeVector *unicodes, size_t index) {
    return (*((hangeul::UnicodeVector *)unicodes))[index];
}

ALLOCFUNCS(PhaseResult, phaseresult)
GETPFUNC(PhaseResult, phaseresult, StateList, states)
GETVFUNC(PhaseResult, phaseresult, bool, processed)

bool phase_put(Phase *phase, StateList *states) {
    auto statesp = (hangeul::StateList *)states;
    auto result = ((hangeul::Phase *)phase)->put(*statesp);
    *statesp = result.states;
    return result.processed;
}

Context *context_create(Phase *processor, Decoder *decoder) {
    return new hangeul::Context((hangeul::Phase *)processor, (hangeul::Decoder *)decoder);
}

DEALLOCFUNC(Context, context)

GETCFUNC(Context, context, StateList, states)
GETCFUNC(Context, context, Decoder, decoder)

bool context_put(Context *context, InputSource input) {
    return ((hangeul::Context *)context)->put(input);
}

void context_flush(Context *context) {
    ((hangeul::Context *)context)->flush();
}

void context_truncate(Context *context) {
    ((hangeul::Context *)context)->truncate();
}

void context_get_commited(Context *context, UnicodeVector *buffer) {
    *(hangeul::UnicodeVector *)buffer = ((hangeul::Context *)context)->commited();
}

void context_get_composed(Context *context, UnicodeVector *buffer) {
    *(hangeul::UnicodeVector *)buffer = ((hangeul::Context *)context)->composed();
}

Phase *bypass_phase() {
    static hangeul::Phase *obj = nullptr;
    if (obj == nullptr) { obj = new hangeul::BypassPhase; }
    return obj;
}

Decoder *bypass_decoder() {
    static hangeul::Decoder *obj = nullptr;
    if (obj == nullptr) { obj = new hangeul::BypassDecoder; }
    return obj;
}

Phase *ksx5002_from_qwerty_phase() {
    static hangeul::Phase *obj = nullptr;
    if (obj == nullptr) { obj = new hangeul::KSX5002::FromQwertyPhase; }
    return obj;
}

Decoder *ksx5002_decoder() {
    static hangeul::Decoder *obj = nullptr;
    if (obj == nullptr) { obj = new hangeul::KSX5002::Decoder; }
    return obj;
}

uint32_t ksx5002_label(char key) {
    static hangeul::QwertyToKeyStrokePhase stroke_phase;
    static hangeul::KSX5002::Layout layout;
    hangeul::State state;
    state[1] = key;
    hangeul::StateList states;
    states.push_back(state);
    auto result = stroke_phase.put(states);
    auto stroke = result.states.front()[2];
    auto annotation = layout.translate(stroke, states);
    auto unicode = layout.label(annotation);
    return unicode;
}
