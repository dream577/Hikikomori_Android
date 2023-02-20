package com.violet.libbasetools.util

import android.content.Context
import java.io.File
import java.io.FileOutputStream
import java.text.SimpleDateFormat
import java.util.*

/**
 * 安卓中有"内部存储"和"外部存储"的概念:
 *  (1)内部存储: 默认只能被所属app所访问，当应用被卸载之后，内部存储中的文件也将被删除; 如果在创建内部存储文件的时候将文件属性设置成可读,
 *     其他app能够访问自己应用的数据，前提是知道该应用的包名.内部存储空间十分有限，因而显得可贵，另外，它也是系统本身和系统应用程序主要的
 *     数据存储所在地，一旦内部存储空间耗尽，手机也就无法使用了。所以对于内部存储空间，要尽量避免使用
 *
 *  (2)外部存储: 在4.4（API19）以前的手机自身带的存储卡就是内部存储，而扩展的SD卡就是外部存储;
 *     从4.4的系统开始, 开始有了机身存储的外部存储和SD卡外部存储, 4.4系统之后访问外部存储的方法：
 *     val files: Array<File> = getExternalFilesDirs(mContext, Environment.MEDIA_MOUNTED)
 *
 * 内部存储访问api：
 *     1、Environment.getDataDirectory() = /data                                                      获取内部存储的根路径
 *     2、getFilesDir().getAbsolutePath() = /data/user/0/{PackageName}/files                          获取某个应用在内部存储中的files路径
 *     3、getCacheDir().getAbsolutePath() = /data/user/0/{PackageName}/cache                          获取某个应用在内部存储中的cache路径
 *     4、getDir(“myFile”, MODE_PRIVATE).getAbsolutePath() = /data/user/0/{PackageName}/app_myFile    获取某个应用在内部存储中的自定义路径
 *
 *    注意：方法2,3,4的路径中都带有包名，说明他们是属于某个应用
 *
 * 外部存储访问api：
 *     5、Environment.getExternalStorageDirectory().getAbsolutePath() = /storage/emulated/0                 获取外部存储的根路径
 *     6、Environment.getExternalStoragePublicDirectory(“”).getAbsolutePath() = /storage/emulated/0         获取外部存储的根路径
 *     7、getExternalFilesDir(“”).getAbsolutePath() = /storage/emulated/0/Android/data/{PackageName}/files  获取某个应用在外部存储中的files路径
 *     8、getExternalCacheDir().getAbsolutePath() = /storage/emulated/0/Android/data/{PackageName}/cache    获取某个应用在外部存储中的files路径
 */


fun createFile(context: Context, extension: String): File {
    val sdf = SimpleDateFormat("yyyy_MM_dd_HH_mm_ss_SSS", Locale.US)
    return File(context.filesDir, "IMG_${sdf.format(Date())}.$extension")
}

/**
 * 拷贝assert文件夹下的文件到sd中
 */
fun copyAssetsDirToSDCard(context: Context, assetsDirName: String, path: String) {
    var sdCardPath = path
    KLog.d(
        "FileUtil",
        "copyAssetsDirToSDCard() called with: context = [$context], assetsDirName = [$assetsDirName], sdCardPath = [$sdCardPath]"
    )
    try {
        val list = context.assets.list(assetsDirName) ?: return
        if (list.isEmpty()) {
            val inputStream = context.assets.open(assetsDirName)
            val mByte = ByteArray(1024)
            var bt = 0
            val file = File(
                sdCardPath + File.separator + assetsDirName.substring(
                    assetsDirName.lastIndexOf('/')
                )
            )
            if (!file.exists()) {
                file.createNewFile()
            } else {
                return
            }
            val fos = FileOutputStream(file)
            while ((inputStream.read(mByte).also { bt = it }) != -1) {
                fos.write(mByte, 0, bt)
            }
            fos.flush()
            inputStream.close()
            fos.close()
        } else {
            var subDirName = assetsDirName
            if (assetsDirName.contains("/")) {
                subDirName = assetsDirName.substring(assetsDirName.lastIndexOf('/') + 1)
            }
            sdCardPath = sdCardPath + File.separator + subDirName
            val file = File(sdCardPath)
            if (!file.exists()) file.mkdirs()
            for (s: String in list) {
                copyAssetsDirToSDCard(context, assetsDirName + File.separator + s, sdCardPath)
            }
        }
    } catch (e: Exception) {
        e.printStackTrace()
    }
}

