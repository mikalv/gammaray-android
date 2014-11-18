/*****************************************************************************
 * nbd-test.c                                                                *
 *                                                                           *
 * This file contains a NBD <--> Redis test server.                          *
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
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "color.h"
#include "nbd.h"

#define USAGE "%s <export name> <export file> <Redis IP> <Redis port> <Redis DB> " \
"<Export Size> <NBD Bind IP> <NBD Port> <Old Handshake y|n>\n"

struct nbd_handle
{
    int fd;
    uint64_t size;
    uint64_t handle;
    char* export_name;
    char* redis_server;
    uint32_t name_len;
    uint16_t redis_port;
    uint16_t redis_db;
    struct event_base* eb;
    struct evconnlistener* conn;
    struct redisAsyncContext* redis_c;
};

int main(int argc, char* argv[])
{
    struct nbd_handle* handle;
   
    if (argc < 9)
    {
        fprintf_light_red(stderr, USAGE, argv[0]);
        exit(EXIT_FAILURE);
    }

    fprintf_blue(stdout, "nbd-queuer-test program by: Wolfgang Richter "
                         "<wolf@cs.cmu.edu>\n");

    handle = nbd_init_both(argv[1], argv[2], argv[3], atoi(argv[4]), atoi(argv[5]),
                            atoll(argv[6]), argv[7], argv[8],
                            (strncmp(argv[9], "y", 1) == 0) ||
                            (strncmp(argv[9], "Y", 1) == 0));

    /* handle = nbd_init_redis(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]), */
    /*                         atoll(argv[5]), argv[6], argv[7], */
    /*                         (strncmp(argv[8], "y", 1) == 0) || */
    /*                         (strncmp(argv[8], "Y", 1) == 0)); */

    assert(handle != NULL);
    assert(handle->fd != 0);
    assert(strncmp(argv[1], handle->export_name, strlen(argv[1])) == 0);
    assert(handle->eb != NULL);
    assert(handle->conn != NULL);
    assert(handle->size >= 0);

    /* special case allow for example /dev/null to appear as a large file */
    if (handle->size == 0) handle->size = 1024*1024*1024*1024LL;
    
    nbd_run_loop(handle);
    nbd_shutdown(handle);

    fprintf_green(stdout, "-- Shutting down --\n");
    return EXIT_SUCCESS;
}
