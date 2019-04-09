#ifndef __ROO_OTHER_HTTP_UTIL_H__
#define __ROO_OTHER_HTTP_UTIL_H__

#include <memory>
#include <vector>
#include <list>
#include <string>
#include <cstring>

#include <curl/curl.h>

// curl wrapper, support keepalived 
#include <other/Log.h>

namespace roo {

class HttpClient {
    
public:
    explicit HttpClient(uint32_t ConnTimeout = 60, uint32_t Timeout = 120) :
        kConnTimeout_(ConnTimeout),
        kTimeout_(Timeout) {
        CurlPrelude();
    }
    ~HttpClient() { }

    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;

    int GetByHttp(const std::string& strUrl){

        if (CurlPrelude())return -1;

        CURL* curl = curl_ptr_->handle();
        curl_easy_setopt(curl, CURLOPT_ENCODING, "UTF-8");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);
        curl_easy_setopt(curl, CURLOPT_POST, 0);
        curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, kConnTimeout_);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, kTimeout_);

        CURLcode res = curl_easy_perform(curl);
        return HandleCurlResult(res);
    }

    int PostByHttp(const std::string& strUrl, const std::string& strData) {

        if (CurlPrelude()) return -1;

        CURL* curl = curl_ptr_->handle();
        curl_easy_setopt(curl, CURLOPT_ENCODING, "UTF-8");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, kConnTimeout_);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, kTimeout_);

        CURLcode res = curl_easy_perform(curl);
        return HandleCurlResult(res);
    }

    // POST WITH headers
    int PostByHttp(const std::string& strUrl, const std::string& strData, const std::list<std::string>& headers) {

        if (CurlPrelude()) return -1;

        CURL* curl = curl_ptr_->handle();
        curl_easy_setopt(curl, CURLOPT_ENCODING, "UTF-8");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, kConnTimeout_);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, kTimeout_);

        struct curl_slist* s_headers = NULL;
        std::list<std::string>::const_iterator it;
        for (it = headers.begin(); it != headers.end(); it++) {
            s_headers = curl_slist_append(s_headers, it->c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, s_headers);

        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(s_headers);

        return HandleCurlResult(res);
    }

    int PostByHttpsVerifyHost(const std::string& strUrl, const std::string& strData, const std::list<std::string>& headers) {

        if (CurlPrelude()) return -1;

        CURL* curl = curl_ptr_->handle();
        curl_easy_setopt(curl, CURLOPT_ENCODING, "UTF-8");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strData.c_str());
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderWriteCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, kConnTimeout_);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, kTimeout_);

        struct curl_slist* s_headers = NULL;
        std::list<std::string>::const_iterator it;
        for (it = headers.begin(); it != headers.end(); it++) {
            s_headers = curl_slist_append(s_headers, it->c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, s_headers);


        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(s_headers);

        return HandleCurlResult(res);
    }

    int PostByHttps(const std::string& strUrl, const std::string& strData, const std::list<std::string>& headers) {

        if (CurlPrelude()) return -1;

        CURL* curl = curl_ptr_->handle();
        curl_easy_setopt(curl, CURLOPT_ENCODING, "UTF-8");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, kConnTimeout_);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, kTimeout_);

        struct curl_slist* s_headers = NULL;
        std::list<std::string>::const_iterator it;
        for (it = headers.begin(); it != headers.end(); it++) {
            s_headers = curl_slist_append(s_headers, it->c_str());
        }

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, s_headers);

        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(s_headers);

        return HandleCurlResult(res);
    }


    std::string GetData() {
        return std::string((char*)&response_data_[0], response_data_.size());
    }

private:

    // static member func, 所以需要使用指针调用其非静态成员
    static size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* data) {

        HttpClient* pHttp = static_cast<HttpClient*>(data);
        size_t pos = pHttp->response_data_.size();
        size_t len = size * nmemb;
        pHttp->response_data_.resize(pos + len);
        char* pRespBuff = pHttp->response_data_.data();
        ::memcpy(pRespBuff + pos, ptr, len);

        return len;
    }

    static size_t HeaderWriteCallback(void* ptr, size_t size, size_t nmemb, void* data) {

        HttpClient* pHttp = static_cast<HttpClient*>(data);
        size_t pos = pHttp->response_header_.size();
        size_t len = size * nmemb;
        pHttp->response_header_.resize(pos + len);
        char* pRespBuff = pHttp->response_header_.data();
        ::memcpy(pRespBuff + pos, ptr, len);

        return len;
    }

// Curl Object define
    class Curl {

    public:
        Curl() {
            curl_ = curl_easy_init();
        }

        ~Curl() {
            if (curl_) {
                curl_easy_cleanup(curl_);
                curl_ = NULL;
            }
        }

        CURL* handle() {
            return curl_;
        }

        Curl(const Curl&) = delete;
        Curl& operator=(const Curl&) = delete;

    private:
        CURL* curl_;
    };

private:

    int CurlPrelude() {

        response_header_.clear();
        response_data_.clear();

        if (!curl_ptr_ || !curl_ptr_->handle()) {
            curl_ptr_.reset(new Curl());
            if (!curl_ptr_ || !curl_ptr_->handle()) {
                log_err("Create Curl failed.");
                return -1;
            }
        }

        return 0;
    }

    int HandleCurlResult(CURLcode res) {

        if (res != CURLE_OK || !curl_ptr_ || !curl_ptr_->handle()) {
            log_err("CURL error with res %d", res);

            // reset ill curl
            curl_ptr_.reset();
            return -1;
        }

        long nStatusCode = 0;
        curl_easy_getinfo(curl_ptr_->handle(), CURLINFO_RESPONSE_CODE, &nStatusCode);

        if (nStatusCode == 200)
            return 0;

        log_err("curl nStatusCode:%ld, res=%d, error:%s", nStatusCode, res, curl_easy_strerror(res));
        return -1;
    }

private:
    std::vector<char> response_header_;
    std::vector<char> response_data_;

    std::unique_ptr<Curl> curl_ptr_;

    uint32_t kConnTimeout_;
    uint32_t kTimeout_;

};

} // roo


#endif // __ROO_OTHER_HTTP_UTIL_H__
