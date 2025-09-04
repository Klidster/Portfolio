package com.example.psyks15cw;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.ToggleButton;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import java.util.List;
import java.util.Locale;

public class AudioBookPlayerPage extends AppCompatActivity {

    private String fileName;
    private String filePath;
    private SeekBar seekBar;
    private TextView currentTimeTextView;
    private TextView totalTimeTextView;
    private AudioBookPlayerService audioBookService;
    private boolean isBound = false;
    private List<AudioC> audioList; // List of all audio files
    private int currentIndex; // Current index in the list
    private Handler handler = new Handler();
    private Runnable updateSeekBarRunnable;

    private final ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            AudioBookPlayerService.LocalBinder binder = (AudioBookPlayerService.LocalBinder) service;
            audioBookService = binder.getService();
            isBound = true;
            initSeekBar();
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            isBound = false;
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_book_player);

        // Get the saved background color preference
        boolean isGreyBackground = getSharedPreferences("Settings", MODE_PRIVATE)
                .getBoolean("is_grey_background", false);

        // Apply the background color to the root layout
        findViewById(R.id.root_layout).setBackgroundColor(
                isGreyBackground ? getResources().getColor(android.R.color.darker_gray)
                        : getResources().getColor(android.R.color.white));

        // Initialize UI elements
        TextView playbackSpeedTextView = findViewById(R.id.playback_speed);
        seekBar = findViewById(R.id.seekBar);
        TextView fileInfoTextView = findViewById(R.id.textView3);
        currentTimeTextView = findViewById(R.id.current_time);
        totalTimeTextView = findViewById(R.id.total_time);
        ToggleButton playPauseToggle = findViewById(R.id.play_pause_toggle);
        Button backButton = findViewById(R.id.back_button);
        Button nextButton = findViewById(R.id.skip_button);

        // Get the initial playback speed from SharedPreferences
        float playbackSpeed = getSharedPreferences("Settings", MODE_PRIVATE)
                .getFloat("playback_speed", 1.0f);
        playbackSpeedTextView.setText(String.format("Speed: %.1fx", playbackSpeed));

        // Retrieve audio list and current index from Intent
        audioList = getIntent().getParcelableArrayListExtra("audioList");
        currentIndex = getIntent().getIntExtra("currentIndex", 0);

        // Display the current audio file name
        if (audioList != null && currentIndex < audioList.size()) {
            AudioC currentAudio = audioList.get(currentIndex);
            fileName = currentAudio.getFilename();
            filePath = currentAudio.getFilepath();
            fileInfoTextView.setText("File Name: " + fileName);
        }

        // Set up play/pause toggle button
        playPauseToggle.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (audioBookService != null) {
                if (isChecked) {
                    audioBookService.play();
                } else {
                    audioBookService.pause();
                }
            }
        });

        // Set up back button
        backButton.setOnClickListener(v -> finish());

        // Set up next button
        nextButton.setOnClickListener(v -> playNextSong());

        // Set up SeekBar listener
        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser && audioBookService != null) {
                    audioBookService.seekTo(progress);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        // Bind to the service
        Intent serviceIntent = new Intent(this, AudioBookPlayerService.class);
        serviceIntent.putExtra("filePath", filePath);
        bindService(serviceIntent, serviceConnection, Context.BIND_AUTO_CREATE);
    }

    // Handle results from the settings page
    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == 100 && resultCode == Activity.RESULT_OK && data != null) {
            // Update playback speed
            float updatedSpeed = data.getFloatExtra("playback_speed", 1.0f);
            TextView playbackSpeedTextView = findViewById(R.id.playback_speed);
            playbackSpeedTextView.setText(String.format("Speed: %.1fx", updatedSpeed));

            if (audioBookService != null) {
                audioBookService.setPlaybackSpeed(updatedSpeed);
            }

            // Update background color
            boolean isGreyBackground = getSharedPreferences("Settings", MODE_PRIVATE)
                    .getBoolean("is_grey_background", false);
            findViewById(R.id.root_layout).setBackgroundColor(
                    isGreyBackground ? getResources().getColor(android.R.color.darker_gray)
                            : getResources().getColor(android.R.color.white));
        }
    }

    // Play the next song in the list
    private void playNextSong() {
        if (audioList != null && currentIndex < audioList.size() - 1) {
            currentIndex++;
            AudioC nextAudio = audioList.get(currentIndex);

            // Stop the current playback
            if (audioBookService != null) {
                audioBookService.stop();
            }

            // Update UI and start playback for the next song
            fileName = nextAudio.getFilename();
            filePath = nextAudio.getFilepath();
            TextView fileInfoTextView = findViewById(R.id.textView3);
            fileInfoTextView.setText("File Name: " + fileName);

            Intent serviceIntent = new Intent(this, AudioBookPlayerService.class);
            serviceIntent.putExtra("filePath", filePath);
            startService(serviceIntent);

            // Reinitialize SeekBar
            initSeekBar();
        }
    }

    // Initialize the SeekBar
    private void initSeekBar() {
        if (audioBookService != null) {
            int duration = audioBookService.getDuration();
            seekBar.setMax(duration);
            seekBar.setProgress(0);
            totalTimeTextView.setText(formatTime(duration));
            currentTimeTextView.setText(formatTime(0));

            // Update SeekBar progress periodically
            updateSeekBarRunnable = new Runnable() {
                @Override
                public void run() {
                    if (audioBookService != null) {
                        int currentPosition = audioBookService.getCurrentPosition();
                        seekBar.setProgress(currentPosition);
                        currentTimeTextView.setText(formatTime(currentPosition));
                        handler.postDelayed(this, 1000);
                    }
                }
            };
            handler.post(updateSeekBarRunnable);
        }
    }

    // Format time in mm:ss
    private String formatTime(int millis) {
        int minutes = (millis / 1000) / 60;
        int seconds = (millis / 1000) % 60;
        return String.format(Locale.getDefault(), "%02d:%02d", minutes, seconds);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (isBound) {
            unbindService(serviceConnection);
            isBound = false;
        }
        if (handler != null && updateSeekBarRunnable != null) {
            handler.removeCallbacks(updateSeekBarRunnable);
        }
    }
}
