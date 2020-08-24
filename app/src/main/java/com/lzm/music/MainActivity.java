package com.lzm.music;

import android.annotation.SuppressLint;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.camera.core.Camera;
import androidx.camera.core.CameraControl;
import androidx.camera.core.CameraSelector;
import androidx.camera.core.FocusMeteringAction;
import androidx.camera.core.ImageCapture;
import androidx.camera.core.ImageCaptureException;
import androidx.camera.core.MeteringPoint;
import androidx.camera.core.MeteringPointFactory;
import androidx.camera.core.Preview;
import androidx.camera.core.SurfaceOrientedMeteringPointFactory;
import androidx.camera.lifecycle.ProcessCameraProvider;
import androidx.camera.view.PreviewView;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;
import com.google.common.util.concurrent.ListenableFuture;
import com.lzm.player.TimeInfo;
import com.lzm.player.listener.MOnErrorListener;
import com.lzm.player.listener.MOnLoadListener;
import com.lzm.player.listener.MOnPauseResumeListener;
import com.lzm.player.listener.MOnPcmInfoListener;
import com.lzm.player.listener.MOnPreparedListener;
import com.lzm.player.listener.MOnRecordTimeListener;
import com.lzm.player.listener.MOnTimeInfoListener;
import com.lzm.player.listener.MOnValueDBListener;
import com.lzm.player.muteenum.MuteEnum;
import com.lzm.player.myplayer.Mplayer;
import com.lzm.player.log.mylog;
import com.lzm.player.opengl.MGLSurfaceView;
import com.lzm.player.util.MTimeUtil;

