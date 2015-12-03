#include <QCoreApplication>
#include <QCommandLineParser>

#include "Generator.h"

const int kDefaultTextureSize = 4096;
const auto kSheetInfo = "result texture path";
const auto kDataInfo = "data file path (default: same path with texture)";
const auto kScaleInfo = "scale image factor (default: 1)";
const auto kTrimInfo = "trims source images according to mode (default: max-alpha, available: all-alpha, none)";
const auto kPaddingInfo = "padding between sprites (default: 0)";
const auto kSuffixInfo = "suffix which will be added to atlas data file (default: will be same as resulting texture)";
const auto kMaxSizeWInfo = "max atlas width. if undefined it will use height instead (default: 4096)";
const auto kMaxSizeHInfo = "max atlas height. if undefined it will use width instead (default: 4096)";
const auto kFormatInfo = "color format of resulting texture (default: rgba8888, available: rgb888, rgb666, rgb555, rgb444, alpha8, grayscale8, mono, rgba8888p)";
const auto kSquareInfo = "makes texture square (default: isn\'t square)";
const auto kFreeSizeInfo = "makes texture non power of 2 (default: is powerOf2)";

static auto _printUsage()->void {
    fprintf(stdout, "\n%s\n", qPrintable("spritesheet [path to directory with source images]"));
    fprintf(stdout, "\n%s\n", qPrintable("required:"));
    fprintf(stdout, "\t%s\t%s\n", qPrintable("--sheet"), kSheetInfo);
    fprintf(stdout, "\n%s\n", qPrintable("optional:"));
    fprintf(stdout, "\t%s\t%s\n", qPrintable("--data"), kDataInfo);
    fprintf(stdout, "\t%s\t%s\n", qPrintable("--scale"), kScaleInfo);
    fprintf(stdout, "\t%s\t%s\n", qPrintable("--trim"), kTrimInfo);
    fprintf(stdout, "\t%s\t%s\n", qPrintable("--padding"), kPaddingInfo);
    fprintf(stdout, "\t%s\t%s\n", qPrintable("--suffix"), kSuffixInfo);
    fprintf(stdout, "\t%s\t%s\n", qPrintable("--max-size-w"), kMaxSizeWInfo);
    fprintf(stdout, "\t%s\t%s\n", qPrintable("--max-size-h"), kMaxSizeHInfo);
    fprintf(stdout, "\t%s\t%s\n", qPrintable("--opt"), kFormatInfo);
    fprintf(stdout, "\t%s\t%s\n", qPrintable("--square"), kSquareInfo);
    fprintf(stdout, "\t%s\t%s\n", qPrintable("--allow-free-size"), kFreeSizeInfo);
}

