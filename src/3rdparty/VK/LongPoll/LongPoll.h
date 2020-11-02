#pragma once

#include "../VK/vk.h"

typedef void (*NewUserEventsCallback)(VkApi*, Json::Value);
typedef void (*NewBotEventsCallback)(VkApi*, Json::Value);

class LongPoll {
	VkApi* api;
public:

	static const int getAttachmets = (1 << 1);
	static const int getAllEvents = (1 << 3);
	static const int getPts = (1 << 5);
	static const int getExtra = (1 << 6);
	static const int getRandomid = (1 << 7);


	struct LongPollSettings {
	public:
		int time;
		bool isGroup;
		long groupId;
		int mode;
		bool work;
		NewUserEventsCallback userCallback;
		NewBotEventsCallback botCallback;
	};

	LongPoll(VkApi* api) {
		this->api = api;
	}

	void startLongPolling(LongPollSettings& settings) noexcept;
	void stopLongPolling() noexcept;

};