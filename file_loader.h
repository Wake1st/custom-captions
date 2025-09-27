#pragma once

#include <string>
#include <vector>
#include <fstream>
#include "json.hpp"

#include "text_time.h"

// for convenience
using json = nlohmann::json;

class FileLoader {
public:
	static void Save(const char* path, std::vector<TimeText> &points) {
		// store json
		json jsonData = {};
		for (int i = 0; i < points.size(); i++) {
			jsonData[i] = {
				{ "time", points[i].time },
				{ "text", points[i].text }
			};
		}

		// save file
		std::ofstream f(path);
		f << std::setw(4) << jsonData << std::endl;
	}

	static void Load(const char* path, std::vector<TimeText> &points) {
		// load file data
		std::ifstream f(path);
		json data = json::parse(f);

		// clean and build new data
		points.clear();
		for (auto& [key, value] : data.items()) {
			TimeText node = { 0 };
			node.time = static_cast<float>(std::atof(key.c_str()));
			node.text = value.get<std::string>();
			
			points.push_back(node);
		}
	}
};

