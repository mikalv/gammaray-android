/*****************************************************************************
 * bson-encoder.h                                                            *
 *                                                                           *
 * This file contains implementations for a C BSON encoder.  It supports     *
 * packing documents together in a single file or stream.  It implements the *
 * BSON (v1.0) standard as defined here: http://bsonspec.org/#/specification *
 *                                                                           *
 *                                                                           *
 *   Authors: Wolfgang Richter <wolf@cs.cmu.edu>                             *
 *                                                                           *
 *                                                                           *
 *   Copyright 2013 Carnegie Mellon University                               *
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
#include "bson.h"
#include "__bson.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define START_SIZE 4096

/* Basic Types
 *
 * byte     1 byte  (8-bits)
 * int32    4 bytes (32-bit signed integer)
 * int64    8 bytes (64-bit signed integer)
 * double   8 bytes (64-bit IEEE 754 floating point)
 *
 */
struct bson_info* bson_init()
{
    struct bson_info* bson_info = (struct bson_info*)
                                  malloc(sizeof(struct bson_info));
    if (bson_info)
    {
        bson_info->buffer = (uint8_t*) malloc(START_SIZE);
        if (bson_info->buffer)
        {
            bson_info->size = START_SIZE;
            bson_info->position = 0;
        }
        else
        {
            return NULL;
        }
    }

    return bson_info;
}

int32_t new_size(int32_t old_size, int32_t needed_size)
{
    while (old_size < needed_size)
    {
        old_size = old_size << 1;
    }
    return old_size;
}

int resize(struct bson_info* bson_info, int32_t needed_size)
{
    if (bson_info->buffer == NULL)
        return EXIT_FAILURE;

    uint8_t* old = bson_info->buffer;
    int32_t old_size = bson_info->size;
    bson_info->size = new_size(bson_info->size, needed_size);
    bson_info->buffer = malloc(bson_info->size);

    memcpy(bson_info->buffer, old, old_size);
    
    if (old)
        free(old);
    
    return EXIT_SUCCESS;
}

int check_size(struct bson_info* bson_info, int32_t added_size)
{
    if (bson_info->position + added_size > bson_info->size)
        return resize(bson_info, bson_info->position + added_size);

    return EXIT_SUCCESS;
}

/* e_name   ::= cstring */
/* cstring  ::= (byte*) "\x00" */
int serialize_cstring(struct bson_info* bson_info, const char* cstr)
{
    if (bson_info == NULL || cstr == NULL)
        return EXIT_FAILURE;

    int32_t cstr_size = strlen(cstr) + 1;

    if (check_size(bson_info, cstr_size))
        return EXIT_FAILURE;

    memcpy(&(bson_info->buffer[bson_info->position]), cstr, cstr_size);
    bson_info->position += cstr_size;

    return EXIT_SUCCESS;
}

/* string   ::= int32 (byte*) "\x00" */
int serialize_string(struct bson_info* bson_info, int32_t* len, uint8_t* str)
{
    if (bson_info == NULL || len == NULL || str == NULL)
        return EXIT_FAILURE;

    int32_t str_size = *len + 1;
    int32_t added_size = str_size + sizeof(int32_t);

    if (check_size(bson_info, added_size))
        return EXIT_FAILURE;
    
    memcpy(&(bson_info->buffer[bson_info->position]), &str_size,
           4);
    bson_info->position += 4;

    memcpy(&(bson_info->buffer[bson_info->position]), str, *len);
    bson_info->position += *len;

    bson_info->buffer[bson_info->position] = 0x00;
    bson_info->position++;

    return EXIT_SUCCESS;
}

int serialize_double(struct bson_info* bson_info, double* dbl)
{
    if (bson_info == NULL || dbl == NULL)
        return EXIT_FAILURE;

    if (check_size(bson_info, 8))
        return EXIT_FAILURE;

    memcpy(&(bson_info->buffer[bson_info->position]), dbl, 8);
    bson_info->position += 8;

    return EXIT_SUCCESS;
}

int serialize_int32(struct bson_info* bson_info, int32_t* int32)
{
    if (bson_info == NULL || int32 == NULL)
        return EXIT_FAILURE;

    if (check_size(bson_info, 4))
        return EXIT_FAILURE;

    memcpy(&(bson_info->buffer[bson_info->position]), int32, 4);
    bson_info->position += 4;

    return EXIT_SUCCESS;
}

int serialize_int64(struct bson_info* bson_info, int64_t* int64)
{
    if (bson_info == NULL || int64 == NULL)
        return EXIT_FAILURE;

    if (check_size(bson_info, 8))
        return EXIT_FAILURE;

    memcpy(&(bson_info->buffer[bson_info->position]), int64, 8);
    bson_info->position += 8;

    return EXIT_SUCCESS;
}

