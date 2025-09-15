#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include <format>
#include <string>
#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "stb_image.h"

#include "marker.h"
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

        bool marker_added = MarkerWindow(io);
        InputWindow(marker_added);
    }

private:
	const char* audioPath = "";
    std::vector<TimeText> timeTexts;

    int image_width = 0;
    int image_height = 0;
    GLuint image_texture = 0;

    std::vector<Marker> markers;
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

    bool MarkerWindow(ImGuiIO& io) {
        // create a window of a specific size
        ImGui::SetNextWindowSize(ImVec2(line_width, 600), ImGuiCond_FirstUseEver);
        ImGui::Begin("Waveform");

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

        // ensures we don't place a marker while moving another one
        bool is_actively_dragging = false;

        // the marker height should be relative to the whole image
        float relative_height = io.MousePos.y - window_pos.y + unseen_image_height;

        // draw existing markers
        for (int i = 0; i < markers.size(); i++) {
            // the relative height is the absolute and the window heights without the unseen height
            bool hovered_over_marker = markers[i].IsHoveredOver(ImVec2(window_pos.x, window_pos.y - unseen_image_height), io.MousePos);

            // first check for removal, and exit loop since nothing is needed
            if (hovered_over_marker && ImGui::IsMouseClicked(GLFW_MOUSE_BUTTON_2)) {
                markers.erase(markers.begin() + i);
                break;
            }

            // set when selected, toggle off active when 
            if (hovered_over_marker && ImGui::IsMouseClicked(GLFW_MOUSE_BUTTON_1)) {
                markers[i].SetActive(true);
            }
            else if (markers[i].GetActive() && ImGui::IsMouseReleased(GLFW_MOUSE_BUTTON_1)) {
                markers[i].SetActive(false);
            }

            // drag if active
            if (markers[i].GetActive()) {

                // check the neighbors to ensure no overlap
                if (i + 1 < markers.size() && markers[i + 1].GetLowerBound() < markers[i].GetUpperBound()) {
                    markers[i].SetHeightFromUpperBound(markers[i + 1].GetLowerBound());
                }
                else if (0 < i - 1 && markers[i].GetLowerBound() < markers[i - 1].GetUpperBound()) {
                    markers[i].SetHeightFromLowerBound(markers[i - 1].GetUpperBound());
                }
                else {
                    markers[i].SetHeight(relative_height);
                }

                // since we're draggin this marker, no other needs to be checked
                is_actively_dragging = true;
                break;
            }
        }

        // add a new marker
        bool added_new_marker = false;
        if (window_is_hovered && !is_actively_dragging) {
            // draw a horizontal line at the cursor, relative to current window
            ImGui::GetWindowDrawList()->AddLine(
                ImVec2(window_pos.x, io.MousePos.y),
                ImVec2(window_pos.x + line_width, io.MousePos.y),
                IM_COL32(0, 200, 0, 255),
                line_thickness
            );

            // attempt to insert on left mouse click
            if (ImGui::IsMouseClicked(GLFW_MOUSE_BUTTON_1)) {
                // create a new marker where the mouse is
                TimeMarker new_marker = TimeMarker(&line_width, relative_height);

                // if no insert, show user feedback
                added_new_marker = CanInsertMarker(&new_marker);
                if (!added_new_marker) {
                    new_marker.ShowError(ImVec2(window_pos.x, window_pos.y - unseen_image_height), io.MousePos);
                }
            }
        }

        ImGui::EndChild();

        ImGui::End();

        return added_new_marker;
    }

    bool CanInsertMarker(TimeMarker *marker) {
        // simply insert if there are no other markers
        if (markers.size() == 0) {
            markers.push_back(*marker);
            return true;
        }

        // insert marker at the lower position if it clears
        if (marker->GetUpperBound() < markers[0].GetLowerBound()) {
            markers.insert(markers.begin(), *marker);
            return true;
        }

        // insert marker between it's nearest neighbors
        for (int i = 1; i < markers.size(); i++) {
            // insert if lower than the current and higher than the previous bounds
            if (markers[i - 1].GetUpperBound() < marker->GetLowerBound() &&
                marker->GetUpperBound() < markers[i].GetLowerBound()
            ) {
                markers.insert(markers.begin() + i, *marker);
                return true;
            }
        }

        // insert if higher than the second highest bound
        if (markers[markers.size() - 1].GetUpperBound() < marker->GetLowerBound()) {
            markers.push_back(*marker);
            return true;
        }

        // no more places to insert
        return false;
    }

    void InputWindow(bool create_input) {
        ImGui::SetNextWindowSize(ImVec2(512, 600), ImGuiCond_FirstUseEver);
        ImGui::Begin("Text");

        //// add a new input
        //if (create_input) {
        //    TextInput input = TextInput();
        //    textInputs.push_back(input);
        //}

        // display all existing inputs
        for (int i = 0; i < markers.size(); i++) {
            std::string label = std::format("Input-{}", i + 1);
            ImGui::InputText(label.c_str(), markers[i].buffer, 64);
        }

        ImGui::End();
    }
};


