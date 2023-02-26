//
// Created by bronyna on 2023/2/26.
//

#include <cstdlib>
#include "NativeAssertManager.h"

static VioletAssertManager *instance = nullptr;

VioletAssertManager::VioletAssertManager() = default;

char *VioletAssertManager::GetAssertFile(char *name) {
    if (manager == nullptr) return nullptr;
    AAsset *assertFile = AAssetManager_open(manager, name, AASSET_MODE_BUFFER);
    long length = AAsset_getLength(assertFile);
    char *buf = (char *) malloc(length);
    AAsset_read(assertFile, buf, length);
    AAsset_close(assertFile);
    return buf;
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