#pragma once

#include <Windows.h>
#include <stddef.h>
#include <string>
#include <vector>

#include "../JSON/json.h"

typedef std::pair<std::string, std::string> Parameter;
typedef std::vector<Parameter> Request;

class Api {
public:
	//Api() noexcept;	
	void Init(std::string, std::string, Request) noexcept;
	Json::Value SendRequest(std::string, Request) noexcept;
	Api copy() noexcept;
};