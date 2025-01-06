package com.xysantai.myisp;

import static com.xysantai.myisp.MyApplication.SHARED_PREFERENCES_NAME;
import static com.xysantai.myisp.MyApplication.className1;
import static com.xysantai.myisp.MyApplication.mHairColor;
import static com.xysantai.myisp.MyApplication.mHairDensity;
import static com.xysantai.myisp.MyApplication.mSkinColor;
import static com.xysantai.myisp.MyApplication.mSkinColorLevel;
import static com.xysantai.myisp.MyApplication.mSkinGlossLevel;
import static com.xysantai.myisp.MyApplication.packageName;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.core.content.FileProvider;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.ProgressDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.serialport.SerialPortFinder;
import android.os.Bundle;
import android.os.Environment;
import android.text.InputType;
import android.text.Layout;
import android.text.TextUtils;
import android.text.format.Formatter;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;


import com.lzy.okgo.OkGo;
import com.lzy.okgo.callback.FileCallback;
import com.lzy.okgo.model.Response;
import com.lzy.okgo.model.Progress;
import com.lzy.okgo.request.base.Request;


import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.channels.FileChannel;
import java.util.Locale;
import java.util.Random;


public class MainActivity extends AppCompatActivity implements AdapterView.OnItemSelectedListener {

    static {
        System.loadLibrary("myisp");
    }

    public native int programFlashFromJNI(String filePath, String ssId, String baudRate);
    public native void showAlertFromJNI(String message);
    public native String getShowMessage();
    public native String showResponseMsgFromJNI(String message);
    public native String showChipTypeFromJNI(String message);
    public native void showChipTypeFromJNI();

