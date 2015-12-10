/* ImageTrim.cpp
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

#include "ImageTrim.h"
#include <QImage>

QImage ImageTrim::createImage(const QImage& sourceImage, bool maxAlphaValue, QRect& cropRect) {
    if (sourceImage.width() < 2 || sourceImage.height() < 2 || !sourceImage.hasAlphaChannel()) {
        return sourceImage;
    }

    cropRect = getBoundingBox(sourceImage, maxAlphaValue);
    if (cropRect.x() > 0 ||
        cropRect.y() > 0 ||
        cropRect.width() < sourceImage.width() ||
        cropRect.height() < sourceImage.height())
    {
        return sourceImage.copy(cropRect);
    }
    return sourceImage;
}

QRect ImageTrim::getBoundingBox(const QImage& sourceImage, bool maxAlphaValue) {
    int left = sourceImage.width() - 1;
    int right = 0;
    int top = sourceImage.height() - 1;
    int bottom = 0;

    for (int y = 0; y < sourceImage.height(); ++y) {
        QRgb* row = (QRgb*)sourceImage.scanLine(y);
        bool rowFilled = false;

        for (int x = 0; x < sourceImage.width(); ++x) {
            const int alpha = qAlpha(row[x]);
            if ((!maxAlphaValue && alpha == 255) || (maxAlphaValue && alpha)) {
                rowFilled = true;
                right = std::max(right, x);
                if (left > x) {
                    left = x;
                    x = right; // shortcut to only search for new right bound from here
                }
            }
        }

        if (rowFilled) {
            top = std::min(top, y);
            bottom = y;
        }
    }

    return QRect(QPoint(left, top), QPoint(right, bottom));
}

