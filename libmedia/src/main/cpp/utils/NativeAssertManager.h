//
// Created by 郝留凯 on 2023/2/25.
//

#ifndef HIKIKOMORI_NATIVEASSERTMANAGER_H
#define HIKIKOMORI_NATIVEASSERTMANAGER_H

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

namespace violet {

    class VioletAssertManager {
    private:
        AAssetManager *manager = nullptr;

    public:
        VioletAssertManager(AAssetManager *aAssetManager) {
            manager = aAssetManager;
        }

        char *GetFShaderCharArray(char *name) {
            if (manager == nullptr) return nullptr;
            AAsset *assertFile = AAssetManager_open(manager, name, AASSET_MODE_BUFFER);
            char *dataBuffer = (char *) AAsset_getBuffer(assertFile);
            AAsset_close(assertFile);
            return dataBuffer;
        }

        char *GetVShaderCharArray(char *name) {
            return nullptr;
        }
    };

    static VioletAssertManager *instance;
}

#endif //HIKIKOMORI_NATIVEASSERTMANAGER_H
