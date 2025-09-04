package com.example.psyks15cw;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

public class settingsActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        // Playback speed slider
        SeekBar playbackSpeedSlider = findViewById(R.id.playback_speed_slider);
        TextView playbackSpeedValue = findViewById(R.id.playback_speed_value);

        // Load initial playback speed from SharedPreferences
        float initialSpeed = getSharedPreferences("Settings", MODE_PRIVATE)
                .getFloat("playback_speed", 1.0f);

        // Set slider position and text
        playbackSpeedSlider.setProgress((int) ((initialSpeed - 0.5f) * 10)); // Map 0.5-2.0 to 0-15
        playbackSpeedValue.setText(String.format("Speed: %.1fx", initialSpeed));

        playbackSpeedSlider.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                float speed = 0.5f + (progress / 10.0f); // Map 0-15 to 0.5-2.0
                playbackSpeedValue.setText(String.format("Speed: %.1fx", speed));

                // Save the speed setting in SharedPreferences
                getSharedPreferences("Settings", MODE_PRIVATE)
                        .edit()
                        .putFloat("playback_speed", speed)
                        .apply();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        // Background color switch
        Switch backgroundColorSwitch = findViewById(R.id.background_colour_switch);

        // Load saved background color preference
        boolean isGreyBackground = getSharedPreferences("Settings", MODE_PRIVATE)
                .getBoolean("is_grey_background", false);

        // Set the switch state based on the saved preference
        backgroundColorSwitch.setChecked(isGreyBackground);

        // Listen for changes to the switch
        backgroundColorSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            // Save the selected background color in SharedPreferences
            getSharedPreferences("Settings", MODE_PRIVATE)
                    .edit()
                    .putBoolean("is_grey_background", isChecked)
                    .apply();
        });

        // Set up the return button
        Button returnButton = findViewById(R.id.button2);
        returnButton.setOnClickListener(v -> {
            // Send the updated settings back to the previous activity
            Intent resultIntent = new Intent();
            setResult(Activity.RESULT_OK, resultIntent);
            finish();
        });
    }
}