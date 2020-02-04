#include "config.h"

#include <gst/gst.h>

#include "gst_private.h"
#include "gst_cbs.h"

#include <stdarg.h>

#define COBJMACROS
#define NONAMELESSUNION

#include "mfapi.h"
#include "mferror.h"
#include "mfidl.h"
#include "mfobjects.h"

#include "wine/debug.h"
#include "wine/heap.h"
#include "wine/list.h"

WINE_DEFAULT_DEBUG_CHANNEL(mfplat);

struct sample_request
{
    struct list entry;
    IUnknown *token;
};

struct media_source;

struct media_stream
{
    IMFMediaStream IMFMediaStream_iface;
    LONG ref;
    struct media_source *parent_source;
    IMFMediaEventQueue *event_queue;
    IMFStreamDescriptor *descriptor;
    GstElement *appsink;
    GstPad *their_src, *appsink_sink;
    /* usually reflects state of source */
    enum
    {
        STREAM_INACTIVE,
        STREAM_ENABLED,
        STREAM_PAUSED,
        STREAM_RUNNING,
        STREAM_SHUTDOWN,
    } state;
    BOOL eos;
    CRITICAL_SECTION dispatch_samples_cs;
    struct list sample_requests;
    unsigned int pending_samples;
};

struct media_source
{
    IMFMediaSource IMFMediaSource_iface;
    LONG ref;
    IMFMediaEventQueue *event_queue;
    IMFByteStream *byte_stream;
    struct media_stream **streams;
    ULONG stream_count;
    IMFPresentationDescriptor *pres_desc;
    GstElement *demuxer;
    GstPad *my_src, *their_sink;
    enum
    {
        SOURCE_OPENING,
        SOURCE_STOPPED, /* (READY) */
        SOURCE_PAUSED,
        SOURCE_RUNNING,
        SOURCE_SHUTDOWN,
    } state;
    CRITICAL_SECTION streams_cs;
    HANDLE init_complete_event;
};

/* stream */

static void media_source_notify_stream_ended(struct media_source *source);
static void stream_dispatch_samples(struct media_stream *This)
{
    struct sample_request *req, *cursor2;

    if (This->state != STREAM_RUNNING && This->state != STREAM_SHUTDOWN)
        return;

    EnterCriticalSection(&This->dispatch_samples_cs);

    LIST_FOR_EACH_ENTRY_SAFE(req, cursor2, &This->sample_requests, struct sample_request, entry)
    {
        IMFSample *sample;

        if (This->state == STREAM_SHUTDOWN
        /* Not sure if this is correct: */
         || (!(This->pending_samples) && This->eos))
        {
            if (req->token)
            {
                IUnknown_Release(req->token);
            }
            list_remove(&req->entry);
            continue;
        }

        if (!(This->pending_samples))
        {
            break;
        }

        /* Get the sample from the appsink, then construct an IMFSample */
        /* We do this in the dispatch function so we can have appsink buffer for us */
        {
            GstSample *gst_sample;

            TRACE("Trying to pull sample\n");

            g_signal_emit_by_name (This->appsink, "pull-sample", &gst_sample);
            if (!gst_sample)
            {
                ERR("Appsink has no samples and pending_samples != 0\n");
                break;
            }

            sample = mf_sample_from_gst_sample(gst_sample);
        }

        if (req->token)
        {
            IMFSample_SetUnknown(sample, &MFSampleExtension_Token, req->token);
        }

        IMFMediaEventQueue_QueueEventParamUnk(This->event_queue, MEMediaSample, &GUID_NULL, S_OK, (IUnknown *)sample);

        if (req->token)
        {
            IUnknown_Release(req->token);
        }

        list_remove(&req->entry);

        This->pending_samples--;
    }

    if (This->eos && !This->pending_samples && This->state == STREAM_RUNNING)
    {
        PROPVARIANT empty;
        empty.vt = VT_EMPTY;

        IMFMediaEventQueue_QueueEventParamVar(This->event_queue, MEEndOfStream, &GUID_NULL, S_OK, &empty);
        media_source_notify_stream_ended(This->parent_source);
    }
    LeaveCriticalSection(&This->dispatch_samples_cs);
}

static inline struct media_stream *impl_from_IMFMediaStream(IMFMediaStream *iface)
{
    return CONTAINING_RECORD(iface, struct media_stream, IMFMediaStream_iface);
}

static HRESULT WINAPI media_stream_QueryInterface(IMFMediaStream *iface, REFIID riid, void **out)
{
    struct media_stream *This = impl_from_IMFMediaStream(iface);

    TRACE("(%p)->(%s %p)\n", This, debugstr_guid(riid), out);

    if (IsEqualIID(riid, &IID_IMFMediaStream) ||
        IsEqualIID(riid, &IID_IMFMediaEventGenerator) ||
        IsEqualIID(riid, &IID_IUnknown))
    {
        *out = &This->IMFMediaStream_iface;
    }
    else
    {
        FIXME("(%s, %p)\n", debugstr_guid(riid), out);
        *out = NULL;
        return E_NOINTERFACE;
    }

    IUnknown_AddRef((IUnknown*)*out);
    return S_OK;
}

static ULONG WINAPI media_stream_AddRef(IMFMediaStream *iface)
{
    struct media_stream *This = impl_from_IMFMediaStream(iface);
    ULONG ref = InterlockedIncrement(&This->ref);

    TRACE("(%p) ref=%u\n", This, ref);

    return ref;
}

static ULONG WINAPI media_stream_Release(IMFMediaStream *iface)
{
    struct media_stream *This = impl_from_IMFMediaStream(iface);

    ULONG ref = InterlockedDecrement(&This->ref);

    TRACE("(%p) ref=%u\n", This, ref);

    if (!ref)
    {
        ERR("incomplete cleanup\n");
        IMFMediaEventQueue_Release(This->event_queue);
        IMFMediaSource_Release(&This->parent_source->IMFMediaSource_iface);
        heap_free(This);
    }

    return ref;
}

static HRESULT WINAPI media_stream_GetEvent(IMFMediaStream *iface, DWORD flags, IMFMediaEvent **event)
{
    struct media_stream *This = impl_from_IMFMediaStream(iface);

    TRACE("(%p)->(%#x, %p)\n", This, flags, event);

    if (This->state == STREAM_SHUTDOWN)
        return MF_E_SHUTDOWN;

    return IMFMediaEventQueue_GetEvent(This->event_queue, flags, event);
}

static HRESULT WINAPI media_stream_BeginGetEvent(IMFMediaStream *iface, IMFAsyncCallback *callback, IUnknown *state)
{
    struct media_stream *This = impl_from_IMFMediaStream(iface);

    TRACE("(%p)->(%p, %p)\n", This, callback, state);

    if (This->state == STREAM_SHUTDOWN)
        return MF_E_SHUTDOWN;

    return IMFMediaEventQueue_BeginGetEvent(This->event_queue, callback, state);
}

