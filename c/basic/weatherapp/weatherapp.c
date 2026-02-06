// Simple CLI weather app using libcurl and cJSON
// Usage: weather <city> [-f]
// Requires environment variable OPENWEATHER_API_KEY

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"

struct memory {
    char *response;
    size_t size;
};

static size_t write_cb(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct memory *mem = (struct memory *)userp;

    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    if(ptr == NULL) return 0; // out of memory

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;

    return realsize;
}

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s <city> [-f]\n", prog);
    fprintf(stderr, "  -f  show temperature in Fahrenheit (default Celsius)\n");
}

int main(int argc, char **argv) {
    if(argc < 2) {
        usage(argv[0]);
        return 1;
    }

    int to_fahrenheit = 0;
    char city[256] = {0};
    for(int i = 1; i < argc; ++i) {
        if(strcmp(argv[i], "-f") == 0) {
            to_fahrenheit = 1;
        } else if(strlen(city) == 0) {
            strncpy(city, argv[i], sizeof(city) - 1);
        } else {
            // append additional words for multi-word cities
            strncat(city, " ", sizeof(city) - strlen(city) - 1);
            strncat(city, argv[i], sizeof(city) - strlen(city) - 1);
        }
    }

    const char *api_key = getenv("OPENWEATHER_API_KEY");
    if(!api_key) {
        fprintf(stderr, "Error: OPENWEATHER_API_KEY not set. Get a key from https://openweathermap.org/ and set it as an environment variable.\n");
        return 2;
    }

    CURL *curl = curl_easy_init();
    if(!curl) {
        fprintf(stderr, "Error: could not init curl\n");
        return 3;
    }

    struct memory chunk = {0};
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "weatherapp/1.0");

    // escape city
    char *esc_city = curl_easy_escape(curl, city, 0);
    if(!esc_city) {
        fprintf(stderr, "Error: failed to URL-encode city name\n");
        curl_easy_cleanup(curl);
        return 4;
    }

    const char *units = to_fahrenheit ? "imperial" : "metric";
    char url[512];
    snprintf(url, sizeof(url), "https://api.openweathermap.org/data/2.5/weather?q=%s&units=%s&appid=%s", esc_city, units, api_key);
    curl_free(esc_city);

    curl_easy_setopt(curl, CURLOPT_URL, url);

    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
        fprintf(stderr, "Error: curl request failed: %s\n", curl_easy_strerror(res));
        free(chunk.response);
        curl_easy_cleanup(curl);
        return 5;
    }

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if(http_code != 200) {
        // Try to show API message if present
        cJSON *err = cJSON_Parse(chunk.response);
        if(err) {
            cJSON *msg = cJSON_GetObjectItemCaseSensitive(err, "message");
            if(cJSON_IsString(msg) && msg->valuestring) {
                fprintf(stderr, "API error: %s (HTTP %ld)\n", msg->valuestring, http_code);
                cJSON_Delete(err);
                free(chunk.response);
                curl_easy_cleanup(curl);
                return 7;
            }
            cJSON_Delete(err);
        }
        fprintf(stderr, "HTTP error: %ld\n", http_code);
        free(chunk.response);
        curl_easy_cleanup(curl);
        return 7;
    }

    // parse JSON
    cJSON *root = cJSON_Parse(chunk.response);
    if(!root) {
        fprintf(stderr, "Error: failed to parse JSON\n");
        free(chunk.response);
        curl_easy_cleanup(curl);
        return 6;
    }

    cJSON *name = cJSON_GetObjectItemCaseSensitive(root, "name");
    cJSON *main = cJSON_GetObjectItemCaseSensitive(root, "main");
    cJSON *weather = cJSON_GetObjectItemCaseSensitive(root, "weather");

    double temp = 0.0;
    int humidity = -1;
    const char *desc = "(no description)";
    char cityname[256] = {0};

    if(cJSON_IsString(name) && (name->valuestring != NULL)) {
        strncpy(cityname, name->valuestring, sizeof(cityname)-1);
    }

    if(cJSON_IsObject(main)) {
        cJSON *t = cJSON_GetObjectItemCaseSensitive(main, "temp");
        if(cJSON_IsNumber(t)) temp = t->valuedouble;
        cJSON *h = cJSON_GetObjectItemCaseSensitive(main, "humidity");
        if(cJSON_IsNumber(h)) humidity = h->valueint;
    }

    if(cJSON_IsArray(weather)) {
        cJSON *w0 = cJSON_GetArrayItem(weather, 0);
        if(cJSON_IsObject(w0)) {
            cJSON *d = cJSON_GetObjectItemCaseSensitive(w0, "description");
            if(cJSON_IsString(d) && (d->valuestring != NULL)) desc = d->valuestring;
        }
    }

    printf("%s\n", cityname[0] ? cityname : city);
    printf("Temperature: %.1f %s\n", temp, to_fahrenheit ? "F" : "C");
    if(humidity >= 0) printf("Humidity: %d%%\n", humidity);
    printf("Conditions: %s\n", desc);

    cJSON_Delete(root);
    free(chunk.response);
    curl_easy_cleanup(curl);
    return 0;
}
