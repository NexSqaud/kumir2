#pragma once

#include <string>

#include "Api/api.h"

class Friends {
private:
	Api* api;
public:
	struct addParams {
		long UserId{};
		std::string Text{};
		bool Follow{};
	};

	struct deleteParams {
		long UserId{};
	};

	struct getRequestsParams {
		int Offset{};
		int Count{};
		bool Extended{};
		bool NeedMutual{};
		bool Out{};
		int Sort{};
		bool NeedViewed{};
		bool Suggested{};
		std::string Ref{};
		std::string Fields{};
	};

	Friends(Api api) {
		this->api = &api;
	}

	Json::Value add(addParams params) noexcept;
	Json::Value Delete (deleteParams params) noexcept;
	Json::Value getRequests(getRequestsParams params) noexcept;

};