package com.violet.hikikomori.viewmodel.media

import android.app.Application
import android.content.Context
import android.database.Cursor
import android.net.Uri
import android.provider.MediaStore
import androidx.core.net.toUri
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import com.violet.hikikomori.model.ImageItem
import com.violet.hikikomori.model.VideoItem
import com.violet.hikikomori.viewmodel.BaseViewModel
import com.violet.libbasetools.model.Event
import com.violet.libbasetools.util.checkVideoDir
import io.reactivex.rxjava3.android.schedulers.AndroidSchedulers
import io.reactivex.rxjava3.core.Observable
import io.reactivex.rxjava3.schedulers.Schedulers
import java.io.File

class FileViewModel(application: Application) : BaseViewModel(application) {
    private val _videoListLiveData: MutableLiveData<Event<List<VideoItem>>> = MutableLiveData()

    private val _imageListLiveData: MutableLiveData<Event<List<ImageItem>>> = MutableLiveData()

    fun queryVideos(mContext: Context) {
        Observable.just(1)
            .map { getVideoList(mContext) }
            .doOnSubscribe { startLoading() }
            .subscribeOn(Schedulers.io())
            .observeOn(AndroidSchedulers.mainThread())
            .doOnNext { _videoListLiveData.postValue(Event(it)) }
            .doFinally { endLoading() }
            .subscribe()
    }

    fun queryImages(mContext: Context) {
        Observable.just(1)
            .map { getImageList(mContext) }
            .doOnSubscribe { startLoading() }
            .subscribeOn(Schedulers.io())
            .observeOn(AndroidSchedulers.mainThread())
            .doOnNext { _imageListLiveData.postValue(Event(it)) }
            .doFinally { endLoading() }
            .subscribe()
    }

    fun getVideoListLiveData(): LiveData<Event<List<VideoItem>>> {
        return _videoListLiveData;
    }

    fun getImageListLiveData(): LiveData<Event<List<ImageItem>>> {
        return _imageListLiveData;
    }


    private fun getVideoList(context: Context): MutableList<VideoItem> {
        val list: MutableList<VideoItem> = ArrayList()
        val contentResolver = context.contentResolver
        val cursor: Cursor? = contentResolver.query(
            MediaStore.Video.Media.EXTERNAL_CONTENT_URI, null, null, null, null
        )
        cursor?.let {
            while (it.moveToNext()) {
                val id = it.getInt(it.getColumnIndexOrThrow(MediaStore.Video.Media._ID))
                val title = it.getString(it.getColumnIndexOrThrow(MediaStore.Video.Media.TITLE))
                val album = it.getString(it.getColumnIndexOrThrow(MediaStore.Video.Media.ALBUM))
                val artist = it.getString(it.getColumnIndexOrThrow(MediaStore.Video.Media.ARTIST))
                val displayName = it.getString(it.getColumnIndexOrThrow(MediaStore.Video.Media.DISPLAY_NAME))
                val mimeType = it.getString(it.getColumnIndexOrThrow(MediaStore.Video.Media.MIME_TYPE))
                val path = it.getString(it.getColumnIndexOrThrow(MediaStore.Video.Media.DATA))
                val duration = it.getInt(it.getColumnIndexOrThrow(MediaStore.Video.Media.DURATION))
                val size = it.getLong(it.getColumnIndexOrThrow(MediaStore.Video.Media.SIZE))
                val video = VideoItem(id, title, album, artist, displayName, mimeType, path, size, duration)
                list.add(video)
            }
            cursor.close()
        }

        val videoDirPath = checkVideoDir(context)
        val files = File(videoDirPath).listFiles()
        if (files != null) {
            for (file in files) {
                list.add(VideoItem(list.size, file.name, file.path))
            }
        }
        return list
    }


    private fun getImageList(context: Context): MutableList<ImageItem> {
        val list: MutableList<ImageItem> = ArrayList()
        val contentResolver = context.contentResolver
//        val cursor: Cursor? = contentResolver.query(
//            MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, null, null, null
//        )
        val cursor: Cursor? = contentResolver.query(
            context.filesDir.toUri(), null, null, null, null
        )
        cursor?.let {
            while (it.moveToNext()) {
                val vId = it.getInt(it.getColumnIndexOrThrow(MediaStore.Images.Media._ID))
                val vTitle = it.getString(it.getColumnIndexOrThrow(MediaStore.Images.Media.TITLE))
                val vDisplayName = it.getString(it.getColumnIndexOrThrow(MediaStore.Images.Media.DISPLAY_NAME))
                val vMimeType = it.getString(it.getColumnIndexOrThrow(MediaStore.Images.Media.MIME_TYPE))
                val vPath = it.getString(it.getColumnIndexOrThrow(MediaStore.Images.Media.DATA))
                val vSize = it.getLong(it.getColumnIndexOrThrow(MediaStore.Images.Media.SIZE))
                val vHeight = it.getInt(it.getColumnIndexOrThrow(MediaStore.Images.Media.HEIGHT))
                val vWidth = it.getInt(it.getColumnIndexOrThrow(MediaStore.Images.Media.WIDTH))
                val vOrientation = it.getInt(it.getColumnIndexOrThrow(MediaStore.Images.Media.ORIENTATION))
                val image = ImageItem(vId, vTitle, vPath, vHeight, vWidth, vOrientation)
                list.add(image)
            }
            cursor.close()
        }
        return list
    }
}