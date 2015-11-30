#include "Generator.h"
#include "imageTools/ImageTrim.h"
#include "binPack/MaxRectsBinPack.h"
#include "imageTools/imagerotate.h"
#include "plist/plistserializer.h"
#include "ImageSorter.h"

#include <QDirIterator>
#include <QPainter>
#include <QImageWriter>
#include <QImageReader>
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
    ImageSorter::FrameSizes frameSizes;
    std::transform(imageData->begin(), imageData->end(), std::back_inserter(frameSizes), [](const std::pair<QString, _Data>& data) {
        return std::make_pair(data.first, data.second.cropRect.size());
    });

    auto area = std::accumulate(imageData->begin(), imageData->end(), 0, [](int sum, const std::pair<QString, _Data>& data) {
        return data.second.duplicate ? sum : sum + (data.second.cropRect.width() * data.second.cropRect.height());
    });

    ImageSorter sorter(frameSizes);
    const auto sortedFrames = sorter.sort();

    _adjustSortedPaths(*sortedFrames, *imageData);

    int notUsedPercent = kBasePercent;
    int left, top, right, bottom;
    QVariantMap frames;
    std::unique_ptr<QImage> result;

    bool enoughSpace;
    do {
        enoughSpace = true;
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

        for (auto it = sortedFrames->begin(); it != sortedFrames->end(); ++it) {
            const auto imageDataIt = imageData->find(*it);
            if (imageDataIt == imageData->end())
                continue;

            const auto& beforeTrimSize = imageDataIt->second.beforeCropSize;
            const auto& cropRect = imageDataIt->second.cropRect;
            QVariantMap frameInfo;

            if (!imageDataIt->second.duplicate) {
                QImage image(imageDataIt->second.pathOrDuplicateFrameName);

                bool orientation = cropRect.width() > cropRect.height();
                auto packedRect = bin.Insert(cropRect.width() + _padding * 2, cropRect.height() + _padding * 2, rbp::MaxRectsBinPack::RectBestLongSideFit);

                if (packedRect.height > 0) {
                    const bool isRotated = packedRect.width > packedRect.height != orientation;

                    QRect finalRect(packedRect.x + _padding,
                                    packedRect.y + _padding,
                                    cropRect.width() - 2 * _padding,
                                    cropRect.height() - 2 * _padding);

                    frameInfo["rotated"] = isRotated;
                    frameInfo["frame"] = finalRect;
                    frameInfo["sourceColorRect"] = QString("{{%1,%2},{%3,%4}}").arg(
                                QString::number(cropRect.x()),
                                QString::number(cropRect.y()),
                                QString::number(finalRect.width()),
                                QString::number(finalRect.height()));

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
                } else {
                    enoughSpace = false;
                    break;
                }
            } else {
                const auto otherImageInfo = frames[imageDataIt->second.pathOrDuplicateFrameName].toMap();
                frameInfo["rotated"] = otherImageInfo["rotated"].toBool();
                frameInfo["frame"] = otherImageInfo["frame"].toRect();
                frameInfo["sourceColorRect"] = QString("{{%1,%2},{%3,%4}}").arg(
                            QString::number(cropRect.x()),
                            QString::number(cropRect.y()),
                            QString::number(cropRect.width() - 2 * _padding),
                            QString::number(cropRect.height() - 2 * _padding));
            }

            if (beforeTrimSize.width() != cropRect.width() || beforeTrimSize.height() != cropRect.height()) {
                const int w = std::floor(cropRect.x() + 0.5f * (-beforeTrimSize.width() + cropRect.width()));
                const int h = std::floor(-cropRect.y() + 0.5f * (beforeTrimSize.height() - cropRect.height()));
                frameInfo["offset"] = QString("{%1,%2}").arg(QString::number(w), QString::number(h));
            } else {
                frameInfo["offset"] = "{0,0}";
            }

            frameInfo["sourceSize"] = QString("{%1,%2}").arg(
                        QString::number(beforeTrimSize.width()),
                        QString::number(beforeTrimSize.height()));

            frames[*it] = frameInfo;
        }
        painter.end();
    } while (!enoughSpace);

    _removeTempFiles(*imageData);

    QRect finalCrop(QPoint(left, top), QPoint(right, bottom));
    finalCrop.setSize(_fitSize(finalCrop.size()));
    _adjustFrames(frames, [&finalCrop](QRect& rect) {
        rect.setX(rect.x() - finalCrop.x());
        rect.setY(rect.y() - finalCrop.y());
    });

    return _saveResults(result->copy(finalCrop), frames, finalImagePath);
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

