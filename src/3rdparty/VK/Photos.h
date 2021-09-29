#pragma once

#include <string>

#include "Api/api.h"

class Photos {
private:
	Api* api;
public:
	struct getMessagesUploadServerParams {
		long PeerId{};
	};

	struct saveMessagesPhotoParams {
		std::string Photo{};
		int Server{};
		std::string Hash{};
	};

	struct searchParams {
		std::string Query{};
		double Lat{};
		double Long{};
		long StartTime{};
		long EndTime{};
		int Sort{};
		int Offset{};
		int Count{};
		int Radius{};
	};

	Photos(Api api) {
		this->api = &api;
	}

	Json::Value getMessagesUploadServer(getMessagesUploadServerParams params) noexcept;
	Json::Value saveMessagesPhoto(saveMessagesPhotoParams params) noexcept;
	Json::Value search(searchParams params) noexcept;

};