package com.violet.hikikomori.view.media.file.video

import android.Manifest
import android.graphics.Rect
import android.view.View
import androidx.lifecycle.ViewModelProvider
import androidx.recyclerview.widget.GridLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.violet.hikikomori.R
import com.violet.hikikomori.databinding.FragmentCommonListPageBinding
import com.violet.hikikomori.model.MediaItem
import com.violet.hikikomori.view.base.BaseBindingFragment
import com.violet.hikikomori.view.media.player.VideoPlayActivity
import com.violet.hikikomori.view.media.file.MediaAdapterCallback
import com.violet.hikikomori.viewmodel.media.FileViewModel
import com.violet.libbasetools.util.KLog
import pub.devrel.easypermissions.EasyPermissions

class VideoListFragment : BaseBindingFragment<FragmentCommonListPageBinding>(),
    MediaAdapterCallback {

    companion object {
        const val TAG = "VideoListFragment"
    }

    private lateinit var mViewModel: FileViewModel
    private lateinit var mAdapter: VideoListAdapter

    override fun initData() {
        super.initData()
        mViewModel = ViewModelProvider(this).get(FileViewModel::class.java)

        if (EasyPermissions.hasPermissions(
                requireContext(),
                Manifest.permission.READ_EXTERNAL_STORAGE
            )
        ) {
            mViewModel.queryVideos(requireContext())
        }
    }

    override fun initView() {
        super.initView()

        mAdapter = VideoListAdapter(requireContext())
        mAdapter.setMediaAdapterCallback(this)
        val layoutManager = GridLayoutManager(requireContext(), 2, RecyclerView.VERTICAL, false)
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
    }

    override fun subscribeToViewModel() {
        super.subscribeToViewModel()
        mViewModel.getVideoListLiveData().observe(this) { event ->
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
        VideoPlayActivity.launch(requireContext(), mediaBean.path)
    }
}