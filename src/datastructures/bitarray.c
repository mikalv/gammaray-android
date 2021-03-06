/*****************************************************************************
 * bitarray.c                                                                *
 *                                                                           *
 * This file contains implementations for functions implementing an in-memory*
 * bit array.                                                                *
 *                                                                           *
 *                                                                           *
 *   Authors: Wolfgang Richter <wolf@cs.cmu.edu>                             *
 *                                                                           *
 *                                                                           *
 *   Copyright 2013-2014 Carnegie Mellon University                          *
 *                                                                           *
 *   Licensed under the Apache License, Version 2.0 (the "License");         *
 *   you may not use this file except in compliance with the License.        *
 *   You may obtain a copy of the License at                                 *
 *                                                                           *
 *       http://www.apache.org/licenses/LICENSE-2.0                          *
 *                                                                           *
 *   Unless required by applicable law or agreed to in writing, software     *
 *   distributed under the License is distributed on an "AS IS" BASIS,       *
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*
 *   See the License for the specific language governing permissions and     *
 *   limitations under the License.                                          *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "color.h"
#include "bitarray.h"
#include "bson.h"
#include "util.h"

struct bitarray
{
    uint8_t* array;
    uint64_t len;
};

bool bitarray_get_bit(struct bitarray* bits, uint64_t bit)
{
    if (bit < bits->len)
        return bits->array[bit/8] & (1 << (bit & 0x07));
    else
        return false;
}

void bitarray_set_bit(struct bitarray* bits, uint64_t bit)
{
    if (bit < bits->len)
        bits->array[bit/8] |= 1 << (bit & 0x07);
}

void bitarray_unset_bit(struct bitarray* bits, uint64_t bit)
{
    if (bit < bits->len)
        bits->array[bit/8] &= (0xff ^ (1 << (bit & 0x07)));
}

void bitarray_set_all(struct bitarray* bits)
{
    memset(bits->array, 0xff, bits->len / 8);
}

void bitarray_unset_all(struct bitarray* bits)
{
    memset(bits->array, 0x00, bits->len / 8);
}

void bitarray_c_array_dump(struct bitarray* bits)
{
    uint64_t i;
    fprintf(stdout, "uint8 bits[] = { ");
    for (i = 0; i < bits->len / 8; i++)
        fprintf(stdout, ", 0x%"PRIx8, bits->array[i]);
    fprintf(stdout, "} \n");
}

void bitarray_print(struct bitarray* bits)
{
    fprintf_yellow(stdout, "bits->array [pointer]: %p\n", bits->array);
    fprintf_yellow(stdout, "bits->len [bits]: %"PRIu64"\n", bits->len);
    fprintf_light_yellow(stdout, " -- hexdump(bits->array) -- \n");
    hexdump(bits->array, bits->len / 8);
    fprintf_light_yellow(stdout, " -- end hexdump(bits->array) -- \n");
    bitarray_c_array_dump(bits);
}

struct bitarray* bitarray_init(uint64_t len)
{
    struct bitarray* bits = (struct bitarray*) malloc(sizeof(struct bitarray));

    if (bits)
    {
        bits->len = ((uint64_t) 1) << highest_set_bit64(len);
        bits->array = (uint8_t*) malloc((bits->len + 7) / 8);
        bitarray_unset_all(bits);
    }

    return bits;
}

struct bitarray* bitarray_init_data(uint8_t* data, uint64_t len)
{
    struct bitarray* bits = (struct bitarray*) malloc(sizeof(struct bitarray));

    if (bits)
    {
        bits->len = ((uint64_t) 1) << highest_set_bit64(len);
        bits->array = (uint8_t*) malloc((bits->len + 7) / 8);
        memcpy(bits->array, data, len / 8);
    }

    return bits;
}

void bitarray_destroy(struct bitarray* bits)
{
    if (bits)
    {
        if (bits->array)
            free(bits->array);
        bits->array = NULL;
        free(bits);
    }
}

uint64_t bitarray_get_array(struct bitarray* bits, uint8_t** array)
{
    *array = bits->array;
    return bits->len / 8;
}

int bitarray_serialize(struct bitarray* bits, FILE* serializef)
{
    struct bson_info* serialized;
    struct bson_kv value;
    uint8_t* array;
    uint64_t size;
    int ret;

    if (bits->len == 0)
        return EXIT_FAILURE;

    array = bits->array;
    size = bits->len / 8;
    serialized = bson_init();

    value.type = BSON_STRING;
    value.size = strlen("metadata_filter");
    value.key = "type";
    value.data = "metadata_filter";

    bson_serialize(serialized, &value);

    value.type = BSON_BINARY;
    value.size = size;
    value.key = "bitarray";
    value.data = array;

    bson_serialize(serialized, &value);
    bson_finalize(serialized);
    ret = bson_writef(serialized, serializef);
    bson_cleanup(serialized);

    return ret;
}
