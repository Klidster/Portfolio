#include <list>
#include <vector>
#include <algorithm>

#include "point.h"

point evaluate(float t, std::list<point> P) {
    if (P.size() == 1)
        return P.front();

    std::list<point> next;
    auto it = P.begin();
    auto next_it = std::next(it);

    while (next_it != P.end()) {
        next.push_back((1 - t) * (*it) + t * (*next_it));
        ++it;
        ++next_it;
    }

    return evaluate(t, next);
}

std::vector<point> EvaluateBezierCurve(std::vector<point> ctrl_points, int num_evaluations) {
    std::vector<point> curve;
    for (int i = 0; i <= num_evaluations; ++i) {
        float t = float(i) / num_evaluations;
        curve.push_back(evaluate(t, std::list<point>(ctrl_points.begin(), ctrl_points.end())));
    }
    return curve;
}

float* MakeFloatsFromVector(std::vector<point> curve, int& num_verts, int& num_floats, float r, float g, float b)
{
    num_verts = curve.size();
    num_floats = num_verts * 6; // 3 pos + 3 color

    float* data = new float[num_floats];
    for (int i = 0; i < num_verts; ++i) {
        data[i * 6 + 0] = curve[i].x;
        data[i * 6 + 1] = curve[i].y;
        data[i * 6 + 2] = curve[i].z;
        data[i * 6 + 3] = r;
        data[i * 6 + 4] = g;
        data[i * 6 + 5] = b;
    }
    return data;
}
