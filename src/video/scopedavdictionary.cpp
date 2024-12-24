#include "scopedavdictionary.h"

#include <QString>

extern "C"
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <libavformat/avformat.h>
#pragma GCC diagnostic pop
}

void ScopedAVDictionary::Setter::operator=(const char* value)
{
    av_dict_set(dict, key, value, 0);
}

void ScopedAVDictionary::Setter::operator=(const QString& value)
{
    *this = value.toStdString().c_str();
}

void ScopedAVDictionary::Setter::operator=(std::int64_t value)
{
    av_dict_set_int(dict, key, value, 0);
}

ScopedAVDictionary::~ScopedAVDictionary()
{
    av_dict_free(&options);
}

ScopedAVDictionary::Setter ScopedAVDictionary::operator[](const char* key)
{
    return {&options, key};
}

AVDictionary** ScopedAVDictionary::get()
{
    return &options;
}
