#ifndef QuestionHandler_H
#define QuestionHandler_H

#include <iostream>
#include <string>
#include <sstream>

#include "PlayerDataHolder.h"

class QuestionHandler {
	QuestionHandler() {}

	static int getIntegerInputSafely(std::string question) {
		std::string input;
		int result;

		while (true) {
			std::cout << question << std::endl;
			std::getline(std::cin, input);

			std::stringstream stream(input);
			if (stream >> result)
				break;
			std::cout << "Incorrect input, please try again" << std::endl;
		}

		return result;
	}

	static bool equalsChar(unsigned char c1, unsigned char c2) {
		return std::tolower(c1) == std::tolower(c2);
	}

	static bool equals(std::string s1, std::string s2) {
		if (s1.length() == s2.length()) {
			return std::equal(s2.begin(), s2.end(), s1.begin(), s1.end(), equalsChar);
		}
	}

	static bool getBooleanInputSafely(std::string question) {
		std::string input;
		
		while (true) {
			std::cout << question << std::endl;
			std::getline(std::cin, input);

			if (equals(input, "yes")) {
				return true;
			}
			if (equals(input, "no")) {
				return false;
			}
		}
	}
public:
	static void getPlayerInfo() {
		std::cout << "Information about You:" << std::endl;
		
		std::cout << "What is your name?" << std::endl;
		std::getline(std::cin, PlayerDataHolder::Name);

		std::string gender;
		std::cout << "What is your gender?" << std::endl;
		std::getline(std::cin, PlayerDataHolder::Gender);

		PlayerDataHolder::Age = getIntegerInputSafely("How old are you?");
		PlayerDataHolder::PlaysRegularly = getBooleanInputSafely("Do you play games regularly?");
		PlayerDataHolder::IsEsportAthlete = getBooleanInputSafely("Are you an esport athlete?");
	}
};

#endif