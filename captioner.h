#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "stb_image.h"

#include "time_marker.h"
#include "text_time.h"


//  ffmpeg -i dauacity.wav -filter_complex "[0:a]showwavespic=s=2560x2560,crop=2560:2560[v]" -map "[v]" -frames:v 1 output.png
//  ffmpeg -i output.png -vf "rotate=PI/2" rotated.png


class Captioner {
public:
    Captioner() {
        bool ret = LoadTextureFromFile("./data/rotated.png", &image_texture, &image_width, &image_height);
        IM_ASSERT(ret);
    }

    void update() {
        ImGuiIO& io = ImGui::GetIO();

        // create a window of a specific size
        ImGui::SetNextWindowSize(ImVec2(line_width, 600), ImGuiCond_FirstUseEver);
        ImGui::Begin("OpenGL Texture Text");

        // ensure we know if the current window is hovered
        bool window_is_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

        ImGui::BeginChild("Test");

        // get the child window position
        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();
        line_width = window_size.x;

        // the scrollable height is the portion of the image not visable in the window
        float scrollable_image_height = image_height - window_size.y;
        if (scrollable_image_height < 0)
            scrollable_image_height = 0.0f;

        // the position of the marker height is the window height minus the upper unseen portion of the image
        float scroll_ratio = ImGui::GetScrollY() / ImGui::GetScrollMaxY();
        float unseen_image_height = scrollable_image_height * scroll_ratio;
        
        // draw the audio waveform
        ImGui::Image(
            (ImTextureID)(intptr_t)image_texture, 
            ImVec2(window_size.x, image_height), 
            ImVec2(0.3, 0), 
            ImVec2(0.7, 1)
        );

        // store for later check on movement
        bool is_actively_dragging = false;

        // draw existing marker
        for (int i = 0; i < markers.size(); i++) {
            // the relative height is the absolute and the window heights without the unseen height
            bool hovered_over_marker = markers[i].IsHoveredOver(ImVec2(window_pos.x, window_pos.y - unseen_image_height), io.MousePos);

            // set when selected, toggle off active when 
            if (hovered_over_marker && ImGui::IsMouseClicked(GLFW_MOUSE_BUTTON_1)) {
                markers[i].SetActive(true);
            }
            else if (markers[i].GetActive() && ImGui::IsMouseReleased(GLFW_MOUSE_BUTTON_1)) {
                markers[i].SetActive(false);
            }
            
            // drag if active
            if (markers[i].GetActive()) {
                is_actively_dragging = true;
                
                // TODO: ensure no dragging beyond neighbors
                markers[i].Drag(io.MousePos.y - window_pos.y + unseen_image_height);
            }
        }

        
        // add a new line, or drag an existing marker
        if (window_is_hovered && !is_actively_dragging) {
            // draw a horizontal line at the cursor, relative to current window
            ImGui::GetWindowDrawList()->AddLine(
                ImVec2(window_pos.x, io.MousePos.y),
                ImVec2(window_pos.x + line_width, io.MousePos.y),
                IM_COL32(0, 200, 0, 255), 
                line_thickness
            );

            if (ImGui::IsMouseClicked(GLFW_MOUSE_BUTTON_1)) {
                // the marker height should be relative to the whole image
                markers.push_back(TimeMarker(&line_width, io.MousePos.y - window_pos.y + unseen_image_height));
            }
        }

        ImGui::EndChild();

        ImGui::End();
    }

private:
	const char* audioPath = "";
    std::vector<TimeText> timeTexts;

    int image_width = 0;
    int image_height = 0;
    GLuint image_texture = 0;

    std::vector<TimeMarker> markers;
    float line_width = 256.0f;
    float line_thickness = 1.0f;
    
    // Simple helper function to load an image into a OpenGL texture with common settings
    bool LoadTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture, int* out_width, int* out_height)
    {
        // Load from file
        int image_width = 0;
        int image_height = 0;
        unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
        if (image_data == NULL)
            return false;

        // Create a OpenGL texture identifier
        GLuint image_texture;
        glGenTextures(1, &image_texture);
        glBindTexture(GL_TEXTURE_2D, image_texture);

        // Setup filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Upload pixels into texture
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        stbi_image_free(image_data);

        *out_texture = image_texture;
        *out_width = image_width;
        *out_height = image_height;

        return true;
    }

    // Open and read a file, then forward to LoadTextureFromMemory()
    bool LoadTextureFromFile(const char* file_name, GLuint* out_texture, int* out_width, int* out_height)
    {
        FILE* f = fopen(file_name, "rb");
        if (f == NULL)
            return false;
        fseek(f, 0, SEEK_END);
        size_t file_size = (size_t)ftell(f);
        if (file_size == -1)
            return false;
        fseek(f, 0, SEEK_SET);
        void* file_data = IM_ALLOC(file_size);
        fread(file_data, 1, file_size, f);
        fclose(f);
        bool ret = LoadTextureFromMemory(file_data, file_size, out_texture, out_width, out_height);
        IM_FREE(file_data);
        return ret;
    }
};


