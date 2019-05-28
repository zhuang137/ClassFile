package com.whulzz.classfile;

import android.content.res.AssetManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    public void btnOnClick(View view) {
        try {
            String apkPath = getApplicationInfo().sourceDir;
            ZipFile zipFile = new ZipFile(apkPath);
            ZipEntry entry = zipFile.getEntry("assets/Test.class");
            File extractToPath = new File(getApplicationInfo().dataDir, "Test.class");
            if (!extractToPath.exists()) {
                extractToPath.createNewFile();

                if (entry != null) {
                    InputStream is = zipFile.getInputStream(entry);
                    OutputStream os = new BufferedOutputStream(new FileOutputStream(extractToPath));

                    byte[] buf = new byte[8192];
                    int size = 0;
                    while ((size = is.read(buf)) != -1) {
                        os.write(buf, 0, size);
                    }
                    is.close();
                    os.flush();
                    os.close();
                }
            }
            if (extractToPath.exists()) {
                MyApplication.init(extractToPath.getAbsolutePath());
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
