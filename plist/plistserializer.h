/*
Copyright (c) 2010 Reilly Watson

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.*/

#ifndef PLISTSERIALIZER_H
#define PLISTSERIALIZER_H

// Qt includes
#include <QIODevice>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QDomElement>
#include <QDomDocument>
#include <QString>

class PListSerializer {
public:
	static QString toPList(const QVariant &variant);
private:
	static QDomElement serializeElement(QDomDocument &doc, const QVariant &variant);
	static QDomElement serializeMap(QDomDocument &doc, const QVariantMap &map);
	static QDomElement serializeList(QDomDocument &doc, const QVariantList &list);
};

#endif // PLISTSERIALIZER_H
