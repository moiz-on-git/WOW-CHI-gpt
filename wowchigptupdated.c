/* 
%%%%%%%#####*****+++++++++*#%%%%%%%%%%%%%%%%%%%****************************========+
@@@@@@@@@@@@@@@@@@@%%##%%%%%%%%%%%%%%%%%%%%%%%%#**##################*******===++===+
@@%%%@%%%%@%%%%###%%@%%%%%%%%%%%%%%%%@@@%%%%%%%%%########################**========+
%%%%%%%###%%%%%%%@@@%%%%@@@@@%%%%%%%%%%@@%%%%%%%%%%#######################+=======+*
*#%%%%%%%%%%#%@@@@@%%%%%%@@@@@@@@%@@%%%%@@%%%@@%%%%%######################+=======+*
%%%%%%@@@%%%@@@%%%%%%%%@@@@%%%%@@%%%%@%@@@%%%@%%%%%%%#####################+=======+*
%%%%@@@%%%@@@@%%%%%%%@@@@%%%%@@@@%%@@@@@@@@%%@@@@%%%%#####################+=======+*
%%%%%@%#@@@@%%%%%%%%%@@@@@@@%%@@@@@@@%%%%%%%%%%%%%%%######################========++
@@@@%#%@@@@@@@@@@%%%%@@@@@@@@@@@@%%#*******%%%%%@%%%#####################*========++
@@%%%@@%%%%%@@@@@%%%%@@@@@@@@@%%%##***#%%%##%%@%%%%%#####################*======++++
%%@@@%%%%@%%%%%@@%%%%%@@@@@@%%@@%%%#*#%#%##**#%%*#%%%%%%#################+++++++++++
@@@%%%%%%%@%%%%%%%%%%%%%@@@@%%%%##%#******+++*##*#%%%%%%%################+++++++++++
%%%%%%%%%%%%%%%%%%%%%%%%@%@@%#######+++**#******+%%%%%%%%%%##############+++++++++++
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%####%%%####**##***##%%%%%%%%%%%############*+++++++++=+
%%%%%%%%%%%%%%%%%%%%%%%%%%%#%%%%%%%#*****###**#%%%%%%%%%%%%%%###########*+++++++++=+
%%%%%%%%%%@@@@%%%%%%%%%%%%%%%%%%%##*++**+**##*##@%%%%%%%%%%%############*+++++++++=+
%%%%%%%%%%@@@@%%%%%%%%%%%%%%%@%%%%#####******#**@%%%%%%%%%%%%###########*+++++++++=+
%%%%%%%%%%%%%@@@%%%%%%%%%%%%%%%%%%####****###***%%*#%%%%%%%%%###########*+++++++++=*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@%%%%%%%%##****%%#*#%%%%%%%%###########+++++++++++*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@@%%###****#%%#***%%%%%%############+++++++++++*
%%%%%%%%%%%%%%%%%%%%%#%##%%#%#%%%%%%%%%###****#%%#****####%%############+++++++++++*
%%%%%%%%%%%#*+%%%%%%#%%#####%%%%%%%########**####**####%#**#*###########+++++++++++*
%%%%%%%%%%%##*%%#%*++#%##*##%%##%%%%%###########%*########**#*#########*+++++++++++*
%%%%%%%%%%##**%#*+*##+*#%%%@@%**#%%%%############*##########*****######*+++++++++++*
%%%%%%%%%###*****#*++#%###%%%##*#%%%%##########%##%###%###**###*#***###++++++++++++*
####%%#%%#####*#****#%%%%#%####%%###########%%#######%%###########**###++++++++++==+
%%%%%%###%#********#%%@%%%%%%#%%%###########%##%@%%%%%%%###%#**#*****##++++++++++==+
@@@@@@@@@%%###***+#%%%%%##%#%##%#%%######%###%#%%#%#%%%#%%%#*######****++++++++++==+
%%%%%%%%%%%%####*#%@%%%%%#%#######%%######%#%%@%####%%%%%%####%##+##*#**+++++++++==+
########*#%%#####%%@%%%%%###%%#%%%%%%%####%%%%#%##%#%%%%%%%%%#######*******+++++===+
####%%%**%%%#####%%%%#%%%##%%%#%%%#%%%%%%#%####%##%##%%%%%%####%##*##*****#*****===+
####%%%#%#%#####@@@@%#%%%#%%####%%%%%%%%%#%%%%#######%%%%%%@%%%%###*##*####*******++
###%%%%#%*####*+%@@%%#%%%#%%#%#%%%%#%%%%%#%%%%####%%#%%%%%%%%#%%%%%%##**#*###**++++*
##%%%%%%%%#****+#@@%%###%##%%##=*#=#*+#===+#=#*=#+*+*%%%%#%%%%%%%#%%#####*####**+++*
##%%%%%%%%%%%@%%%%%%%#%%%%%%%#**=++++#+*%#=*+*==*=%+*#%%%%%%@@%%%%%%#########*#*****
##%%%%%%%%%%@@%%%%%%%%%#%#%%%%#%==%==##=*+=##=+#-+%*##%%%%%%%%%%%####%#%%#######**##
##%%%%%%%%%%%@@@@@%%##%#%%%%%%%%%%###%#%%#%#######%%%#%%%%%%%@@@%%%%###########****#
###%%%%%%@@@@@@@@@@%%%%#%%%%%%%%%%%%%%%%%%%%#%%#%%%%%##%###%%%%%%%%%%%%#%%##*####***
#%%@@@@@@@@@@@@@@@@%%%%#%%%#%%%%%%%%%%%%%%###%#%%%%%%%####%#%%@%%%%%%%%######*######
%%@@@@@@@@@@@@@@%%%%%%##%%%#%%%%#%#%%%#%%##%#%%%%%%%%%%%#%#%%%%%%%%%%%%%%#%#########
%%@@@@@@@@@%##%%%%%%%%%%%%%#%%%%%%%%%%%%%#%%%%%%#%%%%%%%##%%%%%%%%%%%%%%%##%#%######

*/




