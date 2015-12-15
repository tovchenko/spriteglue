/* Generator.h
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

#ifndef GENERATOR_H
#define GENERATOR_H

#include <QImage>
#include <memory>
#include <set>

class Generator {
public:
    enum TrimMode {
        NONE,
        ALL_ALPHA,
        MAX_ALPHA
    };

    Generator(const QString& inputImageDirPath);

    auto setScale(float scale)->void { _scale = scale; }
    auto setMaxSize(const QSize& size)->void { _maxSize = size; }
    auto setPadding(int padding)->void { _padding = padding; }
    auto setMargin(int margin)->void { _margin = margin; }
    auto setTrimMode(TrimMode mode)->void { _trim = mode; }
    auto setIsSquare(bool square)->void { _square = square; }
    auto setIsPowerOf2(bool isPow2)->void { _isPowerOf2 = isPow2; }
    auto setOutputFormat(QImage::Format format)->void { _outputFormat = format; }
    auto setTextureSuffixInData(const QString& suffix)->void { _suffix = suffix; }

    auto generateTo(const QString& finalImagePath, const QString& plistPath="")->bool;

protected:
    struct _Data {
        _Data() : duplicated(false), adjusted(false) {}
        QSize   beforeCropSize;
        QRect   cropRect;
        QString pathOrDuplicateFrameName;
        bool    duplicated;
        bool    adjusted;
    };
    typedef std::map<QString, _Data> ImageData;

    static auto _roundToPowerOf2(int value)->int;
    static auto _floorToPowerOf2(int value)->int;
    static auto _adjustFrames(QVariantMap& frames, const std::function<void(QRect&)>& cb)->void;
    static auto _removeTempFiles(const ImageData& paths)->void;
    static auto _checkDuplicate(const QImage& image, const std::map<QString, QString> paths, QString& out)->bool;
    static auto _adjustSortedPaths(std::vector<QString>& paths, ImageData& imageData)->void;
    auto _saveResults(const QImage& image, const QVariantMap& frames, const QString& finalImagePath, const QString& plistPath) const->bool;
    auto _fitSize(const QSize& size, bool& optimal) const->QSize;
    auto _readFileList() const->std::shared_ptr<std::set<QString>>;
    auto _processImages() const->std::shared_ptr<ImageData>;

    float           _scale = 1.0f;
    QSize           _maxSize = { 0, 0 };
    int             _padding = 0;
    int             _margin = 1;
    TrimMode        _trim = MAX_ALPHA;
    bool            _square = false;
    bool            _isPowerOf2 = false;
    QImage::Format  _outputFormat = QImage::Format_RGBA8888;
    QString         _suffix;

    QString         _inputImageDirPath;
};

#endif // GENERATOR_H
