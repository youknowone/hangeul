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

void state_set(State *state, int32_t key, int32_t value) {
    (*((hangeul::State *)state))[key] = value;
}

ALLOCFUNCS(UnicodeVector, unicodevector)

size_t unicodevector_size(UnicodeVector *unicodes) {
    return ((hangeul::UnicodeVector *)unicodes)->size();
}

Unicode unicodevector_get(UnicodeVector *unicodes, size_t index) {
    return (*((hangeul::UnicodeVector *)unicodes))[index];
}

ALLOCFUNCS(PhaseResult, phaseresult)
GETPFUNC(PhaseResult, phaseresult, State, state)
GETVFUNC(PhaseResult, phaseresult, bool, processed)

bool phase_put(Phase *phase, State *state) {
    auto statesp = (hangeul::State *)state;
    auto result = ((hangeul::Phase *)phase)->put(*statesp);
    *statesp = result.state;
    return result.processed;
}

Context *context_create(Phase *handler, Phase *combinator, Decoder *decoder) {
    return new hangeul::Context((hangeul::Phase *)handler, (hangeul::Phase *)combinator, (hangeul::Decoder *)decoder);
}

DEALLOCFUNC(Context, context)

GETCFUNC(Context, context, State, state)
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
    if (obj == nullptr) { obj = new hangeul::BlockPhase; }
    return obj;
}

Decoder *bypass_decoder() {
    static hangeul::Decoder *obj = nullptr;
    if (obj == nullptr) { obj = new hangeul::BypassDecoder; }
    return obj;
}

Phase *ksx5002_from_qwerty_handler() {
    static hangeul::Phase *obj = nullptr;
    static hangeul::KSX5002::Combinator *combinator = nullptr;
    if (obj == nullptr) {
        combinator = new hangeul::KSX5002::Combinator();
        obj = new hangeul::KSX5002::FromQwertyHandler(combinator);
    }
    return obj;
}

Phase *ksx5002_combinator() {
    static hangeul::Phase *obj = nullptr;
    if (obj == nullptr) { obj = new hangeul::KSX5002::Combinator; }
    return obj;
}

Decoder *ksx5002_decoder() {
    static hangeul::Decoder *obj = nullptr;
    static hangeul::KSX5002::Combinator *combinator = nullptr;
    if (obj == nullptr) {
        combinator = new hangeul::KSX5002::Combinator();
        obj = new hangeul::KSX5002::Decoder(combinator);
    }
    return obj;
}

Phase *danmoum_combinator() {
    static hangeul::Phase *obj = nullptr;
    if (obj == nullptr) { obj = new hangeul::Danmoum::Combinator; }
    return obj;
}

Phase *danmoum_from_qwerty_handler() {
    static hangeul::Phase *obj = nullptr;
    static hangeul::Danmoum::Combinator *combinator = nullptr;
    if (obj == nullptr) {
        combinator = new hangeul::Danmoum::Combinator();
        obj = new hangeul::Danmoum::FromQwertyHandler(combinator);
    }
    return obj;
}

Decoder *danmoum_decoder() {
    static hangeul::Decoder *obj = nullptr;
    static hangeul::Danmoum::Combinator *combinator = nullptr;
    if (obj == nullptr) {
        combinator = new hangeul::Danmoum::Combinator();
        obj = new hangeul::KSX5002::Decoder(combinator);
    }
    return obj;
}

Phase *cheonjiin_combinator() {
    static hangeul::Phase *obj = nullptr;
    if (obj == nullptr) { obj = new hangeul::Cheonjiin::Combinator; }
    return obj;
}

Phase *cheonjiin_from_tenkey_handler() {
    static hangeul::Phase *obj = nullptr;
    static hangeul::Cheonjiin::Combinator *combinator = nullptr;
    if (obj == nullptr) {
        combinator = new hangeul::Cheonjiin::Combinator();
        obj = new hangeul::Cheonjiin::FromTenkeyHandler(combinator);
    }
    return obj;
}

Decoder *cheonjiin_decoder() {
    static hangeul::Decoder *obj = nullptr;
    static hangeul::Cheonjiin::Combinator *combinator = nullptr;
    if (obj == nullptr) {
        combinator = new hangeul::Cheonjiin::Combinator();
        obj = new hangeul::Cheonjiin::Decoder(combinator);
    }
    return obj;
}

uint32_t ksx5002_label(char key) {
    static hangeul::QwertyToKeyStrokePhase stroke_phase;
    static hangeul::KSX5002::Layout layout;
    hangeul::State state;
    state[-1] = key;
    auto result = stroke_phase.put(state);
    auto annotation = layout.translate(state);
    auto unicode = layout.label(annotation);
    return unicode;
}
