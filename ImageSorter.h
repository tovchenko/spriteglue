#ifndef IMAGESORTER_H
#define IMAGESORTER_H

#include <QString>
#include <QSize>
#include <vector>
#include <map>
#include <functional>

class ImageSorter {
public:
    enum SortMode {
        HEIGHT,
        WIDTH,
        AREA,
        MAXSIDE
    };

    ImageSorter(const QString& inputImageDirPath);

    auto sort(const SortMode mode = SortMode::MAXSIDE)->std::vector<QString>;

protected:
    struct Info {
        QString path;
        QSize   size;
    };

    static std::map<SortMode, std::function<bool(const Info&, const Info&)>> _sMultiSorters;
    typedef std::vector<std::function<int(const Info&, const Info&)>> FuncList;

    static bool _msort(const Info& a, const Info& b, FuncList criteria);
    static int _sortWidth(const Info& a, const Info& b);
    static int _sortHeight(const Info& a, const Info& b);
    static int _sortArea(const Info& a, const Info& b);
    static int _sortMax(const Info& a, const Info& b);
    static int _sortMin(const Info& a, const Info& b);

    std::vector<Info>  _files;
};

#endif // IMAGESORTER_H
