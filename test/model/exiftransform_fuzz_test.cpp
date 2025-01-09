#include "src/model/exiftransform.h"

#include <cassert>
#include <cstdlib>
#include <cstring>

namespace {

void test_getOrientation(const uint8_t* data, size_t size)
{
    ExifTransform::Orientation orientation =
        ExifTransform::getOrientation(QByteArray(reinterpret_cast<const char*>(data), size));
    assert(orientation >= ExifTransform::Orientation::TopLeft
           && orientation <= ExifTransform::Orientation::LeftBottom);
}

void test_applyTransformation(const uint8_t* data, size_t size)
{
    if (size == 0) {
        return;
    }
    ExifTransform::Orientation orientation = static_cast<ExifTransform::Orientation>(data[0] % 8);

    QImage image = QImage::fromData(data + 1, size - 1);
    if (image.isNull()) {
        return;
    }
    QImage transformed = applyTransformation(image, orientation);
    assert(!transformed.isNull());
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size);
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext&, const QString&) {
        // Ignore messages
    });

    if (size == 0)
        return 0;
    switch (data[0]) {
    case 0:
        test_getOrientation(data + 1, size - 1);
        break;
    case 1:
        test_applyTransformation(data + 1, size - 1);
        break;
    default:
        break;
    }
    return 0;
}
