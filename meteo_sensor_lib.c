/*
In order to use this file you need to install lib4curl.
When compiling this code, include the curl library with "-lcurl" param.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

//#define URL "https://jsonplaceholder.typicode.com/posts"  // URL del server API
#define URL "api.32connect.net/demetro_staging/streams.php"  // URL del server API
//#define URL "api.32connect.net/demetro_staging/get_sensor_data.php"  // URL del server API

// Funzione di callback per scrivere la risposta
size_t write_callback(void *ptr, size_t size, size_t nmemb, char *data) {
    strcat(data, ptr);
    return size * nmemb;
}

int connectAPI(const char *_token, const char *_deviceID, const char *_sensorData) {
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    char response[4096] = "";  // Buffer per la risposta

    // Corpo JSON da inviar con i parametri passati come stringhe
    char json_body[1024];
    snprintf(json_body, sizeof(json_body), "{\"token\": \"%s\", \"eventType\": \"NO_ACK_MESSAGE\", \"deviceId\": \"%s\", \"payloads\":{\"sensorData\": [\"%s\"]}}", _token, _deviceID, _sensorData);

    // Inizializza libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        // Impostiamo l'URL di destinazione
        curl_easy_setopt(curl, CURLOPT_URL, URL);

        // Aggiungiamo gli header per il contenuto JSON
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");

        // Impostiamo l'header
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Impostiamo il corpo della richiesta (POST)
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_body);

        // Funzione di callback per gestire la risposta
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

        // Eseguiamo la richiesta
        res = curl_easy_perform(curl);

        // Controlliamo se c'e stato un errore
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("Risposta del server: %s\n", response);  // Stampa la risposta JSON
        }

        // Pulisci gli header
        curl_slist_free_all(headers);

        // Pulisci libcurl
        curl_easy_cleanup(curl);
    }

    // Finalizza libcurl
    curl_global_cleanup();

    return 0;
}

int main() {

printf("\nOra chiamo la funzione del curl\n");
connectAPI("HIDDEN-FOR-PRIVACY", "HIDDEN-FOR-PRIVACY", "HIDDEN-FOR-PRIVACY");
return 0;
}
