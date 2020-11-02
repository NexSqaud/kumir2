#pragma once

#include <string>

#include "Api/api.h"

class Users {
private:
	Api* api;
public:
	struct getParams {
		std::string UserIds{};
		std::string Fields{};
		std::string NameCase{};
	};

	Users(Api api) {
		this->api = &api;
	}

	Json::Value get(getParams params) noexcept;

};