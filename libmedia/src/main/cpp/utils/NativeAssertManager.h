//
// Created by 郝留凯 on 2023/2/25.
//

#ifndef HIKIKOMORI_NATIVEASSERTMANAGER_H
#define HIKIKOMORI_NATIVEASSERTMANAGER_H

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>


class VioletAssertManager {
private:
    AAssetManager *manager = nullptr;

    VioletAssertManager();

public:
    static VioletAssertManager *GetInstance();

    void SetAssertManager(AAssetManager *aAssetManager);

    char *GetAssertFile(char *name);

    char *GetVShaderCharArray(char *name) {
        return nullptr;
    }
};


#endif //HIKIKOMORI_NATIVEASSERTMANAGER_H
