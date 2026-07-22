#include "zipolib/z_error.h"
#include "zipolib/parse_json.h"
#include "zipolib/z_string.h"
#include "parson/parson.h"

#include <iostream>
#include <string>
#include <curl/curl.h>

// Callback function to handle incoming data from the server
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    // Append the received characters to our response string
    static_cast<std::string*>(userp)->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

int main() {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    // Initialize libcurl
    curl = curl_easy_init();
    if(!curl) {
        std::cout << "curl_easy_init failed" << std::endl;
        return 1;
    }
    int iter_max=10000;
    int iter=0;
    z_string s;
    int current_count=0;
    zp_text_parser p;
    U64 us_start= z_time_get_ticks_us();

    while (iter++<iter_max) {


        // GET
        //s.format("http://localhost:8000/pingpong?count=%d",current_count);

        // POST
        s.format("http://localhost:8000/test");
        //s.format("http://localhost:8000/pong");


        //s.format("http://timer3b:8000/pingpong?count=%d",current_count);
        // Set target URL
        curl_easy_setopt(curl, CURLOPT_URL, s.c_str());
        z_string s;
        z_string msg_out;
        z_json_stream js(s);
        js.obj_start();
        js.keyval_int("count",current_count);

        js.obj_end();
        // The JSON data payload
        //const char *json_data = "{\"name\": \"John Doe\", \"age\": 30}";
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, s.c_str());

        // Build the application/json header
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");

        // Pass the headers to curl
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Follow redirects if any (HTTP 3xx)
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Pass our callback function to handle data stream
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

        // Pass the pointer to our string variable where the data will be stored
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Execute the HTTP GET request
        res = curl_easy_perform(curl);

        // Check for request errors
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            break;
        } else {
            // Print out the captured data
            zp_text_parser p;
            z_json_obj obj=p.parseJsonObj(readBuffer.c_str(),readBuffer.length());

            //std::cout << "SUCCESS! Response data:\n" << readBuffer << std::endl;
            I64 count_rx=obj.get_int("count");
            if (count_rx!= (current_count+1)) {
                z_json_stream s(gz_stdout,true);

                obj.print(s) ;

                Z_ERROR_LOG("count does not match %lld != %ld\n", count_rx, current_count+1);
                break;
            }

            current_count=count_rx;
            if (current_count%1000 ==0) {
               // printf("currentcount=%d\n",current_count);
            }
            readBuffer="";

        }

    }
    U64 us_end= z_time_get_ticks_us();
    U64 diff=us_end-us_start;
    double calls_per_sec=iter_max/((double)diff/1000000.0);
    printf("%d iters in %lu us, %lu per call, %lf calls per sec \n",iter_max,diff,diff/iter_max,calls_per_sec);



        // Clean up resources
    curl_easy_cleanup(curl);

    
    return 0;
}
