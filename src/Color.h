/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
Copyright 1995-2007 Sun Microsystems, Inc.  All Rights Reserved.
This code was ported from the Java library java.awt.Color to C++.
It is licenced under GPL V2 with the classpath exception.
http://grepcode.com/file/repository.grepcode.com/java/root/jdk/openjdk/6-b14/java/awt/Color.java
*/

#ifndef _CPP_COLOR_H_
#define _CPP_COLOR_H_

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <array>

class Color {
public:
    static std::array<float, 3>* RGBtoHSB(int r, int g, int b, std::array<float, 3>* hsbvals);
    static int HSBtoRGB(float hue, float saturation, float brightness);
};

#endif // _CPP_COLOR_H_
