#ifndef PlayerDataHolder_H
#define PlayerDataHolder_H

#include <string>

class PlayerDataHolder {
	PlayerDataHolder() {}
public:
	static std::string Name;
	static std::string Gender;
	static int Age;
	static bool PlaysRegularly;
	static bool IsEsportAthlete;
};
#endif