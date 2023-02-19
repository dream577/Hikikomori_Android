package com.violet.hikikomori.view.media

import android.os.Bundle
import android.view.View
import androidx.navigation.Navigation
import com.violet.hikikomori.R
import com.violet.hikikomori.databinding.FragmentMediaFileBinding
import com.violet.hikikomori.view.base.BaseBindingFragment

private const val ARG_PARAM1 = "param1"
private const val ARG_PARAM2 = "param2"

class MediaFileFragment : BaseBindingFragment<FragmentMediaFileBinding>(), View.OnClickListener {
    private var param1: String? = null
    private var param2: String? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        arguments?.let {
            param1 = it.getString(ARG_PARAM1)
            param2 = it.getString(ARG_PARAM2)
        }
    }

    companion object {
        @JvmStatic
        fun newInstance(param1: String, param2: String) =
            MediaFileFragment().apply {
                arguments = Bundle().apply {
                    putString(ARG_PARAM1, param1)
                    putString(ARG_PARAM2, param2)
                }
            }
    }

    override fun initView() {
        super.initView()
        mBinding.listener = this
    }

    override fun getLayoutId(): Int = R.layout.fragment_media_file

    override fun onClick(v: View?) {
        when (v?.id) {
            R.id.btn_play_video -> {
                Navigation.findNavController(v)
                    .navigate(R.id.action_mediaFileFragment_to_videoPlayFragment)
            }
        }
    }
}