package com.xysantai.myisp;

import android.app.Application;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.res.Configuration;

import java.io.File;
import java.util.Locale;

public class MyApplication extends Application {
    public static String ispFile ;
    public static String apkFile;
    public static String hostUrl = "http://www.xysantai.com/xyst/upload";

    public static String packageName = "com.xyst.mbt2023";
    public static String className1 = "com.xyst.mbt2023.HomeActivity";
    public static String className2 = "com.xyst.mbt2023.IplActivity";
    public static String className3 = "com.xyst.mbt2023.MainHrActivity";
    public static String className4 = "com.xyst.mbt2023.MainShrActivity";
    public static String SHARED_PREFERENCES_NAME = "hair-removal";
    public static String mHairDensity ="";
    public static String mHairColor ="";
    public static String mSkinColor ="";
    public static String mSkinColorLevel ="";
    public static String mSkinGlossLevel ="";

    private static Context appcontext;
    private static MyApplication instance;

    public MyApplication() {
        instance = this;
    }

    public static Context getContext() {
        if (instance == null) {
            synchronized (MyApplication.class) {
                if (instance == null) {
                    instance = new MyApplication();
                }
            }
        }
        return instance;
    }


    @Override
    public void onCreate() {
        super.onCreate();

        appcontext = this.getApplicationContext();
        //default english
        Configuration config = getResources().getConfiguration();
        config.locale = Locale.ENGLISH;
        //config.locale = Locale.CHINA;
        getResources().updateConfiguration(config, getResources().getDisplayMetrics());
        
        SharedPreferences sp = getSharedPreferences("UpgradeFiles", MODE_PRIVATE);
        ispFile = sp.getString("IspFilename", "VER_1_A_B286.bin");
        apkFile = sp.getString("ApkFilename", "VER_1_500~800WUART40J.apk");
    }



    public static boolean isAppInstalled(Context context, String pgName) {
        PackageManager pm = context.getPackageManager();
        try {
            pm.getPackageInfo(pgName, PackageManager.GET_ACTIVITIES);
            return true;
        } catch (PackageManager.NameNotFoundException e) {
            return false;
        }
    }

    public static boolean delFile(String path) {
        if (path != null) {
            File file = new File(path);
            if (file.exists()) {
                if (file.delete()) {
                    return true;
                } else {
                    return false;
                }
            }
        }
        return true;
    }







}
