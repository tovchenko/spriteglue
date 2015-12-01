#include <QCoreApplication>
#include <QCommandLineParser>

#include "Generator.h"

static auto _printUsage()->void {
    fprintf(stdout, "\n%s\n", qPrintable("spritesheet"));
    fprintf(stdout, "\t%s\n", qPrintable("--trim"));
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
    QCommandLineOption trimOption(QStringList() << "t" << "trim", "needs to trim images.", "trim");
    QCommandLineOption alphaOption(QStringList() << "a" << "alpha", "crop");
    QCommandLineOption maxSizeWOption(QStringList() << "w" << "max-size-w", "max atlas width. if undefined it will use height instead.", "width");
    QCommandLineOption maxSizeHOption(QStringList() << "h" << "max-size-h", "max atlas height. if undefined it will use width instead.", "height");
    cmd.addOptions(QList<QCommandLineOption>() << scaleOption << trimOption << alphaOption << maxSizeWOption << maxSizeHOption);
    cmd.process(app.arguments());
    const QStringList srcPath = cmd.positionalArguments();
    if (srcPath.length() < 1) {
        fprintf(stderr, "%s\n", qPrintable("No source images passed."));
        _printUsage();
        return 1;
    }

    Generator spritesheet(*srcPath.begin());
    spritesheet.setMaxSize(QSize(4096, 4096));
    //spritesheet.trim(Generator::TrimMode::NONE);
    //spritesheet.setPadding(1);
    //spritesheet.enableSquare(true);
    //spritesheet.enablePowerOf2(true);
    spritesheet.setMetaInfoSuffix("pvr.ccz");
    if (spritesheet.generateTo("/Users/tovchenko/Desktop/test/myatlas.png"))
        return 0;
    return 1;
}

