#pragma once

#include "imgui.h"

const int line_thickness = 1;
const int hover_thickness = 20;

class Marker {
public:
	char buffer[64] = "";

	Marker(float *_width, float _height) {
		width = _width;
		height = _height;
	}

	bool IsHoveredOver(ImVec2 relative_position, ImVec2 mouse_pos, float zoom) {
		float rel_y = relative_position.y + height * zoom;
		float half_height = hover_thickness / 2.0f;

		bool is_hovered = relative_position.x < mouse_pos.x
			&& mouse_pos.x < relative_position.x + *width
			&& rel_y - half_height < mouse_pos.y
			&& mouse_pos.y < rel_y + half_height;

		// draw a thick line if hovered, or just a regular one
		if (is_hovered) {
			ImGui::GetWindowDrawList()->AddLine(
				ImVec2(relative_position.x, relative_position.y + height * zoom),
				ImVec2(relative_position.x + *width, relative_position.y + height * zoom),
				IM_COL32(0, 100, 160, 120),
				hover_thickness
			);
		}

		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(relative_position.x, relative_position.y + height * zoom),
			ImVec2(relative_position.x + *width, relative_position.y + height * zoom),
			IM_COL32(0, 200, 0, 255),
			line_thickness
		);

		return is_hovered;
	}

	void ShowError(ImVec2 relative_position, ImVec2 mouse_pos, float zoom) {
		ImGui::GetWindowDrawList()->AddLine(
			ImVec2(relative_position.x, relative_position.y + height * zoom),
			ImVec2(relative_position.x + *width, relative_position.y + height * zoom),
			IM_COL32(180, 0, 60, 120),
			hover_thickness
		);
	}

	void SetHeight(float relative_height) {
		height = relative_height;
	}

	bool GetActive() {
		return active;
	}

	void SetActive(bool value) {
		active = value;
	}

	float GetLowerBound() {
		return height - hover_thickness / 2.0f;
	}

	float GetUpperBound() {
		return height + hover_thickness / 2.0f;
	}

	/// <summary>
	/// Sets the height relative to this markers own lower bound.
	/// </summary>
	/// <param name="lower_bound"></param>
	void SetHeightFromLowerBound(float lower_bound) {
		height = lower_bound + hover_thickness / 2.0f;
	}

	/// <summary>
	/// Sets the height relative to this markers own upper bound.
	/// </summary>
	/// <param name="upper_bound"></param>
	void SetHeightFromUpperBound(float upper_bound) {
		height = upper_bound - hover_thickness / 2.0f;
	}

private:
	float height = 0;
	float *width = 0;
	bool active = false;
};
