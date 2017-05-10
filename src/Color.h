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

#ifndef _CPP_COLOR_H_
#define _CPP_COLOR_H_

#include <assert.h>
#include <stdlib.h>
#include <math.h>

class Color {
public:
    static float* RGBtoHSB(int r, int g, int b, float* hsbvals);
    static int HSBtoRGB(float hue, float saturation, float brightness);
};

#endif // _CPP_COLOR_H_

