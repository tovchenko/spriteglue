#ifndef IMAGECOMBINE_H
#define IMAGECOMBINE_H

#include <QSize>
#include <QImage>

class ImageCombine {
public:
    static QImage createImage(const QMap<QString, QPoint>& drawData, const QSize& size, bool withAlpha=true);
};

#endif // IMAGECOMBINE_H
