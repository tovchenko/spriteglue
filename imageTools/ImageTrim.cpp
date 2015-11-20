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
    int left = sourceImage.width();
    int right = 0;
    int top = sourceImage.height();
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

