//
// Created by bronyna on 2023/2/26.
//

#include "NativeAssertManager.h"

static VioletAssertManager *instance = nullptr;

VioletAssertManager::VioletAssertManager() {}

char *VioletAssertManager::GetAssertFile(char *name) {
    if (manager == nullptr) return nullptr;
    AAsset *assertFile = AAssetManager_open(manager, name, AASSET_MODE_BUFFER);
    char *dataBuffer = (char *) AAsset_getBuffer(assertFile);
    AAsset_close(assertFile);
    return dataBuffer;
}

void VioletAssertManager::SetAssertManager(AAssetManager *aAssetManager) {
    manager = aAssetManager;
}

VioletAssertManager *VioletAssertManager::GetInstance() {
    if (instance == nullptr) {
        instance = new VioletAssertManager();
    }
    return instance;
}