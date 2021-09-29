#pragma once

#include <string>
#include "../JSON/json.h"

class BotEvent {
public:
	std::string type;
	Json::Value object;
	long groupId{};
	std::string eventId;

	BotEvent(Json::Value value) {
		if (value.isMember("type"))
			type = value["type"].asCString();
		if (value.isMember("object"))
			object = value["object"];
		if (value.isMember("group_id"))
			groupId = value["group_id"].asInt64();
		if (value.isMember("event_id"))
			eventId = value["event_id"].asCString();
	}

};