#include <stdio.h> //standard header
#include <stdlib.h> //standard library
#include <string.h> //string library
#include <curl/curl.h> // For HTTP requests

// Structure to hold API response
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Callback function for libcurl to write the API response
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realSize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realSize + 1);
    if (ptr == NULL) {
        printf("Not enough memory\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realSize);
    mem->size += realSize;
    mem->memory[mem->size] = 0;

    return realSize;
}

// Function to send the input to Gemini API and get the response
char *send_to_gemini(const char *input_text, const char *api_key) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk = {0};

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize CURL\n");
        return NULL;
    }

    // Prepare JSON payload
    char payload[1024];
    snprintf(payload, sizeof(payload),
             "{\"contents\": [{\"parts\": [{\"text\": \"%s\"}]}]}", input_text);

    // Prepare URL with API key
    char url[512];
    snprintf(url, sizeof(url), "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=%s", api_key);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);  // Use the formatted URL
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    // Disable SSL verification 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "CURL request failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return chunk.memory;
}

// Function to parse the Gemini API response and extract relevant information
char *parse_gemini_response(const char *json_response) {
    // Check for motivational keywords
    if (strstr(json_response, "inspire")|| strstr(json_response, "first") || strstr(json_response, "motivate")  || strstr(json_response, "winning") || strstr(json_response, "excellence") || strstr(json_response, "milestone") || strstr(json_response, "goal") || strstr(json_response, "accomplishment") || strstr(json_response, "triumph") || strstr(json_response, "comeback") || strstr(json_response, "confidence") || strstr(json_response, "strength") || strstr(json_response, "positivity") || strstr(json_response, "optimism") || strstr(json_response, "focus") || strstr(json_response, "gym") || strstr(json_response, "resilience") || strstr(json_response, "motivation") || strstr(json_response, "empower") || strstr(json_response, "resolve") || strstr(json_response, "breakthrough") || strstr(json_response, "endurance") || strstr(json_response, "determination") || strstr(json_response, "overcome")|| strstr(json_response, "success") || strstr(json_response, "gym")|| strstr(json_response, "first") || strstr(json_response, "success") || strstr(json_response, "achievement") || strstr(json_response, "goal") || strstr(json_response, "dream")) {
        return "motivational";
    }

    // Check for boring keywords
    if (strstr(json_response, "laundry") || strstr(json_response, "cleaning") ||  strstr(json_response, "class") || strstr(json_response, "chore") || strstr(json_response, "collage") || strstr(json_response, "homework")  || strstr(json_response, "oneweek")||strstr(json_response, "study") ) {
        return "boring";
    }

    // Check for interesting keywords
    if (strstr(json_response, "game") || strstr(json_response, "food") || strstr(json_response, "pubg") || strstr(json_response, "cricket") || strstr(json_response, "mobile") || strstr(json_response, "computer") || strstr(json_response, "call of duty") || strstr(json_response, "YouTube") || strstr(json_response, "meme") || strstr(json_response, "mam") || strstr(json_response, "computer science")||strstr(json_response,"v") ) {
        return "interesting";
    }

    return "boring"; // Default to boring
}

