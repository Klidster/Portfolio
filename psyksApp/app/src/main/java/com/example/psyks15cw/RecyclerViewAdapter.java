package com.example.psyks15cw;

import android.content.Context;
import android.content.Intent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.TextView;

import androidx.core.content.ContextCompat;
import androidx.recyclerview.widget.RecyclerView;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

public class RecyclerViewAdapter extends RecyclerView.Adapter<RecyclerViewAdapter.RecycleViewHolder> {

    private static List<AudioC> list;
    private Context context;
    private LayoutInflater layoutInflater;

    public RecyclerViewAdapter(Context context , List<AudioC> list)
    {
        this.list = list;
        this.context = context;
        this.layoutInflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    @Override
    public RecycleViewHolder onCreateViewHolder(ViewGroup parent, int viewType)
    {
        View itemView = layoutInflater.inflate(R.layout.audioclayout, parent, false);
        return new RecycleViewHolder(itemView);
    }

    @Override
    public void onBindViewHolder(RecycleViewHolder holder, int position)
    {
        holder.bind(list.get(position));
    }

    @Override
    public int getItemCount()
    {
        return list.size();
    }

    class RecycleViewHolder extends RecyclerView.ViewHolder {
        private final TextView view;

        RecycleViewHolder(View itemView)
        {
            super(itemView);
            view = itemView.findViewById(R.id.textView2);

            // Handle item click
            view.setOnClickListener(v -> {
                int position = getAdapterPosition();
                if (position != RecyclerView.NO_POSITION) {
                    // Get the fileName and filePath for the clicked item
                    String fileName = list.get(position).getFilename();
                    String filePath = list.get(position).getFilepath();

                    // Create an Intent to start AudioBookPlayerPage activity
                    Intent activityIntent = new Intent(context, AudioBookPlayerPage.class);
                    activityIntent.putExtra("fileName", fileName);
                    activityIntent.putExtra("filePath", filePath);
                    activityIntent.putParcelableArrayListExtra("audioList", new ArrayList<>(list)); // Pass the list
                    activityIntent.putExtra("currentIndex", position); // Pass the index
                    context.startActivity(activityIntent);

                    // Create an Intent to start the AudioBookPlayerService
                    Intent serviceIntent = new Intent(context, AudioBookPlayerService.class);
                    serviceIntent.putExtra("filePath", filePath);

                    // Start the service
                    ContextCompat.startForegroundService(context, serviceIntent);
                }
            });
        }

        // Bind data to the view
        void bind(final AudioC audio)
        {
            String title = audio.getFilename();
            view.setText(title);  // Set the song name
        }
    }
}


