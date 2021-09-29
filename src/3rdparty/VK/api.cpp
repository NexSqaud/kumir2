#include "api.h"

#include <curl/curl.h>

std::string apiBaseAddress;
std::string apiRequestAddress;
Request apiBaseRequest;

//Api::Api() noexcept
//{
//
//}

void Api::Init(std::string baseAddress, std::string requestAddress, Request baseRequest) noexcept
{
	apiBaseAddress = baseAddress;
	apiRequestAddress = requestAddress;
	apiBaseRequest = baseRequest;
}

static size_t writer(char* data, size_t size, size_t nmemb, std::string* buffer) {
	if (buffer) {
		buffer->append(data, size * nmemb);
		return size * nmemb;
	}
	return 0;
}

Json::Value Api::SendRequest(std::string method, Request requestBody) noexcept
{
	CURL* curl = curl_easy_init();
	std::stringstream stream;
	std::string request;
	for (int i = 0; i < apiBaseRequest.size(); i++) {
		Parameter param = apiBaseRequest[i];
		request.append(param.first);
		request.append("=");
		request.append(param.second);
		request.append(i != apiBaseRequest.size() - 1 ? "&" : "");
	}
	for (int i = 0; i < requestBody.size(); i++) {
		Parameter param = requestBody[i];
		char* encoded = curl_easy_escape(curl, param.second.c_str(), 0);
		if (encoded) {
			request.append("&");
			request.append(param.first);
			request.append("=");
			request.append(encoded);
			curl_free(encoded);
		}
	}
	std::string url;
	url.append(apiBaseAddress);
	url.append(apiRequestAddress);
	url.append(method);
	url.append("?");
	url.append(request);


	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "CPP API LIB/1.0");

		std::string response;
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		auto res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			fprintf(stderr, "base address: %s\nmethod: %s\nrequest:%s\nurl: %s\ncurl failed: %s\n",
				apiBaseAddress.c_str(),
				method.c_str(),
				request.c_str(),
				url.c_str(),
				curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		curl = NULL;

		Json::Value responseJson;
		stream << response.c_str();
		stream >> responseJson;

		return responseJson;
	}
	std::string error{ "{\"error\":\"curl error\"}" };
	Json::Value errorResp;
	stream << error;
	stream >> errorResp;
	return errorResp;
}

Api Api::copy() noexcept {
	Api copied{};
	copied.Init(apiBaseAddress, apiRequestAddress, apiBaseRequest);
	return copied;
}
