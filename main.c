#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"

struct Mem {
	char *memory;
	size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct Mem *mem = (struct Mem *) userp;

	char *ptr = realloc(mem->memory, mem->size + realsize + 1);
	
	if(!ptr) {
		printf("realloc failed\n");
		return 0;
	}

	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}


void ProcessData(char *jsonString) {
	if (jsonString == NULL) return;

	cJSON *json = cJSON_Parse(jsonString);

	//printf("\n%s\n", jsonString);

	if (json == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if(error_ptr != NULL) {
			fprintf(stderr, "error parsing: %s", error_ptr);
		}
		return;
	}


	cJSON *current_weather = cJSON_GetObjectItemCaseSensitive(json, "current_weather");

	if (current_weather == NULL) {
		printf("error: current weather null\n");
	} else {

		cJSON *temperatur = cJSON_GetObjectItemCaseSensitive(current_weather, "temperature");
		cJSON *windspeed = cJSON_GetObjectItemCaseSensitive(current_weather, "windspeed");

		if (cJSON_IsNumber(temperatur) && cJSON_IsNumber(windspeed)) {
			printf("\n------------------------------\n");
			printf("current weather:\n");
			printf("Temp:	%lf °C\n", temperatur->valuedouble);
			printf("Windspeed:	%lf km/h\n", windspeed->valuedouble);
			printf("------------------------------\n");
		} else {
			printf("data present, wrong format\n");
		}

		cJSON_Delete(json);
	}
}



int main() {

	CURL *curl_handle;
	CURLcode res;

	struct Mem chunk;

	chunk.memory = malloc(1);
	chunk.size = 0;

	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();

	if(curl_handle) {
		curl_easy_setopt(curl_handle, CURLOPT_URL, "https://api.open-meteo.com/v1/forecast?latitude=48.8&longitude=11.34&current_weather=true");
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		res = curl_easy_perform(curl_handle);

		if(res != CURLE_OK) {
			fprintf(stderr, "curl failed: %s\n", curl_easy_strerror(res));
		} else {
			if (chunk.size > 0) {
				ProcessData(chunk.memory);
			} else {
				printf("no data recieved\n");
			}
		}
		curl_easy_cleanup(curl_handle);
	}

	free(chunk.memory);

	curl_global_cleanup();

	return 0;
}
