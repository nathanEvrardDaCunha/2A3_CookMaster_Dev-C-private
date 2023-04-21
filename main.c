#include <stdio.h>
#include <curl/curl.h>
#include <jansson.h>
#include <string.h>

void print_json_object(json_t *json_obj);

void print_json_array(json_t *json_array, int index);

// fonction de rappel appelée pour chaque morceau de données reçues
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t realsize = size * nmemb;
    char *response_str = (char *)userdata;

    // concatène le morceau de données reçu avec les données précédentes
    strcat(response_str, ptr);

    return realsize;
}

int main(void)
{
    CURL *curl;
    CURLcode res;
    char url[2000];
    char response_str[100000] = ""; // variable pour stocker la réponse de la requête
    json_t *json_obj;

    sprintf(url, "https://api.openweathermap.org/data/2.5/weather?lat=44.34&lon=10.99&appid=a55d1a3187441c503ba1f6f5a914b0be");

    curl = curl_easy_init();
    if(curl) {
        //Configure la session pour se connecter à l'url stocké dans url
        curl_easy_setopt(curl, CURLOPT_URL, url);

        //Configure la session CURL pour suivre les redirections HTTP
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        //Configure la session CURL pour utiliser la fonction de rappel write_callback
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        //Configure la session CURL pour écrire les données reçues dans response_str
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_str);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        curl_easy_cleanup(curl);

        // crée l'objet json à partir de la chaîne de caractères response_str
        json_obj = json_loads(response_str, 0, NULL);

        // vérifie si l'objet json a été créé avec succès
        if(!json_obj)
        {
            fprintf(stderr, "Error parsing json response: %s\n", response_str);
            return 1;
        }

        // affiche l'objet json
        print_json_object(json_obj);

        // libère la mémoire allouée à l'objet json
        json_decref(json_obj);
    }

    return 0;
}

//Fonction récursive pour afficher un objet JSON
void print_json_object(json_t *json_obj) {
    const char *key;
    json_t *value;

    //Pour chaque paire clé-valeur de l'objet JSON
    json_object_foreach(json_obj, key, value) {
        printf("%s: ", key);
        //En fonction du type de valeur, l'afficher
        switch(json_typeof(value)) {
            case JSON_OBJECT:
                printf("{\n");
                print_json_object(value); //Appel récursif pour afficher l'objet JSON
                printf("}\n");
                break;
            case JSON_ARRAY:
                printf("[\n");
                print_json_array(value, 0); //Appel de la fonction pour afficher le tableau JSON
                printf("]\n");
                break;
            case JSON_STRING:
                printf("%s\n", json_string_value(value));
                break;
            case JSON_INTEGER:
                printf("%lld\n", json_integer_value(value));
                break;
            case JSON_REAL:
                printf("%f\n", json_real_value(value));
                break;
            case JSON_TRUE:
                printf("true\n");
                break;
            case JSON_FALSE:
                printf("false\n");
                break;
            case JSON_NULL:
                printf("null\n");
                break;
        }
    }
}

void print_json_array(json_t *json_array, int index) {
    // Vérifie si l'entrée est bien un tableau JSON
    if (!json_is_array(json_array)) {
        printf("Error: input is not a json array\n");
        return;
    }

    // Récupère la taille du tableau
    size_t size = json_array_size(json_array);

    // Vérifie si l'index est dans les limites du tableau
    if (index >= (int)size || index < 0) {
        printf("Error: index %d is out of bounds (array size: %lu)\n", index, size);
        return;
    }

    // Récupère la valeur correspondant à l'index spécifié
    json_t *value = json_array_get(json_array, index);

    // Si la valeur est un tableau, affiche-le
    if (json_is_array(value)) {
        printf("[\n");
        print_json_array(value, 0);
        printf("]\n");
    }
    // Si la valeur est un objet, affiche-le
    else if (json_is_object(value)) {
        print_json_object(value);
    }
    // Sinon, affiche la valeur simple correspondante
    else {
        // Convertit la valeur en chaîne de caractères
        char *value_str = json_dumps(value, JSON_ENCODE_ANY);
        printf("%s\n", value_str);
        // Libère la mémoire allouée par json_dumps
        free(value_str);
    }
}