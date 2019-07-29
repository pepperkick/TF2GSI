
#include "entities.h"

#include "cdll_int.h"
#include "client_class.h"
#include "icliententity.h"

#include "common.h"
#include "exceptions.h"
#include "ifaces.h"

std::unordered_map<std::string, std::unordered_map<std::string, int>> Entities::classPropOffsets;

void Entities::DumpAllProps(std::string className) {
	ClientClass* cc = Interfaces::pClientDLL->GetAllClasses();

	while (cc) {
		if (className.compare(cc->GetName()) == 0) {
			RecvTable* table = cc->m_pRecvTable;

			DumpAllProps(table);
		}

		cc = cc->m_pNext;
	}
}

void Entities::DumpAllProps(RecvTable* table) {
	for (int i = 0; i < table->GetNumProps(); i++) {
		RecvProp* currentProp = table->GetProp(i);

		std::string name = currentProp->GetName();
		int offset = currentProp->GetOffset();

		if (name.compare("m_flMapResetTime") == 0) {
			int i = 1;
		}

		if (currentProp->GetType() == DPT_DataTable) {
			DumpAllProps(currentProp->GetDataTable());
		}
	}
}

bool Entities::RetrieveClassPropOffset(std::string className, std::vector<std::string> propertyTree) {
	std::string propertyString = ConvertTreeToString(propertyTree);

	if (classPropOffsets[className].find(propertyString) != classPropOffsets[className].end()) {
		return true;
	}

	ClientClass *cc = Interfaces::pClientDLL->GetAllClasses();

	while (cc) {
		if (className.compare(cc->GetName()) == 0) {
			RecvTable *table = cc->m_pRecvTable;

			int offset = 0;
			RecvProp *prop = nullptr;

			if (table) {
				for (std::string propertyName : propertyTree) {
					int subOffset = 0;

					if (prop && prop->GetType() == DPT_DataTable) {
						table = prop->GetDataTable();
					}

					if (!table) {
						return false;
					}

					if (GetSubProp(table, propertyName.c_str(), prop, subOffset)) {
						offset += subOffset;
					}
					else {
						return false;
					}

					table = nullptr;
				}

				classPropOffsets[className][propertyString] = offset;

				return true;
			}
		}
		cc = cc->m_pNext;
	}

	return false;
}

int Entities::GetClassPropOffset(std::string className, std::vector<std::string> propertyTree) {
	if (!RetrieveClassPropOffset(className, propertyTree)) {
		return -1;
	}

	std::string propertyString = ConvertTreeToString(propertyTree);

	return classPropOffsets[className][propertyString];
}

void* Entities::GetEntityValueAtOffset(IClientEntity* entity, int offset) {
	return (void*)((unsigned long)(entity)+(unsigned long)(offset));
}

void *Entities::GetEntityProp(IClientEntity *entity, std::vector<std::string> propertyTree) {
	std::string className = entity->GetClientClass()->GetName();

	if (!RetrieveClassPropOffset(className, propertyTree)) {
		throw invalid_class_prop(className.c_str());
	}

	std::string propertyString = ConvertTreeToString(propertyTree);

	return GetEntityValueAtOffset(entity, classPropOffsets[className][propertyString]);
}	

bool Entities::GetSubProp(RecvTable *table, const char *propName, RecvProp *&prop, int &offset) {
	for (int i = 0; i < table->GetNumProps(); i++) {
		offset = 0;

		RecvProp *currentProp = table->GetProp(i);

		if (strcmp(currentProp->GetName(), propName) == 0) {
			prop = currentProp;
			offset = currentProp->GetOffset();
			return true;
		}

		if (currentProp->GetType() == DPT_DataTable) {
			if (GetSubProp(currentProp->GetDataTable(), propName, prop, offset)) {
				offset += currentProp->GetOffset();
				return true;
			}
		}
	}

	return false;
}

const char* Entities::GetEntityClassname(IClientEntity* entity) {
	ClientClass* clientClass = entity->GetClientClass();
	
	return clientClass->GetName();
}

bool Entities::CheckEntityBaseclass(IClientEntity* entity, std::string baseclass) {
	if (!entity) return false;

	ClientClass* clientClass = entity->GetClientClass();

	if (clientClass) {
		return CheckClassBaseclass(clientClass, baseclass);
	}

	return false;
}

bool Entities::CheckClassBaseclass(ClientClass *clientClass, std::string baseclass) {
	RecvTable *sTable = clientClass->m_pRecvTable;

	if (sTable) {
		return CheckTableBaseclass(sTable, baseclass);
	}

	return false;
}

bool Entities::CheckTableBaseclass(RecvTable *sTable, std::string baseclass) {
	if (std::string(sTable->GetName()).compare("DT_" + baseclass) == 0) {
		return true;
	}

	for (int i = 0; i < sTable->GetNumProps(); i++) {
		RecvProp *sProp = sTable->GetProp(i);

		if (strcmp(sProp->GetName(), "baseclass") != 0) {
			continue;
		}

		RecvTable *sChildTable = sProp->GetDataTable();
		if (sChildTable) {
			return CheckTableBaseclass(sChildTable, baseclass);
		}
	}

	return false;
}

ClientClass* Entities::GetClientClass(const char* className)
{
	ClientClass* cc = Interfaces::GetClientDLL()->GetAllClasses();
	while (cc)
	{
		if (!stricmp(className, cc->GetName()))
			return cc;

		cc = cc->m_pNext;
	}

	return nullptr;
}

RecvProp* Entities::FindRecvProp(const char* className, const char* propName, bool recursive)
{
	auto cc = GetClientClass(className);
	if (!cc)
		return nullptr;

	return FindRecvProp(cc->m_pRecvTable, propName, recursive);
}

RecvProp* Entities::FindRecvProp(RecvTable* table, const char* propName, bool recursive)
{
	for (int i = 0; i < table->m_nProps; i++)
	{
		auto& prop = table->m_pProps[i];
		if (!strcmp(prop.m_pVarName, propName))
			return &prop;
	}

	return nullptr;
}