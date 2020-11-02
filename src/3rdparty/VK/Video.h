#pragma once

#include <string>

#include "Api/api.h"

class Video {
private:
	Api* api;
public:
	struct searchParams {
		std::string Query{};
		int Sort{};
		bool HD{};
		bool Adult{};
		std::string Filters{};
		bool SearchOwn{};
		int Offset{};
		int Longer{};
		int Shorter{};
		int Count{};
		bool Extended{};
	};

	Video(Api api) {
		this->api = &api;
	}

	Json::Value search(searchParams params) noexcept;

};