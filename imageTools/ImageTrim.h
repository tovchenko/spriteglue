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
