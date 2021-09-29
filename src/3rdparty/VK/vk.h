#pragma once

#include <string>

#include "api.h"

#include "Friends.h"
#include "Groups.h"
#include "Messages.h"
#include "Photos.h"
#include "Video.h"
#include "Users.h"

class VkApi {
private:
	Api api{};
public:
	long userId{ 0 };

	Friends friends{ api.copy() };
	Groups groups{ api.copy() };
	Messages messages{ api.copy() };
	Photos photos{ api.copy() };
	Video video{ api.copy() };
	Users users{ api.copy() };

	VkApi() {
		this->userId = 0;
	}

	VkApi(long userId) {
		this->userId = userId;
	}

	void Authorize(std::string token) noexcept {
		api.Init("https://api.vk.com", "/method/", { {"v", "5.103"}, {"access_token", token} });
	}
};