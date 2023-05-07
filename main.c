#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

// Cette fonction est utilisée par libcurl comme un rappel pour écrire les données reçues
// Les données sont écrites dans responseBuffer, qui est une chaîne de caractères dans ce cas.
size_t writeApiResponseToBuffer(char *data, size_t blockSize, size_t blockCount, void *responseBuffer) {

    // Calculer la taille réelle des données
    size_t receivedDataSize = blockSize * blockCount; 

    char *responseStr = (char *)responseBuffer; 

    // Ajouter les nouvelles données à la fin de la chaîne
    strcat(responseStr, data); 

    return receivedDataSize; 

}


// Cette fonction extrait une valeur spécifique d'un objet JSON en fonction de la clé fournie.
// Elle parcourt récursivement les sous-objets et tableaux pour trouver la clé.
json_t *extractValueFromJsonByKey(json_t *jsonRoot, const char *searchKey)
{
    if (!json_is_object(jsonRoot)) {
        fprintf(stderr, "Le JSON fourni n'est pas un objet.\n");
        return NULL;
    }

    json_t *foundValue = NULL;
    const char *currentKey;

    json_object_foreach(jsonRoot, currentKey, foundValue) {
        if (strcmp(searchKey, currentKey) == 0) {
            return foundValue;
        }
        else if (json_is_object(foundValue)) {
            json_t *nestedValue = extractValueFromJsonByKey(foundValue, searchKey);
            if (nestedValue != NULL) {
                return nestedValue;
            }
        }
        else if (json_is_array(foundValue)) {
            size_t arraySize = json_array_size(foundValue);
            for (size_t i = 0; i < arraySize; i++) {
                json_t *nestedValue = extractValueFromJsonByKey(json_array_get(foundValue, i), searchKey);
                if (nestedValue != NULL) {
                    return nestedValue;
                }
            }
        }
    }

    return NULL;
}


void fetchApiDataAndPrintValues(const char *apiUrl, const char *searchKeys[], size_t numOfKeys) {
    CURL *apiRequest;
    CURLcode requestResult;
    char apiResponse[4096] = { 0 };

    apiRequest = curl_easy_init();
    if (apiRequest) {
        curl_easy_setopt(apiRequest, CURLOPT_URL, apiUrl);
        curl_easy_setopt(apiRequest, CURLOPT_WRITEFUNCTION, writeApiResponseToBuffer);
        curl_easy_setopt(apiRequest, CURLOPT_WRITEDATA, apiResponse);

        requestResult = curl_easy_perform(apiRequest);

        if (requestResult != CURLE_OK) {
            fprintf(stderr, "Erreur lors de la requête CURL: %s\n", curl_easy_strerror(requestResult));
        }
        else {
            json_error_t jsonError;
            json_t *jsonRoot = json_loads(apiResponse, 0, &jsonError);
            if (jsonRoot) {
                for (size_t i = 0; i < numOfKeys; i++) {
                    json_t *value = extractValueFromJsonByKey(jsonRoot, searchKeys[i]);
                    if (value) {
                        if (json_is_number(value)) {
                            double numberValue = json_number_value(value);
                            printf("La valeur de '%s' est %f.\n", searchKeys[i], numberValue);
                        }
                        else if (json_is_string(value)) {
                            const char *stringValue = json_string_value(value);
                            printf("La valeur de '%s' est : %s.\n", searchKeys[i], stringValue);
                        }
                        else {
                            fprintf(stderr, "La clé '%s' n'a pas été trouvée ou n'est pas du bon type.\n", searchKeys[i]);
                        }
                    }
                }
                json_decref(jsonRoot);
            }
            else {
                fprintf(stderr, "Erreur lors de l'analyse de la réponse JSON: %s\n", jsonError.text);
            }
        }
        curl_easy_cleanup(apiRequest);
    }
}


int main(void) {
    printf("\n----------------------------------------------------\n");
    printf("      Bienvenue dans l'outil d'extraction d'API     \n");
    printf("----------------------------------------------------\n\n");
    
    char apiUrl[1024];
    printf("Veuillez entrer l'URL de l'API :\n");
    scanf("%1023s", apiUrl);

    int numOfKeys;
    printf("\nCombien de clés voulez-vous entrer ?\n");
    scanf("%d", &numOfKeys);

    char searchKeys[numOfKeys][256];
    printf("\nVeuillez entrer les clés une par une :\n");
    for (int i = 0; i < numOfKeys; i++) {
        printf("Clé n°%d : ", i + 1);
        scanf("%255s", searchKeys[i]);
    }

    printf("\nTraitement en cours...\n\n");

    const char *keysPtrs[numOfKeys];
    for (int i = 0; i < numOfKeys; i++) {
        keysPtrs[i] = searchKeys[i];
    }

    fetchApiDataAndPrintValues(apiUrl, keysPtrs, numOfKeys);

    printf("\nL'extraction des données est terminée. Merci d'avoir utilisé l'outil d'extraction d'API.\n\n");

    return 0;
}

