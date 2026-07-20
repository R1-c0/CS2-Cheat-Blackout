#pragma once
enum bones : int {
    head = 6, neck = 5, spine = 4, spine_1 = 2,
    left_shoulder = 8, left_arm = 9, left_hand = 11,
    cock = 0,
    right_shoulder = 13, right_arm = 14, right_hand = 16,
    left_hip = 22, left_knee = 23, left_feet = 24,
    right_hip = 25, right_knee = 26, right_feet = 27
};
struct BoneConnection { int bone1, bone2; BoneConnection(int b1,int b2):bone1(b1),bone2(b2){} };
static BoneConnection boneConnections[] = {
    {6,5},{5,4},{4,0},{4,8},{8,9},{9,11},{4,13},{13,14},{14,16},
    {4,2},{0,22},{0,25},{22,23},{23,24},{25,26},{26,27}
};
