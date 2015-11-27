#include <QCoreApplication>

#include "Generator.h"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    Generator spritesheet("/Users/tovchenko/Desktop/atlas");
    spritesheet.setMaxSize(QSize(4096, 4096));
    //spritesheet.trim(Generator::TrimMode::NONE);
    //spritesheet.setPadding(1);
    //spritesheet.enableSquare(true);
    //spritesheet.enablePowerOf2(true);
    if (spritesheet.generateTo("/Users/tovchenko/Desktop/test/myatlas.png"))
        return 0;
    return 1;
}

