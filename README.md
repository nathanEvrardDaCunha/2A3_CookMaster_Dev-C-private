# 2A3_CookMaster_Dev-C

gcc -o mon_programme main.c -lcurl -ljansson

int main(void) {
    const char *url = "https://api.openweathermap.org/data/2.5/weather?lat=47.34&lon=10.99&appid=a55d1a3187441c503ba1f6f5a914b0be";
    const char *keys[] = { "visibility" };
    size_t keys_size = sizeof(keys) / sizeof(keys[0]);
    request_api_and_extract_keys(url, keys, keys_size);

    return 0;
}

