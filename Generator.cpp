/* Generator.cpp
Copyright (C) 2015 Taras Tovchenko
Email: doctorset@gmail.com

You can redistribute and/or modify this software under the terms of the GNU
General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this
program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA */

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
#include <map>

const int kBasePercent = 10;
const int kStepPercent = 2;
const int kAreaInnerPaddingMagic = 2;

Generator::Generator(const QString& inputImageDirPath)
    : _inputImageDirPath(inputImageDirPath) {
}

auto Generator::generateTo(const QString& finalImagePath, const QString& plistPath)->bool {
    auto imageData = _processImages();
    ImageSorter::FrameSizes frameSizes;
    std::transform(imageData->begin(), imageData->end(), std::back_inserter(frameSizes), [](const std::pair<QString, _Data>& data) {
        return std::make_pair(data.first, data.second.cropRect.size());
    });

    auto area = std::accumulate(imageData->begin(), imageData->end(), 0, [this](int sum, const std::pair<QString, _Data>& data) {
        return data.second.duplicated
            ? sum
            : sum + (
                (data.second.cropRect.width() + 2 * _padding + _innerPadding / kAreaInnerPaddingMagic) *
                (data.second.cropRect.height() + 2 * _padding + _innerPadding / kAreaInnerPaddingMagic));
    });

    ImageSorter sorter(frameSizes);
    const auto sortedFrames = sorter.sort();

    _adjustSortedPaths(*sortedFrames, *imageData);

    int notUsedPercent = kBasePercent;
    int desiredRatioWidth = _maxSize.width() / _maxSize.height();
    desiredRatioWidth = desiredRatioWidth != 0 ? desiredRatioWidth : 1;
    int desiredRatioHeight = _maxSize.height() / _maxSize.width();
    desiredRatioHeight = desiredRatioHeight != 0 ? desiredRatioHeight : 1;

    int left, top, right, bottom;
    QVariantMap frames;
    QImage result(_maxSize, _outputFormat);
    QRect finalCrop(QPoint(0, 0), _maxSize);

    bool optimal = true;
    bool enoughSpace;
    const bool nonSquarePowerOf2 = !_square && _isPowerOf2;
    do {
        enoughSpace = true;
        if (!nonSquarePowerOf2 || optimal)
            notUsedPercent += kStepPercent;
        
        const int side = floor(sqrtf(area + area * notUsedPercent / 100));
        QSize beforeSize(side, side);
        if (nonSquarePowerOf2 && !optimal) {
            beforeSize.setWidth(_floorToPowerOf2(side));
            beforeSize.setHeight(_roundToPowerOf2(side));
            optimal = true;
        } else if (_square && _isPowerOf2) {
            beforeSize.setWidth(_roundToPowerOf2(side));
            beforeSize.setHeight(_roundToPowerOf2(side));
        } else if (!_square) {
            beforeSize.setWidth(side * desiredRatioWidth);
            beforeSize.setHeight(side * desiredRatioHeight);
        }

        rbp::MaxRectsBinPack bin(beforeSize.width(), beforeSize.height());
        result.fill(QColor(0, 0, 0 ,0));
        QPainter painter(&result);
        frames.clear();
        left = beforeSize.width() - 1;
        top = beforeSize.height() - 1;
        right = 0;
        bottom = 0;

        for (const auto& frame : *sortedFrames) {
            const auto imageDataIt = imageData->find(frame);
            if (imageDataIt == imageData->end())
                continue;

            const auto& beforeTrimSize = imageDataIt->second.beforeCropSize;
            const auto& cropRect = imageDataIt->second.cropRect;
            QVariantMap frameInfo;

            if (!imageDataIt->second.duplicated) {
                QImage image(imageDataIt->second.pathOrDuplicateFrameName);

                bool orientation = cropRect.width() > cropRect.height();
                const auto packedRect = bin.Insert(cropRect.width() + _padding * 2 + _innerPadding, cropRect.height() + _padding * 2 + _innerPadding, rbp::MaxRectsBinPack::RectBestLongSideFit);

                if (packedRect.height > 0) {
                    const bool isRotated = packedRect.width > packedRect.height != orientation;

                    frameInfo["rotated"] = isRotated;
                    frameInfo["frame"] = QRect(packedRect.x + _padding,
                                            packedRect.y + _padding,
                                            cropRect.width(),
                                            cropRect.height());

                    if (isRotated)
                        image = rotate90(image);
                    painter.drawImage(packedRect.x + _padding, packedRect.y + _padding, image);

                    if (packedRect.x < left)
                        left = packedRect.x;
                    if (packedRect.y < top)
                        top = packedRect.y;
                    if (packedRect.x + packedRect.width - 1 > right)
                        right = packedRect.x + packedRect.width - 1;
                    if (packedRect.y + packedRect.height - 1 > bottom)
                        bottom = packedRect.y + packedRect.height - 1;
                } else {
                    enoughSpace = false;
                    break;
                }
            } else {
                const auto otherImageInfo = frames[imageDataIt->second.pathOrDuplicateFrameName].toMap();
                frameInfo["rotated"] = otherImageInfo["rotated"].toBool();
                frameInfo["frame"] = otherImageInfo["frame"].toRect();
            }

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
                                QString::number(cropRect.width()),
                                QString::number(cropRect.height()));

            frameInfo["sourceSize"] = QString("{%1,%2}").arg(
                        QString::number(beforeTrimSize.width() + 2 * _padding + _innerPadding),
                        QString::number(beforeTrimSize.height() + 2 * _padding + _innerPadding));

            frames[frame] = frameInfo;
        }
        painter.end();

        right -= _innerPadding;
        bottom -= _innerPadding;
        finalCrop = QRect(QPoint(left, top), QPoint(right, bottom));
        finalCrop.setSize(_fitSize(finalCrop.size(), optimal));   
    } while (!enoughSpace || (nonSquarePowerOf2 && !optimal));

    _removeTempFiles(*imageData);

    if (finalCrop.width() > _maxSize.width() || finalCrop.height() > _maxSize.height()) {
        fprintf(stderr, "%s%dx%d%s%dx%d\n", qPrintable(finalImagePath + " "), finalCrop.width(), finalCrop.height(), " - too large for available max size: ", _maxSize.width(), _maxSize.height());
        return false;
    }

    _adjustFrames(frames, [&finalCrop](QRect& rect) {
        rect.setX(rect.x() - finalCrop.x());
        rect.setY(rect.y() - finalCrop.y());
    });

    return _saveResults(result.copy(finalCrop), frames, finalImagePath, plistPath);
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
        if (!data.second.duplicated) {
            QFile file(data.second.pathOrDuplicateFrameName);
            file.remove();
        }
    }
}

