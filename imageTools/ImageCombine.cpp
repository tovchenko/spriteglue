#include "ImageCombine.h"

#include <QMap>
#include <iterator>
#include <QImage>
#include <QPainter>

QImage ImageCombine::createImage(const QMap<QString, QPoint>& drawData, const QSize& size, bool withAlpha) {
    QImage combinedImage(size, withAlpha ? QImage::Format_RGBA8888 : QImage::Format_RGB888);
    combinedImage.fill(QColor(0, 0, 0, 0));
    QPainter painter(&combinedImage);

    for (auto it = drawData.begin(); it != drawData.end(); ++it) {
        QImage image(it.key());
        painter.drawImage(it.value().x(), it.value().y(), image);
    }
    painter.end();
    return combinedImage;
}