static HRESULT WINAPI media_stream_EndGetEvent(IMFMediaStream *iface, IMFAsyncResult *result, IMFMediaEvent **event)
{
    struct media_stream *This = impl_from_IMFMediaStream(iface);

    TRACE("(%p)->(%p, %p)\n", This, result, event);

    if (This->state == STREAM_SHUTDOWN)
        return MF_E_SHUTDOWN;

    return IMFMediaEventQueue_EndGetEvent(This->event_queue, result, event);
}

static HRESULT WINAPI media_stream_QueueEvent(IMFMediaStream *iface, MediaEventType event_type, REFGUID ext_type,
        HRESULT hr, const PROPVARIANT *value)
{
    struct media_stream *This = impl_from_IMFMediaStream(iface);

    TRACE("(%p)->(%d, %s, %#x, %p)\n", This, event_type, debugstr_guid(ext_type), hr, value);

    if (This->state == STREAM_SHUTDOWN)
        return MF_E_SHUTDOWN;

    return IMFMediaEventQueue_QueueEventParamVar(This->event_queue, event_type, ext_type, hr, value);
}

static HRESULT WINAPI media_stream_GetMediaSource(IMFMediaStream *iface, IMFMediaSource **source)
{
    struct media_stream *This = impl_from_IMFMediaStream(iface);

    FIXME("stub (%p)->(%p)\n", This, source);

    if (This->state == STREAM_SHUTDOWN)
        return MF_E_SHUTDOWN;

    return E_NOTIMPL;
}

static HRESULT WINAPI media_stream_GetStreamDescriptor(IMFMediaStream* iface, IMFStreamDescriptor **descriptor)
{
    struct media_stream *This = impl_from_IMFMediaStream(iface);

    TRACE("(%p)->(%p)\n", This, descriptor);

    if (This->state == STREAM_SHUTDOWN)
        return MF_E_SHUTDOWN;

    IMFStreamDescriptor_AddRef(This->descriptor);
    *descriptor = This->descriptor;

    return S_OK;
}

static HRESULT WINAPI media_stream_RequestSample(IMFMediaStream *iface, IUnknown *token)
{
    struct media_stream *This = impl_from_IMFMediaStream(iface);
    struct sample_request *req;

    TRACE("(%p)->(%p)\n", iface, token);

    if (This->state == STREAM_SHUTDOWN)
        return MF_E_SHUTDOWN;

    if (This->state == STREAM_INACTIVE || This->state == STREAM_ENABLED)
    {
        WARN("Stream isn't active\n");
        return MF_E_MEDIA_SOURCE_WRONGSTATE;
    }

    if (This->eos && !This->pending_samples)
        return MF_E_END_OF_STREAM;

    req = heap_alloc(sizeof(*req));
    if (token)
        IUnknown_AddRef(token);
    req->token = token;
    list_add_tail(&This->sample_requests, &req->entry);

    stream_dispatch_samples(This);

    return S_OK;
}

static const IMFMediaStreamVtbl media_stream_vtbl =
{
    media_stream_QueryInterface,
    media_stream_AddRef,
    media_stream_Release,
    media_stream_GetEvent,
    media_stream_BeginGetEvent,
    media_stream_EndGetEvent,
    media_stream_QueueEvent,
    media_stream_GetMediaSource,
    media_stream_GetStreamDescriptor,
    media_stream_RequestSample
};

static GstFlowReturn stream_new_sample(GstElement *appsink, gpointer user)
{
    struct media_stream *This = (struct media_stream *) user;

    TRACE("(%p) got sample\n", This);

    if (This->state == STREAM_INACTIVE)
    {
        ERR("got sample on inactive stream\n");
    }

    This->pending_samples++;
    stream_dispatch_samples(This);
    return GST_FLOW_OK;
}

void stream_eos(GstElement *appsink, gpointer user)
{
    struct media_stream *This = (struct media_stream *) user;

    TRACE("(%p) EOS\n", This);

    This->eos = TRUE;

    stream_dispatch_samples(This);
}

static void media_stream_teardown(struct media_stream *This)
{
    TRACE("(%p)\n", This);

    This->state = STREAM_SHUTDOWN;

    if (This->their_src)
        gst_object_unref(GST_OBJECT(This->their_src));
    if (This->appsink_sink)
        gst_object_unref(GST_OBJECT(This->appsink_sink));
    if (This->appsink)
    {
        gst_element_set_state(This->appsink, GST_STATE_NULL);
        gst_object_unref(GST_OBJECT(This->appsink));
    }

    /* Frees pending requests and samples when state == STREAM_SHUTDOWN */
    stream_dispatch_samples(This);

    if (This->descriptor)
        IMFStreamDescriptor_Release(This->descriptor);
    if (This->event_queue)
        IMFMediaEventQueue_Release(This->event_queue);
    if (This->parent_source)
        IMFMediaSource_Release(&This->parent_source->IMFMediaSource_iface);

    DeleteCriticalSection(&This->dispatch_samples_cs);
}

static HRESULT media_stream_constructor(struct media_source *source, GstPad *pad, DWORD stream_id, struct media_stream **out_stream)
{
    HRESULT hr;
    GstCaps *caps = NULL;
    IMFMediaType *media_type;
    IMFMediaTypeHandler *type_handler;
    struct media_stream *This = heap_alloc_zero(sizeof(*This));

    TRACE("(%p %p)->(%p)\n", source, pad, out_stream);

    This->state = STREAM_INACTIVE;
    This->pending_samples = 0;
    list_init(&This->sample_requests);
    This->eos = FALSE;
    InitializeCriticalSection(&This->dispatch_samples_cs);

    if (FAILED(hr = IMFMediaSource_AddRef(&source->IMFMediaSource_iface)))
    {
        goto fail;
    }
    This->parent_source = source;

    if (FAILED(hr = MFCreateEventQueue(&This->event_queue)))
    {
        goto fail;
    }

    caps = gst_pad_query_caps(pad, NULL);

    if (!(caps))
    {
        goto fail;
    }

    if (FAILED(hr = MFCreateMediaType(&media_type)))
    {
        goto fail;
    }

    caps = gst_caps_make_writable(caps);
    media_type = mfplat_media_type_from_caps(caps);
    gst_caps_unref(caps);
    caps = NULL;

    MFCreateStreamDescriptor(stream_id, 1, &media_type, &This->descriptor);

    IMFStreamDescriptor_GetMediaTypeHandler(This->descriptor, &type_handler);
    IMFMediaTypeHandler_SetCurrentMediaType(type_handler, media_type);
    IMFMediaTypeHandler_Release(type_handler);
    IMFMediaType_Release(media_type);
    media_type = NULL;

    /* Setup appsink element, but don't link it to the demuxer (it isn't selected by default) */
    if (!(This->appsink = gst_element_factory_make("appsink", NULL)))
    {
        hr = E_OUTOFMEMORY;
        goto fail;
    }

    g_object_set(This->appsink, "emit-signals", TRUE, NULL);
    g_signal_connect(This->appsink, "new-sample", G_CALLBACK(stream_new_sample_wrapper), This);
    g_signal_connect(This->appsink, "eos", G_CALLBACK(stream_eos_wrapper), This);

    /* always in playing state */
    gst_element_set_state(This->appsink, GST_STATE_PLAYING);

    This->appsink_sink = gst_element_get_static_pad(This->appsink, "sink");

    This->their_src = pad;
    gst_pad_set_element_private(pad, This);

    This->IMFMediaStream_iface.lpVtbl = &media_stream_vtbl;
    This->ref = 1;

    TRACE("->(%p)\n", This);

    *out_stream = This;
    return S_OK;

    fail:
    WARN("Failed to construct media stream, hr %#x.\n", hr);

    /* Destroy temporary objects */
    if (caps)
        gst_caps_unref(caps);
    if (media_type)
        IMFMediaType_Release(media_type);

    media_stream_teardown(This);
    heap_free(This);
    return hr;
}

