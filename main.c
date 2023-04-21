#include <stdio.h>
#include <curl/curl.h>
#include <jansson.h>

int main(void)
{
    CURL *curl;
    CURLcode res;
    char url[2000];
    sprintf(url, "https://api.openweathermap.org/data/2.5/weather?lat=44.34&lon=10.99&appid=a55d1a3187441c503ba1f6f5a914b0be");

    curl = curl_easy_init();
    if(curl) {
        //Configure la session pour se connecter à l'url stocké dans url
        curl_easy_setopt(curl, CURLOPT_URL, url); 

        //Configure la session CURL pour suivre les redirections HTTP
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        //Configure la session CURL pour utiliser la session fwrite pour écrire les données reçues
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);

        //Configure la session CURL pour écrire les données reçues sur la sortie standard
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, stdout);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
    }

    return 0;
}