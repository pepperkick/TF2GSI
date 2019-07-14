
#pragma once

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "Color.h"
#include "dbg.h"
#include "steam/steamclientpublic.h"
#include "strtools.h"

class ConCommand;

#define GAME_PANEL_MODULE "ClientDLL"
#define PLUGIN_VERSION "1.1.0"
#define PRINT_TAG() ConColorMsg(Color(0, 153, 153, 255), "[GSI] ")
#define COLOR_DEBUG Color(255, 255, 255, 255)
#define COLOR_INFO Color(255, 255, 0, 255)
#define COLOR_ERROR Color(255, 0, 0, 255)
#define COLOR_SUCCESS Color(0, 255, 0, 255)

inline void FindAndReplaceInString(std::string &str, const std::string &find, const std::string &replace) {
	if (find.empty())
		return;

	size_t start_pos = 0;

	while ((start_pos = str.find(find, start_pos)) != std::string::npos) {
		str.replace(start_pos, find.length(), replace);
		start_pos += replace.length();
	}
}

inline int ColorRangeRestrict(int color) {
	if (color < 0) return 0;
	else if (color > 255) return 255;
	else return color;
}

inline bool IsInteger(const std::string &s) {
	if (s.empty() || !isdigit(s[0])) return false;

	char *p;
	strtoull(s.c_str(), &p, 10);

	return (*p == 0);
}

inline CSteamID ConvertTextToSteamID(std::string textID) {
	if (IsInteger(textID)) {
		uint64_t steamID = strtoull(textID.c_str(), nullptr, 10);

		return CSteamID(steamID);
	}

	return CSteamID();
}

inline std::string ConvertTreeToString(std::vector<std::string> tree) {
	std::stringstream ss;
	std::string string;

	for (std::string branch : tree) {
		ss << ">";
		ss << branch;
	}

	ss >> string;

	return string;
}

inline void GetPropIndexString(int index, char string[]) {
	V_snprintf(string, sizeof(string), "%03i", index);
}

inline std::string GetVGUITexturePath(std::string normalTexturePath) {
	std::string path = "../";
	path += normalTexturePath;

	return path;
}

template<typename ...Args>
inline void LogDebug(PRINTF_FORMAT_STRING const tchar* pMsg, Args&& ... args) {
#ifdef DEBUG
	PRINT_TAG();
	((void)ConColorMsg(COLOR_DEBUG, pMsg, std::forward<Args>(args)), ...);
#endif
}

inline void LogDebug(PRINTF_FORMAT_STRING const tchar* pMsg) {
#ifdef DEBUG
	PRINT_TAG();
	ConColorMsg(COLOR_DEBUG, pMsg);
#endif
}

template<typename ...Args>
inline void LogInfo(PRINTF_FORMAT_STRING const tchar* pMsg, Args&& ... args) {
	PRINT_TAG();
	((void)ConColorMsg(COLOR_INFO, pMsg, std::forward<Args>(args)), ...);
}

inline void LogInfo(PRINTF_FORMAT_STRING const tchar* pMsg) {
	PRINT_TAG();
	ConColorMsg(COLOR_INFO, pMsg);
}

template<typename ...Args>
inline void LogError(PRINTF_FORMAT_STRING const tchar* pMsg, Args&& ... args) {
	PRINT_TAG();
	((void)ConColorMsg(COLOR_ERROR, pMsg, std::forward<Args>(args)), ...);
}

inline void LogError(PRINTF_FORMAT_STRING const tchar* pMsg) {
	PRINT_TAG();
	ConColorMsg(COLOR_ERROR, pMsg);
}

template<typename ...Args>
inline void LogSuccess(PRINTF_FORMAT_STRING const tchar* pMsg, Args&& ... args) {
	PRINT_TAG();
	((void)ConColorMsg(COLOR_SUCCESS, pMsg, std::forward<Args>(args)), ...);
}

inline void LogSuccess(PRINTF_FORMAT_STRING const tchar* pMsg) {
	PRINT_TAG();
	ConColorMsg(COLOR_SUCCESS, pMsg);
}
