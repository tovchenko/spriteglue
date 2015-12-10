/* ImageSorter.cpp
Copyright (C) 2015 Taras Tovchenko
Email: doctorset@gmail.com

You can redistribute and/or modify this software under the terms of the GNU
General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this
program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA */

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
