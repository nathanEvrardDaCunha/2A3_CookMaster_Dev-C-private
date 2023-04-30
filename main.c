#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

// Cette fonction est utilisée par libcurl comme un rappel pour écrire les données.
// Les données sont écrites dans userdata, qui est une chaîne de caractères dans ce cas.
size_t writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {

    // Calculer la taille réelle des données
    size_t realSize = size * nmemb; 

    char *responseStr = (char *)userdata; 

    // Ajouter les nouvelles données à la fin de la chaîne
    strcat(responseStr, ptr); 

    return realSize; 

}

// Cette fonction prend un objet JSON et une clé, et renvoie la valeur associée à cette clé.
// Si la clé est présente dans un sous-objet ou un tableau, la fonction les parcourt récursivement.
json_t *getValueFromJson(json_t *root, const char *key)
{
    if (!json_is_object(root)) {
        fprintf(stderr, "Le JSON fourni n'est pas un objet.\n");
        return NULL;
    }

    json_t *value = NULL;
    const char *objKey;

    json_object_foreach(root, objKey, value) {
        if (strcmp(key, objKey) == 0) {
            return value;
        }
        else if (json_is_object(value)) {
            json_t *innerValue = getValueFromJson(value, key);
            if (innerValue != NULL) {
                return innerValue;
            }
        }
        else if (json_is_array(value)) {
            size_t arrSize = json_array_size(value);
            for (size_t i = 0; i < arrSize; i++) {
                json_t *innerValue = getValueFromJson(json_array_get(value, i), key);
                if (innerValue != NULL) {
                    return innerValue;
                }
            }
        }
    }

    return NULL;
}

void requestApiAndExtractKeys(const char *url, const char *keys[], size_t keysSize) {
    CURL *curl;
    CURLcode res;
    char responseStr[4096] = { 0 };

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // On définit la fonction de rappel qui sera appelée avec les données de la réponse
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);

        // On définit où les données de la réponse seront écrites
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, responseStr);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "Erreur lors de la requête CURL: %s\n", curl_easy_strerror(res));
        }
        else {

            // Si la requête a réussi, on analyse la réponse JSON
            json_error_t error;
            json_t *root = json_loads(responseStr, 0, &error);
            if (root) {

                // Si l'analyse a réussi, on extrait les clés demandées
                for (size_t i = 0; i < keysSize; i++) {
                    json_t *value = getValueFromJson(root, keys[i]);
                    if (value) {
                        if (json_is_number(value)) {
                            double number = json_number_value(value);
                            printf("La valeur de '%s' est %f.\n", keys[i], number);
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

    int keysSize;
    printf("\nCombien de clés voulez-vous entrer ?\n");
    scanf("%d", &keysSize);

    char keys[keysSize][256];
    printf("\nVeuillez entrer les clés une par une :\n");
    for (int i = 0; i < keysSize; i++) {
        printf("Clé n°%d : ", i + 1);
        scanf("%255s", keys[i]);
    }

    printf("\nTraitement en cours...\n\n");

    const char *keysPtrs[keysSize];
    for (int i = 0; i < keysSize; i++) {
        keysPtrs[i] = keys[i];
    }

    requestApiAndExtractKeys(url, keysPtrs, keysSize);

    printf("\nL'extraction des données est terminée. Merci d'avoir utilisé cet extracteur d'API.\n\n");

    return 0;
}
