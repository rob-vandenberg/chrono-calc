#include "ui/programicon.h"

#include <QIcon>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QSvgRenderer>

#include <cmath>

// --- Version ------------------------------------------------------------
// src/ui/programicon.cpp 0.9.14

// --- Version history ----------------------------------------------------
// v0.9.14: Initial: the program's own icon drawn from its SVG file, built into the program

namespace chrono_calc {

QPixmap programIconPixmap(int size, qreal devicePixelRatio)
{
    // The SVG file is drawn by QtSvg itself, so no image plugin of the computer is needed
    QSvgRenderer renderer(QStringLiteral(":/chrono-calc.svg"));
    const int pixels = static_cast<int>(std::lround(size * devicePixelRatio));
    QImage image(pixels, pixels, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    renderer.render(&painter);
    painter.end();
    QPixmap pixmap = QPixmap::fromImage(image);
    pixmap.setDevicePixelRatio(devicePixelRatio);
    return pixmap;
}

QIcon programIcon()
{
    QIcon icon;
    for (const int size : {16, 22, 24, 32, 48, 64, 128, 256}) {
        icon.addPixmap(programIconPixmap(size, 1.0));
    }
    return icon;
}

}  // namespace chrono_calc
