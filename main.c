#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

// La fonction de rappel pour écrire les données de réponse
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t realsize = size * nmemb;
    char *response_str = (char *)userdata;
    strcat(response_str, ptr);
    return realsize;
}

// Fonction pour extraire la température à partir de la clé "temp" dans un objet JSON
double get_temperature_from_json(json_t *root)
{
    double temp = 0.0;

    json_t *main_obj = json_object_get(root, "main");
    if (main_obj && json_is_object(main_obj)) {
        json_t *temp_obj = json_object_get(main_obj, "temp");
        if (temp_obj && json_is_number(temp_obj)) {
            temp = json_number_value(temp_obj);
        }
    }

    return temp;
}

// Fonction pour extraire la description météo à partir d'un index donné dans un tableau JSON
const char* get_weather_description_from_json(json_t *root, size_t index)
{
    const char *description = NULL;

    json_t *weather_arr = json_object_get(root, "weather");
    if (weather_arr && json_is_array(weather_arr)) {
        json_t *weather_obj = json_array_get(weather_arr, index);
        if (weather_obj && json_is_object(weather_obj)) {
            json_t *description_obj = json_object_get(weather_obj, "description");
            if (description_obj && json_is_string(description_obj)) {
                description = json_string_value(description_obj);
            }
        }
    }

    return description;
}

void print_json_as_table(json_t *root)
{
    if (json_is_object(root)) {
        // Afficher les clés et valeurs de l'objet JSON
        printf("| %-30s | %-30s |\n", "Clé", "Valeur");
        printf("+--------------------------------+--------------------------------+\n");
        const char *key;
        json_t *value;
        json_object_foreach(root, key, value) {
            printf("| %-30s | ", key);
            print_json_as_table(value);
        }
    }
    else if (json_is_array(root)) {
        // Afficher les éléments du tableau JSON
        printf("[");
        for (size_t i = 0; i < json_array_size(root); i++) {
            if (i > 0) {
                printf(", ");
            }
            print_json_as_table(json_array_get(root, i));
        }
        printf("]");
    }
    else if (json_is_string(root)) {
        // Afficher la chaîne de caractères JSON
        printf("%s", json_string_value(root));
    }
    else if (json_is_integer(root)) {
        // Afficher l'entier JSON
        printf("%lld", (long long)json_integer_value(root));
    }
    else if (json_is_real(root)) {
        // Afficher le nombre JSON
        printf("%.2f", json_real_value(root));
    }
    else if (json_is_true(root)) {
        // Afficher le booléen JSON "true"
        printf("true");
    }
    else if (json_is_false(root)) {
        // Afficher le booléen JSON "false"
        printf("false");
    }
    else if (json_is_null(root)) {
        // Afficher la valeur nulle JSON
        printf("null");
    }
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
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openweathermap.org/data/2.5/weather?lat=47.34&lon=10.99&appid=a55d1a3187441c503ba1f6f5a914b0be");

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
            // Analyser la réponse JSON
            json_error_t error;
            json_t *root = json_loads(response_str, 0, &error);
            if (root) {
                // Afficher la réponse JSON sous forme de tableau
                printf("La réponse JSON est la suivante :\n");
                print_json_as_table(root);

                // Extraire la température
                double temp = get_temperature_from_json(root);
                printf("La température est de %.2f Kelvin.\n", temp);

                // Extraire la description météo
                const char *description = get_weather_description_from_json(root, 0);
                printf("La description météo est : %s.\n", description);

                // Libérer l'objet JSON
                json_decref(root);
            }
            else {
                fprintf(stderr, "Erreur lors de l'analyse de la réponse JSON: %s\n", error.text);
            }
        }

        // Libérer CURL
        curl_easy_cleanup(curl);
    }

    return 0;
}

