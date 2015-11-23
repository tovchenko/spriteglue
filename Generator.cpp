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

#include <cmath>

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
        auto packedRect = bin.Insert(cropRect.width() + _padding * 2, cropRect.height() + _padding * 2, rbp::MaxRectsBinPack::RectBestShortSideFit);

        if (packedRect.height > 0) {
            QVariantMap frameInfo;
            const bool isRotated = packedRect.width > packedRect.height != orientation;

            QRect finalRect(packedRect.x + _padding,
                            packedRect.y + _padding,
                            (isRotated ? packedRect.height : packedRect.width) - 2 * _padding,
                            (isRotated ? packedRect.width : packedRect.height) - 2 * _padding);

            frameInfo["rotated"] = isRotated;
            frameInfo["frame"] = finalRect;

            if (beforeTrimSize.width() != cropRect.width() || beforeTrimSize.height() != cropRect.height()) {
                const int w = std::floor(cropRect.x() + 0.5f * (-beforeTrimSize.width() + cropRect.width()));
                const int h = std::floor(-cropRect.y() + 0.5f * (beforeTrimSize.height() - cropRect.height()));
                frameInfo["offset"] = QString("{%1,%2}").arg(QString::number(w), QString::number(h));
            } else {
                frameInfo["offset"] = "{0,0}";
            }

            frameInfo["sourceColorRect"] = QString("{{%1,%2},{%3,%4}}").arg(
                        QString::number(cropRect.x()),
                        QString::number(cropRect.y()),
                        QString::number(finalRect.width()),
                        QString::number(finalRect.height()));

            frameInfo["sourceSize"] = QString("{%1,%2}").arg(
                        QString::number(beforeTrimSize.width()),
                        QString::number(beforeTrimSize.height()));

            if (isRotated)
                image = rotate90(image);

            painter.drawImage(packedRect.x + _padding, packedRect.y + _padding, image);

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

    QRect finalCrop(QPoint(left, top), QPoint(right, bottom));
    if (_square) {
        const auto maxSide = std::max(finalCrop.width(), finalCrop.height());
        finalCrop.setWidth(maxSide);
        finalCrop.setHeight(maxSide);
    }

    _adjustFrames(frames, [&finalCrop](QRect& rect) {
        rect.setX(rect.x() - finalCrop.x());
        rect.setY(rect.y() - finalCrop.y());
    });

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

auto Generator::_adjustFrames(QVariantMap& frames, const std::function<void(QRect&)>& cb)->void {
    for (auto it = frames.begin(); it != frames.end(); ++it) {
        QVariantMap frame = qvariant_cast<QVariantMap>(*it);
        QRect outRect = frame["frame"].toRect();
        cb(outRect);
        frame["frame"] = QString("{{%1,%2},{%3,%4}}").arg(
                         QString::number(outRect.x()),
                         QString::number(outRect.y()),
                         QString::number(outRect.width()),
                         QString::number(outRect.height()));
        *it = frame;
    }
}

