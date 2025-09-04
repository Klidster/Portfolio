package com.example.psyks15cw;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import androidx.activity.EdgeToEdge;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.Manifest;
import android.os.Build;

import java.io.File;
import java.util.ArrayList;

public class MainActivity extends AppCompatActivity {
    private final ArrayList<AudioC> musicList = new ArrayList<>();
    private static final String TAG = "MusicLoader";

    public static final int MY_REQUEST_CODE = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_main);


        if(ContextCompat.checkSelfPermission(this, Manifest.permission.READ_MEDIA_AUDIO) == PackageManager.PERMISSION_DENIED) {
            permissionReq.launch(new ActivityResultContracts.RequestPermission().toString());
        }

        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
        load();

        RecyclerView view = findViewById(R.id.recyclerView);
        view.setLayoutManager(new LinearLayoutManager(this));
        RecyclerViewAdapter adp = new RecyclerViewAdapter(this,musicList);
        view.setAdapter(adp);
        Button sendButton = findViewById(R.id.button);

        sendButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                Intent intent = new Intent(MainActivity.this, settingsActivity.class);
                startActivityForResult (intent, MY_REQUEST_CODE);

            }
        });

    }

    public void load() {
        File musicFolder = new File("/storage/self/primary/Music");
        Log.d(TAG, "Checking if music folder exists...");

        if (musicFolder.exists() && musicFolder.isDirectory()) {
            Log.i(TAG, "Music folder found: " + musicFolder.getAbsolutePath());
            File[] files = musicFolder.listFiles();
            int i = 0;

            if (files == null || files.length == 0) {
                Log.e(TAG, "No files found in the directory.");
                return;
            }

            while (i < files.length) {
                File file = files[i];
                Log.d(TAG, "Checking file: " + file.getName());

                // Ensure it's a file and ends with .mp3
                if (file.isFile() && file.getName().toLowerCase().endsWith(".mp3")) {
                    Log.i(TAG, "Music file found: " + file.getName());
                    musicList.add(new AudioC(file.getName(), file.getPath()));
                } else if (file.isDirectory()) {
                    Log.d(TAG, "Skipping directory: " + file.getName());
                } else {
                    Log.d(TAG, "Skipping non-MP3 file: " + file.getName());
                }
                i++;
            }
        } else {
            Log.e(TAG, "The specified folder does not exist or is not a directory.");
        }

        Log.i(TAG, "Music loading complete. Total MP3 files: " + musicList.size());
    }

    public final ActivityResultLauncher<String> permissionReq =
            registerForActivityResult(new ActivityResultContracts.RequestPermission(), isGranted ->{
               if(isGranted) {
                   Log.d("PermStatus","Granted");
               } else {
                   Log.d("PermStatus", "Denied");
               }
            });
}

