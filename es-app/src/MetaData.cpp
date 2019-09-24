#include "MetaData.h"

#include "utils/FileSystemUtil.h"
#include "Log.h"
#include <pugixml/src/pugixml.hpp>

MetaDataDecl gameDecls[] = {
	// key,         type,                   default,            statistic,  name in GuiMetaDataEd,  prompt in GuiMetaDataEd
	{"name",        MD_STRING,              "",                 false,      "NOME",               "INSIRA O NOME DO JOGO"},
	{"sortname",    MD_STRING,              "",                 false,      "NOME DE CLASSIFICAÇÃO",   "INSIRA O NOME DE CLASSIFICAÇÃO DO JOGO"},
	{"desc",        MD_MULTILINE_STRING,    "",                 false,      "DESCRIÇÃO",          "INSIRA A DESCRIÇÃO"},
	{"image",       MD_PATH,                "",                 false,      "IMAGEM",             "INSIRA O CAMINHO DA IMAGEM"},
	{"video",       MD_PATH     ,           "",                 false,      "VÍDEO",              "INSIRA O CAMINHO DO VÍDEO"},
	{"marquee",     MD_PATH,                "",                 false,      "LETREIRO",           "INSIRA O CAMINHO DO LETREIRO"},
	{"thumbnail",   MD_PATH,                "",                 false,      "MINIATURA",          "INSIRA O CAMINHO DA MINIATURA"},
	{"rating",      MD_RATING,              "0.000000",         false,      "AVALIAÇÃO",          "INSIRA A AVALIAÇÃO"},
	{"releasedate", MD_DATE,                "not-a-date-time",  false,      "LANÇAMENTO",         "INSIRA A DATA DE LANÇAMENTO"},
	{"developer",   MD_STRING,              "DESCONHECIDO",     false,      "DESENVOLVEDOR",      "INSIRA A DESENVOLVEDOR"},
	{"publisher",   MD_STRING,              "DESCONHECIDO",     false,      "PUBLICADOR",         "INSIRA A PUBLICADOR"},
	{"genre",       MD_STRING,              "DESCONHECIDO",     false,      "GÊNERO",             "INSIRA O GÊNERO"},
	{"players",     MD_INT,                 "1",                false,      "JOGADOR",            "INSIRA O NÚMERO DE JOGADORES"},
	{"favorite",    MD_BOOL,                "false",            false,      "FAVORITO",           "COLOQUE FAVORITO LIG./DESL."},
	{"hidden",      MD_BOOL,                "false",            false,      "OCULTO",             "COLOQUE OCULTO LIG./DESL." },
	{"kidgame",     MD_BOOL,                "false",            false,      "INFANTIL",           "COLOQUE COMO INFANTIL LIG./DESL."},
	{"playcount",   MD_INT,                 "0",                true,       "VEZES JOGADO",       "INSIRA O NÚMERO DE VEZES JOGADO"},
	{"lastplayed",  MD_TIME,                "0",                true,       "ÚLT. PARTIDA",       "INSIRA A ÚLT. PARTIDA JOGADA"}
};
const std::vector<MetaDataDecl> gameMDD(gameDecls, gameDecls + sizeof(gameDecls) / sizeof(gameDecls[0]));

