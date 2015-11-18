#include <QCoreApplication>

#include "Generator.h"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    Generator spritesheet("/Users/tovchenko/Desktop/atlas");
    spritesheet.setMaxSize(QSize(1024, 1900));
    spritesheet.generateTo("/Users/tovchenko/Desktop/test/myatlas.png");

    return 0;
}

