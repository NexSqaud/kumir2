#pragma once

#include <string>

#include "../JSON/json.h"

enum class MessageFlags : int {
	Unread = (1 << 0),
	Outbox = (1 << 1),
	Replied = (1 << 2),
	Important = (1 << 3),
	Chat = (1 << 4),
	Friends = (1 << 5),
	Spam = (1 << 6),
	Deleted = (1 << 7),
	Fixed = (1 << 8),
	Media = (1 << 9),
	Hidden = (1 << 16),
	DeleteForAll = (1 << 17),
	NotDelivered = (1 << 18)
};

class LongPollMessage {
public:
	long messageId{};
	int flags{};
	long peerId{};
	long time{};
	std::string text{};
	std::string title{};
	long fromId{};

	LongPollMessage(Json::Value json) {
		messageId = json[1].asInt64();
		flags = json[2].asInt();
		peerId = json[3].asInt64();
		time = json[4].asInt64();
		text = json[5].asString();
		if (json[6].isMember("title"))
			title = json[6]["title"].asString();
		if (json[6].isMember("from"))
			fromId = std::stol(json[6]["from"].asString());
	}
};

struct Attachment {
public:
	std::string type;
	long id = 0;
	long ownerId = 0;
	std::string accessKey;

	Attachment(Json::Value value) {
		if (value.isMember("type")) {
			type = value["type"].asString();
			if (value.isMember(type))
			{
				if (value[type].isMember("id")) id = value[type]["id"].asInt64();
				if (value[type].isMember("owner_id")) ownerId = value[type]["owner_id"].asInt64();
				if (value[type].isMember("access_key"))accessKey = value[type]["access_key"].asString();
			}
		}
	}

};

struct Message {
	long id = 0;
	long date = 0;
	long peerId = 0;
	long fromId = 0;
	const char* text{};
	long randomId = 0;
	const char* ref{};
	const char* refSource{};
	std::vector<Attachment> attachments{};
	bool important = false;
	const char* payload{};
	std::vector<Message> forwardedMessages{};
	Message* replyMessage{};
	long updateTime = 0;

	Message() {

	}

	Message(Json::Value value) {
		if (value.isMember("id")) id = value["id"].asInt64();
		if (value.isMember("date")) date = value["date"].asInt64();
		if (value.isMember("peer_id")) peerId = value["peer_id"].asInt64();
		if (value.isMember("from_id")) fromId = value["from_id"].asInt64();
		if (value.isMember("text")) text = value["text"].asCString();
		if (value.isMember("random_id")) randomId = value["random_id"].asInt64();
		if (value.isMember("ref")) ref = value["ref"].asCString();
		if (value.isMember("ref_source")) refSource = value["ref_source"].asCString();
		if (value.isMember("attachments"))
			for (int i = 0; i < value["attachments"].size(); i++)
				attachments.push_back({ value["attachments"][i] });
		if (value.isMember("important")) important = value["important"].asBool();
		if (value.isMember("payload")) payload = value["payload"].asCString();
		if (value.isMember("fwd_messages"))
			for (int i = 0; i < value["fwd_messages"].size(); i++)
				forwardedMessages.push_back({ value["fwd_messages"][i] });
		if (value.isMember("reply_message")) replyMessage = new Message{ value["reply_message"] };
		if (value.isMember("update_time")) updateTime = value["update_time"].asInt64();
	}

};