package com.violet.hikikomori;

import android.Manifest;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.databinding.DataBindingUtil;

import com.violet.hikikomori.databinding.ActivityMainBinding;
import com.violet.libmedia.VioletMediaClient;

import pub.devrel.easypermissions.EasyPermissions;

public class MainActivity extends AppCompatActivity implements SurfaceHolder.Callback, View.OnClickListener {

    private ActivityMainBinding mBinding;
    private String path = "/storage/emulated/0/视频/[Airota][Fate stay night Heaven's Feel III.spring song][Movie][BDRip 1080p AVC AAC][CHS].mp4";
    private VioletMediaClient client;

    private String readPermission = Manifest.permission.READ_EXTERNAL_STORAGE;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mBinding = DataBindingUtil.setContentView(this, R.layout.activity_main);
        mBinding.surface.getHolder().addCallback(this);
        mBinding.setListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (!EasyPermissions.hasPermissions(this, readPermission)) {
            EasyPermissions.requestPermissions(this, "读取存储", 101, readPermission);
        } else {
        }

    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        client = new VioletMediaClient();
        client.init(path, holder.getSurface());
        client.play();
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {

    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.pause:
                client.pause();
                break;
            case R.id.resume:
                client.resume();
                break;
        }
    }
}