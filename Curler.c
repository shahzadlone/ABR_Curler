// =====================================================[ Curler - to Curl a bunch of ABR(MPEG-DASH and HLS) URLs and Check which ones are UP ]=====================================================
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
# include <curl/curl.h>
# include <libxml/parser.h>
# include <libxml/tree.h>
// ######################
# define URLS_BUFFER 1000  
# define MAX_CURL_TIME 3L // If stuck on a URL while curling then move on to the next URL to curl if these many seconds elapsed.

// Defines to assist in printing in different colors. 
// Note: Important to have %s where you want to initiate the color change
// Example: printf( "%sHello, Shahzad\n", blue_str );
# define normal_str  "\x1B[0m"
# define red_str  "\x1B[31m"
# define green_str  "\x1B[32m"
# define yellow_str  "\x1B[33m"
# define blue_str  "\x1B[34m"
# define mag_str  "\x1B[35m"
# define cyan_str  "\x1B[36m"
# define white_str  "\x1B[37m"


void PrintStats(int worked, int total, int hlsWorked, int dashWorked) {
    printf( "\x1B[36m=========================================\n"); 
    printf( "Done Running the Script on all the URLs:\n"); 
    printf("%d out of %d URLs are up.\n", worked, total);

    // Print HLS STATS
    if ( hlsWorked <= 0 ) {
        printf("NO HLS URL is up.\n");
    }
    else if ( hlsWorked == 1 ) {
        printf("1 HLS URL is up.\n");
    }
    else {
        printf("%d HLS URLs are up.\n", hlsWorked);
    }

    // Print MPD STATS
    if ( dashWorked <= 0 ) {
        printf("NO DASH URL is up.\n");
    }
    else if ( dashWorked == 1 ) {
        printf("1 DASH URL is up.\n");
    }
    else {
        printf("%d DASH URLs are up.\n", dashWorked);
    }   
    
    printf( "=========================================\x1B[0m\n"); 
}

// Return false if not a URL and true otherwise.
bool isUrl(char * lineToCheck) { 
    
    if( ( strncmp( lineToCheck, "www.", 4 ) == 0 ) ||
        ( strncmp( lineToCheck, "http://", 7 ) == 0 )
      ) {
        return (true);
    }

    return(false);
}

// Returns true if this URL is a valid HLS (is UP). Otherwise returns false.
bool CheckIfValidM3U(char * curl_output) {
    
   if (!curl_output) {
        #ifdef PRINTING
        printf("Unable to parse m3u - not enough information after curling \n");
        #endif // PRINTING  
        return( false );
   } 

    // Check If HLS is UP (is valid HLS URL or no).
    else if (strncmp(curl_output, "#EXTM3U", 7) == 0) {
        return( true ); 
    }

    else { // If Not a Valid HLS URL.
        return( false );
    }
	
}

// Returns true if this URL is a valid DASH (is UP). Otherwise returns false.
bool CheckIfValidMPD(xmlDoc *doc) {
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
    int mpdsWorking = 0;
    int m3usWorking = 0;

	char* urlsFileName ;
	urlsFileName = argv[1];

    char line[URLS_BUFFER][URLS_BUFFER];
    FILE * URL_file = NULL; 
    URL_file  = fopen(urlsFileName , "r");

    int lineCountIndex = 0;
    while(fgets(line[lineCountIndex], URLS_BUFFER, URL_file)) {
        // get rid of ending '\n' from fgets
        line[lineCountIndex][strlen(line[lineCountIndex]) - 1] = '\0';
        ++(lineCountIndex);
    }


    int totalUrlCount = 0;
    const int totalLineCount = lineCountIndex;
    // Curl all the URLs and print all non-URLs.
    for(lineCountIndex = 0; lineCountIndex < totalLineCount; ++(lineCountIndex)) {

        char* url = line[lineCountIndex];
        // Check if url is valid.
        if (isUrl(url ) == true) {
            ++(totalUrlCount );
        }
        else { // Not a URL then must be a comment. So just print it.
            printf("\n%s***** Non-URL Comment Encountered in File: %s *****%s\n", yellow_str , line[lineCountIndex], normal_str); 
            continue;
        }	

        CURL *curl;
	    CURLcode res;
	    char* data;
	    xmlDoc *doc = NULL;

    	printf("Reading from %s\n", url);
       
        data = handleUrl(url);
    
        if(data) {

            #ifdef PRINTING
            printf("Checking if Curl is Valid... \n");
            #endif // PRINTING
            
            // Check if HLS is UP.
            bool hlsResult = CheckIfValidM3U(data);

            // XML Needed to Check for MPD Validation.
            doc = xmlReadMemory(data, strlen(data), url, NULL, 0);
            // Check if DASH is UP.
            bool dashResult = CheckIfValidMPD(doc); 

            if ( hlsResult == true ) { 
                printf("%sURL:[ %s ] ~~~~~ M3U(HLS) is UP!\n%s", green_str, url, normal_str); 
                ++(m3usWorking);
                ++(urlsWorking);
            }
            else if ( dashResult == true ) { 
                printf("%sURL:[ %s ] ~~~~~ MPD(DASH) is UP!\n%s", green_str, url, normal_str); 
                ++(mpdsWorking);
                ++(urlsWorking);
            }
            else if ( ( hlsResult == false )  &&  ( dashResult == false ) ) { 
                printf("%sURL:[ %s ] ~~~~~ is DOWN!\n%s", red_str, url, normal_str); 
            }
            else {
                printf("%sRARE CASE HIT TALK TO SHAHZAD(shahzadlone@gmail.com)\n%s", blue_str, normal_str); 
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
    PrintStats(urlsWorking, totalUrlCount, m3usWorking, mpdsWorking);
	
    return 0;
}
