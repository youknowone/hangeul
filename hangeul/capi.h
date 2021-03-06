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
#else
#define bool int
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
void unicodevector_append(UnicodeVector *unicodes, Unicode unicode);
Unicode unicodevector_get(UnicodeVector *unicodes, size_t index);

PhaseResult *phaseresult_create();
void phase_resultfree(PhaseResult *);

StateList *phaseresult_get_states(PhaseResult *);
bool phaseresult_get_processed(PhaseResult *);

bool phase_put(Phase *phase, StateList *states);

Context *context_create(Phase *handler, Phase *combinator, Decoder *decoder);
StateList *context_get_states(Context *context);
Decoder *context_get_decoder(Context *context);
bool context_put(Context *context, InputSource input);
void context_flush(Context *context);
void context_truncate(Context *context);
void context_get_commited(Context *context, UnicodeVector *buffer);
void context_get_composed(Context *context, UnicodeVector *buffer);

void nfc_to_nfd(UnicodeVector *nfc, UnicodeVector *nfd);

Phase *bypass_phase();
Decoder *bypass_decoder();
Phase *ksx5002_from_qwerty_handler();
Phase *ksx5002_combinator();
Decoder *ksx5002_decoder();
Phase *danmoum_combinator();
Phase *danmoum_from_qwerty_handler();
Decoder *danmoum_decoder();

Phase *cheonjiin_from_tenkey_handler();
Phase *cheonjiin_combinator();
Decoder *cheonjiin_decoder();
Phase *alphabet_from_tenkey_handler();
Phase *number_from_tenkey_handler();
Decoder *alphabet_tenkey_decoder();
Decoder *number_tenkey_decoder();


uint32_t ksx5002_label(char key);

#if __cplusplus
}
#endif

#endif /* defined(__hangeul__capi__) */
