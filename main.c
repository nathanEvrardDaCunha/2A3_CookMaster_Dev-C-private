#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

// La fonction de rappel pour écrire les données de réponse
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t realsize = size * nmemb;
    char *response_str = (char *)userdata;
    strcat(response_str, ptr);
    return realsize;
}

int main(void)
{
    CURL *curl;
    CURLcode res;
    char response_str[4096] = { 0 };

    // Initialiser CURL
    curl = curl_easy_init();
    if (curl) {
        // Définir l'URL de la requête
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openweathermap.org/data/2.5/weather?lat=44.34&lon=10.99&appid=a55d1a3187441c503ba1f6f5a914b0be");

        // Définir la fonction de rappel pour écrire les données de réponse
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        // Passer un pointeur vers la chaîne de réponse comme données utilisateur
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_str);

        // Exécuter la requête
        res = curl_easy_perform(curl);

        // Vérifier les erreurs de CURL
        if (res != CURLE_OK) {
            fprintf(stderr, "Erreur lors de la requête CURL: %s\n", curl_easy_strerror(res));
        }
        else {
            // Afficher la réponse de la requête
            printf("Réponse de la requête : %s\n", response_str);
        }

        // Libérer CURL
        curl_easy_cleanup(curl);
    }

    return 0;
}