/* source */

static inline struct media_source *impl_from_IMFMediaSource(IMFMediaSource *iface)
{
    return CONTAINING_RECORD(iface, struct media_source, IMFMediaSource_iface);
}

static HRESULT WINAPI media_source_QueryInterface(IMFMediaSource *iface, REFIID riid, void **out)
{
    struct media_source *This = impl_from_IMFMediaSource(iface);

    TRACE("(%p)->(%s %p)\n", This, debugstr_guid(riid), out);

    if (IsEqualIID(riid, &IID_IMFMediaSource) ||
        IsEqualIID(riid, &IID_IMFMediaEventGenerator) ||
        IsEqualIID(riid, &IID_IUnknown))
    {
        *out = &This->IMFMediaSource_iface;
    }
    else
    {
        FIXME("(%s, %p)\n", debugstr_guid(riid), out);
        *out = NULL;
        return E_NOINTERFACE;
    }

    IUnknown_AddRef((IUnknown*)*out);
    return S_OK;
}

static ULONG WINAPI media_source_AddRef(IMFMediaSource *iface)
{
    struct media_source *This = impl_from_IMFMediaSource(iface);
    ULONG ref = InterlockedIncrement(&This->ref);

    TRACE("(%p) ref=%u\n", This, ref);

    return ref;
}

static ULONG WINAPI media_source_Release(IMFMediaSource *iface)
{
    struct media_source *This = impl_from_IMFMediaSource(iface);
    ULONG ref = InterlockedDecrement(&This->ref);

    TRACE("(%p) ref=%u\n", This, ref);

    if (!ref)
    {
        if (This->state != SOURCE_SHUTDOWN)
            ERR("Application has freed media source without calling ::Shutdown\n");
        heap_free(This);
    }

    return ref;
}

static HRESULT WINAPI media_source_GetEvent(IMFMediaSource *iface, DWORD flags, IMFMediaEvent **event)
{
    struct media_source *This = impl_from_IMFMediaSource(iface);

    TRACE("(%p)->(%#x, %p)\n", This, flags, event);

    if (This->state == SOURCE_SHUTDOWN)
        return MF_E_SHUTDOWN;

    return IMFMediaEventQueue_GetEvent(This->event_queue, flags, event);
}

static HRESULT WINAPI media_source_BeginGetEvent(IMFMediaSource *iface, IMFAsyncCallback *callback, IUnknown *state)
{
    struct media_source *This = impl_from_IMFMediaSource(iface);

    TRACE("(%p)->(%p, %p)\n", This, callback, state);

    if (This->state == SOURCE_SHUTDOWN)
        return MF_E_SHUTDOWN;

    return IMFMediaEventQueue_BeginGetEvent(This->event_queue, callback, state);
}

static HRESULT WINAPI media_source_EndGetEvent(IMFMediaSource *iface, IMFAsyncResult *result, IMFMediaEvent **event)
{
    struct media_source *This = impl_from_IMFMediaSource(iface);

    TRACE("(%p)->(%p, %p)\n", This, result, event);

    if (This->state == SOURCE_SHUTDOWN)
        return MF_E_SHUTDOWN;

    return IMFMediaEventQueue_EndGetEvent(This->event_queue, result, event);
}

static HRESULT WINAPI media_source_QueueEvent(IMFMediaSource *iface, MediaEventType event_type, REFGUID ext_type,
        HRESULT hr, const PROPVARIANT *value)
{
    struct media_source *This = impl_from_IMFMediaSource(iface);

    TRACE("(%p)->(%d, %s, %#x, %p)\n", This, event_type, debugstr_guid(ext_type), hr, value);

    if (This->state == SOURCE_SHUTDOWN)
        return MF_E_SHUTDOWN;

    return IMFMediaEventQueue_QueueEventParamVar(This->event_queue, event_type, ext_type, hr, value);
}

static HRESULT WINAPI media_source_GetCharacteristics(IMFMediaSource *iface, DWORD *characteristics)
{
    struct media_source *This = impl_from_IMFMediaSource(iface);

    FIXME("(%p)->(%p): stub\n", This, characteristics);

    if (This->state == SOURCE_SHUTDOWN)
        return MF_E_SHUTDOWN;

    return E_NOTIMPL;
}

static HRESULT WINAPI media_source_CreatePresentationDescriptor(IMFMediaSource *iface, IMFPresentationDescriptor **descriptor)
{
    struct media_source *This = impl_from_IMFMediaSource(iface);

    TRACE("(%p)->(%p)\n", This, descriptor);

    if (This->state == SOURCE_SHUTDOWN)
        return MF_E_SHUTDOWN;

    if (!(This->pres_desc))
    {
        return MF_E_NOT_INITIALIZED;
    }

    IMFPresentationDescriptor_Clone(This->pres_desc, descriptor);

    return S_OK;
}

