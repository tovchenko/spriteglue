#include <QCoreApplication>
#include <QCommandLineParser>

#include "Generator.h"

const int kDefaultTextureSize = 4096;

static auto _printUsage()->void {
    fprintf(stdout, "\n%s\n", qPrintable("spritesheet"));
    fprintf(stdout, "\t%s\t%s\n", qPrintable("--trim"), qPrintable("none, all-alpha, max-alpha(default)"));
}

auto main(int argc, char *argv[])->int {
    if (argc < 3) {
        // ./spriteheet imagesDir finalTexturePath
        _printUsage();
        return 1;
    }

    QCoreApplication app(argc, argv);
    QCommandLineParser cmd;
    QCommandLineOption scaleOption(QStringList() << "s" << "scale", "scale image factor.", "scale");
    QCommandLineOption trimOption(QStringList() << "t" << "trim", "needs to trim images according to mode.", "trim");
    QCommandLineOption paddingOption(QStringList() << "p" << "padding", "padding between sprites.", "padding");
    QCommandLineOption maxSizeWOption(QStringList() << "w" << "max-size-w", "max atlas width. if undefined it will use height instead.", "width");
    QCommandLineOption maxSizeHOption(QStringList() << "h" << "max-size-h", "max atlas height. if undefined it will use width instead.", "height");
    cmd.addOptions(QList<QCommandLineOption>() << scaleOption << trimOption << paddingOption << maxSizeWOption << maxSizeHOption);
    cmd.process(app.arguments());

    const QStringList srcPath = cmd.positionalArguments();
    if (srcPath.length() < 1) {
        fprintf(stderr, "%s\n", qPrintable("No source images passed."));
        _printUsage();
        return 1;
    }
    Generator spritesheet(*srcPath.begin());
    
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

    int padding = 1;
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


    //spritesheet.trim(Generator::TrimMode::NONE);
    //spritesheet.setPadding(1);
    //spritesheet.enableSquare(true);
    //spritesheet.enablePowerOf2(true);
    spritesheet.setMetaInfoSuffix("pvr.ccz");
    if (spritesheet.generateTo("/Users/tovchenko/Desktop/test/myatlas.png"))
        return 0;
    return 1;
}

