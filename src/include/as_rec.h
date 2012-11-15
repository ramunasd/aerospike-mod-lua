#pragma once

#include "as_val.h"

typedef struct as_rec_s as_rec;
typedef struct as_rec_hooks_s as_rec_hooks;

/**
 * Record Structure
 * Contains a pointer to the source of the record and 
 * hooks that interface with the source.
 *
 * @field source contains record specific data.
 * @field hooks contains the record interface that works with the source.
 */
struct as_rec_s {
    as_val _;
    void * source;
    const as_rec_hooks * hooks;
};

/**
 * Record Interface.
 * Provided functions that interface with the records.
 */
struct as_rec_hooks_s {
    const as_val * (*get)(const as_rec *, const char *);
    const int (*set)(const as_rec *, const char *, const as_val *);
    const int (*free)(as_rec *);
};

/**
 * Create a new record backed by source and supported by hooks.
 *
 * @param source the source backing the record.
 * @param hooks the hooks that support the record.
 */
as_rec * as_rec_create(void *, const as_rec_hooks *);

int as_rec_update(as_rec *, void *, const as_rec_hooks *);

void * as_rec_source(const as_rec *);

/**
 * Get a bin value by name.
 *
 * Proxies to `r->hooks->get(r, name, value)`
 *
 * @param r the record to read the bin value from.
 * @param name the name of the bin.
 * @param a val containing the value in the bin.
 */
const as_val * as_rec_get(const as_rec *, const char *);

/**
 * Set the value of a bin.
 *
 * Proxies to `r->hooks->set(r, name, value)`
 *
 * @param r the record to write the bin value to.
 * @param name the name of the bin.
 * @param value the value of the bin.
 */
const int as_rec_set(const as_rec *, const char *, const as_val *);

/**
 * Free the record.
 * This will free the record object, the source and hooks.
 *
 * Proxies to `r->hooks->free(r)`
 *
 * @param r the record to be freed.
 */
const int as_rec_free(as_rec *);

as_val * as_rec_toval(const as_rec *);

as_rec * as_rec_fromval(const as_val *);