import java.io.File;
import java.util.concurrent.Executor;
import java.util.concurrent.TimeUnit;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "lzm";
    private Mplayer mplayer;
    private TextView tvTime;
    //private PreviewView mviewfinder;
    private ImageCapture mImageCapture;
    private MGLSurfaceView mglSurfaceView;
    private int mFacingCam = CameraSelector.LENS_FACING_BACK;//默认打开后摄
    private int REQUEST_CODE_PERMISSIONS = 10; //arbitrary number, can be changed accordingly
    private final String[] REQUIRED_PERMISSIONS = new String[]{"android.permission.CAMERA",
            "android.permission.WRITE_EXTERNAL_STORAGE",
            "android.permission.WRITE_USER_DICTIONARY"};

    private SeekBar seekBar;
    private SeekBar volume_seekBar;
    private int position = 0;//seek位置
    private  boolean isJumpseektime = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mplayer = new Mplayer();
        setContentView(R.layout.activity_main);
        //找到时间轴的布局
        tvTime = findViewById(R.id.tv_time);
        //找到相机view的布局

        seekBar = findViewById(R.id.seekbar_seek);
        volume_seekBar = findViewById(R.id.volume_bar);
        //mviewfinder = findViewById(R.id.viewFinder);
        mglSurfaceView = findViewById(R.id.glsurfaceview);
        mplayer.setMglSurfaceView(mglSurfaceView);


        mplayer.setVolume(50);//默认音量
        //mplayer.setMute(MuteEnum.MUTE_THREED);//默认立体声

        //监听布局变化
        /*mviewfinder.addOnLayoutChangeListener(new View.OnLayoutChangeListener() {
            @Override
            public void onLayoutChange(View view, int i, int i1, int i2, int i3, int i4, int i5, int i6, int i7) {
                updateTransform();
            }
        });
*/


        //监听接口回调，获取C++层回调给JAVA函数的数据
        mplayer.setmOnPreparedListener(new MOnPreparedListener() {
            @Override
            public void onPrepared() {
                mylog.d("onPrepared");
                //ffmpeg开始解码
                mplayer.start();
            }
        });
        //监听接口回调，获取C++层回调给JAVA函数的数据
        mplayer.setmOnLoadListener(new MOnLoadListener() {
            @Override
            public void onLoad(boolean load) {
                if (load) {
                    mylog.d("OnLoading......");
                } else {
                    mylog.d("playing......");
                }
            }
        });
        //监听接口回调，获取C++层回调给JAVA函数的数据
        mplayer.setmOnPauseResumeListener(new MOnPauseResumeListener() {
            @Override
            public void onPause(boolean pause) {
                if (pause) {
                    mylog.d("pausing......");

                } else {
                    mylog.d("playing......");
                }
            }
        });

        //监听接口获得C++回调的播放时间信息并在JAVA层显示
        mplayer.setmOnTimeInfoListener(new MOnTimeInfoListener() {
            @Override
            public void onTimeInfo(TimeInfo timeInfo) {
                Message message = Message.obtain();
                message.what = 1;
                message.obj = timeInfo;
                handler.sendMessage(message);
            }
        });

        //监听接口C++回调给java的信息
        mplayer.setmOnErrorListener(new MOnErrorListener() {
            @Override
            public void OnError(int code, String msg) {
                mylog.d("code:"+code+",msg"+msg);
            }
        });

        //分贝回调
        mplayer.setmOnValueDBListener(new MOnValueDBListener() {
            @Override
            public void onDbValue(int db) {
                //打印底层回调上来的db
                mylog.d("db is %d" + db);
            }
        });
        //录音时间回调
        mplayer.setmOnRecordTimeListener(new MOnRecordTimeListener() {
            @Override
            public void onRecordTime(double recordTime) {
                mylog.d("record time is"+recordTime);
            }
        });
        mplayer.setmOnPcmInfoListener(new MOnPcmInfoListener() {
            @Override
            public void onPcmInfo(byte[] retPcmBuffer, int retPcmBufferSize) {
                mylog.d("retPcmBufferSize is"+retPcmBufferSize);
            }

            @Override
            public void onPcmRate(int samplerate,int bit, int channels) {
                mylog.d("samplerate is"+samplerate);
            }
        });
        //音量seek
        volume_seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                mplayer.setVolume(i);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        //播放seek
        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                //进度条滚动时也会调用此方法，因此必须严格限制仅在手动拖动时才有效
                //bug：后台在播放，但是seekbar出错
                //duration =-1 时 debug时seekbar会卡住，但正常运行OK
                if(mplayer.getDuaration()>0 && isJumpseektime){
                    //总时长*百分比
                    position = mplayer.getDuaration()* i /100;
                }

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                //滑动的时候要时间跳转
                isJumpseektime=true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                //滑动结束后seek
                mplayer.seek(position);
                isJumpseektime=false;
            }
        });
    }


    public void prepared(View view) {

    }
    //开始播放按下
    public void begin(View view) {
        //mplayer.setSource(Environment.getExternalStorageDirectory()+"/test.mp4");
        //mplayer.setSource("/storage/emulated/0/DCIM/Camera/VID_20200824_144409.mp4.tmp);
        mplayer.setSource("http://vjs.zencdn.net/v/oceans.mp4");

        //准备数据并投喂给队列
        mplayer.prepared();
    }

    public void pause(View view) {
        mplayer.pause();
    }

    public void resume(View view) {
        mplayer.resume();
    }

    @SuppressLint("HandlerLeak")
    Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            //没有滑动时打印播放时间

            super.handleMessage(msg);
            if (msg.what == 1) {
                if(!isJumpseektime){
                    TimeInfo timeInfo = (TimeInfo) msg.obj;
                    tvTime.setText(MTimeUtil.secdsToDateFormat(timeInfo.getTotalTime(), timeInfo.getTotalTime())
                            + "/" + MTimeUtil.secdsToDateFormat(timeInfo.getCurrentTime(), timeInfo.getTotalTime()));
                    //当前进度*100/总时间
                    seekBar.setProgress(timeInfo.getCurrentTime()*100/timeInfo.getTotalTime());
                }
            }
        }
    };

    public void stop(View view) {
        mplayer.stop();
    }
    //需要改进seek方法
    public void seek(View view) {
        mplayer.seek(200);
    }

    //播放下一个
    public void next(View view) {
        //mplayer.playNext("/sdcard/netease/cloudmusic/Music/蔡健雅 - 红色高跟鞋.mp3");
    }



   /* //获取权限并开启摄像头
    public void startCap(View view){
        if(allPermissionGranted()){
            startCamera();

        }else{
            ActivityCompat.requestPermissions(this, REQUIRED_PERMISSIONS, REQUEST_CODE_PERMISSIONS);
            Toast.makeText(this,"error permission",Toast.LENGTH_SHORT).show();
        }
    }
    //开启摄像头
    public void startCamera(){
        final ListenableFuture<ProcessCameraProvider> cameraProviderFuture = ProcessCameraProvider.getInstance(this);
        cameraProviderFuture.addListener(new Runnable() {
            @SuppressLint({"RestrictedApi", "ClickableViewAccessibility"})
            @Override
            public void run() {
                try{

                    //将相机生命周期与活动的生命周期绑定，camerax自己释放
                    ProcessCameraProvider cameraProvider = cameraProviderFuture.get();

                    //预览的capture，支持角度换算
                    Preview preview = new Preview.Builder().build();

                    //创建图像的capture
                    mImageCapture = new ImageCapture.Builder()
                            .setCaptureMode(ImageCapture.CAPTURE_MODE_MAXIMIZE_QUALITY)
                            .setFlashMode(ImageCapture.FLASH_MODE_AUTO)
                            .build();

                    //选择后置摄像头
                    CameraSelector cameraSelector = new CameraSelector.Builder().requireLensFacing(mFacingCam).build();
                    //释放可能存在的预览
                    cameraProvider.unbindAll();
                    //将数据绑定到相机的生命周期中
                    Camera camera = cameraProvider.bindToLifecycle(MainActivity.this,cameraSelector,preview,mImageCapture);
                    //控制对焦
                    final CameraControl cameraControl = camera.getCameraControl();

                    mviewfinder.setOnTouchListener(new View.OnTouchListener() {
                        @Override
                        public boolean onTouch(View view, MotionEvent motionEvent) {

                            Executor executor = null;

                            MeteringPointFactory factory = new SurfaceOrientedMeteringPointFactory(mviewfinder.getWidth(),mviewfinder.getHeight());
                            Log.d("touchlocation:", String.valueOf(motionEvent.getX()));
                            MeteringPoint point  =factory.createPoint(motionEvent.getX(),
                                    motionEvent.getY());

                            FocusMeteringAction action = new FocusMeteringAction.Builder(point,FocusMeteringAction.FLAG_AF)
                                    .setAutoCancelDuration(1, TimeUnit.SECONDS)
                                    .build();
                            ListenableFuture future =cameraControl.startFocusAndMetering(action);

                            *//*future.addListener(()->{
                                try {
                                    FocusMeteringResult result = (FocusMeteringResult) future.get();
                                    if(result.isFocusSuccessful()){
                                        Log.d("success focus", String.valueOf(result.isFocusSuccessful()));
                                    }
                                }catch (Exception e){

                                }
                            }, executor);*//*

                            return false;
                        }
                    });



                    //将预览的surface给相机预览
                    preview.setSurfaceProvider(mviewfinder.createSurfaceProvider());

                }catch (Exception e){
                    e.printStackTrace();
                }

            }
        }, ContextCompat.getMainExecutor(this));
    }

    //更新布局
    private void updateTransform(){

    }

    public void takePhoto(View view){
        if(mImageCapture!=null){
            //create folder
            *//*File dir = new File(getExternalCacheDir()"/camlzmx");
            if(!dir.exists()){
                dir.mkdirs();
            }*//*
            //create files
            final File file = new File(getExternalCacheDir()+"/lzm"+System.currentTimeMillis()+".jpg");
            Log.d("Main",getExternalCacheDir()+"/lzm"+System.currentTimeMillis()+".jpg");
            if(file.exists()){
                file.delete();
            }

            //create pkg data
            ImageCapture.OutputFileOptions outputFileOptions = new ImageCapture.OutputFileOptions.Builder(file).build();
            //take photo
            mImageCapture.takePicture(outputFileOptions,
                    ContextCompat.getMainExecutor(this),
                    new ImageCapture.OnImageSavedCallback(){
                        @Override
                        public void onImageSaved(@NonNull ImageCapture.OutputFileResults outputFileResults) {
                            //回调信息
                            Toast.makeText(MainActivity.this,"save success in"+file.getAbsolutePath(),
                                    Toast.LENGTH_SHORT).show();
                        }

                        @Override
                        public void onError(@NonNull ImageCaptureException exception) {
                            //出错回调
                            Toast.makeText(MainActivity.this,"save failed",
                                    Toast.LENGTH_SHORT).show();
                        }
                    });
        }
    }

    public void switchCamera(View view){
        mFacingCam = mFacingCam==CameraSelector.LENS_FACING_FRONT ?
                CameraSelector.LENS_FACING_BACK : CameraSelector.LENS_FACING_FRONT;
        startCamera();
    }
*/
    /*//申请权限的回调
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        //start camera when permissions have been granted otherwise exit app
        if(requestCode == REQUEST_CODE_PERMISSIONS){
            if(allPermissionGranted()){
                startCamera();
            } else{
                Toast.makeText(this, "Permissions not granted by the user.", Toast.LENGTH_SHORT).show();
                finish();
            }
        }
    }

    //向用户申请权限
    public boolean allPermissionGranted() {

        for (String permission : REQUIRED_PERMISSIONS) {
            if (ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }

        return true;
    }
*/
    //左右声道切换
    public void turnleft(View view) {
        mplayer.setMute(MuteEnum.MUTE_LEFT);
    }

    public void turnright(View view) {
        mplayer.setMute(MuteEnum.MUTE_RIGHT);
    }

    public void threed(View view) {
        mplayer.setMute(MuteEnum.MUTE_THREED);
    }
}
