#pragma once
#include "imgui/imgui.h"

struct RGBs { float r, g, b; };

namespace Render {
    inline void DrawRect(int x, int y, int w, int h, RGBs c, int t=1) {
        ImGui::GetBackgroundDrawList()->AddRect(ImVec2((float)x,(float)y), ImVec2((float)(x+w),(float)(y+h)),
            IM_COL32((int)(c.r*255),(int)(c.g*255),(int)(c.b*255),255), 0, 0, (float)t);
    }
    inline void Line(float x1, float y1, float x2, float y2, RGBs c, float t=1.f) {
        ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x1,y1), ImVec2(x2,y2),
            IM_COL32((int)(c.r*255),(int)(c.g*255),(int)(c.b*255),255), t);
    }
    inline void Circle(float x, float y, float r, RGBs c) {
        ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(x,y), r,
            IM_COL32((int)(c.r*255),(int)(c.g*255),(int)(c.b*255),255));
    }
    inline void DrawText(float x, float y, const char* t, RGBs c) {
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(x,y),
            IM_COL32((int)(c.r*255),(int)(c.g*255),(int)(c.b*255),255), t);
    }
}
