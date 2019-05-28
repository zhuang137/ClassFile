package com.whulzz.classfile;

import android.app.Application;

public class MyApplication extends Application {

    static {
        System.loadLibrary("clfreader");
    }

    public static native void init(String path);
}
