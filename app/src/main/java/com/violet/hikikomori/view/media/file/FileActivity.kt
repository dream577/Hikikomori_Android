package com.violet.hikikomori.view.media.file

import android.content.Context
import android.content.Intent
import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatDelegate
import androidx.lifecycle.ViewModelProvider
import androidx.viewpager2.widget.ViewPager2
import com.google.android.material.tabs.TabLayoutMediator
import com.violet.hikikomori.R
import com.violet.hikikomori.databinding.ActivityFileBinding
import com.violet.hikikomori.view.base.BaseBindingActivity
import com.violet.hikikomori.view.media.file.image.ImageListFragment
import com.violet.hikikomori.view.media.file.video.VideoListFragment
import com.violet.hikikomori.viewmodel.media.FileViewModel

class FileActivity : BaseBindingActivity<ActivityFileBinding>() {

    companion object {
        @JvmStatic
        fun launch(context: Context) {
            val intent = Intent(context, FileActivity::class.java)
            context.startActivity(intent)
        }
    }

    private lateinit var mViewModel: FileViewModel
    private lateinit var mAdapter: FragmentListAdapter
    private lateinit var pages: List<MediaPage>

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
    }

    override fun onClick(v: View?) {
    }

    override fun initData() {
        super.initData()
        mViewModel = ViewModelProvider(this).get(FileViewModel::class.java)
    }

    override fun initView() {
        requestWindowFeature(AppCompatDelegate.FEATURE_SUPPORT_ACTION_BAR)
        super.initView()
        setSupportActionBar(mBinding.fileToolbar)

        pages = listOf(
            MediaPage(VideoListFragment(), getString(R.string.title_video)),
            MediaPage(ImageListFragment(), getString(R.string.title_image))
        )
        mAdapter = FragmentListAdapter(this, pages)

        mBinding.apply {
            fileToolbar.setNavigationOnClickListener { finish() }
            viewPager.registerOnPageChangeCallback(object : ViewPager2.OnPageChangeCallback() {
                override fun onPageSelected(position: Int) {
                    super.onPageSelected(position)
                    tabLayout.setScrollPosition(position, 0f, true)
                }
            })

            viewPager.adapter = mAdapter

            TabLayoutMediator(tabLayout, viewPager) { tab, position ->
                tab.text = pages[position].title
            }.attach()
        }
    }

    override fun subscribeToViewModel() {
        super.subscribeToViewModel()
    }

    override fun getLayoutId(): Int {
        return R.layout.activity_file
    }
}