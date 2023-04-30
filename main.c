#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t realsize = size * nmemb;
    char *response_str = (char *)userdata;
    strcat(response_str, ptr);
    return realsize;
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


void request_api_and_extract_keys(const char *url, const char *keys[], size_t keys_size) {
    CURL *curl;
    CURLcode res;
    char response_str[4096] = { 0 };

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_str);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Erreur lors de la requête CURL: %s\n", curl_easy_strerror(res));
        }
        else {
            json_error_t error;
            json_t *root = json_loads(response_str, 0, &error);
            if (root) {
                for (size_t i = 0; i < keys_size; i++) {
                    json_t *value = get_value_from_json(root, keys[i]);
                    if (value) {
                        if (json_is_number(value)) {
                            double number = json_number_value(value);
                            printf("La valeur de '%s' est %.2f.\n", keys[i], number);
                        }
                        else if (json_is_string(value)) {
                            const char *str = json_string_value(value);
                            printf("La valeur de '%s' est : %s.\n", keys[i], str);
                        }
                        else {
                            fprintf(stderr, "La clé '%s' n'a pas été trouvée ou n'est pas du bon type.\n", keys[i]);
                        }
                    }
                }

                json_decref(root);
            }
            else {
                fprintf(stderr, "Erreur lors de l'analyse de la réponse JSON: %s\n", error.text);
            }
        }

        curl_easy_cleanup(curl);
    }
}

int main(void) {
    printf("\n----------------------------------------------------\n");
    printf("           Bienvenue dans votre extracteur d'API     \n");
    printf("----------------------------------------------------\n\n");
    
    char url[1024];
    printf("Veuillez entrer l'URL de l'API :\n");
    scanf("%1023s", url);

    int keys_size;
    printf("\nCombien de clés voulez-vous entrer ?\n");
    scanf("%d", &keys_size);

    char keys[keys_size][256];
    printf("\nVeuillez entrer les clés une par une :\n");
    for (int i = 0; i < keys_size; i++) {
        printf("Clé n°%d : ", i + 1);
        scanf("%255s", keys[i]);
    }

    printf("\nTraitement en cours...\n\n");

    const char *keys_ptrs[keys_size];
    for (int i = 0; i < keys_size; i++) {
        keys_ptrs[i] = keys[i];
    }

    request_api_and_extract_keys(url, keys_ptrs, keys_size);

    printf("\nL'extraction des données est terminée. Merci d'avoir utilisé cet extracteur d'API.\n\n");

    return 0;
}



