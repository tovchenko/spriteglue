#include "Generator.h"
#include "imageTools/ImageTrim.h"
#include "binPack/MaxRectsBinPack.h"
#include "imageTools/imagerotate.h"
#include "plist/plistserializer.h"
#include "ImageSorter.h"

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
    auto imageData = _scaleTrimIfNeeded();
    std::vector<QString> paths;
    std::transform(imageData->begin(), imageData->end(), std::back_inserter(paths), [](const std::pair<QString, _Data>& data) {
        return data.first;
    });

    ImageSorter sorter(paths);
    const auto sortedPaths = sorter.sort();
    const auto beforeSize = _fitSize(_maxSize, true);

    QVariantMap frames;
    rbp::MaxRectsBinPack bin(beforeSize.width(), beforeSize.height());
    QImage result(beforeSize, _outputFormat);
    QPainter painter(&result);
    int left = beforeSize.width();
    int top = beforeSize.height();
    int right = 0;
    int bottom = 0;

    for (auto it = sortedPaths->begin(); it != sortedPaths->end(); ++it) {
        QImage image(*it);
        const auto& beforeTrimSize = imageData->at(*it).beforeCropSize;
        const auto& cropRect = imageData->at(*it).cropRect;

        bool orientation = cropRect.width() > cropRect.height();
        auto packedRect = bin.Insert(cropRect.width() + _padding * 2, cropRect.height() + _padding * 2, rbp::MaxRectsBinPack::RectBestLongSideFit);

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

            frames[imageData->at(*it).basename] = frameInfo;
        } else {
            painter.end();
            fprintf(stdout, "%ux%u %s\n", _maxSize.width(), _maxSize.height(), qPrintable(" too small."));
            return false;
        }

        QFile file(*it);
        file.remove();
    }

    painter.end();

    QImageWriter writer(finalImagePath);
    writer.setFormat("PNG");

    QRect finalCrop(QPoint(left, top), QPoint(right, bottom));
    finalCrop.setSize(_fitSize(finalCrop.size()));
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

auto Generator::_roundToPowerOf2(int value, bool isFloor)->int {
    int power = 2;
    while (value > power) {
        power *= 2;
    }
    return isFloor ? power / 2 : power;
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

auto Generator::_fitSize(const QSize& size, bool isFloor) const->QSize {
    QSize result = size;

    if (_square) {
        auto side = isFloor ? std::min(result.width(), result.height()) : std::max(result.width(), result.height());
        result = QSize(side, side);
    }

    if (_isPowerOf2) {
        result.setWidth(_roundToPowerOf2(result.width(), isFloor));
        result.setHeight(_roundToPowerOf2(result.height(), isFloor));
    }
    return result;
}

auto Generator::_readFileList() const->std::shared_ptr<std::vector<QString>> {
    auto result = std::make_shared<std::vector<QString>>();
    QDirIterator it(_inputImageDirPath, QStringList() << "*.*", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        result->push_back(it.next());
    }
    return result;
}

auto Generator::_scaleTrimIfNeeded() const->std::shared_ptr<ImageData> {
    auto result = std::make_shared<std::map<QString, _Data>>();
    auto files = _readFileList();
    for (auto it = files->begin(); it != files->end(); ++it) {
        QImage image(*it);

        if (_scale < 1.0f) {
            image = image.scaled(_scale * image.width(), _scale * image.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        const QSize beforeTrimSize = image.size();
        QRect cropRect(QPoint(0, 0), beforeTrimSize);
        if (_trim != TrimMode::NONE) {
            image = ImageTrim::createImage(image, _trim == TrimMode::MAX_ALPHA, cropRect);
        }

        const QFileInfo fileInfo(*it);
        const QString tmpImagePath = QDir::tempPath() + QDir::separator() + fileInfo.baseName() + '.' + "png";
        QImageWriter writer(tmpImagePath);
        writer.setFormat("PNG");
        if (writer.write(image)) {
            _Data data;
            data.beforeCropSize = beforeTrimSize;
            data.cropRect = cropRect;
            data.basename = fileInfo.baseName() + '.' + fileInfo.completeSuffix();
            result->insert(std::make_pair(tmpImagePath, data));
        }
    }
    return result;
}