auto Generator::_removeTempFiles(const ImageData& paths)->void {
    for (auto data : paths) {
        if (!data.second.duplicate) {
            QFile file(data.second.pathOrDuplicateFrameName);
            file.remove();
        }
    }
}

auto Generator::_saveResults(const QImage& image, const QVariantMap& frames, const QString& finalImagePath)->bool {
    QImageWriter writer(finalImagePath);
    writer.setFormat("png");

    if (writer.write(image)) {
        fprintf(stdout, "%s\n", qPrintable(finalImagePath + " - success"));

        QFileInfo info(finalImagePath);
        QFile plistFile(info.dir().path() + QDir::separator() + info.baseName() + ".plist");
        if (plistFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&plistFile);

            QVariantMap meta;
            meta["format"] = 2;
            meta["realTextureFileName"] = meta["textureFileName"] = info.baseName() + '.' + info.completeSuffix();
            meta["size"] = QString("{%1,%2}").arg(QString::number(image.size().width()), QString::number(image.size().height()));

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

auto Generator::_checkDuplicate(const QImage& image, const ImageData& otherImages, QString& out)->bool {
    for (auto it = otherImages.begin(); it != otherImages.end(); ++it) {
        if (it->second.duplicate)
            continue;

        QImageReader reader(it->second.pathOrDuplicateFrameName);
        if (reader.canRead()) {
            if (image.size() == reader.size()) {
                auto size = image.size();
                auto image2 = reader.read();
                if (image.pixel(0, 0) == image2.pixel(0, 0) &&
                    image.pixel(size.width() - 1, size.height() - 1) == image2.pixel(size.width() - 1, size.height() - 1) &&
                    image.pixel(size.width() / 2, size.height() / 2) == image2.pixel(size.width() / 2, size.height() / 2))
                {
                    if (image == image2) {
                       out = it->first;
                       return true;
                    }
                }
            }
        }
    }
    return false;
}

auto Generator::_adjustSortedPaths(std::vector<QString>& paths, ImageData& imageData)->void {
    for (auto frameNameIt = paths.begin(); frameNameIt != paths.end();) {
        const auto idIt = imageData.find(*frameNameIt);
        if (idIt == imageData.end() || !idIt->second.duplicate || idIt->second.adjusted) {
            ++frameNameIt;
            continue;
        }

        auto notDuplicateIt = std::find(paths.begin(), paths.end(), idIt->second.pathOrDuplicateFrameName);
        if (notDuplicateIt == paths.end())
            throw std::exception();

        const auto tmp = *frameNameIt;
        paths.erase(frameNameIt);
        paths.insert(notDuplicateIt + 1, tmp);
        idIt->second.adjusted = true;
    }
}

auto Generator::_fitSize(const QSize& size) const->QSize {
    QSize result = size;

    if (_square) {
        auto side = std::max(result.width(), result.height());
        result = QSize(side, side);
    }

    if (_isPowerOf2) {
        result.setWidth(_roundToPowerOf2(result.width()));
        result.setHeight(_roundToPowerOf2(result.height()));
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
    auto result = std::make_shared<ImageData>();
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

        QString duplicateFrameName;
        if (_checkDuplicate(image, *result, duplicateFrameName)) {
            _Data data;
            data.beforeCropSize = beforeTrimSize;
            data.cropRect = cropRect;
            data.pathOrDuplicateFrameName = duplicateFrameName;
            data.duplicate = true;
            result->insert(std::make_pair(fileInfo.baseName() + '.' + fileInfo.completeSuffix(), data));
        } else {
            const QString tmpImagePath = QDir::tempPath() + QDir::separator() + fileInfo.baseName();
            QImageWriter writer(tmpImagePath);
            writer.setFormat("png");
            if (writer.write(image)) {
                _Data data;
                data.beforeCropSize = beforeTrimSize;
                data.cropRect = cropRect;
                data.pathOrDuplicateFrameName = tmpImagePath;
                result->insert(std::make_pair(fileInfo.baseName() + '.' + fileInfo.completeSuffix(), data));
            }
        }
    }
    return result;
}
