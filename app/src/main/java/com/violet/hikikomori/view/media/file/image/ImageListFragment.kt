package com.violet.hikikomori.view.media.file.image

import android.graphics.Rect
import android.view.View
import androidx.lifecycle.ViewModelProvider
import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.StaggeredGridLayoutManager
import com.violet.hikikomori.R
import com.violet.hikikomori.databinding.FragmentCommonListPageBinding
import com.violet.hikikomori.model.MediaItem
import com.violet.hikikomori.view.base.BaseBindingFragment
import com.violet.hikikomori.view.media.file.MediaAdapterCallback
import com.violet.hikikomori.viewmodel.media.FileViewModel
import com.violet.libbasetools.util.KLog
import pub.devrel.easypermissions.EasyPermissions

class ImageListFragment : BaseBindingFragment<FragmentCommonListPageBinding>(),
    MediaAdapterCallback {
    private lateinit var mViewModel: FileViewModel
    private lateinit var mAdapter: ImageListAdapter

    override fun initData() {
        super.initData()
        mViewModel = ViewModelProvider(this).get(FileViewModel::class.java)
    }

    override fun initView() {
        super.initView()

        mAdapter = ImageListAdapter(requireContext())
        mAdapter.setMediaAdapterCallback(this)
        val layoutManager = StaggeredGridLayoutManager(2, StaggeredGridLayoutManager.VERTICAL)
//        layoutManager.gapStrategy = StaggeredGridLayoutManager.GAP_HANDLING_NONE  // todo 瀑布式布局滑动item交换位置问题
        val divider = 16
        val itemDecoration: RecyclerView.ItemDecoration = object : RecyclerView.ItemDecoration() {
            override fun getItemOffsets(
                outRect: Rect,
                view: View,
                parent: RecyclerView,
                state: RecyclerView.State
            ) {
                outRect.left = divider / 2
                outRect.right = divider / 2
            }
        }

        mBinding.run {
            rvListView.run {
                this.layoutManager = layoutManager
                this.addItemDecoration(itemDecoration)
                this.adapter = mAdapter
            }
        }

        if (EasyPermissions.hasPermissions(
                requireContext(),
                "android.permission.READ_EXTERNAL_STORAGE"
            )
        ) {
            mViewModel.queryImages(requireContext())
        } else {
            EasyPermissions.requestPermissions(
                requireActivity(),
                "显示图片",
                101,
                "android.permission.READ_EXTERNAL_STORAGE"
            )
        }
    }

    override fun subscribeToViewModel() {
        super.subscribeToViewModel()
        mViewModel.getImageListLiveData().observe(this) { event ->
            if (!event.isHandled()) {
                val list = event.getDataIfNotHandled()
                if (list != null) {
                    mAdapter.refreshMediaList(list)
                }
            }
        }

        mViewModel.loadingLiveData.observe(this) {
            KLog.d(TAG, it)
            mBinding.loadingView.visibility = if (it) View.VISIBLE else View.GONE
        }
    }

    override fun getLayoutId(): Int = R.layout.fragment_common_list_page

    override fun onClick(mediaBean: MediaItem) {

    }
}