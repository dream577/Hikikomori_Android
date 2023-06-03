package com.violet.hikikomori.viewmodel.media

import android.app.Application
import androidx.databinding.ObservableBoolean
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData

class VideoPlayViewModel(application: Application) : AndroidViewModel(application) {
    val isFullScreen = ObservableBoolean(false)

    private val _fullScreenLiveData = MutableLiveData(false)
    val mFullScreenLiveData: LiveData<Boolean>
        get() = _fullScreenLiveData

    fun toggleFullScreen() {
        val isLandScape = !isFullScreen.get()
        this.isFullScreen.set(isLandScape)
        _fullScreenLiveData.value = isLandScape
    }
}