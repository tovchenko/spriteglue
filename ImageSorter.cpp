#include "ImageSorter.h"

#include <QDirIterator>
#include <QImageReader>

using namespace std::placeholders;
typedef ImageSorter _IS;

ImageSorter::ImageSorter(const std::vector<QString>& filePaths) {
    for (auto it = filePaths.begin(); it != filePaths.end(); ++it) {
        QImageReader reader(*it);
        if (reader.canRead()) {
            Info data;
            data.path = *it;
            data.size = reader.size();
            _files.push_back(data);
        }
    }
}

auto ImageSorter::sort(const SortMode mode)->std::shared_ptr<std::vector<QString>> {
    std::sort(_files.begin(), _files.end(), _sMultiSorters[mode]);
    auto result = std::make_shared<std::vector<QString>>();
    std::transform(_files.begin(), _files.end(), std::back_inserter(*result), [](const Info& info) {
        return info.path;
    });
    return result;
}

decltype(ImageSorter::_sMultiSorters) ImageSorter::_sMultiSorters = {
    { _IS::SortMode::HEIGHT, std::bind(_IS::_msort, _1, _2, _IS::FuncList({_IS::_sortHeight, _IS::_sortWidth}))},
    { _IS::SortMode::WIDTH, std::bind(_IS::_msort, _1, _2, _IS::FuncList({_IS::_sortWidth, _IS::_sortHeight}))},
    { _IS::SortMode::AREA, std::bind(_IS::_msort, _1, _2, _IS::FuncList({_IS::_sortArea, _IS::_sortHeight, _IS::_sortWidth}))},
    { _IS::SortMode::MAXSIDE, std::bind(_IS::_msort, _1, _2, _IS::FuncList({_IS::_sortMax, _IS::_sortMin, _IS::_sortHeight, _IS::_sortWidth}))}
};

bool ImageSorter::_msort(const Info& a, const Info& b, FuncList criteria) {
    for (size_t n = 0; n < criteria.size(); ++n) {
        const int diff = criteria[n](a, b);
        if (diff != 0)
            return diff < 0;
    }
    return 0;
}

int ImageSorter::_sortWidth(const Info& a, const Info& b) {
    return b.size.width() - a.size.width();
}

int ImageSorter::_sortHeight(const Info& a, const Info& b) {
    return b.size.height() - a.size.height();
}

int ImageSorter::_sortArea(const Info& a, const Info& b) {
    return b.size.width() * b.size.height() - a.size.width() * a.size.height();
}

int ImageSorter::_sortMax(const Info& a, const Info& b) {
    return std::max(b.size.width(), b.size.height()) - std::max(a.size.width(), a.size.height());
}

int ImageSorter::_sortMin(const Info& a, const Info& b) {
    return std::min(b.size.width(), b.size.height()) - std::min(a.size.width(), a.size.height());
}
