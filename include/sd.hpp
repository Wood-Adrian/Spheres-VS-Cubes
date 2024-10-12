#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <ogcsys.h>
#include <gccore.h>

#include "nlohmann/json.hpp"

//loads and returns the save file as a JSON object
void LoadSaveFile(nlohmann::json& jsonSave);

//writes save file to sd card
//@return true if save successful, false if exception occurs
bool WriteSaveFile(nlohmann::json& jsonSave, std::string& REMOVE);

//tests if write lock is on on the sd card
//@return true if write-lock is ON, false if OFF (can be written to)
bool WriteLockTest();