static HRESULT WINAPI media_source_Start(IMFMediaSource *iface, IMFPresentationDescriptor *descriptor,
                                     const GUID *time_format, const PROPVARIANT *start_position)
{
    struct media_source *This = impl_from_IMFMediaSource(iface);
    PROPVARIANT empty_var;
    empty_var.vt = VT_EMPTY;

    TRACE("(%p)->(%p, %p, %p)\n", This, descriptor, time_format, start_position);

    if (This->state == SOURCE_SHUTDOWN)
        return MF_E_SHUTDOWN;

    /* Find out which streams are active */
    for (unsigned int i = 0; i < This->stream_count; i++)
    {
        IMFStreamDescriptor *stream_desc;
        DWORD in_stream_id;
        BOOL selected;

        IMFPresentationDescriptor_GetStreamDescriptorByIndex(descriptor, i, &selected, &stream_desc);
        IMFStreamDescriptor_GetStreamIdentifier(stream_desc, &in_stream_id);

        for (unsigned int k = 0; k < This->stream_count; k++)
        {
            DWORD cur_stream_id;

            IMFStreamDescriptor_GetStreamIdentifier(This->streams[k]->descriptor, &cur_stream_id);

            if (in_stream_id == cur_stream_id)
            {
                BOOL was_active = This->streams[k]->state != STREAM_INACTIVE;
                This->streams[k]->state = selected ? STREAM_RUNNING : STREAM_INACTIVE;
                if (selected)
                {
                    IMFMediaEventQueue_QueueEventParamUnk(This->event_queue,
                        was_active ? MEUpdatedStream : MENewStream, &GUID_NULL,
                        S_OK, (IUnknown*) &This->streams[k]->IMFMediaStream_iface);
                    IMFMediaEventQueue_QueueEventParamVar(This->streams[k]->event_queue,
                        MEStreamStarted, &GUID_NULL, S_OK, &empty_var);
                    stream_dispatch_samples(This->streams[k]);
                }
            }
        }

        IMFStreamDescriptor_Release(stream_desc);
    }

    if (!IsEqualIID(time_format, &GUID_NULL) || start_position->vt != VT_EMPTY)
    {
        WARN("ignoring start time\n");
        return MF_E_UNSUPPORTED_TIME_FORMAT;
    }

    This->state = SOURCE_RUNNING;
    gst_element_set_state(This->demuxer, GST_STATE_PLAYING);

    IMFMediaEventQueue_QueueEventParamVar(This->event_queue, MESourceStarted, &GUID_NULL, S_OK, &empty_var);

    return S_OK;
}

static HRESULT WINAPI media_source_Stop(IMFMediaSource *iface)
{
    struct media_source *This = impl_from_IMFMediaSource(iface);

    FIXME("(%p): stub\n", This);

    if (This->state == SOURCE_SHUTDOWN)
        return MF_E_SHUTDOWN;

    return E_NOTIMPL;
}

static HRESULT WINAPI media_source_Pause(IMFMediaSource *iface)
{
    struct media_source *This = impl_from_IMFMediaSource(iface);

    FIXME("(%p): stub\n", This);

    if (This->state == SOURCE_SHUTDOWN)
        return MF_E_SHUTDOWN;

    return E_NOTIMPL;
}

static HRESULT media_source_teardown(struct media_source *This)
{
    if (This->my_src)
        gst_object_unref(G_OBJECT(This->my_src));
    if (This->their_sink)
        gst_object_unref(G_OBJECT(This->their_sink));
    if (This->demuxer)
    {
        gst_element_set_state(This->demuxer, GST_STATE_NULL);
        gst_object_unref(G_OBJECT(This->demuxer));
    }
    if (This->pres_desc)
        IMFPresentationDescriptor_Release(This->pres_desc);
    if (This->event_queue)
        IMFMediaEventQueue_Release(This->event_queue);
    if (This->byte_stream)
        IMFByteStream_Release(This->byte_stream);

    for (unsigned int i = 0; i < This->stream_count; i++)
    {
        media_stream_teardown(This->streams[i]);
        IMFMediaStream_Release(&This->streams[i]->IMFMediaStream_iface);
    }

    if (This->stream_count)
        heap_free(This->streams);

    if (This->init_complete_event)
        CloseHandle(This->init_complete_event);
    DeleteCriticalSection(&This->streams_cs);

    return S_OK;
}

static HRESULT WINAPI media_source_Shutdown(IMFMediaSource *iface)
{
    struct media_source *This = impl_from_IMFMediaSource(iface);

    TRACE("(%p)\n", This);

    This->state = SOURCE_SHUTDOWN;
    return media_source_teardown(This);
}

static const IMFMediaSourceVtbl IMFMediaSource_vtbl =
{
    media_source_QueryInterface,
    media_source_AddRef,
    media_source_Release,
    media_source_GetEvent,
    media_source_BeginGetEvent,
    media_source_EndGetEvent,
    media_source_QueueEvent,
    media_source_GetCharacteristics,
    media_source_CreatePresentationDescriptor,
    media_source_Start,
    media_source_Stop,
    media_source_Pause,
    media_source_Shutdown,
};

GstFlowReturn pull_from_bytestream(GstPad *pad, GstObject *parent, guint64 ofs, guint len,
        GstBuffer **buf)
{
    struct media_source *This = gst_pad_get_element_private(pad);
    IMFByteStream *byte_stream = This->byte_stream;
    BOOL is_eof;
    GstMapInfo info;
    ULONG bytes_read;
    HRESULT hr;

    TRACE("gstreamer requesting %u bytes at %s from source %p into buffer %p\n", len, wine_dbgstr_longlong(ofs), This, buf);

    if (ofs != GST_BUFFER_OFFSET_NONE)
    {
        if (FAILED(IMFByteStream_SetCurrentPosition(byte_stream, ofs)))
            return GST_FLOW_ERROR;
    }

    if (FAILED(IMFByteStream_IsEndOfStream(byte_stream, &is_eof)))
        return GST_FLOW_ERROR;
    if (is_eof)
        return GST_FLOW_EOS;

    *buf = gst_buffer_new_and_alloc(len);
    gst_buffer_map(*buf, &info, GST_MAP_WRITE);
    hr = IMFByteStream_Read(byte_stream, info.data, len, &bytes_read);
    gst_buffer_unmap(*buf, &info);

    gst_buffer_set_size(*buf, bytes_read);

    if (FAILED(hr))
    {
        return GST_FLOW_ERROR;
    }
    GST_BUFFER_OFFSET(*buf) = ofs;
    return GST_FLOW_OK;
}

static gboolean query_bytestream(GstPad *pad, GstObject *parent, GstQuery *query)
{
    struct media_source *This = gst_pad_get_element_private(pad);
    GstFormat format;
    QWORD bytestream_len;
    gboolean ret;

    TRACE("GStreamer queries source %p for %s\n", This, GST_QUERY_TYPE_NAME(query));

    if (FAILED(IMFByteStream_GetLength(This->byte_stream, &bytestream_len)))
        return FALSE;

    switch (GST_QUERY_TYPE(query))
    {
        case GST_QUERY_DURATION:
        {
            LONGLONG duration;

            gst_query_parse_duration (query, &format, NULL);
            if (format == GST_FORMAT_PERCENT) {
                gst_query_set_duration (query, GST_FORMAT_PERCENT, GST_FORMAT_PERCENT_MAX);
                return TRUE;
            }
            ret = gst_pad_query_convert (pad, GST_FORMAT_BYTES, bytestream_len, format, &duration);
            gst_query_set_duration(query, format, duration);
            return ret;
        }
        case GST_QUERY_SEEKING:
        {
            gst_query_parse_seeking (query, &format, NULL, NULL, NULL);
            if (format != GST_FORMAT_BYTES)
            {
                WARN("Cannot seek using format \"%s\".\n", gst_format_get_name(format));
                return FALSE;
            }
            gst_query_set_seeking(query, GST_FORMAT_BYTES, 1, 0, bytestream_len);
            return TRUE;
        }
        case GST_QUERY_SCHEDULING:
        {
            gst_query_set_scheduling(query, GST_SCHEDULING_FLAG_SEEKABLE, 1, -1, 0);
            gst_query_add_scheduling_mode(query, GST_PAD_MODE_PULL);
            return TRUE;
        }
        case GST_QUERY_CAPS:
        {
            GstCaps *caps, *filter;

            gst_query_parse_caps(query, &filter);

            caps = gst_caps_new_any();

            if (filter) {
                GstCaps* filtered;
                filtered = gst_caps_intersect_full(
                        filter, caps, GST_CAPS_INTERSECT_FIRST);
                gst_caps_unref(caps);
                caps = filtered;
            }
            gst_query_set_caps_result(query, caps);
            gst_caps_unref(caps);
            return TRUE;
        }
        default:
        {
            WARN("Unhandled query type %s\n", GST_QUERY_TYPE_NAME(query));
            return FALSE;
        }
    }
}

