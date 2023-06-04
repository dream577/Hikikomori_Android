package com.violet.hikikomori.viewmodel.media

import android.app.Application
import androidx.databinding.ObservableBoolean
import androidx.databinding.ObservableInt
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData

class VideoPlayViewModel(application: Application) : AndroidViewModel(application) {
    companion object {
        const val HOUR_MIN_SEC = "%3s:%2s:%1s"
        const val MIN_SEC = "%2s:%1s"
    }

    val isFullScreen = ObservableBoolean(false)
    val progress = ObservableInt(0)

    private val _fullScreenLiveData = MutableLiveData(false)
    val mFullScreenLiveData: LiveData<Boolean>
        get() = _fullScreenLiveData

    fun toggleFullScreen() {
        val isLandScape = !isFullScreen.get()
        this.isFullScreen.set(isLandScape)
        _fullScreenLiveData.value = isLandScape
    }

    fun formatTime(timestamp: Long) {
        val hour = timestamp / 1000 / 60 / 60
        val min = timestamp / 1000 / 60 % 60
        val sec = timestamp / 1000 % 60
        var hourStr = ""
        var minStr = ""
        var secStr = ""

    }
}