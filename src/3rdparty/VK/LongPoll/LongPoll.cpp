#include "LongPoll.h"

#include <thread>
#include <iostream>
#include <curl/curl.h>

bool work;
long ts;
long pts;
int mode;
std::string key;
std::string botTs;
Messages::getLongPollServerParams userParams;
Groups::getLongPollServerParams botParams;

bool checkErrors(Json::Value value) {
	if (value.isMember("response")) {
		return true;
	}
	else {
		std::cout << value["error"].asCString();
		return false;
	}
}

void updateLongPollInfo(VkApi* api) noexcept {
	Json::Value json = api->messages.getLongPollServer(userParams);
	key = json["response"]["key"].asCString();
	ts = json["response"]["key"].asInt64();
}

bool checkLongPollErrors(VkApi* api, Json::Value &value) {
	if (value.isMember("failed")) {
		switch (value["failed"].asInt()) {
		case 1:
			ts = value["ts"].asInt64();
			break;
		case 2:
			key = api->messages.getLongPollServer(userParams)["response"]["key"].asCString();
			break;
		case 3:
			updateLongPollInfo(api);
			break;
		case 4:
			break;
		default:
			break;
		}
		std::cout << "Error while long polling: " << value["failed"].asInt() << "\n";
		return false;
	}
	return true;
}

std::string makeRequestUrl(std::string serverUrl, int timeout, bool user) noexcept {
	Request request;
	request.push_back({ "act", "a_check" });
	request.push_back({ "key", key.c_str() });
	request.push_back({ "ts", user ? std::to_string(ts) : botTs });
	request.push_back({ "wait", std::to_string(timeout) });
	if(user) request.push_back({ "mode",std::to_string(mode) });
	if(user) request.push_back({ "version", "3" });
	std::string requestUrl;
	requestUrl.append(serverUrl);
	requestUrl.append("?");
	for (int i = 0; i < request.size(); i++) {
		auto param = request[i];
		requestUrl.append(param.first);
		requestUrl.append("=");
		requestUrl.append(param.second);
		requestUrl.append(i < request.size() - 1 ? "&" : "");
	}
	return requestUrl;
}

static size_t writer(char* data, size_t size, size_t nmemb, std::string* buffer) {
	if (buffer) {
		buffer->append(data, size * nmemb);
		return size * nmemb;
	}
	return 0;
}

void processUserLongPoll(std::string serverUrl, int timeout, NewUserEventsCallback callback, VkApi* api) noexcept {
	auto curl = curl_easy_init();
	std::string requestUrl = makeRequestUrl(serverUrl, timeout, true);
	std::stringstream stream;
	curl_easy_setopt(curl, CURLOPT_URL, requestUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "CPP VK LIB/1.0");
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout + 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout + 1);
	std::string content;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
	auto res = curl_easy_perform(curl);
	if (res != CURLE_OK)
		fprintf(stderr, "curl failed: %s\n",
			curl_easy_strerror(res));
	Json::Value value;
	stream << content.c_str();
	if (stream.good())
		stream >> value;
	else {
		std::cout << "error empty string";
		return;
	}
	while (checkLongPollErrors(api, value)) {
		auto oldTs = ts;
		ts = value["ts"].asInt64();
		Json::Value updates;
		if (mode & LongPoll::getPts) {
			//std::cout << value << "\n";
			
			Messages::getLongPollHistoryParams historyParams;
			historyParams.Ts = oldTs;
			historyParams.Pts = 0;
			historyParams.PreviewLength = 0;
			historyParams.Onlines = false;
			historyParams.Fields = "";
			historyParams.EventsLimit = 1000;
			historyParams.MessagesLimit = 200;
			historyParams.Version = 3;
			long lastId = 0;
			//for (auto obj : value["updates"])
			//	if (obj[0] == 4) {
			//		lastId = obj[1].asInt64() - 1;
			//		break;
			//	}
			historyParams.MaxMessageId = lastId;
			//std::cout << "Request history with ts = " << oldTs << " pts = " << pts << " maxMessageId = " << lastId << "\n";
			auto history = api->messages.getLongPollHistory(historyParams);
			updates = history["response"]["messages"];
			//std::cout << "Messages:\n" << history["response"]["messages"] << "\n\n";
		}
		else
			updates = value["updates"];
		std::thread t(callback, api, updates);
		t.detach();
		requestUrl = makeRequestUrl(serverUrl, timeout, true);
		content.clear();
		curl_easy_setopt(curl, CURLOPT_URL, requestUrl.c_str());
		curl_easy_perform(curl);
		stream.clear();
		stream << content.c_str();
		if(stream.good())
		stream >> value;
	}
	curl_easy_cleanup(curl);
}

void ProcessBotsLongPoll(std::string serverUrl, int timeout, NewBotEventsCallback callback, VkApi* api) noexcept {
	auto curl = curl_easy_init();
	std::string requestUrl = makeRequestUrl(serverUrl, timeout, false);
	std::stringstream stream;
	curl_easy_setopt(curl, CURLOPT_URL, requestUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "CPP VK LIB/1.0");
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout + 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout + 1);
	std::string content;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
	auto res = curl_easy_perform(curl);
	if (res != CURLE_OK)
		fprintf(stderr, "curl failed: %s\n",
			curl_easy_strerror(res));
	Json::Value value;
	stream << content.c_str();
	if (stream.good())
		stream >> value;
	else {
		std::cout << "error empty string";
		return;
	}
	while (checkLongPollErrors(api, value)) {
		botTs = value["ts"].asCString();
		auto updates = value["updates"];
		std::thread t(callback, api, updates);
		t.detach();
		requestUrl.clear();
		requestUrl = makeRequestUrl(serverUrl, timeout, false);
		content.clear();
		curl_easy_setopt(curl, CURLOPT_URL, requestUrl.c_str());
		curl_easy_perform(curl);
		stream.clear();
		stream << content.c_str();
		if (stream.good())
			stream >> value;
	}
	curl_easy_cleanup(curl);
}

void LongPoll::startLongPolling(LongPollSettings& settings) noexcept {
	work = settings.work;
	if (!settings.isGroup) {
		userParams.GroupId = settings.groupId;
		userParams.NeedPts = (settings.mode & getPts);
		userParams.Version = 3;
		while (work) {
			Json::Value server = api->messages.getLongPollServer(userParams);
			if (server.isMember("error")) {
				std::cout << server["error"] << "\n";
				return;
			}
			mode = settings.mode;
			int wait = settings.time;
			std::string serverUrl;
			serverUrl.append("https://");
			serverUrl.append(server["response"]["server"].asCString());
			key = server["response"]["key"].asCString();
			ts = server["response"]["ts"].asInt64();
			std::cout << "Starting user long poll wathing for " << api->userId << "\n";
			processUserLongPoll(serverUrl, wait, settings.userCallback, api);
		}
	}
	else {
		botParams.groupId = settings.groupId;
		while (work) {
			Json::Value server = api->groups.getLongPollServer(botParams);
			std::string serverUrl;
			serverUrl.append(server["response"]["server"].asCString());
			key = server["response"]["key"].asCString();
			botTs = server["response"]["ts"].asCString();
			std::cout << "Starting bots long poll wathing for " << settings.groupId << "\n";
			ProcessBotsLongPoll(serverUrl, settings.time, settings.botCallback, api);
		}
	}
}

void LongPoll::stopLongPolling() noexcept {
	work = false;
}