static gboolean activate_bytestream_pad_mode(GstPad *pad, GstObject *parent, GstPadMode mode, gboolean activate)
{
    struct media_source *source = gst_pad_get_element_private(pad);

    TRACE("%s source pad for mediasource %p in %s mode.\n",
            activate ? "Activating" : "Deactivating", source, gst_pad_mode_get_name(mode));

    /* There is no push mode in mfplat */

    switch (mode) {
      case GST_PAD_MODE_PULL:
        return TRUE;
      default:
        return FALSE;
    }
    return FALSE;
}

static gboolean process_bytestream_pad_event(GstPad *pad, GstObject *parent, GstEvent *event)
{
    struct media_source *This = gst_pad_get_element_private(pad);

    TRACE("filter %p, type \"%s\".\n", This, GST_EVENT_TYPE_NAME(event));

    switch (event->type) {
        default:
            WARN("Ignoring \"%s\" event.\n", GST_EVENT_TYPE_NAME(event));
        case GST_EVENT_TAG:
        case GST_EVENT_QOS:
        case GST_EVENT_RECONFIGURE:
            return gst_pad_event_default(pad, parent, event);
    }
    return TRUE;
}

static void source_stream_added(GstElement *element, GstPad *pad, gpointer user)
{
    struct media_stream *stream;
    struct media_source *source = (struct media_source *) user;
    struct media_stream **new_stream_array;
    gchar *g_stream_id;
    const char *stream_id_string;
    DWORD stream_id;

    EnterCriticalSection(&source->streams_cs);

    g_stream_id = gst_pad_get_stream_id(pad);
    stream_id_string = strstr(g_stream_id, "/");
    sscanf(stream_id_string, "/%03u", &stream_id);
    TRACE("stream-id: %u\n", stream_id);
    g_free(g_stream_id);

    /* find existing stream */
    for (unsigned int i = 0; i < source->stream_count; i++)
    {
        DWORD existing_stream_id;
        IMFStreamDescriptor *descriptor = source->streams[i]->descriptor;

        if (FAILED(IMFStreamDescriptor_GetStreamIdentifier(descriptor, &existing_stream_id)))
            goto leave;

        if (existing_stream_id == stream_id)
        {
            struct media_stream *existing_stream = source->streams[i];

            TRACE("Found existing stream %p\n", existing_stream);

            if (!existing_stream->appsink_sink)
            {
                ERR("Couldn't find our appsink sink\n");
                goto leave;
            }

            existing_stream->their_src = pad;
            gst_pad_set_element_private(pad, existing_stream);

            if (existing_stream->state != STREAM_INACTIVE)
            {
                GstPadLinkReturn err = gst_pad_link(existing_stream->their_src, existing_stream->appsink_sink);
                if (err != GST_PAD_LINK_OK)
                {
                    ERR("Error linking demuxer to appsink %u\n", err);
                }
            }
            goto leave;
        }
    }

    if (FAILED(media_stream_constructor(source, pad, stream_id, &stream)))
    {
        goto leave;
    }

    if (!(new_stream_array = heap_realloc(source->streams, (source->stream_count + 1) * (sizeof(*new_stream_array)))))
    {
        ERR("Failed to add stream to source\n");
        goto leave;
    }

    source->streams = new_stream_array;
    source->streams[source->stream_count++] = stream;

    leave:
    LeaveCriticalSection(&source->streams_cs);
    return;
}

static void source_stream_removed(GstElement *element, GstPad *pad, gpointer user)
{
    struct media_stream *stream;

    if (gst_pad_get_direction(pad) != GST_PAD_SRC)
    {
        return;
    }

    stream = (struct media_stream *) gst_pad_get_element_private(pad);

    if (stream)
    {
        if (stream->their_src != pad)
        {
            ERR("assert: unexpected pad/user combination!!!");
            return;
        }
        if (stream->state != STREAM_INACTIVE)
        {
            gst_pad_unlink(stream->their_src, stream->appsink_sink);
        }

        stream->their_src = NULL;
        gst_pad_set_element_private(pad, NULL);
    }
}

static void source_all_streams(GstElement *element, gpointer user)
{
    IMFStreamDescriptor **descriptors;
    struct media_source *source = (struct media_source *) user;

    EnterCriticalSection(&source->streams_cs);
    if (source->state != SOURCE_OPENING)
        goto leave;

    /* Init presentation descriptor */

    descriptors = heap_alloc(source->stream_count * sizeof(IMFStreamDescriptor*));
    for (unsigned int i = 0; i < source->stream_count; i++)
    {
        IMFMediaStream_GetStreamDescriptor(&source->streams[i]->IMFMediaStream_iface, &descriptors[i]);
    }

    if (FAILED(MFCreatePresentationDescriptor(source->stream_count, descriptors, &source->pres_desc)))
        goto leave;

    /*if (SUCCEEDED(IMFByteStream_GetItem(source->byte_stream, &MF_BYTESTREAM_CONTENT_TYPE, &mime_type)))
    {
        IMFPresentationDescriptor_SetItem(source->pres_desc, &MF_PD_MIME_TYPE, mime_type);
        PropVariantClear(&mime_type);
    }*/

    for (unsigned int i = 0; i < source->stream_count; i++)
    {
        IMFStreamDescriptor_Release(descriptors[i]);
    }
    heap_free(descriptors);

    SetEvent(source->init_complete_event);

    leave:
    LeaveCriticalSection(&source->streams_cs);
}

static void media_source_notify_stream_ended(struct media_source *This)
{
    PROPVARIANT empty;
    empty.vt = VT_EMPTY;

    /* A stream has ended, check whether all have */
    for (unsigned int i = 0; i < This->stream_count; i++)
    {
        struct media_stream *stream = This->streams[i];

        if (!stream->eos)
            return;
    }

    IMFMediaEventQueue_QueueEventParamVar(This->event_queue, MEEndOfPresentation, &GUID_NULL, S_OK, &empty);
}

