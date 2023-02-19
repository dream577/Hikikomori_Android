package com.violet.hikikomori.viewmodel

import android.app.Application
import androidx.databinding.ObservableBoolean
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.MutableLiveData

open class BaseViewModel(application: Application) : AndroidViewModel(application) {
    val loadingLiveData: MutableLiveData<Boolean> = MutableLiveData()

    val loadingFailLiveData: MutableLiveData<Boolean> = MutableLiveData()

    val isLoading = ObservableBoolean(false)

    protected fun startLoading() {
        isLoading.set(true)
        loadingLiveData.postValue(true)
    }

    protected fun endLoading() {
        isLoading.set(false)
        loadingLiveData.postValue(false)
    }
}