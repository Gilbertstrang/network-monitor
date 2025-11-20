#include <network-monitor/file-downloader.h>
#include <filesystem>

#include <curl/curl.h>

using NetworkMonitor::DownloadFile;

bool NetworkMonitor::DownloadFile(
    const std::string& fileUrl,
    const std::filesystem::path& destination,
    const std::filesystem::path& caCertFile
) {
    
    CURL* curl = curl_easy_init();
    CURLcode res;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, fileUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_CAINFO, caCertFile.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

        std::FILE* fp {fopen(destination.string().c_str(), "wb")};
        if (fp == nullptr) {
            curl_easy_cleanup(curl);
            return false;
        }

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        fclose(fp);
        return res == CURLE_OK;
    }
    return false;

}

