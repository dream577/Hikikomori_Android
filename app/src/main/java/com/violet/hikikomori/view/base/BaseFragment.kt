package com.violet.hikikomori.view.base

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.violet.libbasetools.KLog

abstract class BaseFragment : Fragment() {
    protected val TAG = "FragmentObserver"

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        KLog.d(TAG, "${javaClass.simpleName} created!!!")
        initData()
        val view = super.onCreateView(inflater, container, savedInstanceState)
        initView()
        subscribeToViewModel()
        return view
    }

    override fun onDestroyView() {
        super.onDestroyView()
        KLog.d(TAG, "${javaClass.simpleName} destroyed!!!")
    }

    protected abstract fun initData()

    protected abstract fun initView()

    protected abstract fun subscribeToViewModel()
}