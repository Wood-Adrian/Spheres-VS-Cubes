#include "sd.hpp"

using nlohmann::json;

static std::string saveFolderPath = "sd:/apps/3dwii/data/";
static std::string saveName = "save.json";
static std::string saveCorruptName = "save_corruptbackup.json";
static std::string writeTestName = "test.txt";

//checks if save file exists on sd card
//@return true if save.json exists, false if not
static bool SaveExists() {
	std::ifstream saveStream(saveFolderPath + saveName);
	return saveStream.good();
}

//creates save file (only if file does not exist)
//@return >0 on success, 0 on fail, -1 on save.json already exists
static int CreateSaveFile() {
	if (SaveExists()) return -1;

	std::ofstream saveStreamOut(saveFolderPath + saveName);

	json baseJsonSave;

	try {
		baseJsonSave = R"(
			{
				"save": {
					"cubux": 150,
					"cubeunlocks": [
						{
							"id": 0,
							"name": "Red Cube",
							"path0": [false, false, false, false],
							"path1": [false, false, false, false]
						},
						{
							"id": 1,
							"name": "Green Cube",
							"path0": [false, false, false, false],
							"path1": [false, false, false, false]
						}
					]
				}
			}
		)"_json;

		saveStreamOut << std::setw(2) << baseJsonSave;
		return 1;
	}
	catch (json::exception& ex) {
		std::cout << ex.what() << std::endl;
		return 0;
	}
}

//if "erroroldsave" exists, corrupt save was found but new save successfully generated
//if "errornewsave" exists, save file was not able to be loaded so halt program!
void LoadSaveFile(nlohmann::json& jsonSave) {
	//this will only run if no file exists
	if (CreateSaveFile() == 0) {
		jsonSave["errornewsave"] = std::string("Unable to create new save! Did you leave the write-lock on the sd card on by accident ?");
	}

	std::ifstream saveStreamIn(saveFolderPath + saveName);
	std::string errorString = "";

	try {
		saveStreamIn >> jsonSave;
	}
	catch (json::exception& ex) {
		errorString = ex.what();
		saveStreamIn.close();

		remove((saveFolderPath + saveCorruptName).c_str());
		if (rename((saveFolderPath + saveName).c_str(), (saveFolderPath + saveCorruptName).c_str())) {
			jsonSave["errornewsave"] = errorString + std::string("---Also couldn't rename save so no valid save file actually exists (did you leave the write-lock on by accident?)");
			return;
		}

		CreateSaveFile();

		std::ifstream saveStreamInNew(saveFolderPath + saveName);
		try {
			saveStreamInNew >> jsonSave;
		}
		catch (json::exception& ex) {
			jsonSave["errornewsave"] = errorString + std::string("\nAlso couldn't create new save (although old corrupt save has been renamed) (idk how this could possibly appear)");
			return;
		}
	}

	if (errorString.length() > 0) {
		jsonSave["erroroldsave"] = errorString;
	}

	//todo: validate save file has properly formatted data
}

bool WriteSaveFile(nlohmann::json& jsonSave, std::string& REMOVE) {
	jsonSave.erase("erroroldsave");
	jsonSave.erase("errornewsave");
	std::ofstream saveStreamOut(saveFolderPath + saveName);
	if (!saveStreamOut.good()) {
		REMOVE = "stream out not good!";
		return false;
	}

	try {
		saveStreamOut << std::setw(2) << jsonSave;
		REMOVE = "saved properly!";
		return true;
	}
	catch (std::exception& ex) {
		REMOVE = "exception while streaming out!";
		return false;
	}
}

bool WriteLockTest() {
	std::remove((saveFolderPath + writeTestName).c_str());

	std::ofstream streamOut(saveFolderPath + writeTestName);
	streamOut << ":3c";
	streamOut.close();

	std::ifstream streamIn(saveFolderPath + writeTestName);
	bool returnValue = streamIn.fail();
	streamIn.close();

	std::remove((saveFolderPath + writeTestName).c_str());

	return returnValue;
}

/*
* note these examples from the json github
	// read a JSON file
	std::ifstream i("file.json");
	json j;
	i >> j;

	// write prettified JSON to another file
	std::ofstream o("pretty.json");
	o << std::setw(4) << j << std::endl;
*/