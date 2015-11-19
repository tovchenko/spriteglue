#include "Generator.h"
#include "imageTools/ImageTrim.h"
#include "binPack/MaxRectsBinPack.h"
#include "imageTools/imagerotate.h"
#include "plist/plistserializer.h"

#include <QDirIterator>
#include <QPainter>
#include <QImageWriter>
#include <QVariantMap>
#include <QTextStream>

Generator::Generator(const QString& inputImageDirPath)
    : _inputImageDirPath(inputImageDirPath) {
}

auto Generator::generateTo(const QString& finalImagePath)->bool {
    rbp::MaxRectsBinPack bin(_maxSize.width(), _maxSize.height());
    QImage result(_maxSize, _outputFormat);
    QPainter painter(&result);
    int left = _maxSize.width();
    int top = _maxSize.height();
    int right = 0;
    int bottom = 0;
    QVariantMap frames;

    QDirIterator it(_inputImageDirPath, QStringList() << "*.*", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString filePath = it.next();
        const QFileInfo fileInfo(filePath);
        QImage image(filePath);

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
            QVariantMap frameInfo;
            frameInfo["frame"] = QString("{{%1,%2},{%3,%4}}").arg(
                        QString::number(packedRect.x),
                        QString::number(packedRect.y),
                        QString::number(packedRect.width),
                        QString::number(packedRect.height));

            if (packedRect.width > packedRect.height != orientation)
                image = rotate90(image);
            painter.drawImage(packedRect.x, packedRect.y, image);

            if (packedRect.x < left)
                left = packedRect.x;
            if (packedRect.y < top)
                top = packedRect.y;
            if (packedRect.x + packedRect.width > right)
                right = packedRect.x + packedRect.width;
            if (packedRect.y + packedRect.height > bottom)
                bottom = packedRect.y + packedRect.height;

            frames[fileInfo.baseName() + '.' + fileInfo.completeSuffix()] = frameInfo;
        } else {
            painter.end();
            fprintf(stdout, "%ux%u %s\n", _maxSize.width(), _maxSize.height(), qPrintable(" too small."));
            return false;
        }
    }

    painter.end();

    QImageWriter writer(finalImagePath);
    writer.setFormat("PNG");
    const QRect finalCrop(QPoint(left, top), QPoint(right, bottom));

    if (writer.write(result.copy(finalCrop))) {
        fprintf(stdout, "%s\n", qPrintable(finalImagePath + " - success"));

        QFileInfo info(finalImagePath);
        QFile plistFile(info.dir().path() + QDir::separator() + info.baseName() + ".plist");
        if (plistFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&plistFile);

            QVariantMap meta;
            meta["format"] = 2;
            meta["realTextureFileName"] = meta["textureFileName"] = info.baseName() + '.' + info.completeSuffix();
            meta["size"] = QString("{%1,%2}").arg(QString::number(finalCrop.width()), QString::number(finalCrop.height()));

            QVariantMap root;
            root["frames"] = frames;
            root["metadata"] = meta;
            out << PListSerializer::toPList(root);
            plistFile.close();

            return true;
        }
    }
    return false;
}

auto Generator::_roundToPowerOf2(float value)->float {
    float power = 2.0f;
    while (value > power) {
        power *= 2;
    }
    return power;
}