static HRESULT media_source_constructor(IMFByteStream *bytestream, const char *demuxer_name, struct media_source **out_media_source)
{
    GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE(
        "mf_src",
        GST_PAD_SRC,
        GST_PAD_ALWAYS,
        GST_STATIC_CAPS_ANY);

    struct media_source *This = heap_alloc_zero(sizeof(*This));
    int ret;
    HRESULT hr;

    if (!This)
        return E_OUTOFMEMORY;

    This->state = SOURCE_OPENING;
    InitializeCriticalSection(&This->streams_cs);
    This->init_complete_event = CreateEventA(NULL, TRUE, FALSE, NULL);

    /* Setup interface early as the streams interact with us during initialization */
    This->IMFMediaSource_iface.lpVtbl = &IMFMediaSource_vtbl;
    This->ref = 1;

    if (FAILED(hr = IMFByteStream_QueryInterface(bytestream, &IID_IMFByteStream, (void **)&This->byte_stream)))
    {
        goto fail;
    }

    if (FAILED(hr = MFCreateEventQueue(&This->event_queue)))
        goto fail;

    /* create demuxer */

    This->my_src = gst_pad_new_from_static_template(&src_template, "mf-src");
    gst_pad_set_element_private(This->my_src, This);
    gst_pad_set_getrange_function(This->my_src, pull_from_bytestream_wrapper);
    gst_pad_set_query_function(This->my_src, query_bytestream_wrapper);
    gst_pad_set_activatemode_function(This->my_src, activate_bytestream_pad_mode_wrapper);
    gst_pad_set_event_function(This->my_src, process_bytestream_pad_event_wrapper);

    This->demuxer = gst_element_factory_make(demuxer_name, NULL);
    if (!(This->demuxer))
    {
        WARN("Failed to create demuxer for source\n");
        hr = E_OUTOFMEMORY;
        goto fail;
    }

    This->their_sink = gst_element_get_static_pad(This->demuxer, "sink");

    if ((ret = gst_pad_link(This->my_src, This->their_sink)) < 0)
    {
        WARN("Failed to link our bytestream pad to the demuxer input\n");
        hr = E_OUTOFMEMORY;
        goto fail;
    }

    g_signal_connect(This->demuxer, "pad-added", G_CALLBACK(source_stream_added_wrapper), This);
    g_signal_connect(This->demuxer, "pad-removed", G_CALLBACK(source_stream_removed_wrapper), This);
    g_signal_connect(This->demuxer, "no-more-pads", G_CALLBACK(source_all_streams_wrapper), This);

    gst_element_set_state(This->demuxer, GST_STATE_PLAYING);
    ret = gst_element_get_state(This->demuxer, NULL, NULL, -1);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        ERR("Failed to play source.\n");
        hr = E_OUTOFMEMORY;
        goto fail;
    }

    WaitForSingleObject(This->init_complete_event, INFINITE);
    CloseHandle(This->init_complete_event);
    This->init_complete_event = NULL;

    gst_element_set_state(This->demuxer, GST_STATE_READY);
    if (!(This->pres_desc))
    {
        hr = E_FAIL;
        goto fail;
    }

    This->state = SOURCE_STOPPED;

    *out_media_source = This;
    return S_OK;

    fail:
    WARN("Failed to construct MFMediaSource, hr %#x.\n", hr);

    media_source_teardown(This);
    heap_free(This);
    return hr;
}

/* IMFByteStreamHandler */

struct container_stream_handler_result
{
    struct list entry;
    IMFAsyncResult *result;
    MF_OBJECT_TYPE obj_type;
    IUnknown *object;
};

struct container_stream_handler
{
    IMFByteStreamHandler IMFByteStreamHandler_iface;
    IMFAsyncCallback IMFAsyncCallback_iface;
    LONG refcount;
    const char *demuxer_name;
    struct list results;
    CRITICAL_SECTION cs;
};

static struct container_stream_handler *impl_from_IMFByteStreamHandler(IMFByteStreamHandler *iface)
{
    return CONTAINING_RECORD(iface, struct container_stream_handler, IMFByteStreamHandler_iface);
}

static struct container_stream_handler *impl_from_IMFAsyncCallback(IMFAsyncCallback *iface)
{
    return CONTAINING_RECORD(iface, struct container_stream_handler, IMFAsyncCallback_iface);
}

