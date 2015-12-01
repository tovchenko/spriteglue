#ifndef GENERATOR_H
#define GENERATOR_H

#include <QImage>
#include <memory>

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
    auto setTrimMode(TrimMode mode)->void { _trim = mode; }
    auto setIsSquare(bool square)->void { _square = square; }
    auto setIsPowerOf2(bool isPow2)->void { _isPowerOf2 = isPow2; }
    auto setOutputFormat(QImage::Format format)->void { _outputFormat = format; }
    auto setMetaInfoSuffix(const QString& suffix)->void { _suffix = suffix; }

    auto generateTo(const QString& finalImagePath)->bool;

protected:
    struct _Data {
        _Data() : duplicate(false), adjusted(false) {}
        QSize   beforeCropSize;
        QRect   cropRect;
        QString pathOrDuplicateFrameName;
        bool    duplicate;
        bool    adjusted;
    };
    typedef std::map<QString, _Data> ImageData;

    static auto _roundToPowerOf2(int value)->int;
    static auto _floorToPowerOf2(int value)->int;
    static auto _adjustFrames(QVariantMap& frames, const std::function<void(QRect&)>& cb)->void;
    static auto _removeTempFiles(const ImageData& paths)->void;
    static auto _checkDuplicate(const QImage& image, const ImageData& otherImages, QString& out)->bool;
    static auto _adjustSortedPaths(std::vector<QString>& paths, ImageData& imageData)->void;
    auto _saveResults(const QImage& image, const QVariantMap& frames, const QString& finalImagePath) const->bool;
    auto _fitSize(const QSize& size) const->QSize;
    auto _readFileList() const->std::shared_ptr<std::vector<QString>>;
    auto _scaleTrimIfNeeded() const->std::shared_ptr<ImageData>;

    float           _scale = 1.0f;
    QSize           _maxSize = { 0, 0 };
    int             _padding = 0;
    TrimMode        _trim = MAX_ALPHA;
    bool            _square = false;
    bool            _isPowerOf2 = false;
    QImage::Format  _outputFormat = QImage::Format_RGBA8888;
    QString         _suffix;

    QString         _inputImageDirPath;
};

#endif // GENERATOR_H
