#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <string.h>
#include <curl/curl.h>

size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s)
{
    size_t newLength = size * nmemb;
    try
    {
        s->append((char *)contents, newLength);
    }
    catch (std::bad_alloc &e)
    {
        //handle memory problem
        return 0;
    }
    return newLength;
}

std::string getData()
{
    // prints hello world
    CURL *curl;
    CURLcode res;

    /* get a curl handle */
    curl = curl_easy_init();

    if (curl)
    {
        std::string s;
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }

        std::cout << "Program finished!" << std::endl;
        /* always cleanup */
        curl_easy_cleanup(curl);
	return s;
    }

    return "";
}

int postData()
{
    // prints hello world
    CURL *curl;
    CURLcode res;
    char *postthis = "badPoint";

    /* get a curl handle */
    curl = curl_easy_init();

    if (curl)
    {
        std::string s;
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postthis);

        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }

        std::cout << "Program finished!" << std::endl;
        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    return 0;
}

void write()
{
    std::string data = getData();
    std::cout << "data:" << data << '\n';
    const char* dataChars = data.data();
    std::cout << "data:" << dataChars << '\n';
}

void read(int* armStatusPtr)
{
    long status = 2;
    switch (status) {
	case 1:
	    write();
            break;
	case 2:
	    postData();
	    break;
    }
}

int main()
{
    std::cout << "test" << std::endl;
    int b = 3;
    int* a = &b;
    read(a);
}