static HRESULT WINAPI container_stream_handler_QueryInterface(IMFByteStreamHandler *iface, REFIID riid, void **obj)
{
    TRACE("%p, %s, %p.\n", iface, debugstr_guid(riid), obj);

    if (IsEqualIID(riid, &IID_IMFByteStreamHandler) ||
            IsEqualIID(riid, &IID_IUnknown))
    {
        *obj = iface;
        IMFByteStreamHandler_AddRef(iface);
        return S_OK;
    }

    WARN("Unsupported %s.\n", debugstr_guid(riid));
    *obj = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI container_stream_handler_AddRef(IMFByteStreamHandler *iface)
{
    struct container_stream_handler *handler = impl_from_IMFByteStreamHandler(iface);
    ULONG refcount = InterlockedIncrement(&handler->refcount);

    TRACE("%p, refcount %u.\n", handler, refcount);

    return refcount;
}

static ULONG WINAPI container_stream_handler_Release(IMFByteStreamHandler *iface)
{
    struct container_stream_handler *handler = impl_from_IMFByteStreamHandler(iface);
    ULONG refcount = InterlockedDecrement(&handler->refcount);
    struct container_stream_handler_result *result, *next;

    TRACE("%p, refcount %u.\n", iface, refcount);

    if (!refcount)
    {
        LIST_FOR_EACH_ENTRY_SAFE(result, next, &handler->results, struct container_stream_handler_result, entry)
        {
            list_remove(&result->entry);
            IMFAsyncResult_Release(result->result);
            if (result->object)
                IUnknown_Release(result->object);
            heap_free(result);
        }
        DeleteCriticalSection(&handler->cs);
        heap_free(handler);
    }

    return refcount;
}

struct create_object_context
{
    IUnknown IUnknown_iface;
    LONG refcount;

    IPropertyStore *props;
    IMFByteStream *stream;
    WCHAR *url;
    DWORD flags;
};

static struct create_object_context *impl_from_IUnknown(IUnknown *iface)
{
    return CONTAINING_RECORD(iface, struct create_object_context, IUnknown_iface);
}

static HRESULT WINAPI create_object_context_QueryInterface(IUnknown *iface, REFIID riid, void **obj)
{
    TRACE("%p, %s, %p.\n", iface, debugstr_guid(riid), obj);

    if (IsEqualIID(riid, &IID_IUnknown))
    {
        *obj = iface;
        IUnknown_AddRef(iface);
        return S_OK;
    }

    WARN("Unsupported %s.\n", debugstr_guid(riid));
    *obj = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI create_object_context_AddRef(IUnknown *iface)
{
    struct create_object_context *context = impl_from_IUnknown(iface);
    ULONG refcount = InterlockedIncrement(&context->refcount);

    TRACE("%p, refcount %u.\n", iface, refcount);

    return refcount;
}

static ULONG WINAPI create_object_context_Release(IUnknown *iface)
{
    struct create_object_context *context = impl_from_IUnknown(iface);
    ULONG refcount = InterlockedDecrement(&context->refcount);

    TRACE("%p, refcount %u.\n", iface, refcount);

    if (!refcount)
    {
        if (context->props)
            IPropertyStore_Release(context->props);
        if (context->stream)
            IMFByteStream_Release(context->stream);
        if (context->url)
            heap_free(context->url);
        heap_free(context);
    }

    return refcount;
}

static const IUnknownVtbl create_object_context_vtbl =
{
    create_object_context_QueryInterface,
    create_object_context_AddRef,
    create_object_context_Release,
};

static WCHAR *heap_strdupW(const WCHAR *str)
{
    WCHAR *ret = NULL;

    if (str)
    {
        unsigned int size;

        size = (lstrlenW(str) + 1) * sizeof(WCHAR);
        ret = heap_alloc(size);
        if (ret)
            memcpy(ret, str, size);
    }

    return ret;
}

static HRESULT WINAPI container_stream_handler_BeginCreateObject(IMFByteStreamHandler *iface, IMFByteStream *stream, const WCHAR *url, DWORD flags,
        IPropertyStore *props, IUnknown **cancel_cookie, IMFAsyncCallback *callback, IUnknown *state)
{
    struct container_stream_handler *this = impl_from_IMFByteStreamHandler(iface);
    struct create_object_context *context;
    IMFAsyncResult *caller, *item;
    HRESULT hr;

    TRACE("%p, %s, %#x, %p, %p, %p, %p.\n", iface, debugstr_w(url), flags, props, cancel_cookie, callback, state);

    if (cancel_cookie)
        *cancel_cookie = NULL;

    if (FAILED(hr = MFCreateAsyncResult(NULL, callback, state, &caller)))
        return hr;

    context = heap_alloc(sizeof(*context));
    if (!context)
    {
        IMFAsyncResult_Release(caller);
        return E_OUTOFMEMORY;
    }

    context->IUnknown_iface.lpVtbl = &create_object_context_vtbl;
    context->refcount = 1;
    context->props = props;
    if (context->props)
        IPropertyStore_AddRef(context->props);
    context->flags = flags;
    context->stream = stream;
    if (context->stream)
        IMFByteStream_AddRef(context->stream);
    if (url)
        context->url = heap_strdupW(url);
    if (!context->stream)
    {
        IMFAsyncResult_Release(caller);
        IUnknown_Release(&context->IUnknown_iface);
        return E_OUTOFMEMORY;
    }

    hr = MFCreateAsyncResult(&context->IUnknown_iface, &this->IMFAsyncCallback_iface, (IUnknown *)caller, &item);
    IUnknown_Release(&context->IUnknown_iface);
    IMFAsyncResult_Release(caller);
    if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(hr = MFPutWorkItemEx(MFASYNC_CALLBACK_QUEUE_IO, item)))
        {
            if (cancel_cookie)
                IMFAsyncResult_GetState(item, cancel_cookie);
        }

        IMFAsyncResult_Release(item);
    }

    return hr;
}

static HRESULT WINAPI container_stream_handler_EndCreateObject(IMFByteStreamHandler *iface, IMFAsyncResult *result,
        MF_OBJECT_TYPE *obj_type, IUnknown **object)
{
    struct container_stream_handler *this = impl_from_IMFByteStreamHandler(iface);
    struct container_stream_handler_result *found = NULL, *cur;
    HRESULT hr;

    TRACE("%p, %p, %p, %p.\n", iface, result, obj_type, object);

    EnterCriticalSection(&this->cs);

    LIST_FOR_EACH_ENTRY(cur, &this->results, struct container_stream_handler_result, entry)
    {
        if (result == cur->result)
        {
            list_remove(&cur->entry);
            found = cur;
            break;
        }
    }

    LeaveCriticalSection(&this->cs);

    if (found)
    {
        *obj_type = found->obj_type;
        *object = found->object;
        hr = IMFAsyncResult_GetStatus(found->result);
        IMFAsyncResult_Release(found->result);
        heap_free(found);
    }
    else
    {
        *obj_type = MF_OBJECT_INVALID;
        *object = NULL;
        hr = MF_E_UNEXPECTED;
    }

    return hr;
}

static HRESULT WINAPI container_stream_handler_CancelObjectCreation(IMFByteStreamHandler *iface, IUnknown *cancel_cookie)
{
    struct container_stream_handler *this = impl_from_IMFByteStreamHandler(iface);
    struct container_stream_handler_result *found = NULL, *cur;

    TRACE("%p, %p.\n", iface, cancel_cookie);

    EnterCriticalSection(&this->cs);

    LIST_FOR_EACH_ENTRY(cur, &this->results, struct container_stream_handler_result, entry)
    {
        if (cancel_cookie == (IUnknown *)cur->result)
        {
            list_remove(&cur->entry);
            found = cur;
            break;
        }
    }

    LeaveCriticalSection(&this->cs);

    if (found)
    {
        IMFAsyncResult_Release(found->result);
        if (found->object)
            IUnknown_Release(found->object);
        heap_free(found);
    }

    return found ? S_OK : MF_E_UNEXPECTED;
}

static HRESULT WINAPI container_stream_handler_GetMaxNumberOfBytesRequiredForResolution(IMFByteStreamHandler *iface, QWORD *bytes)
{
    FIXME("stub (%p %p)\n", iface, bytes);
    return E_NOTIMPL;
}

static const IMFByteStreamHandlerVtbl container_stream_handler_vtbl =
{
    container_stream_handler_QueryInterface,
    container_stream_handler_AddRef,
    container_stream_handler_Release,
    container_stream_handler_BeginCreateObject,
    container_stream_handler_EndCreateObject,
    container_stream_handler_CancelObjectCreation,
    container_stream_handler_GetMaxNumberOfBytesRequiredForResolution,
};

