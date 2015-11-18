#include "Generator.h"
#include "imageTools/ImageTrim.h"
#include "binPack/MaxRectsBinPack.h"
#include "imageTools/imagerotate.h"

#include <QDirIterator>
#include <QPainter>
#include <QImageWriter>

Generator::Generator(const QString& inputImageDirPath)
    : _inputImageDirPath(inputImageDirPath) {
}

auto Generator::generateTo(const QString& finalImagePath)->bool {
    rbp::MaxRectsBinPack bin(_maxSize.width(), _maxSize.height());
    QImage result(_maxSize, _outputFormat);
    QPainter painter(&result);

    QDirIterator it(_inputImageDirPath, QStringList() << "*.*", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QImage image(it.next());

        if (_scale < 1.0f) {
            image = image.scaled(_scale * image.width(), _scale * image.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        const QSize beforeTrimSize = image.size();
        QRect cropRect(QPoint(0, 0), beforeTrimSize);
        if (_trim != TrimMode::NONE) {
            image = ImageTrim::createImage(image, _trim == TrimMode::MAX_ALPHA, cropRect);
        }

        bool orientation = cropRect.width() > cropRect.height();
        auto packedRect = bin.Insert(cropRect.width(), cropRect.height(), rbp::MaxRectsBinPack::RectBestShortSideFit);

        if (packedRect.height > 0) {
            if (packedRect.width > packedRect.height != orientation)
                image = rotate90(image);
            painter.drawImage(packedRect.x, packedRect.y, image);
        } else {
            painter.end();
            fprintf(stdout, "%ux%u %s%\n", _maxSize.width(), _maxSize.height(), qPrintable(" too small."));
            return false;
        }
    }

    painter.end();

    QImageWriter writer(finalImagePath);
    writer.setFormat("PNG");
    writer.write(result);
    fprintf(stdout, "%s\n", qPrintable(finalImagePath + " - success"));
    return true;
}

auto Generator::_roundToPowerOf2(float value)->float {
    float power = 2.0f;
    while (value > power) {
        power *= 2;
    }
    return power;
}

