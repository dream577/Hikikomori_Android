package com.violet.hikikomori.view.base

import android.text.TextUtils
import android.view.View
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import com.violet.hikikomori.R
import com.violet.hikikomori.model.ViewPager
import com.violet.hikikomori.view.util.AnimUtil.DIRECTION_ENTER_FROM_LEFT
import com.violet.hikikomori.view.util.AnimUtil.DIRECTION_ENTER_FROM_RIGHT
import com.violet.hikikomori.view.util.AnimUtil.DIRECTION_OUT_FROM_LEFT
import com.violet.hikikomori.view.util.AnimUtil.DIRECTION_OUT_FROM_RIGHT
import com.violet.hikikomori.view.util.AnimUtil.getAnimation
import com.violet.hikikomori.viewmodel.base.PagerManager
import com.violet.libbasetools.util.tag

abstract class FragmentPagerActivity : BaseActivity() {

    protected lateinit var pagerManager: PagerManager

    override fun initData() {
        super.initData()
        pagerManager = ViewModelProvider(this)[PagerManager::class.java]
        registerFragment()
    }

    override fun subscribeToViewModel() {
        super.subscribeToViewModel()
        pagerManager.mPagerLiveData.observe(this) {
            val pager = it.getDataIfNotHandled()
            if (pager != null) replaceFragment(pager)
        }
    }

    abstract fun registerFragment()

    fun <T : BaseFragment> registerFragment(tag: String, cls: Class<T>) {
        pagerManager.registerFragment(tag, cls)
    }

    fun <T : BaseFragment> registerFragment(cls: Class<T>) {
        pagerManager.registerFragment(cls.tag, cls)
    }

    private fun replaceFragment(viewPager: ViewPager) {
        val isFirst = pagerManager.pagerStack.isEmpty()
        var flag = false

        if (!isFirst) {
            for (pager in pagerManager.pagerStack) {
                if (TextUtils.equals(viewPager.tag, pager.tag)) {
                    flag = true
                    break
                }
            }
        }

        val transaction = supportFragmentManager.beginTransaction()
        if (flag) {
            while (pagerManager.pagerStack.size > 1 && TextUtils.equals(
                    pagerManager.pagerStack.peek().tag, viewPager.tag
                )
            ) {
                pagerManager.pagerStack.pop()
                supportFragmentManager.popBackStack()
            }
        } else {
            // 设置进出场动画
            var enterAnim = getAnimation(DIRECTION_ENTER_FROM_RIGHT)
            val outAnim = getAnimation(DIRECTION_OUT_FROM_LEFT)
            val popEnterAnim = getAnimation(DIRECTION_ENTER_FROM_LEFT)
            val popOutAnim = getAnimation(DIRECTION_OUT_FROM_RIGHT)
            if (isFirst) enterAnim = 0
            transaction.setCustomAnimations(enterAnim, outAnim, popEnterAnim, popOutAnim)

            if (pagerManager.pagerMap[viewPager.tag] != null) {
                val fragment = pagerManager.pagerMap[viewPager.tag]!!.newInstance() as Fragment
                fragment.arguments = viewPager.bundle
                pagerManager.pagerStack.push(viewPager)
                transaction.replace(R.id.fragment_container, fragment, viewPager.tag)
                transaction.addToBackStack(null)
            }
        }
        transaction.commit()
    }


    override fun onBackPressed() {
        // 按下返回键时优先由栈顶的fragment进行处理
        val fragment = supportFragmentManager.findFragmentById(R.id.fragment_container)
        if (fragment is BaseFragment) {
            val handled = fragment.onBackPressed()
            if (handled) return
        }
        // 如果返回事件栈顶的fragment没有处理，则返回
        val backStackCount = supportFragmentManager.backStackEntryCount
        if (backStackCount > 1) {
            supportFragmentManager.popBackStack()
        } else {
            finish()
        }
    }

    override fun getLayoutId(): Int = R.layout.activity_fragment_pager

    override fun onClick(v: View?) {
        TODO("Not yet implemented")
    }
}