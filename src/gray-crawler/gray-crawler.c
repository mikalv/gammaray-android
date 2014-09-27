/*****************************************************************************
 * gray-crawler.c                                                            *
 *                                                                           *
 * Analyze a raw disk image and produce summary datastructures of            *
 * the partition table, and file system metadata.                            *
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

#include <stdint.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "color.h"
#include "ext4.h"
#include "gray-crawler.h"
#include "mbr.h"
#include "ntfs.h"

#include <sys/socket.h>
#include <arpa/inet.h>

/* supported file system serializers */
struct gray_fs_crawler crawlers[] = {
    GRAY_FS(ext4),
    GRAY_FS(ntfs),
    //GRAY_FS(fat32), TODO
    {NULL, NULL, NULL, NULL} /* guard value */
};

/* utility function */
void cleanup(FILE* disk, FILE* sockfp, struct bitarray* bits)
{
    if (disk)
        fclose(disk);

    if (sockfp)
        fclose(sockfp); 

    if (bits)
        bitarray_destroy(bits);
}

/* main thread of execution */
int main(int argc, char* args[])
{
    FILE *disk = NULL, *sockfp = NULL;
    struct gray_fs_crawler* crawler;
    struct bitarray* bits = NULL;
    struct stat fstats;
    struct disk_mbr mbr;
    struct fs fsdata;
    bool present;
    int i;

    fprintf_blue(stdout, "Raw Disk Crawler -- By: Wolfgang Richter "
                         "<wolf@cs.cmu.edu>\n");

    if (argc < 3)
    {
        fprintf_light_red(stderr, "Usage: %s <raw disk file> "
                                  "<BSON output file>\n", 
                                  args[0]);
        return EXIT_FAILURE;
    }

    fprintf_cyan(stdout, "Analyzing Disk: %s\n\n", args[1]);

    disk = fopen(args[1], "r");

    int sockfd;
    struct sockaddr_in servaddr;

    sockfd=socket(AF_INET,SOCK_STREAM,0);

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=inet_addr(args[2]);
    servaddr.sin_port=htons(32000);

    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    if (sockfd < 0) {
        fprintf_light_red(stderr,
          "Error opening FILE descriptor for address '%s'.\n", args[2]);
        return EXIT_FAILURE;
    }
    sockfp = fdopen(sockfd, "w+");
    if (sockfp == NULL) {
        fprintf_light_red(stderr,
          "Error opening FILE pointer from socket descrittor.\n");
        return EXIT_FAILURE;
    }

    if (disk == NULL)
    {
        fprintf_light_red(stderr, "Error opening raw disk file '%s'. "
                                  "Does it exist?\n", args[1]);
        return EXIT_FAILURE;
    }

    /* pull MBR info */
    if (mbr_parse_mbr(disk, &mbr))
    {
        cleanup(disk, sockfp, bits);
        fprintf_light_red(stderr, "Error reading MBR from disk. Aborting\n");
        return EXIT_FAILURE;
    }

    mbr_print_mbr(mbr);

    if (fstat(fileno(disk), &fstats))
    {
        cleanup(disk, sockfp, bits);
        fprintf_light_red(stderr, "Error getting fstat info on disk image.\n");
        return EXIT_FAILURE;
    }

    bits = bitarray_init(fstats.st_size / 4096);

    if (bits == NULL)
    {
        cleanup(disk, sockfp, bits);
        fprintf_light_red(stderr, "Error allocating bitarray.\n");
        return EXIT_FAILURE;
    }

    if (mbr_serialize_mbr(mbr, bits, sockfp))
    {
        cleanup(disk, sockfp, bits);
        fprintf_light_red(stderr, "Error serializing MBR.\n");
        return EXIT_FAILURE;
    }

    for (i = 0; i < 4; i++) {
        fsdata = (struct fs) {i, 0, NULL, NULL, NULL};

        fsdata.pt_off = mbr_partition_offset(mbr, i);
        fsdata.bits = bits;

        if (fsdata.pt_off > 0)
        {
            crawler = crawlers;
            present = false;

            while (crawler->fs_name && !present) {

                fprintf_white(stdout, "\nProbing for %s... ",
                                      crawler->fs_name);
                
                if (crawler->probe(disk, &fsdata))
                {
                    fprintf_white(stdout, "not found.\n");
                }
                else
                {
                    fprintf_light_white(stdout, "found %s file system!\n",
                                                 crawler->fs_name);

                    present = true;

                    if (mbr_serialize_partition(i, mbr, sockfp))
                    {
                        crawler->cleanup(&fsdata);
                        cleanup(disk, sockfp, bits);
                        fprintf_light_red(stderr, "Error serializing "
                                                  "partition entry.\n");
                        return EXIT_FAILURE;
                    }
                    
                    if (crawler->serialize(disk, &fsdata, sockfp))
                    {
                        crawler->cleanup(&fsdata);
                        cleanup(disk, sockfp, bits);
                        fprintf_light_red(stderr, "Error serializing "
                                                  "file system.\n");
                        return EXIT_FAILURE;
                    }
                }

                crawler->cleanup(&fsdata);
                
                crawler++;
            }
        }
    }

    bitarray_serialize(bits, sockfp);
    fflush(sockfp);
    cleanup(disk, sockfp, bits);
    return EXIT_SUCCESS;
}