auto main(int argc, char *argv[])->int {
    if (argc < 3) {
        // ./spriteheet imagesDir finalTexturePath
        _printUsage();
        return 1;
    }

    QCoreApplication app(argc, argv);
    QCommandLineParser cmd;
    QCommandLineOption sheetOption(QStringList() << "sheet", kSheetInfo, "sheet");
    QCommandLineOption dataOption(QStringList() << "data", kDataInfo, "data");
    QCommandLineOption scaleOption(QStringList() << "scale", kScaleInfo, "scale");
    QCommandLineOption trimOption(QStringList() << "trim", kTrimInfo, "trim");
    QCommandLineOption paddingOption(QStringList() << "padding", kPaddingInfo, "padding");
    QCommandLineOption suffixOption(QStringList() << "suffix", kSuffixInfo, "suffix");
    QCommandLineOption maxSizeWOption(QStringList() << "max-size-w", kMaxSizeWInfo, "width");
    QCommandLineOption maxSizeHOption(QStringList() << "max-size-h", kMaxSizeHInfo, "height");
    QCommandLineOption formatOption(QStringList() << "opt", kFormatInfo, "format");
    QCommandLineOption squareOption(QStringList() << "square", kSquareInfo);
    QCommandLineOption freeSizeOption(QStringList() << "allow-free-size", kFreeSizeInfo);
    cmd.addOptions(QList<QCommandLineOption>() << sheetOption << dataOption << scaleOption << trimOption << paddingOption << suffixOption <<
                   maxSizeWOption << maxSizeHOption << formatOption << squareOption << freeSizeOption);
    cmd.process(app.arguments());

    const QStringList srcPath = cmd.positionalArguments();
    if (srcPath.length() < 1) {
        fprintf(stderr, "%s\n", qPrintable("No source images passed."));
        _printUsage();
        return 1;
    }
    Generator spritesheet(*srcPath.begin());

    if (!cmd.isSet(sheetOption)) {
        fprintf(stderr, "%s\n", qPrintable("No destination texture path passed."));
        _printUsage();
        return 1;
    }

    QString dataPath;
    if (cmd.isSet(dataOption))
        dataPath = cmd.value(dataOption);
    
    float scale = 1.0f;
    if (cmd.isSet(scaleOption)) {
        bool ok = false;
        scale = cmd.value(scaleOption).toFloat(&ok);
        if (!ok) {
            fprintf(stderr, "%s\n", qPrintable("The value after --scale is not a number value"));
            _printUsage();
            return 1;
        }
    }
    spritesheet.setScale(scale);

    int maxWidth = -1;
    if (cmd.isSet(maxSizeWOption)) {
        bool ok = false;
        maxWidth = cmd.value(maxSizeWOption).toInt(&ok);
        if (!ok) {
            fprintf(stderr, "%s\n", qPrintable("The value after --max-size-w is not a number value"));
            _printUsage();
            return 1;
        }
    }
    int maxHeight = -1;
    if (cmd.isSet(maxSizeHOption)) {
        bool ok = false;
        maxHeight = cmd.value(maxSizeHOption).toInt(&ok);
        if (!ok) {
            fprintf(stderr, "%s\n", qPrintable("The value after --max-size-h is not a number value"));
            _printUsage();
            return 1;
        }
    }
    const auto tmpWidth = maxWidth;
    if (-1 == maxWidth)
        maxWidth = -1 == maxHeight ? kDefaultTextureSize : maxHeight;
    if (-1 == maxHeight)
        maxHeight = -1 == tmpWidth ? kDefaultTextureSize : tmpWidth;
    spritesheet.setMaxSize(QSize(maxWidth, maxHeight));

    auto trimMode = Generator::TrimMode::MAX_ALPHA;
    if (cmd.isSet(trimOption)) {
        if (cmd.value(trimOption) == "all-alpha")
            trimMode = Generator::TrimMode::ALL_ALPHA;
        else if (cmd.value(trimOption) == "none")
            trimMode = Generator::TrimMode::NONE;
    }
    spritesheet.setTrimMode(trimMode);

    int padding = 0;
    if (cmd.isSet(paddingOption)) {
        bool ok = false;
        padding = cmd.value(paddingOption).toInt(&ok);
        if (!ok) {
            fprintf(stderr, "%s\n", qPrintable("The value after --padding is not a number value"));
            _printUsage();
            return 1;
        }
    }
    spritesheet.setPadding(padding);

    if (cmd.isSet(suffixOption))
        spritesheet.setTextureSuffixInData(cmd.value(suffixOption));

    QImage::Format format = QImage::Format::Format_RGBA8888;
    if (cmd.isSet(formatOption)) {
        const auto fmt = cmd.value(formatOption);
        if ("rgb888" == fmt) format = QImage::Format::Format_RGB888;
        else if ("rgb666" == fmt) format = QImage::Format::Format_RGB666;
        else if ("rgb555" == fmt) format = QImage::Format::Format_RGB555;
        else if ("rgb444" == fmt) format = QImage::Format::Format_RGB444;
        else if ("alpha8" == fmt) format = QImage::Format::Format_Alpha8;
        else if ("grayscale8" == fmt) format = QImage::Format::Format_Grayscale8;
        else if ("mono" == fmt) format = QImage::Format::Format_Mono;
        else if ("rgba8888p" == fmt) format = QImage::Format::Format_RGBA8888_Premultiplied;
    }
    spritesheet.setOutputFormat(format);

    spritesheet.setIsSquare(cmd.isSet(squareOption));
    spritesheet.setIsPowerOf2(!cmd.isSet(freeSizeOption));

    if (spritesheet.generateTo(cmd.value(sheetOption), dataPath))
        return 0;

    fprintf(stderr, "%s\n", qPrintable("Something went wrong!"));
    _printUsage();
    return 1;
}

