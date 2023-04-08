package com.violet.hikikomori.viewmodel.base

import android.app.Application
import android.os.Bundle
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import com.violet.hikikomori.model.ViewPager
import com.violet.hikikomori.view.base.BaseFragment
import com.violet.libbasetools.model.Event
import com.violet.libbasetools.util.tag
import java.util.Stack

class PagerManager(application: Application) : AndroidViewModel(application) {
    val pagerStack: Stack<ViewPager> = Stack()
    val pagerMap = HashMap<String, Class<*>>()

    private val _pagerLiveData = MutableLiveData<Event<ViewPager>>()

    val mPagerLiveData: LiveData<Event<ViewPager>>
        get() = _pagerLiveData

    fun <T : BaseFragment> registerFragment(tag: String, cls: Class<T>) {
        pagerMap[tag] = cls
    }

    fun enterFragment(tag: String) {
        enterFragment(tag, null)
    }

    fun enterFragment(tag: String, bundle: Bundle?) {
        _pagerLiveData.value = Event(ViewPager(tag, bundle))
    }

    fun <T : BaseFragment> enterFragment(cls: Class<T>) {
        enterFragment(cls.tag)
    }

    fun <T : BaseFragment> enterFragment(cls: Class<T>, bundle: Bundle?) {
        enterFragment(cls.tag, bundle)
    }


}