int serialize_objectid(struct bson_info* bson_info, uint8_t objectid[12])
{
    if (bson_info == NULL || objectid == NULL)
        return EXIT_FAILURE;

    if (check_size(bson_info, 12))
        return EXIT_FAILURE;

    memcpy(&(bson_info->buffer[bson_info->position]), objectid, 12);
    bson_info->position += 12;

    return EXIT_SUCCESS;
}

int serialize_boolean(struct bson_info* bson_info, bool* boolean)
{
    if (bson_info == NULL)
        return EXIT_FAILURE;

    if (check_size(bson_info, 1))
        return EXIT_FAILURE;

    bson_info->buffer[bson_info->position] = *boolean ? 0x01 : 0x00;
    bson_info->position++;

    return EXIT_SUCCESS;
}

/* NOTE: bson_info != document */
int serialize_document(struct bson_info* bson_info,
                       struct bson_info* document)
{
    if (bson_info == NULL || document == NULL)
        return EXIT_FAILURE;

    if (check_size(bson_info, document->position))
        return EXIT_FAILURE;
    
    memmove(&(bson_info->buffer[bson_info->position]), document->buffer,
            document->position);
    bson_info->position += document->position;

    return EXIT_SUCCESS;
}

int serialize_binary(struct bson_info* bson_info, int32_t* len,
                     uint8_t subtype, uint8_t* binary)
{
    if (bson_info == NULL || len == NULL || binary == NULL)
        return EXIT_FAILURE;

    int32_t added_size = 4 + 1 + *len;

    if (check_size(bson_info, added_size))
        return EXIT_FAILURE;

    memcpy(&(bson_info->buffer[bson_info->position]), len, 4);
    bson_info->position += 4;

    bson_info->buffer[bson_info->position] = subtype;
    bson_info->position++;

    memcpy(&(bson_info->buffer[bson_info->position]), binary, *len);
    bson_info->position += *len;

    return EXIT_SUCCESS;
}

/*
 * element  ::= "\x01" e_name double
 *         |    "\x02" e_name string            UTF-8 string
 *         |    "\x03" e_name document          Embedded document
 *         |    "\x04" e_name document          Array
 *         |    "\x05" e_name binary
 *         |    "\x06" e_name Undefined         Deprecated
 *         |    "\x07" e_name (byte*12) ObjectId
 *         |    "\x08" e_name "\x00"    Boolean "false"
 *         |    "\x08" e_name "\x01"    Boolean "true"
 *         |    "\x09" e_name int64 UTC datetime
 *         |    "\x0A" e_name Null value
 *         |    "\x0B" e_name cstring cstring   Regular expression
 *         |    "\x0C" e_name string (byte*12)  DBPointer — Deprecated
 *         |    "\x0D" e_name string            JavaScript code
 *         |    "\x0E" e_name string            Symbol
 *         |    "\x0F" e_name code_w_s          JavaScript code w/ scope
 *         |    "\x10" e_name int32             32-bit Integer
 *         |    "\x11" e_name int64             Timestamp
 *         |    "\x12" e_name int64             64-bit integer
 *         |    "\xFF" e_name Min key
 *         |    "\x7F" e_name Max key
 */
