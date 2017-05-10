/*
 * Copyright (c) 2012-2014 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
 * Copyright 1995-2007 Sun Microsystems, Inc.  All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Sun designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Sun in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 */


// This code was ported from the Java library java.awt.Color to C++.
// It is licenced under GPL V2 with the classpath exception.
// http://grepcode.com/file/repository.grepcode.com/java/root/jdk/openjdk/6-b14/java/awt/Color.java

#include "Color.h"

    float* Color::RGBtoHSB(int r, int g, int b, float* hsbvals) {
        assert(hsbvals != nullptr);
        float hue, saturation, brightness;

        int cmax = (r > g) ? r : g;
        if(b > cmax) cmax = b;
        int cmin = (r < g) ? r : g;
        if(b < cmin) cmin = b;

        brightness = static_cast<float>(cmax) / 255.0f;
        if(cmax != 0)
            saturation = static_cast<float>(cmax - cmin) / static_cast<float>(cmax);
        else
            saturation = 0;
        if(saturation == 0) {
            hue = 0;
        } else {
            float redc = static_cast<float>(cmax - r) / static_cast<float>(cmax - cmin);
            float greenc = static_cast<float>(cmax - g) / static_cast<float>(cmax - cmin);
            float bluec = static_cast<float>(cmax - b) / static_cast<float>(cmax - cmin);
            if(r == cmax)
                hue = bluec - greenc;
            else if(g == cmax)
                hue = 2.0f + redc - bluec;
            else
                hue = 4.0f + greenc - redc;
            hue = hue / 6.0f;
            if(hue < 0)
                hue = hue + 1.0f;
        }
        hsbvals[0] = hue;
        hsbvals[1] = saturation;
        hsbvals[2] = brightness;
        return hsbvals;
    }

    int Color::HSBtoRGB(float hue, float saturation, float brightness) {
        int r = 0, g = 0, b = 0;
        if(saturation == 0) {
            r = g = b = static_cast<int>(brightness * 255.0f + 0.5f);
        } else {
            float h = (hue - static_cast<float>(floor(hue))) * 6.0f;
            float f = h - static_cast<float>(floor(h));
            float p = brightness * (1.0f - saturation);
            float q = brightness * (1.0f - saturation * f);
            float t = brightness * (1.0f - (saturation * (1.0f - f)));
            switch (static_cast<int>(h)) {
            case 0:
                r = static_cast<int>(brightness * 255.0f + 0.5f);
                g = static_cast<int>(t * 255.0f + 0.5f);
                b = static_cast<int>(p * 255.0f + 0.5f);
                break;
            case 1:
                r = static_cast<int>(q * 255.0f + 0.5f);
                g = static_cast<int>(brightness * 255.0f + 0.5f);
                b = static_cast<int>(p * 255.0f + 0.5f);
                break;
            case 2:
                r = static_cast<int>(p * 255.0f + 0.5f);
                g = static_cast<int>(brightness * 255.0f + 0.5f);
                b = static_cast<int>(t * 255.0f + 0.5f);
                break;
            case 3:
                r = static_cast<int>(p * 255.0f + 0.5f);
                g = static_cast<int>(q * 255.0f + 0.5f);
                b = static_cast<int>(brightness * 255.0f + 0.5f);
                break;
            case 4:
                r = static_cast<int>(t * 255.0f + 0.5f);
                g = static_cast<int>(p * 255.0f + 0.5f);
                b = static_cast<int>(brightness * 255.0f + 0.5f);
                break;
            case 5:
                r = static_cast<int>(brightness * 255.0f + 0.5f);
                g = static_cast<int>(p * 255.0f + 0.5f);
                b = static_cast<int>(q * 255.0f + 0.5f);
                break;
            }
        }
        return 0xff000000 | (r << 16) | (g << 8) | (b << 0);
    }