static HRESULT WINAPI container_stream_handler_callback_QueryInterface(IMFAsyncCallback *iface, REFIID riid, void **obj)
{
    if (IsEqualIID(riid, &IID_IMFAsyncCallback) ||
            IsEqualIID(riid, &IID_IUnknown))
    {
        *obj = iface;
        IMFAsyncCallback_AddRef(iface);
        return S_OK;
    }

    WARN("Unsupported %s.\n", debugstr_guid(riid));
    *obj = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI container_stream_handler_callback_AddRef(IMFAsyncCallback *iface)
{
    struct container_stream_handler *handler = impl_from_IMFAsyncCallback(iface);
    return IMFByteStreamHandler_AddRef(&handler->IMFByteStreamHandler_iface);
}

static ULONG WINAPI container_stream_handler_callback_Release(IMFAsyncCallback *iface)
{
    struct container_stream_handler *handler = impl_from_IMFAsyncCallback(iface);
    return IMFByteStreamHandler_Release(&handler->IMFByteStreamHandler_iface);
}

static HRESULT WINAPI container_stream_handler_callback_GetParameters(IMFAsyncCallback *iface, DWORD *flags, DWORD *queue)
{
    return E_NOTIMPL;
}

static HRESULT container_stream_handler_create_object(struct container_stream_handler *This, WCHAR *url, IMFByteStream *stream, DWORD flags,
                                            IPropertyStore *props, IUnknown **out_object, MF_OBJECT_TYPE *out_obj_type)
{
    TRACE("(%p %s %p %u %p %p %p)\n", This, debugstr_w(url), stream, flags, props, out_object, out_obj_type);

    if (!(init_gstreamer()))
        return E_FAIL;

    if (flags & MF_RESOLUTION_MEDIASOURCE)
    {
        HRESULT hr;
        struct media_source *new_source;

        if (FAILED(hr = media_source_constructor(stream, This->demuxer_name, &new_source)))
            return hr;

        TRACE("->(%p)\n", new_source);

        *out_object = (IUnknown*)&new_source->IMFMediaSource_iface;
        *out_obj_type = MF_OBJECT_MEDIASOURCE;

        return S_OK;
    }
    else
    {
        FIXME("flags = %08x\n", flags);
        return E_NOTIMPL;
    }
}

static HRESULT WINAPI container_stream_handler_callback_Invoke(IMFAsyncCallback *iface, IMFAsyncResult *result)
{
    struct container_stream_handler *handler = impl_from_IMFAsyncCallback(iface);
    struct container_stream_handler_result *handler_result;
    MF_OBJECT_TYPE obj_type = MF_OBJECT_INVALID;
    IUnknown *object = NULL, *context_object;
    struct create_object_context *context;
    IMFAsyncResult *caller;
    HRESULT hr;

    caller = (IMFAsyncResult *)IMFAsyncResult_GetStateNoAddRef(result);

    if (FAILED(hr = IMFAsyncResult_GetObject(result, &context_object)))
    {
        WARN("Expected context set for callee result.\n");
        return hr;
    }

    context = impl_from_IUnknown(context_object);

    hr = container_stream_handler_create_object(handler, context->url, context->stream, context->flags, context->props, &object, &obj_type);

    handler_result = heap_alloc(sizeof(*handler_result));
    if (handler_result)
    {
        handler_result->result = caller;
        IMFAsyncResult_AddRef(handler_result->result);
        handler_result->obj_type = obj_type;
        handler_result->object = object;

        EnterCriticalSection(&handler->cs);
        list_add_tail(&handler->results, &handler_result->entry);
        LeaveCriticalSection(&handler->cs);
    }
    else
    {
        if (object)
            IUnknown_Release(object);
        hr = E_OUTOFMEMORY;
    }

    IUnknown_Release(&context->IUnknown_iface);

    IMFAsyncResult_SetStatus(caller, hr);
    MFInvokeCallback(caller);

    return S_OK;
}

static const IMFAsyncCallbackVtbl container_stream_handler_callback_vtbl =
{
    container_stream_handler_callback_QueryInterface,
    container_stream_handler_callback_AddRef,
    container_stream_handler_callback_Release,
    container_stream_handler_callback_GetParameters,
    container_stream_handler_callback_Invoke,
};

HRESULT container_stream_handler_construct(REFIID riid, void **obj, const char *demuxer_name)
{
    struct container_stream_handler *this;
    HRESULT hr;

    TRACE("%s, %p.\n", debugstr_guid(riid), obj);

    this = heap_alloc_zero(sizeof(*this));
    if (!this)
        return E_OUTOFMEMORY;

    list_init(&this->results);
    InitializeCriticalSection(&this->cs);

    this->demuxer_name = demuxer_name;
    this->IMFByteStreamHandler_iface.lpVtbl = &container_stream_handler_vtbl;
    this->IMFAsyncCallback_iface.lpVtbl = &container_stream_handler_callback_vtbl;
    this->refcount = 1;

    hr = IMFByteStreamHandler_QueryInterface(&this->IMFByteStreamHandler_iface, riid, obj);
    IMFByteStreamHandler_Release(&this->IMFByteStreamHandler_iface);

    return hr;
}

/* helper for callback forwarding */
void perform_cb_media_source(struct cb_data *cbdata)
{
    switch(cbdata->type)
    {
    case PULL_FROM_BYTESTREAM:
        {
            struct getrange_data *data = &cbdata->u.getrange_data;
            cbdata->u.getrange_data.ret = pull_from_bytestream(data->pad, data->parent,
                    data->ofs, data->len, data->buf);
            break;
        }
    case QUERY_BYTESTREAM:
        {
            struct query_function_data *data = &cbdata->u.query_function_data;
            cbdata->u.query_function_data.ret = query_bytestream(data->pad, data->parent, data->query);
            break;
        }
    case ACTIVATE_BYTESTREAM_PAD_MODE:
        {
            struct activate_mode_data *data = &cbdata->u.activate_mode_data;
            cbdata->u.activate_mode_data.ret = activate_bytestream_pad_mode(data->pad, data->parent, data->mode, data->activate);
            break;
        }
    case PROCESS_BYTESTREAM_PAD_EVENT:
        {
            struct event_src_data *data = &cbdata->u.event_src_data;
            cbdata->u.event_src_data.ret = process_bytestream_pad_event(data->pad, data->parent, data->event);
            break;
        }
    case SOURCE_STREAM_ADDED:
        {
            struct pad_added_data *data = &cbdata->u.pad_added_data;
            source_stream_added(data->element, data->pad, data->user);
            break;
        }
    case SOURCE_STREAM_REMOVED:
        {
            struct pad_removed_data *data = &cbdata->u.pad_removed_data;
            source_stream_removed(data->element, data->pad, data->user);
            break;
        }
    case SOURCE_ALL_STREAMS:
        {
            struct no_more_pads_data *data = &cbdata->u.no_more_pads_data;
            source_all_streams(data->element, data->user);
            break;
        }
    case STREAM_NEW_SAMPLE:
        {
            struct new_sample_data *data = &cbdata->u.new_sample_data;
            cbdata->u.new_sample_data.ret = stream_new_sample(data->appsink, data->user);
            break;
        }
    case STREAM_EOS:
        {
            struct eos_data *data = &cbdata->u.eos_data;
            stream_eos(data->appsink, data->user);
            break;
        }
    default:
        {
            ERR("Wrong callback forwarder called\n");
            return;
        }
    }
}