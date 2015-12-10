/* ImageTrim.h
Copyright (C) 2015 Taras Tovchenko
Email: doctorset@gmail.com

You can redistribute and/or modify this software under the terms of the GNU
General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this
program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA */

#ifndef IMAGETRIM_H
#define IMAGETRIM_H

#include <QRect>

class QImage;

class ImageTrim {
public:
    static QImage createImage(const QImage& sourceImage, bool maxAlphaValue, QRect& cropRect);

protected:
    static QRect getBoundingBox(const QImage& sourceImage, bool maxAlphaValue);
};

#endif // IMAGETRIM_H
