#pragma once

#include "Api/api.h"

class Groups {
private:
	Api* api;
public:

	struct getLongPollServerParams {
		long groupId;
	};

	Groups(Api api) {
		this->api = &api;
	}

	Json::Value getLongPollServer(getLongPollServerParams) noexcept;

};