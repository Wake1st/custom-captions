#pragma once

#include <chrono>
#include <string>
#include <format>
#include <iostream>
#include <Windows.h> // For PlaySound
#include <mmsystem.h> // For SND_ASYNC, SND_SYNC, etc.
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"

#pragma comment(lib, "Winmm.lib") // Link to the multimedia library

class AudioPlayer {
public:
    void Update(std::string path) {
        // display the audio buttons in the same window as the waveform image
        ImGui::Begin("Waveform");

        ImVec2 window_size = ImGui::GetWindowSize();
        ImGui::SetNextWindowSize(ImVec2(window_size.x, 50), ImGuiCond_FirstUseEver);
        ImGui::BeginChild("Audio");

        if (is_playing)
        {
            // cannot select play while playing
            ImGui::BeginDisabled();
            ImGui::Button("PLAY", ImVec2(60, 20));
            ImGui::EndDisabled();

            // only pause enabled if playing
            if (ImGui::Button("PAUSE", ImVec2(60, 20))) {
                Pause();
                is_playing = false;
            }
        }
        else {
            // only play available if paused
            if (ImGui::Button("PLAY", ImVec2(60, 20))) {
                Play(path);
                is_playing = true;
            }

            // cannot pause while paused
            ImGui::BeginDisabled();
            ImGui::Button("PAUSE", ImVec2(60, 20));
            ImGui::EndDisabled();
        }
            
        ImGui::EndChild();
        ImGui::End();
    }

    double GetPlayTime() {
        std::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> current_duration = end - start;

        if (current_duration.count() == 0.0)
            return 0.0;
        else
            return current_duration.count() / total_duration;
    }

private:
    double total_duration = 0.0;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::time_point();

    bool is_playing = false;

    void Play(std::string path) {
        // get duration
        std::string waveform_duration_cmd = std::format(
            "ffprobe -i {} -show_entries format=duration -v quiet -of csv=\"p=0\"",
            path
        );
        int waveform_result = system(waveform_duration_cmd.c_str());
        std::cin >> total_duration;
        
        // Play a WAV file asynchronously (non-blocking)
        std::wstring w_path(path.begin(), path.end());
        PlaySound(w_path.c_str(), NULL, SND_ASYNC);
        start = std::chrono::high_resolution_clock::now();
    }

    void Pause() {
        // To stop playback of a sound played with SND_ASYNC
        PlaySound(NULL, NULL, 0); 
    }
};
