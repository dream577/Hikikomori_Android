package com.violet.hikikomori;

import android.Manifest;
import android.nfc.Tag;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.databinding.DataBindingUtil;

import com.violet.hikikomori.databinding.ActivityMainBinding;
import com.violet.libmedia.VioletMediaClient;

import pub.devrel.easypermissions.EasyPermissions;

public class MainActivity extends AppCompatActivity implements SurfaceHolder.Callback, View.OnClickListener {

    private final String TAG = MainActivity.class.getSimpleName();
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
    protected void onStop() {
        super.onStop();
        if (client != null) {
            client.stop();
            client = null;
        }
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        Log.d(TAG, "surfaceCreated time=" + System.currentTimeMillis());
        client = new VioletMediaClient();
        client.init(path, holder.getSurface());
        client.onSurfaceCreated(holder.getSurface());
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        Log.d(TAG, "surfaceChanged time=" + System.currentTimeMillis());
        client.onSurfaceChanged(width, height);
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        client.onSurfaceDestroyed(holder.getSurface());
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.play:
                client.play();
                break;
            case R.id.pause:
                client.pause();
                break;
            case R.id.resume:
                client.resume();
                break;
            case R.id.stop:
                client.stop();
                client = null;
                break;
            case R.id.seek:
                client.seekToPosition(50f);
                break;
        }
    }
}