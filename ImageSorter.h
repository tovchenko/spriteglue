/* ImageSorter.h
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

#ifndef IMAGESORTER_H
#define IMAGESORTER_H

#include <QString>
#include <QSize>
#include <vector>
#include <map>
#include <functional>
#include <memory>

class ImageSorter {
public:
    typedef std::pair<QString, QSize> Info;
    typedef std::vector<Info> FrameSizes;

    enum SortMode {
        HEIGHT,
        WIDTH,
        AREA,
        MAXSIDE
    };

    ImageSorter(const FrameSizes& files);

    auto sort(const SortMode mode = SortMode::MAXSIDE)->std::shared_ptr<std::vector<QString>>;

protected:
    static std::map<SortMode, std::function<bool(const Info&, const Info&)>> _sMultiSorters;
    typedef std::vector<std::function<int(const Info&, const Info&)>> FuncList;

    static bool _msort(const Info& a, const Info& b, FuncList criteria);
    static int _sortWidth(const Info& a, const Info& b);
    static int _sortHeight(const Info& a, const Info& b);
    static int _sortArea(const Info& a, const Info& b);
    static int _sortMax(const Info& a, const Info& b);
    static int _sortMin(const Info& a, const Info& b);

    FrameSizes  _files;
};

#endif // IMAGESORTER_H
