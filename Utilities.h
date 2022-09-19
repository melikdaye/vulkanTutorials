//
// Created by Administrator on 22.08.2022.
//

#ifndef VULKANTUTORIALS_UTILITIES_H
#define VULKANTUTORIALS_UTILITIES_H

struct QueueFamilyIndices {
    int graphicsFamily = -1;
    bool isValid(){
        return  graphicsFamily >= 0;
    }
};

#endif //VULKANTUTORIALS_UTILITIES_H
