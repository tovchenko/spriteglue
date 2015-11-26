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

const int kBasePercent = 10;
const int kStepPercent = 2;

Generator::Generator(const QString& inputImageDirPath)
    : _inputImageDirPath(inputImageDirPath) {
}

auto Generator::generateTo(const QString& finalImagePath)->bool {
    auto imageData = _scaleTrimIfNeeded();
    std::vector<QString> paths;
    std::transform(imageData->begin(), imageData->end(), std::back_inserter(paths), [](const std::pair<QString, _Data>& data) {
        return data.first;
    });

    auto area = std::accumulate(imageData->begin(), imageData->end(), 0, [](int sum, const std::pair<QString, _Data>& data) {
        return sum + (data.second.cropRect.width() * data.second.cropRect.height());
    });

    ImageSorter sorter(paths);
    const auto sortedPaths = sorter.sort();
    int notUsedPercent = kBasePercent;
    int left, top, right, bottom;
    QVariantMap frames;
    std::unique_ptr<QImage> result;

    bool enoughSpace = true;
    do {
        notUsedPercent += kStepPercent;
        const int side = floor(sqrtf(area + area * notUsedPercent / 100));
        QSize beforeSize(side, side);
        if (!_square && _isPowerOf2) {
            beforeSize.setWidth(_floorToPowerOf2(side));
            beforeSize.setHeight(_roundToPowerOf2(side));
        } else if (_square && _isPowerOf2) {
            beforeSize.setWidth(_roundToPowerOf2(side));
            beforeSize.setHeight(_roundToPowerOf2(side));
        }

        rbp::MaxRectsBinPack bin(beforeSize.width(), beforeSize.height());
        result = std::move(std::unique_ptr<QImage>(new QImage(beforeSize, _outputFormat)));
        QPainter painter(result.get());
        frames.clear();
        left = beforeSize.width();
        top = beforeSize.height();
        right = 0;
        bottom = 0;

        for (auto it = sortedPaths->begin(); it != sortedPaths->end(); ++it) {
            QImage image(*it);
            const auto& beforeTrimSize = imageData->at(*it).beforeCropSize;
            const auto& cropRect = imageData->at(*it).cropRect;

            bool orientation = cropRect.width() > cropRect.height();
            auto packedRect = bin.Insert(cropRect.width() + _padding * 2, cropRect.height() + _padding * 2, rbp::MaxRectsBinPack::RectBestLongSideFit);

            if (packedRect.height > 0) {
                enoughSpace = true;

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
                enoughSpace = false;
                break;
            }
        }
        painter.end();
    } while (!enoughSpace);

    std::for_each(sortedPaths->begin(), sortedPaths->end(), [](const QString& path) {
        QFile file(path);
        file.remove();
    });

    QRect finalCrop(QPoint(left, top), QPoint(right, bottom));
    finalCrop.setSize(_fitSize(finalCrop.size()));
    _adjustFrames(frames, [&finalCrop](QRect& rect) {
        rect.setX(rect.x() - finalCrop.x());
        rect.setY(rect.y() - finalCrop.y());
    });

    QImageWriter writer(finalImagePath);
    writer.setFormat("png");

    if (writer.write(result->copy(finalCrop))) {
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

auto Generator::_roundToPowerOf2(int value)->int {
    int power = 2;
    while (value > power) {
        power *= 2;
    }
    return power;
}

auto Generator::_floorToPowerOf2(int value)->int {
    int power = 2;
    while (value >= power) {
        power *= 2;
    }
    return power / 2;
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
        result.setWidth(isFloor ? _floorToPowerOf2(result.width()) : _roundToPowerOf2(result.width()));
        result.setHeight(isFloor ? _floorToPowerOf2(result.height()) : _roundToPowerOf2(result.height()));
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
        const QString tmpImagePath = QDir::tempPath() + QDir::separator() + fileInfo.baseName();
        QImageWriter writer(tmpImagePath);
        writer.setFormat("png");
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
