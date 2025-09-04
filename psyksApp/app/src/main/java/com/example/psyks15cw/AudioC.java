package com.example.psyks15cw;

import android.os.Parcel;
import android.os.Parcelable;

public class AudioC implements Parcelable {
    private String filename;
    private String filepath;

    public AudioC(String filename, String filepath) {
        this.filename = filename;
        this.filepath = filepath;
    }

    protected AudioC(Parcel in) {
        filename = in.readString();
        filepath = in.readString();
    }

    public static final Creator<AudioC> CREATOR = new Creator<AudioC>() {
        @Override
        public AudioC createFromParcel(Parcel in) {
            return new AudioC(in);
        }

        @Override
        public AudioC[] newArray(int size) {
            return new AudioC[size];
        }
    };

    public String getFilename() {
        return filename;
    }

    public String getFilepath() {
        return filepath;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(filename);
        dest.writeString(filepath);
    }
}