// Function to play video
void play_video(const char *video_path) {
    char command[512];
    snprintf(command, sizeof(command), "start \"\" \"%s\"", video_path);
    system(command);
}

int main() {
    printf("/$$      /$$  /$$$$$$  /$$      /$$                            /$$$$$$  /$$   /$$ /$$$$$$                      /$$$$$$  /$$$$$$$  /$$$$$$$$\n");
    printf("| $$  /$ | $$ /$$__  $$| $$  /$ | $$                           /$$__  $$| $$  | $$|_  $$_/                     /$$__  $$| $$__  $$|__  $$__/\n");
    printf("| $$ /$$$| $$| $$  \\ $$| $$ /$$$| $$                          | $$  \\__/| $$  | $$  | $$                      | $$  \\__/| $$  \\ $$   | $$   \n");
    printf("| $$/$$ $$ $$| $$  | $$| $$/$$ $$ $$             /$$$$$$      | $$      | $$$$$$$$  | $$         /$$$$$$      | $$ /$$$$| $$$$$$$/   | $$   \n");
    printf("| $$$$_  $$$$| $$  | $$| $$$$_  $$$$            |______/      | $$      | $$__  $$  | $$        |______/      | $$|_  $$| $$____/    | $$   \n");
    printf("| $$$/ \\  $$$| $$  | $$| $$$/ \\  $$$                          | $$    $$| $$  | $$  | $$                      | $$  \\ $$| $$         | $$   \n");
    printf("| $$/   \\  $$|  $$$$$$/| $$/   \\  $$                          |  $$$$$$/| $$  | $$ /$$$$$$                    |  $$$$$$/| $$         | $$   \n");
    printf("|__/     \\__/ \\______/ |__/     \\__/                           \\______/ |__/  |__/|______/                     \\______/ |__/         |__/   \n");
    printf("\n\n");

    char input_text[256];
    char *api_key = "put your api key here"; // API key
    char *response, *result;

    // Get the input text
    printf("Enter a text: ");
    scanf("%255s", input_text);

    // Sending the text to Gemini API
    response = send_to_gemini(input_text, api_key);
    if (!response) {
        fprintf(stderr, "Failed to get response from Gemini API\n");
        return 1;
    }

    // Decide if it's "interesting", "boring", or "motivational"
    result = parse_gemini_response(response);

    // Play the appropriate video
    if (strcmp(result, "boring") == 0) {
        play_video("C:\\chi.mp4.mp4");  
    } else if (strcmp(result, "interesting") == 0) {
        play_video("C:\\wow.mp4.mp4");  
    } else if (strcmp(result, "motivational") == 0) {
        play_video("C:\\system-deepak kalal.mp4");  
    }

    free(response);
    return 0;
}
