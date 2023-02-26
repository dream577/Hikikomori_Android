package com.violet.hikikomori.view.base;

import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.view.ViewCompat;

import com.violet.hikikomori.R;

import java.util.ArrayList;
import java.util.List;

public abstract class BaseActivity extends AppCompatActivity implements View.OnClickListener {
    protected String TAG = getClass().getSimpleName();
    private static final List<AppCompatActivity> activityList = new ArrayList<>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        activityList.add(this);
        initData();
        initView();
        subscribeToViewModel();
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    protected void onRestart() {
        super.onRestart();
    }

    @Override
    protected void onDestroy() {
        activityList.remove(this);
        super.onDestroy();
    }

    protected void initData() {
    }

    protected void initView() {
        setContentView(getLayoutId());
    }

    protected void subscribeToViewModel() {
    }

    protected abstract int getLayoutId();

    protected void showToast(String msg) {
        Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
    }

    protected void finishAllActivity() {
        for (AppCompatActivity activity : activityList) {
            activity.finish();
        }
        activityList.clear();
    }

    protected void showSystemUI() {
        Window window = getWindow();
        // 设置系统状态栏处于可见状态
        window.getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_VISIBLE);
        // 取消全屏
        window.clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        // 取消状态栏透明
        window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
        // 添加Flag把状态栏设为可绘制模式
        window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
        // 设置状态栏颜色
        window.setStatusBarColor(getResources().getColor(R.color.black));
        // 让view不根据系统窗口来调整自己的布局
        ViewGroup mContentView = (ViewGroup) window.findViewById(Window.ID_ANDROID_CONTENT);
        View mChildView = mContentView.getChildAt(0);
        if (mChildView != null) {
            ViewCompat.setFitsSystemWindows(mChildView, false);
            ViewCompat.requestApplyInsets(mChildView);
        }
    }

    protected static void finishAll() {
        for (AppCompatActivity activity : activityList) {
            activity.finish();
        }
        activityList.clear();
    }

    @Override
    public void finish() {
        super.finish();
    }
}
