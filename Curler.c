// ================================================================[ Curler - to Curl a bunch of URLs and Check which ones are UP ]=================================================================
//                                                                              @ShahzadLone for help if needed.
// =================================================================================================================================================================================================

// *************************************************************************************************************************************************************************************************
// Compile: gcc Curler.c -Llibs/ -Ithird_party -lxml2 -lcurl -o curler -Wl,-rpath=libs/ && echo $?
// *************************************************************************************************************************************************************************************************

// #####[ INCLUDES ]#####
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <stdbool.h>
// # include <pthread.h>
// # include <inttypes.h>
// # include <stdint.h>
// # include <time.h>
#include "curl/curl.h"
#include "libxml/parser.h"
#include "libxml/tree.h"
// ######################
#define URLS_BUFFER 200  
#define MAX_CURL_TIME 5L // If stuck on a URL while curling for 5 seconds, then move on to the next URL to curl.

// Defines to assist in printing in different colors. 
// Note: Important to have %s where you want to initiate the color change
// Example: printf( "%sHello, Shahzad\n", blue_str );
#define normal_str  "\x1B[0m"
#define red_str  "\x1B[31m"
#define green_str  "\x1B[32m"
#define yellow_str  "\x1B[33m"
#define blue_str  "\x1B[34m"
#define mag_str  "\x1B[35m"
#define cyan_str  "\x1B[36m"
#define white_str  "\x1B[37m"


void PrintStats(int worked, int total) {
    printf( "\x1B[36m=========================================\n"); 
    printf( "Done Running the Script on all the URLs:\n"); 
    printf("%d out of %d MPDs are up.\n", worked, total);
    printf( "=========================================\x1B[0m\n"); 
}

bool CheckCurlValidOrNot(xmlDoc *doc) {
    xmlNodePtr node = NULL;
    node = xmlDocGetRootElement(doc);

    if (!node) {
        #ifdef PRINTING
        printf("Unable to parse mpd - missing root node\n");
        #endif // PRINTING
        return false;
    }

    if (node->type != XML_ELEMENT_NODE || xmlStrcmp(node->name, (const xmlChar *)"MPD")) {
        #ifdef PRINTING
        printf("Unable to parse mpd - wrong root node name[%s] type[%d]\n", node->name, (int)node->type);
        #endif // PRINTING
        return false;
	}
	
	return true;
	
}

struct urlData {
    size_t size;
    char* data;
};

size_t writeUrlData(void *ptr, size_t size, size_t nmemb, struct urlData *data) {
    size_t index = data->size;
    size_t n = (size * nmemb);
    char* tmp;

    data->size += (size * nmemb);

    tmp = realloc(data->data, data->size + 1); /* +1 for '\0' */

    if(tmp) {
        data->data = tmp;
    } else {
        if(data->data) {
            free(data->data);
        }
        fprintf(stderr, "Failed to allocate memory.\n");
        return 0;
    }

    memcpy((data->data + index), ptr, n);
    data->data[data->size] = '\0';

    return size * nmemb;
}

char *handleUrl(char* url) {
    CURL *curl;
	
    struct urlData data;
    data.size = 0;
    data.data = malloc(8 * 1024); /* reasonable size initial buffer */
    if(NULL == data.data) {
        fprintf(stderr, "Failed to allocate memory.\n");
        return NULL;
    }

    data.data[0] = '\0';

    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeUrlData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        
        // complete within these many seconds
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, MAX_CURL_TIME);
       
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n",  
                        curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);

    }
    return data.data;
}



int main(int argc, char *argv[]) {
	
    if ( argc != 2 ) {
        printf("USAGE: ./curler [Path-To-File-With-The-URLS]\n");
        exit(EXIT_FAILURE);
    }

    int urlsWorking = 0;

	char* urlsFileName ;
	urlsFileName = argv[1];

    char line[URLS_BUFFER][URLS_BUFFER];
    FILE * URL_file = NULL; 
    URL_file  = fopen(urlsFileName , "r");

    int urlCountIndex = 0;
    while(fgets(line[urlCountIndex], URLS_BUFFER, URL_file)) {
        // get rid of ending '\n' from fgets
        line[urlCountIndex][strlen(line[urlCountIndex]) - 1] = '\0';
        ++(urlCountIndex);
    }

    const int totalUrlCount = urlCountIndex;

    // Curl all the URLs
    for(urlCountIndex = 0; urlCountIndex < totalUrlCount; ++(urlCountIndex)) {
	
        CURL *curl;
	    CURLcode res;
	    char* data;
	    xmlDoc *doc = NULL;

        char* url = line[urlCountIndex];

    	printf("Reading from %s\n", url);
       
        data = handleUrl(url);
    
        if(data) {

            #ifdef PRINTING
            printf("%s\n", data);
            printf("Reading XML ... \n");
            #endif // PRINTING

            doc = xmlReadMemory(data, strlen(data), url, NULL, 0);

            #ifdef PRINTING
            printf("Reading XML ... Done\n");
            #endif // PRINTING

            bool result = CheckCurlValidOrNot(doc); 

            #ifdef PRINTING
            printf("Checking if Curl is Valid... \n");
            #endif // PRINTING

            if ( result == true ) { 
                printf("%sURL:[ %s ] , is up!\n%s", green_str, url, normal_str); 
                ++(urlsWorking);
            }
            else if ( result == false ) { 
                printf("%sURL:[ %s ] , is down!\n%s", red_str, url, normal_str); 
            }

            #ifdef PRINTING
            printf("Checking if Curl is Valid... Done\n");
            #endif // PRINTING
    
            free(data);
        }	
        else {
            printf("%sURL:[ %s ] , has Nothing!\n%s", red_str, url, normal_str); 
        }
    } // End of For-Loop.

    // Print the Stats.
    PrintStats(urlsWorking, totalUrlCount);
	
    return 0;
}
