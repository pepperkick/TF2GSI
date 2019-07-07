
#pragma once

#include <unordered_map>
#include <string>
#include <vector>

class ClientClass;
class IClientEntity;
class RecvProp;
class RecvTable;
class C_BaseCombatWeapon;

class Entities {
public:
	static bool RetrieveClassPropOffset(std::string className, std::vector<std::string> propertyTree);
	static int GetClassPropOffset(std::string className, std::vector<std::string> propertyTree);
	template <typename T> static T GetEntityProp(IClientEntity *entity, std::vector<std::string> propertyTree) {
		return reinterpret_cast<T>(GetEntityProp(entity, propertyTree));
	};

	static const char* Entities::GetEntityClassname(IClientEntity* entity);
	static bool CheckEntityBaseclass(IClientEntity* entity, std::string baseclass);

	static ClientClass* Entities::GetClientClass(const char* className);

	static RecvProp* Entities::FindRecvProp(const char* className, const char* propName, bool recursive);
	static RecvProp* Entities::FindRecvProp(RecvTable* table, const char* propName, bool recursive);

private:
	static bool GetSubProp(RecvTable *table, const char *propName, RecvProp *&prop, int &offset);
	static void* GetEntityProp(IClientEntity* entity, std::vector<std::string> propertyTree);

	static bool CheckClassBaseclass(ClientClass *clientClass, std::string baseclass);
	static bool CheckTableBaseclass(RecvTable *sTable, std::string baseclass);

	static std::unordered_map<std::string, std::unordered_map<std::string, int>> classPropOffsets;
};