MetaDataDecl folderDecls[] = {
	{"name",        MD_STRING,              "",                 false,      "NOME",               "INSIRA O NOME DO JOGO"},
	{"sortname",    MD_STRING,              "",                 false,      "NOME DE CLASSIFICAÇÃO",   "INSIRA O NOME DE CLASSIFICAÇÃO DO JOGO"},
	{"desc",        MD_MULTILINE_STRING,    "",                 false,      "DESCRIÇÃO",          "INSIRA A DESCRIÇÃO"},
	{"image",       MD_PATH,                "",                 false,      "IMAGEM",             "INSIRA O CAMINHO DA IMAGEM"},
	{"thumbnail",   MD_PATH,                "",                 false,      "MINIATURA",          "INSIRA O CAMINHO DA MINIATURA"},
	{"video",       MD_PATH,                "",                 false,      "VÍDEO",              "INSIRA O CAMINHO DO VÍDEO"},
	{"marquee",     MD_PATH,                "",                 false,      "LETREIRO",           "INSIRA O CAMINHO DO LETREIRO"},
	{"rating",      MD_RATING,              "0.000000",         false,      "AVALIAÇÃO",          "INSIRA A AVALIAÇÃO"},
	{"releasedate", MD_DATE,                "not-a-date-time",  false,      "LANÇAMENTO",         "INSIRA A DATA DE LANÇAMENTO"},
	{"developer",   MD_STRING,              "DESCONHECIDO",     false,      "DESENVOLVEDOR",      "INSIRA A DESENVOLVEDOR"},
	{"publisher",   MD_STRING,              "DESCONHECIDO",     false,      "PUBLICADOR",         "INSIRA A PUBLICADOR"},
	{"genre",       MD_STRING,              "DESCONHECIDO",     false,      "GÊNERO",             "INSIRA O GÊNERO"},
	{"players",     MD_INT,                 "1",                false,      "JOGADOR",            "INSIRA O NÚMERO DE JOGADORES"}
};
const std::vector<MetaDataDecl> folderMDD(folderDecls, folderDecls + sizeof(folderDecls) / sizeof(folderDecls[0]));

const std::vector<MetaDataDecl>& getMDDByType(MetaDataListType type)
{
	switch(type)
	{
	case GAME_METADATA:
		return gameMDD;
	case FOLDER_METADATA:
		return folderMDD;
	}

	LOG(LogError) << "Invalid MDD type";
	return gameMDD;
}



MetaDataList::MetaDataList(MetaDataListType type)
	: mType(type), mWasChanged(false)
{
	const std::vector<MetaDataDecl>& mdd = getMDD();
	for(auto iter = mdd.cbegin(); iter != mdd.cend(); iter++)
		set(iter->key, iter->defaultValue);
}


MetaDataList MetaDataList::createFromXML(MetaDataListType type, pugi::xml_node& node, const std::string& relativeTo)
{
	MetaDataList mdl(type);

	const std::vector<MetaDataDecl>& mdd = mdl.getMDD();

	for(auto iter = mdd.cbegin(); iter != mdd.cend(); iter++)
	{
		pugi::xml_node md = node.child(iter->key.c_str());
		if(md)
		{
			// if it's a path, resolve relative paths
			std::string value = md.text().get();
			if (iter->type == MD_PATH)
			{
				value = Utils::FileSystem::resolveRelativePath(value, relativeTo, true);
			}
			mdl.set(iter->key, value);
		}else{
			mdl.set(iter->key, iter->defaultValue);
		}
	}

	return mdl;
}

void MetaDataList::appendToXML(pugi::xml_node& parent, bool ignoreDefaults, const std::string& relativeTo) const
{
	const std::vector<MetaDataDecl>& mdd = getMDD();

	for(auto mddIter = mdd.cbegin(); mddIter != mdd.cend(); mddIter++)
	{
		auto mapIter = mMap.find(mddIter->key);
		if(mapIter != mMap.cend())
		{
			// we have this value!
			// if it's just the default (and we ignore defaults), don't write it
			if(ignoreDefaults && mapIter->second == mddIter->defaultValue)
				continue;
			
			// try and make paths relative if we can
			std::string value = mapIter->second;
			if (mddIter->type == MD_PATH)
				value = Utils::FileSystem::createRelativePath(value, relativeTo, true);

			parent.append_child(mapIter->first.c_str()).text().set(value.c_str());
		}
	}
}

void MetaDataList::set(const std::string& key, const std::string& value)
{
	mMap[key] = value;
	mWasChanged = true;
}

const std::string& MetaDataList::get(const std::string& key) const
{
	return mMap.at(key);
}

int MetaDataList::getInt(const std::string& key) const
{
	return atoi(get(key).c_str());
}

float MetaDataList::getFloat(const std::string& key) const
{
	return (float)atof(get(key).c_str());
}

bool MetaDataList::isDefault()
{
	const std::vector<MetaDataDecl>& mdd = getMDD();

	for (unsigned int i = 1; i < mMap.size(); i++) {
		if (mMap.at(mdd[i].key) != mdd[i].defaultValue) return false;
	}

	return true;
}

bool MetaDataList::wasChanged() const
{
	return mWasChanged;
}

void MetaDataList::resetChangedFlag()
{
	mWasChanged = false;
}
