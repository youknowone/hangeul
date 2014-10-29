//
//  capi.h
//  hangeul
//
//  Created by Jeong YunWon on 8/5/14.
//  Copyright (c) 2014 youknowone.org. All rights reserved.
//

#ifndef __hangeul__capi__
#define __hangeul__capi__

#if __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

typedef uint32_t Unicode;
typedef uint32_t InputSource;
typedef void State;
typedef void StateList;
typedef void Phase;
typedef void PhaseResult;
typedef void Decoder;
typedef void UnicodeVector;
typedef void Context;

State *state_create();
void state_free(State *state);
void state_set(State *state, int32_t key, int32_t value);
StateList *statelist_create();
void statelist_append(StateList *states, State *state);

UnicodeVector *unicodevector_create();
void unicodevector_free(UnicodeVector *unicodes);
size_t unicodevector_size(UnicodeVector *unicodes);
Unicode unicodevector_get(UnicodeVector *unicodes, size_t index);

PhaseResult *phaseresult_create();
void phase_resultfree(PhaseResult *);

StateList *phaseresult_get_states(PhaseResult *);
bool phaseresult_get_processed(PhaseResult *);

bool phase_put(Phase *phase, StateList *states);

Context *context_create(Phase *processor, Decoder *decoder);
StateList *context_get_states(Context *context);
Decoder *context_get_decoder(Context *context);
bool context_put(Context *context, InputSource input);
void context_flush(Context *context);
void context_truncate(Context *context);
void context_get_commited(Context *context, UnicodeVector *buffer);
void context_get_composed(Context *context, UnicodeVector *buffer);

Phase *bypass_phase();
Decoder *bypass_decoder();
Phase *ksx5002_from_qwerty_phase();
Decoder *ksx5002_decoder();
Phase *danmoum_from_qwerty_phase();

uint32_t ksx5002_label(char key);
uint32_t danmoum_label(char key);

#if __cplusplus
}
#endif

#endif /* defined(__hangeul__capi__) */
