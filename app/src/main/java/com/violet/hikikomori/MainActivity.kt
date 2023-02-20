package com.violet.hikikomori

import android.Manifest
import android.view.View
import android.view.WindowManager
import com.violet.hikikomori.databinding.ActivityMainBinding
import com.violet.hikikomori.view.base.BaseBindingActivity
import com.violet.hikikomori.view.media.camera.CameraActivity
import com.violet.hikikomori.view.media.file.FileActivity
import pub.devrel.easypermissions.EasyPermissions
import pub.devrel.easypermissions.PermissionRequest

class MainActivity : BaseBindingActivity<ActivityMainBinding>(), View.OnClickListener {
    private val TAG = MainActivity::class.java.simpleName


    override fun initView() {
        super.initView()
        mBinding.listener = this

//        View.SYSTEM_UI_FLAG_LAYOUT_STABLE：            全屏显示时保证尺寸不变。
//        View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN：        Activity全屏显示，状态栏显示在Activity页面上面。
//        View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION：   效果同View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
//        View.SYSTEM_UI_FLAG_HIDE_NAVIGATION：          隐藏导航栏
//        View.SYSTEM_UI_FLAG_FULLSCREEN：Activity       全屏显示，且状态栏被隐藏覆盖掉。
//        View.SYSTEM_UI_FLAG_VISIBLE：Activity          非全屏显示，显示状态栏和导航栏。
//        View.INVISIBLE：Activity                       伸展全屏显示，隐藏状态栏。
//        View.SYSTEM_UI_LAYOUT_FLAGS：                  效果同View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
//        View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY：         必须配合View.SYSTEM_UI_FLAG_FULLSCREEN和View.SYSTEM_UI_FLAG_HIDE_NAVIGATION组合使用，达到的效果是拉出状态栏和导航栏后显示一会儿消失。

//        // 全屏展示
//        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
//            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
//                // 全屏显示，隐藏状态栏和导航栏，拉出状态栏和导航栏显示一会儿后消失。
//                window.decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_LAYOUT_STABLE or
//                        View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN or
//                        View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION or
//                        View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or
//                        View.SYSTEM_UI_FLAG_FULLSCREEN or
//                        View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
//            } else {
//                // 全屏显示，隐藏状态栏
//                window.decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_FULLSCREEN
//            }
//        }
        // 非全屏显示，显示状态栏和导航栏
        window.addFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS)
        window.decorView.systemUiVisibility =
            View.SYSTEM_UI_FLAG_VISIBLE or View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
    }

    override fun onResume() {
        super.onResume()
        if (!EasyPermissions.hasPermissions(this, Manifest.permission.CAMERA)) {
            EasyPermissions.requestPermissions(
                this, "请求获取相机权限", 101, Manifest.permission.CAMERA
            )
        }
    }

    override fun onClick(v: View) {
        when (v.id) {
            R.id.btn_go_media -> CameraActivity.launch(this)
        }
    }

    override fun getLayoutId(): Int = R.layout.activity_main

    private fun updateMediaUI() {
        window.decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_VISIBLE
        window.clearFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS)
        // 显示状态栏
        window.clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN)
        // 取消状态栏透明
        window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS)
        // 添加Flag把状态栏设为可绘制模式
        window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS)
        // 设置状态栏颜色
        window.statusBarColor = resources.getColor(R.color.black)
    }
}