auto Generator::_checkDuplicate(const QImage& image, const std::map<QString, QString> paths, QString& out)->bool {
    for (auto item : paths) {
        QImageReader reader(item.second);
        if (reader.canRead()) {
            if (image.size() == reader.size()) {
                auto size = image.size();
                auto image2 = reader.read();
                if (image.pixel(0, 0) == image2.pixel(0, 0) &&
                    image.pixel(size.width() - 1, size.height() - 1) == image2.pixel(size.width() - 1, size.height() - 1) &&
                    image.pixel(size.width() / 2, size.height() / 2) == image2.pixel(size.width() / 2, size.height() / 2) &&
                    image.pixel(size.width() * 3.0f/4.0f, size.height() / 4) == image2.pixel(size.width() * 3.0f/4.0f, size.height() / 4) &&
                    image.pixel(size.width() / 4, size.height() * 3.0f/4.0f) == image2.pixel(size.width() / 4, size.height() * 3.0f/4.0f))
                {
                    if (image == image2) {
                       out = item.first;
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
        if (idIt == imageData.end() || !idIt->second.duplicated || idIt->second.adjusted) {
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

auto Generator::_saveResults(const QImage& image, const QVariantMap& frames, const QString& finalImagePath, const QString& plistPath) const->bool {
    QImageWriter writer(finalImagePath);
    writer.setFormat("png");

    if (writer.write(image)) {
        fprintf(stdout, "%s\n", qPrintable(finalImagePath + " - success"));

        QFileInfo info(finalImagePath);
        QFile plistFile(plistPath.isEmpty() ? info.dir().path() + QDir::separator() + info.baseName() + ".plist" : plistPath);
        if (plistFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&plistFile);

            QVariantMap meta;
            meta["format"] = 2;
            meta["realTextureFileName"] = meta["textureFileName"] = info.baseName() + '.' + (_suffix.isEmpty() ? info.completeSuffix() : _suffix);
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

auto Generator::_fitSize(const QSize& size, bool& optimal) const->QSize {
    QSize result = size;
    optimal = true;

    if (_square) {
        auto side = std::max(result.width(), result.height());
        result = QSize(side, side);
    }

    if (_isPowerOf2) {
        result.setWidth(_roundToPowerOf2(result.width()));
        result.setHeight(_roundToPowerOf2(result.height()));
        if (size.width() * size.height() <= (result.width() * result.height()) / 2)
            optimal = false;
    }
    return result;
}

auto Generator::_readFileList() const->std::shared_ptr<std::set<QString>> {
    auto result = std::make_shared<std::set<QString>>();
    QDirIterator it(_inputImageDirPath, QStringList() << "*.*", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        result->insert(it.next());
    }
    return result;
}

auto Generator::_processImages() const->std::shared_ptr<ImageData> {
    auto result = std::make_shared<ImageData>();

    const auto files = _readFileList();
    for (auto file : *files) {
        QImage image(file);
        if (_scale < 1.0f)
            image = image.scaledToWidth(_scale * image.width(), Qt::SmoothTransformation);

        const QSize beforeTrimSize = image.size();
        QRect cropRect(QPoint(0, 0), beforeTrimSize);
        if (_trim != TrimMode::NONE)
            image = ImageTrim::createImage(image, _trim == TrimMode::MAX_ALPHA, cropRect);

        const QFileInfo fileInfo(file);
        const QString tmpImagePath = QDir::tempPath() + QDir::separator() + fileInfo.baseName();
        QImageWriter writer(tmpImagePath);
        writer.setFormat("png");
        if (writer.write(image)) {
            _Data data;
            data.beforeCropSize = beforeTrimSize;
            data.cropRect = cropRect;
            data.pathOrDuplicateFrameName = tmpImagePath;
            result->insert(std::make_pair(QDir(_inputImageDirPath).relativeFilePath(file), data));
        }
    }

    if (result->size() < files->size()) {
        fprintf(stderr, "%s\n", "Found an invalid image or the scale coefficient has been chosen too small.");
        result->clear();
    }

    std::map<QString, QString> framePathData;
    for (auto& item : *result) {
        QImage image(item.second.pathOrDuplicateFrameName);
        QString duplicateFrameName;
        const QString itemPath = item.second.pathOrDuplicateFrameName;
        if (_checkDuplicate(image, framePathData, duplicateFrameName)) {
            QFile file(itemPath);
            file.remove();
            item.second.pathOrDuplicateFrameName = duplicateFrameName;
            item.second.duplicated = true;
        } else {
            framePathData.insert(std::make_pair(item.first, itemPath));
        }
    }

    return result;
}
