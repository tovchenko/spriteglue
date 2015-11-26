#include "ImageSorter.h"

using namespace std::placeholders;
typedef ImageSorter _IS;

ImageSorter::ImageSorter(const FrameSizes& files)
: _files(files) {
}

auto ImageSorter::sort(const SortMode mode)->std::shared_ptr<std::vector<QString>> {
    std::sort(_files.begin(), _files.end(), _sMultiSorters[mode]);
    auto result = std::make_shared<std::vector<QString>>();
    std::transform(_files.begin(), _files.end(), std::back_inserter(*result), [](const Info& info) {
        return info.first;
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
    return b.second.width() - a.second.width();
}

int ImageSorter::_sortHeight(const Info& a, const Info& b) {
    return b.second.height() - a.second.height();
}

int ImageSorter::_sortArea(const Info& a, const Info& b) {
    return b.second.width() * b.second.height() - a.second.width() * a.second.height();
}

int ImageSorter::_sortMax(const Info& a, const Info& b) {
    return std::max(b.second.width(), b.second.height()) - std::max(a.second.width(), a.second.height());
}

int ImageSorter::_sortMin(const Info& a, const Info& b) {
    return std::min(b.second.width(), b.second.height()) - std::min(a.second.width(), a.second.height());
}