int serialize_element(struct bson_info* bson_info, struct bson_kv* value)
{
    check_size(bson_info, 1); /* every element adds 1 byte */

    switch (value->type)
    {
        case BSON_DOUBLE:
            bson_info->buffer[bson_info->position] = BSON_DOUBLE;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            serialize_double(bson_info, (double*) value->data);
            break;

        case BSON_STRING:
            bson_info->buffer[bson_info->position] = BSON_STRING;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            serialize_string(bson_info, &(value->size),
                             ((uint8_t*) value->data));
            break;

        case BSON_EMBEDDED_DOCUMENT:
            bson_info->buffer[bson_info->position] = BSON_EMBEDDED_DOCUMENT;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            serialize_document(bson_info, (struct bson_info *) value->data);
            break;

        case BSON_ARRAY:
            bson_info->buffer[bson_info->position] = BSON_ARRAY;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            serialize_document(bson_info, (struct bson_info *) value->data);
            break;

        case BSON_BINARY:
            bson_info->buffer[bson_info->position] = BSON_BINARY;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            serialize_binary(bson_info, &(value->size),
                                        value->subtype,
                                        (uint8_t*)(value->data));
            break; 

        case BSON_UNDEFINED:
            bson_info->buffer[bson_info->position] = BSON_UNDEFINED;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            break;

        case BSON_OBJECTID:
            bson_info->buffer[bson_info->position] = BSON_OBJECTID;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            serialize_objectid(bson_info, (uint8_t*) value->data);
            break;

        case BSON_BOOLEAN:
            bson_info->buffer[bson_info->position] = BSON_BOOLEAN;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            serialize_boolean(bson_info, (bool*) value->data);
            break;

        case BSON_UTC_DATETIME:
            bson_info->buffer[bson_info->position] = BSON_UTC_DATETIME;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            serialize_int64(bson_info, (int64_t*) value->data);
            break;
        
        case BSON_NULL:
            bson_info->buffer[bson_info->position] = BSON_NULL;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            break;

        case BSON_REGEX: /* assumes value->data is an array of two char*'s */
            bson_info->buffer[bson_info->position] = BSON_REGEX;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            serialize_string(bson_info, (int32_t*) value->data,
                             ((uint8_t*) value->data) + 4);
            serialize_string(bson_info, (int32_t*) ((uint8_t*) value->data +
                                                    4 +
                                                    *((int32_t*) value->data)),
                                        ((uint8_t*) value->data) + 8 + 
                                        *((int32_t*) value->data));
            break;

        case BSON_DBPOINTER: /* assumes value->data is a tuple */
            bson_info->buffer[bson_info->position] = BSON_DBPOINTER;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            serialize_string(bson_info, (int32_t*) value->data,
                                        ((uint8_t*) value->data) + 4);
            serialize_objectid(bson_info,
                               (uint8_t*)(((uint8_t*) value->data) + 4 +
                                         *((int32_t*) value->data)));
            break;

        case BSON_JS:
            bson_info->buffer[bson_info->position] = BSON_JS;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            serialize_string(bson_info, (int32_t*) value->data,
                             ((uint8_t*) value->data) + 4);
            break;

        case BSON_SYMBOL:
            bson_info->buffer[bson_info->position] = BSON_SYMBOL;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            serialize_string(bson_info, (int32_t*) value->data,
                             ((uint8_t*) value->data) + 4);
            break;    

        case BSON_JS_CODE:
            bson_info->buffer[bson_info->position] = BSON_JS_CODE;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            serialize_string(bson_info, (int32_t*) value->data,
                                        ((uint8_t*) value->data) + 4);
            serialize_document(bson_info, (struct bson_info*)
                                          (((uint8_t*) value->data) + 4 +
                                          *((int32_t*) value->data)));
            break;

        case BSON_INT32:
            bson_info->buffer[bson_info->position] = BSON_INT32;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            serialize_int32(bson_info, (int32_t*) value->data);
            break;

        case BSON_TIMESTAMP:
            bson_info->buffer[bson_info->position] = BSON_TIMESTAMP;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            serialize_int64(bson_info, (int64_t*) value->data);
            break;

        case BSON_INT64:
            bson_info->buffer[bson_info->position] = BSON_INT64;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            serialize_int64(bson_info, (int64_t*) value->data);
            break;

        case BSON_MIN:
            bson_info->buffer[bson_info->position] = BSON_MIN;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            break;

        case BSON_MAX:
            bson_info->buffer[bson_info->position] = BSON_MAX;
            bson_info->position++;
            serialize_cstring(bson_info, value->key);
            break;
    }

    return EXIT_SUCCESS;
}

int bson_serialize(struct bson_info* bson_info, struct bson_kv* value)
{
    return serialize_element(bson_info, value);
}

/* in place finalize data */
int bson_finalize(struct bson_info* bson_info)
{
    if (bson_info == NULL)
        return EXIT_FAILURE;

    if (check_size(bson_info, 5))
        return EXIT_FAILURE;

    int32_t total_bytes = bson_info->position + 5;

    memmove(bson_info->buffer + 4, bson_info->buffer, bson_info->position);
    memcpy(bson_info->buffer, &total_bytes, 4);
    bson_info->position += 4;

    bson_info->buffer[bson_info->position] = 0x00;
    bson_info->position++;

    return EXIT_SUCCESS;
}

/* save to a file */
int bson_writef(struct bson_info* bson_info, FILE* file)
{
    size_t to_write = bson_info->position;
    size_t written = 0;
    if (bson_info->buffer)
    {
        while (to_write)
        {
            written = fwrite(&(bson_info->buffer[written]), 1, to_write, file);
            if (written != to_write)
            {
                if (feof(file) || ferror(file))
                {
                    free(bson_info->buffer);
                    return EXIT_FAILURE;
                }
            }
            to_write -= written;
        }
    }

    return EXIT_SUCCESS;
}

void bson_reset(struct bson_info* bson_info)
{
    bson_info->position = 0;
}

void bson_cleanup(struct bson_info* bson_info)
{
    if (bson_info->buffer != NULL)
    {
        free(bson_info->buffer);
        bson_info->buffer = NULL;
    }
    bson_info->position = 0;
    bson_info->size = 0;
    free(bson_info);
}
