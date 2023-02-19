package com.violet.hikikomori.view.media.file

import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentActivity
import androidx.viewpager2.adapter.FragmentStateAdapter

class FragmentListAdapter(
    fragmentActivity: FragmentActivity,
    val fragmentList: List<MediaPage>
) :
    FragmentStateAdapter(fragmentActivity) {

    override fun createFragment(position: Int): Fragment {
        return fragmentList[position].fragment
    }

    override fun getItemCount(): Int = fragmentList.size

}