    public void showChipTypeMessage(String message) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mChipType.setText(message);
                mShowReceiveTxt.requestLayout();
            }
        });
    }

    public void showAlert(String message) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(MainActivity.this, message, Toast.LENGTH_LONG).show();
            }
        });
    }

    private Handler mainHandler = new Handler(Looper.getMainLooper());
    public void showReponseMessage(final String message) {

        mainHandler.post(new Runnable() {
            @Override
            public void run() {
                if (message != null){
                    receiveTxt.append(message).append("\n");
                    mShowReceiveTxt.setText(receiveTxt.toString());
                    mShowReceiveTxt.setHorizontallyScrolling(false);
                    mShowReceiveTxt.setMaxLines(Integer.MAX_VALUE);

                    svResult.post(new Runnable() {
                        public void run() {
                            svResult.fullScroll(ScrollView.FOCUS_DOWN);
                        }
                    });
                }
            }
        });
    }

    private Handler mainProgressBarHandler = new Handler(Looper.getMainLooper());
    private volatile int currentProgress = 0;

    public void showProgressBar(final int progressStatus) {
        progressBar.setVisibility(View.VISIBLE);
        currentProgress = progressStatus;
        mainProgressBarHandler.post(new Runnable() {
            @Override
            public void run() {
                progressBar.setProgress(progressStatus);
            }
        });
    }

    public void setProgressMax(final int maxValue) {
        mainProgressBarHandler.post(new Runnable() {
            @Override
            public void run() {
                progressBar.setMax(maxValue);
            }
        });
    }


    private Button btnStart;
    private Button btnBinFileDownload;

    private Button btnDownloadAPK;
    private Button btnInstallAPK;

    private TextView mShowReceiveTxt;

    private  ProgressBar progressBar;
    private TextView tvProgress;

    private EditText mChipType;
    private EditText mOscillator;
    private EditText mHexFile;
    private EditText mFileSize;

    private EditText mApkFile;

    private String mHexFilename;
    private String mApkFilename;

    private Spinner mPathSpinner;
    private Spinner mBaudRateSpinner;
    private ScrollView svResult;

    private Device mDevice;

    private int mDeviceIndex;
    private int mBaudrateIndex;

    private String[] mDevices;
    private String[] mBaudrates;

    ProgressDialog pd = null;
    private StringBuilder receiveTxt = new StringBuilder();

    private static final int REQUEST_PERMISSION_STORAGE = 0x01;

    private String ispFile;
    private String apkFile;


    protected void checkSDCardPermission() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, REQUEST_PERMISSION_STORAGE);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_PERMISSION_STORAGE) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            } else {
                Toast.makeText(MainActivity.this,getResources().getString(R.string.no_download_file_permissions), Toast.LENGTH_LONG).show();
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        OkGo.getInstance().cancelTag(this);
        mainProgressBarHandler.removeCallbacksAndMessages(null);
        mainHandler.removeCallbacksAndMessages(null);

        String destPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/download/apk";
        String fileName = MyApplication.apkFile;
        File destFile = new File(destPath + "/" + fileName);
        if (destFile.exists())
            destFile.delete();

        destPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/download/bin";
        fileName = MyApplication.ispFile;
        destFile = new File(destPath + "/" + fileName);
        if (destFile.exists())
            destFile.delete();

    }



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Configuration config = getResources().getConfiguration();
        config.locale = Locale.ENGLISH;
        getResources().updateConfiguration(config, getResources().getDisplayMetrics());

        initView();
        checkSDCardPermission();
        OkGo.getInstance().init(getApplication());

    }

    void initView() {
        tvProgress = findViewById(R.id.tvProgress);
        progressBar = findViewById(R.id.progressBar);

        btnDownloadAPK = findViewById(R.id.btnDownloadAPK);
        btnInstallAPK = findViewById(R.id.btnInstallAPK);
        btnInstallAPK.setEnabled(false);
        btnStart = findViewById(R.id.btnStart);
        btnBinFileDownload = findViewById(R.id.btnDownload);
        mShowReceiveTxt = findViewById(R.id.sph_showReceiveTxt);
        mShowReceiveTxt.setHorizontallyScrolling(false);
        mShowReceiveTxt.setMaxLines(Integer.MAX_VALUE);
        mChipType = findViewById(R.id.editChipType);
        mOscillator = findViewById(R.id.editOscillator);
        mHexFile = findViewById(R.id.editHexFile);
        mFileSize = findViewById(R.id.editHexFileSize);
        mApkFile = findViewById(R.id.editApkFile);
        svResult = findViewById(R.id.svResult);
        mChipType.setEnabled(false);
        mHexFile.setEnabled(false);
        mApkFile.setEnabled(false);
        mOscillator.setEnabled(false);
        mFileSize.setEnabled(false);
        mPathSpinner = findViewById(R.id.sph_path);
        mPathSpinner.setSelection(0);
        mBaudRateSpinner = findViewById(R.id.sph_baudRate);
        mBaudRateSpinner.setSelection(6);
        mBaudRateSpinner.setOnItemSelectedListener(this);


        SerialPortFinder serialPortFinder = new SerialPortFinder();
        mDevices = serialPortFinder.getAllDevicesPath();
        if (mDevices.length == 0) {
            mDevices = new String[]{getString(R.string.no_serial_device)};
        }
        mBaudrates = getResources().getStringArray(R.array.baud_rate_arr);
        mDevice = new Device(mDevices[0], mBaudrates[6]);

        ArrayAdapter<String> deviceAdapter = new ArrayAdapter<String>(this, R.layout.spinner_default_item, mDevices);
        deviceAdapter.setDropDownViewResource(R.layout.spinner_item);
        mPathSpinner.setAdapter(deviceAdapter);
        mPathSpinner.setOnItemSelectedListener(this);
        mPathSpinner.setSelection(0);
    }

    public void btnDownloadAPKClickHandler(View view){
        ConnectivityManager cm = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo activeNetwork = cm.getActiveNetworkInfo();
        boolean isConnected = activeNetwork != null && activeNetwork.isConnectedOrConnecting();
        if(!isConnected){
            Toast.makeText(MainActivity.this, getResources().getString(R.string.str_network_err), Toast.LENGTH_SHORT).show();
            return;
        }

        SharedPreferences sp = getSharedPreferences("UpgradeFiles", MODE_PRIVATE);
        apkFile = sp.getString("ApkFilename", "");
        MyApplication.apkFile = apkFile;

        if(apkFile.isEmpty()){
            Toast.makeText(MainActivity.this, getResources().getString(R.string.apkfile_not_exist) , Toast.LENGTH_SHORT).show();
            return;
        }


        final String destPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/download/apk";
        final String fileName = MyApplication.apkFile;
        File destFile = new File(destPath + "/" + fileName);
        if (destFile.exists())
            destFile.delete();


        String downLoadUrl = MyApplication.hostUrl + "/" + MyApplication.apkFile ;
        OkGo.<File>post(downLoadUrl)
                .tag(this)
                .execute(new FileCallback(destPath,fileName) {
                    @Override
                    public void onStart(Request<File, ? extends Request> request) {
                        btnDownloadAPK.setEnabled(false);
                        progressBar.setVisibility(View.VISIBLE);
                        tvProgress.setVisibility(View.VISIBLE);
/*                        pd = new ProgressDialog(MainActivity.this);
                        pd.setMessage("Downloading ......");
                        pd.show();*/
                    }

                    @Override
                    public void downloadProgress(Progress progress) {
                        super.downloadProgress(progress);
                        int percent = (int) (progress.fraction * 100);
                        progressBar.setProgress(percent);

                        String speed = Formatter.formatFileSize(MainActivity.this, progress.speed);
                        String downloaded = Formatter.formatFileSize(MainActivity.this, progress.currentSize);
                        String total = Formatter.formatFileSize(MainActivity.this, progress.totalSize);
                        tvProgress.setText(String.format("%d%% %s/s %s/%s", percent, speed, downloaded, total));

                    }

                    @Override
                    public void onSuccess(Response<File> response) {
                        btnDownloadAPK.setEnabled(true);
                        btnInstallAPK.setEnabled(true);
                        mApkFilename = response.body().getAbsolutePath();
                        File akpFile = response.body().getAbsoluteFile();
                        mApkFile.setText(mApkFilename);
                        progressBar.setVisibility(View.INVISIBLE);
                        tvProgress.setVisibility(View.INVISIBLE);
                        //pd.dismiss();
                    }

                    @Override
                    public void onError(Response<File> response) {
                        Toast.makeText(MainActivity.this, getResources().getString(R.string.action_prompt_downloading_failed) + response.message(), Toast.LENGTH_SHORT).show();
                        btnDownloadAPK.setEnabled(true);
                        btnInstallAPK.setEnabled(false);
                        progressBar.setVisibility(View.GONE);
                        tvProgress.setVisibility(View.GONE);
                        //pd.dismiss();
                    }
                });

    }

    public void btnInstallAPKClickHandler(View view){

        AlertDialog.Builder dialog = new AlertDialog.Builder (MainActivity.this);
        dialog.setTitle(getResources().getString(R.string.str_install_apk));
        dialog.setMessage(getResources().getString(R.string.str_ask_newver_apk_install));
        dialog.setCancelable(false);
        dialog.setPositiveButton("OK", new DialogInterface. OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                installAPK();
            }
        });
        dialog.setNegativeButton("Cancel", new DialogInterface. OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                //
            }
        });
        dialog.show();
    }

    private void installAPK() {
        StringBuilder stringBuilder;
        File file = new File(this.mApkFilename);
        Uri uri = Uri.fromFile(file);
        if (!file.exists()){
            Toast.makeText(MainActivity.this, getResources().getString(R.string.apkfile_not_exist), Toast.LENGTH_LONG).show();
            return;
        }
        Intent intent = new Intent("android.intent.action.VIEW");
        if (Build.VERSION.SDK_INT >= 24) {
            intent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            stringBuilder = new StringBuilder();
            stringBuilder.append(getPackageName());
            stringBuilder.append(".fileprovider");
            intent.setDataAndType(FileProvider.getUriForFile((Context)this, stringBuilder.toString(), file), "application/vnd.android.package-archive");
        } else {
            intent.setDataAndType((Uri)uri, "application/vnd.android.package-archive");
            intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        }
        startActivity(intent);
    }

    public void btnDownloadClickHandler(View view) {

        ConnectivityManager cm = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo activeNetwork = cm.getActiveNetworkInfo();
        boolean isConnected = activeNetwork != null && activeNetwork.isConnectedOrConnecting();
        if(!isConnected){
            Toast.makeText(MainActivity.this, getResources().getString(R.string.str_network_err), Toast.LENGTH_SHORT).show();
            return;
        }

        SharedPreferences sp = getSharedPreferences("UpgradeFiles", MODE_PRIVATE);
        ispFile = sp.getString("IspFilename", "");
        MyApplication.ispFile = ispFile;

        if(ispFile.isEmpty()){
            Toast.makeText(MainActivity.this, getResources().getString(R.string.ispfile_not_exist) , Toast.LENGTH_SHORT).show();
            return;
        }

        final String destPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/download/bin";
        final String fileName = MyApplication.ispFile;
        File destFile = new File(destPath + "/" + fileName);
        if (destFile.exists())
            destFile.delete();


        String downLoadUrl = MyApplication.hostUrl + "/" + MyApplication.ispFile ;
        OkGo.<File>post(downLoadUrl)
                .tag(this)
                .execute(new FileCallback(destPath,fileName) {
                    @Override
                    public void onStart(Request<File, ? extends Request> request) {
                        btnBinFileDownload.setEnabled(false);
                        progressBar.setVisibility(View.VISIBLE);
                        tvProgress.setVisibility(View.VISIBLE);
                        //pd = new ProgressDialog(MainActivity.this);
                        //pd.setMessage("Downloading ......");
                        //pd.show();
                    }

                    @Override
                    public void downloadProgress(Progress progress) {
                        super.downloadProgress(progress);
                        int percent = (int) (progress.fraction * 100);
                        progressBar.setProgress(percent);

                        String speed = Formatter.formatFileSize(MainActivity.this, progress.speed);
                        String downloaded = Formatter.formatFileSize(MainActivity.this, progress.currentSize);
                        String total = Formatter.formatFileSize(MainActivity.this, progress.totalSize);
                        tvProgress.setText(String.format("%d%% %s/s %s/%s", percent, speed, downloaded, total));


                        double kbsize = 0;
                        if(progress.totalSize>0){
                            kbsize = progress.totalSize/1024 ;
                            mFileSize.setText(String.valueOf(kbsize));
                        }
                    }

                    @Override
                    public void onSuccess(Response<File> response) {
                     //   Toast.makeText(MainActivity.this, getResources().getString(R.string.action_prompt_downloading_success), Toast.LENGTH_SHORT).show();
                        btnBinFileDownload.setEnabled(true);
                        mHexFilename = response.body().getAbsolutePath();
                        File hexFile = response.body().getAbsoluteFile();
                        mHexFile.setText(mHexFilename);
                        progressBar.setVisibility(View.INVISIBLE);
                        tvProgress.setVisibility(View.INVISIBLE);
                        //pd.dismiss();
                    }

                    @Override
                    public void onError(Response<File> response) {
                      //  Toast.makeText(MainActivity.this, getResources().getString(R.string.action_prompt_downloading_failed) + response.message(), Toast.LENGTH_SHORT).show();
                        btnBinFileDownload.setEnabled(true);
                        progressBar.setVisibility(View.GONE);
                        tvProgress.setVisibility(View.GONE);
                        //pd.dismiss();
                    }
                });


    }

    public void btnStartClickHandler(View view) {
        AlertDialog.Builder dialog = new AlertDialog.Builder (MainActivity.this);
        dialog.setTitle(getResources().getString(R.string.str_upgrade_bin));
        dialog.setMessage(getResources().getString(R.string.str_ask_newver_bin_install));
        dialog.setCancelable(false);
        dialog.setPositiveButton("OK", new DialogInterface. OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                StartProgramFlashProc();
            }
        });
        dialog.setNegativeButton("Cancel", new DialogInterface. OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                //
            }
        });
        dialog.show();

    }

    private void StartProgramFlashProc(){
        String ttySSIdStr = mPathSpinner.getSelectedItem().toString();
        String baudRate = mBaudRateSpinner.getSelectedItem().toString();

        File hexFile = new File(mHexFilename);
        if (!hexFile.exists()){
            Toast.makeText(MainActivity.this, getResources().getString(R.string.file_not_exist), Toast.LENGTH_LONG).show();
            return;
        }

        if (ttySSIdStr.isEmpty()) {
            Toast.makeText(MainActivity.this, getResources().getString(R.string.comport_no_empty), Toast.LENGTH_LONG).show();
            return;
        }
        if (baudRate.isEmpty()) {
            Toast.makeText(MainActivity.this, getResources().getString(R.string.bauaerate_no_empty), Toast.LENGTH_LONG).show();
            return;
        }

        btnStart.setEnabled(false);

        final String ttySSId = "1" ;
/*
        switch (ttySSIdStr) {
            case "/dev/ttyS0":
                ttySSId = "0";
                break;
            case "/dev/ttyS1":
                ttySSId = "1";
                break;
            case "/dev/ttyS2":
                ttySSId = "2";
                break;
            case "/dev/ttyS3":
                ttySSId = "3";
                break;
            case "/dev/ttyS4":
                ttySSId = "4";
                break;
            default:
                ttySSId = "1";
        }
*/
        new Thread(new Runnable() {
            @Override
            public void run() {
                int result = programFlashFromJNI(mHexFilename, ttySSId, baudRate);
            }
        }).start();

    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        switch (parent.getId()) {
            case R.id.sph_path:
                mDeviceIndex = position;
                mDevice.setPath(mDevices[mDeviceIndex]);
                break;
            case R.id.sph_baudRate:
                mBaudrateIndex = position;
                mDevice.setBaudrate(mBaudrates[mBaudrateIndex]);
                break;
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> parent) {
        //
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        if (id == R.id.menu_goback) {
/*            InputDialog dialog = new InputDialog(MainActivity.this);
            dialog.show();*/
            goBack();
            return true;
        } else {
            return super.onOptionsItemSelected(item);
        }
    }

    private void goBack(){
        if (MyApplication.isAppInstalled(MainActivity.this, packageName)) {
            startDiodeLaser();
        } else {
            Toast.makeText(MainActivity.this, getResources().getString(R.string.package_not_exist), Toast.LENGTH_SHORT).show();
        }
    }

    public void startDiodeLaser() {
        SharedPreferences spf = getSharedPreferences(SHARED_PREFERENCES_NAME, Context.MODE_PRIVATE);
        if(spf!=null){
            mHairDensity = spf.getString("HairDensity", "");
            mHairColor = spf.getString("HairColor", "");
            mSkinColor = spf.getString("SkinColor", "");
            mSkinColorLevel = spf.getString("SkinColorLevel", "");
            mSkinGlossLevel = spf.getString("SkinGlossLevel", "");
        }

        Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.setComponent(new ComponentName(packageName, className1));
        intent.putExtra("HairDensity", mHairDensity);
        intent.putExtra("HairColor", mHairColor);
        intent.putExtra("SkinColor", mSkinColor);
        intent.putExtra("SkinColorLevel", mSkinColorLevel);
        intent.putExtra("SkinGlossLevel", mSkinGlossLevel);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        startActivity(intent);

    }

    @Override
    public void onBackPressed() {
        // 弹出提示框
        new AlertDialog.Builder(this)
                .setMessage("确定要退出吗？")
                .setPositiveButton("是", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        MainActivity.super.onBackPressed();
                    }
                })
                .setNegativeButton("否", null)
                .show();
    }




}