#include <QCoreApplication>

#include "Generator.h"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    Generator spritesheet("/Users/tovchenko/Desktop/atlas");
    spritesheet.setMaxSize(QSize(4096, 4096));
    spritesheet.setPadding(20);
    if (spritesheet.generateTo("/Users/tovchenko/Desktop/test/myatlas.png"))
        return 0;
    return 1;
}

