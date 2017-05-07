#pragma once

#include <windows.h>
#include <string>
#include <map>

bool IsWindowFocused();

bool IsKeyDown(DWORD key);
bool IsKeyJustUp(DWORD key, bool exclusive = true);

DWORD str2key(std::string humanReadableKey);
std::string key2str(DWORD key);

// uh wtf
// http://stackoverflow.com/questions/2333728/stdmap-default-value
template <template<class, class, class...> class C, typename K, typename V, typename... Args>
V GetWithDef(const C<K, V, Args...>& m, K const& key, const V & defval)
{
	typename C<K, V, Args...>::const_iterator it = m.find(key);
	if (it == m.end())
		return defval;
	return it->second;
}

std::map<std::string, int> createKeyMap();
