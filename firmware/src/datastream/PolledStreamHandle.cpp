#include <datastream/PolledStreamHandle.hpp>

using namespace dlf::datastream;

PolledStreamHandle::PolledStreamHandle(PolledStream *stream, dlf_stream_idx_t idx, dlf_tick_t sample_interval_ticks, dlf_tick_t sample_phase)
    : AbstractStreamHandle(stream, idx), _sample_interval_ticks(sample_interval_ticks), _sample_phase_ticks(sample_phase) {
}

bool PolledStreamHandle::available(dlf_tick_t tick) {
    bool a = _sample_interval_ticks == 0 || ((tick + _sample_phase_ticks) % _sample_interval_ticks) == 0;

#if defined(DEBUG) && defined(SILLY)
    DEBUG.printf(
        "\tCheck Polled Data\n"
        "\t\tid: %s\n"
        "\t\tAvailable: %d\n",
        stream->id.c_str(), a);
#endif
    return a;
}

size_t PolledStreamHandle::encode_header_into(StreamBufferHandle_t buf) {
#ifdef DEBUG
    DEBUG.printf(
        "\tEncode Polled Header\n"
        "\t\tidx: %d\n"
        "\t\ttype_id: %s\n"
        "\t\tid: %s\n"
        "\t\ttick_interval: %u\n"
        "\t\ttick_phase: %u\n"
        "\t\tnotes: TODO\n",
        idx, stream->type_id.c_str(), stream->id.c_str(), _sample_interval_ticks, _sample_phase_ticks);
#endif
    dlf_polled_stream_header_t h;
    strlcpy(h.type_id, stream->type_id.c_str(), sizeof(h.type_id));
    strlcpy(h.id, stream->id.c_str(), sizeof(h.type_id));
    strlcpy(h.notes, "NOTES....", sizeof(h.type_id));
    h.tick_interval = _sample_interval_ticks;
    h.tick_phase = _sample_phase_ticks;

    xStreamBufferSend(buf, &h, sizeof(h), portMAX_DELAY);
    return 0;
}

size_t PolledStreamHandle::encode_into(StreamBufferHandle_t buf, dlf_tick_t tick) {
#ifdef DEBUG
    DEBUG.printf(
        "\tEncode Polled Data\n"
        "\t\tid: %s\n",
        stream->id.c_str());
#endif
    // Sample data
    return xStreamBufferSend(buf, stream->data_source(), stream->data_size(), 0);
}