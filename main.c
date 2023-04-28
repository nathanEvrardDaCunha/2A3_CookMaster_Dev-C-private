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

void print_json_as_table(json_t *root)
{
    if (!json_is_object(root)) {
        printf("La réponse JSON n'est pas un objet.\n");
        return;
    }

    printf("|------------------------|------------------------|\n");
    printf("|          Clé           |         Valeur         |\n");
    printf("|------------------------|------------------------|\n");

    json_t *value;
    const char *key;
    json_object_foreach(root, key, value) {
        printf("| %22s | ", key);

        if (json_is_string(value)) {
            printf("%22s |\n", json_string_value(value));
        }
        else if (json_is_integer(value)) {
            printf("%22lld |\n", (long long)json_integer_value(value));
        }
        else if (json_is_real(value)) {
            printf("%22f |\n", json_real_value(value));
        }
        else if (json_is_true(value)) {
            printf("%22s |\n", "true");
        }
        else if (json_is_false(value)) {
            printf("%22s |\n", "false");
        }
        else if (json_is_null(value)) {
            printf("%22s |\n", "null");
        }
        else if (json_is_object(value)) {
            printf("%22s |\n", "[object]");
        }
        else if (json_is_array(value)) {
            printf("%22s |\n", "[array]");
        }
    }

    printf("|------------------------|------------------------|\n");
}

json_t *get_value_from_json(json_t *root, const char *key)
{
    if (!json_is_object(root)) {
        fprintf(stderr, "Le JSON fourni n'est pas un objet.\n");
        return NULL;
    }

    json_t *value = NULL;
    const char *obj_key;
    json_object_foreach(root, obj_key, value) {
        if (strcmp(key, obj_key) == 0) {
            return value;
        }
        else if (json_is_object(value)) {
            json_t *inner_value = get_value_from_json(value, key);
            if (inner_value != NULL) {
                return inner_value;
            }
        }
        else if (json_is_array(value)) {
            size_t arr_size = json_array_size(value);
            for (size_t i = 0; i < arr_size; i++) {
                json_t *inner_value = get_value_from_json(json_array_get(value, i), key);
                if (inner_value != NULL) {
                    return inner_value;
                }
            }
        }
    }

    return NULL;
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

                json_t *temp_value = get_value_from_json(root, "temp");
                if (temp_value && json_is_number(temp_value)) {
                    double temp = json_number_value(temp_value);
                    printf("La température est de %.2f Kelvin.\n", temp);
                }
                else {
                    fprintf(stderr, "La clé 'temp' n'a pas été trouvée ou n'est pas un nombre.\n");
                }


                json_t *description_value = get_value_from_json(root, "description");
                if (description_value && json_is_string(description_value)) {
                    const char *description = json_string_value(description_value);
                    printf("La description météo est : %s.\n", description);
                }
                else {
                    fprintf(stderr, "La clé 'description' n'a pas été trouvée ou n'est pas une chaîne de caractères.\n");
                }


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

