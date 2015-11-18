#include <QCoreApplication>

#include "Generator.h"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    Generator spritesheet("/Users/tovchenko/Desktop/atlas");
    spritesheet.setMaxSize(QSize(1856, 1024));
    if (spritesheet.generateTo("/Users/tovchenko/Desktop/test/myatlas.png"))
        return 0;
    return 1;
}

