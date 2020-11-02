#pragma once

#include <string>

#include "Api/api.h"

class Messages {
private:
	Api* api;
public:
	struct sendParams {
		long UserId{};
		long RandomId{};
		long PeerId{};
		std::string Domain{};
		int ChatId{};
		std::string UsersIds{};
		std::string Message{};
		double Lat{};
		double Long{};
		std::string Attachment{};
		long ReplyTo{};
		std::string ForwardMessages{};
		int StickerId{};
		long GroupId{};
		std::string Keyboard{};
		std::string Payload{};
		bool DontParseLinks{};
		bool DisableMentions{};
		std::string Intent{};
	};

	struct deleteParams {
		std::string messagesIds;
		bool spam;
		int groupId;
		bool deleteForAll;
	};

	struct editParams {
		long PeerId{};
		std::string Message{};
		long MessageId{};
		double Lat{};
		double Long{};
		std::string Attachment{};
		bool KeepForwards{};
		bool KeepSnippets{};
		long GroupId{};
		bool DontParseLinks{};
	};

	struct getConversationByIdParams {
		std::string PeerIds{};
		bool Extended{};
		std::string Fields{};
		long GroupId{};
	};

	struct getLongPollServerParams {
		bool NeedPts{};
		long GroupId{};
		int Version{};
	};

	struct getLongPollHistoryParams {
		int Ts{};
		int Pts{};
		int PreviewLength{};
		bool Onlines{};
		std::string Fields{};
		int EventsLimit{};
		int MessagesLimit{};
		long MaxMessageId{};
		int GroupId{};
		int Version{};
		int LastN{};
		bool Credentials{};
	};

	Messages(Api api) {
		this->api = &api;
	}

	Json::Value send(sendParams params) noexcept;
	Json::Value deleteMessages(deleteParams params) noexcept;
	Json::Value edit(editParams params) noexcept;
	Json::Value getConversationById(getConversationByIdParams params) noexcept;
	Json::Value getLongPollServer(getLongPollServerParams params) noexcept;
	Json::Value getLongPollHistory(getLongPollHistoryParams params) noexcept;
};