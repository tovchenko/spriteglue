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
    auto trim(TrimMode mode)->void { _trim = mode; }
    auto enableSquare(bool square)->void { _square = square; }
    auto enablePowerOf2(bool isPow2)->void { _isPowerOf2 = isPow2; }
    auto setOutputFormat(QImage::Format format)->void { _outputFormat = format; }

    auto generateTo(const QString& finalImagePath)->bool;

protected:
    struct _Data {
        _Data() : duplicate(false) {}
        QSize   beforeCropSize;
        QRect   cropRect;
        QString path;
        bool    duplicate;
    };
    typedef std::map<QString, _Data> ImageData;

    static auto _roundToPowerOf2(int value)->int;
    static auto _floorToPowerOf2(int value)->int;
    static auto _adjustFrames(QVariantMap& frames, const std::function<void(QRect&)>& cb)->void;
    static auto _removeTempFiles(const ImageData& paths)->void;
    static auto _saveResults(const QImage& image, const QVariantMap& frames, const QString& finalImagePath)->bool;
    static auto _checkDuplicate(const QImage& image, const ImageData& otherImages, QString& out)->bool;
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

    QString         _inputImageDirPath;
};

#endif // GENERATOR_H
